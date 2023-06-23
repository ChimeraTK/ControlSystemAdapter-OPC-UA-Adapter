import lxml.etree as ET
#import xml.etree.ElementTree as ET
import logging
import re

class MapOption():
  def __init__(self):
    # True/False in case it should be mapped
    self.exclude = None
    # True/False in case history is enabled
    self.history = None

class EncryptionSettings():
  def __init__(self):
    super(EncryptionSettings, self).__init__()
    self.encryptionEnabled = False
    self.addUnsecureEndpoint = False
    self.certificate = None
    self.key = None
    self.blockList = None
    self.trustList = None
    self.issuerList = None
    
  def checkEncryptionSettings(self):
    '''
    Check if encryption settings are valid and include all required information.
    
    @raise RuntimeError: In case the settings are not complete.
    '''
    if self.key == None:
      raise RuntimeError("Server key not set.")
    if self.certificate == None:
      raise RuntimeError("Server certificate not set.")
    if self.trustList == None or (self.issuerList == None or self.blockList == None):
      raise RuntimeError("No trust list or CA related configuration set.")

class Config():
  def __init__(self):
    super(Config, self).__init__()
    self.username = None
    self.password = None
    self.applicationName = None
    self.rootFolder = None
    self.applicationDescription = ""
    self.enableLogin = False
    self.port = None

class XMLVar(MapOption):
  '''
  Class used to store all relevant information of process variables.
  '''
  def __init__(self, var:ET.Element):
    super().__init__()
    self.element = var
    self.name = var.attrib['name']
    # This name is set if the directory should be renamed
    self.newName = None    
    self.valueType = var.find('value_type', namespaces=var.nsmap).text
    self.numberOfElements = int(var.find('numberOfElements', namespaces=var.nsmap).text)
    self.unit = var.find('unit', namespaces=var.nsmap).text
    self.newUnit = None
    self.direction = var.find('direction', namespaces=var.nsmap).text
    self.description = var.find('description', namespaces=var.nsmap).text
    self.newDescription = None
    self.newDestination = None

  def __str__(self):
    return "Variable ({}, {}): {}".format(self.valueType, self.numberOfElements,self.name)

  def generateMapEntry(self, root:ET.Element, path:str):
    if self.newName or self.newDescription or self.newUnit or self.newDestination:
      logging.debug("Adding xml entry for process_variable for {}/{}".format(path.removeprefix('/root'),self.name))
      pv = ET.SubElement(root, "process_variable")
      pv.set("sourceName", path.removeprefix('/root') + '/' + self.name)
      if self.newDescription:
        dest = ET.SubElement(pv, "description")
        dest.text = self.newDescription
      if self.newName:
        name = ET.SubElement(pv, "name")
        name.text = self.newName
      if self.newUnit:
        unit = ET.SubElement(pv, "unit")
        unit.text = self.newUnit
      if self.newDestination:
        dest = ET.SubElement(pv, "destination")
        dest.text = self.newDestination.removeprefix('/root')
      if self.newName:
        pv.set("copy", "True")
      else:
        pv.set("copy", "False")

class XMLDirectory(MapOption):
  '''
  Class used to store all relevant information of CS directories.
  It includes sub directories and process variables.
  '''
  def __init__(self, name: str,  data:ET.Element, level: int, path:str = ''):
    super().__init__()
    self.dirs = []
    self.vars = []
    self.element = data
    self.name = name
    self.path = path + "/" + name
    # This name is set if the directory should be renamed
    self.newName = None
    self.newDescription = None
    self.newDestination = None
    self.hierarchyLevel = level
    self.parseDir(data)
      
  def parseDir(self, data:ET.Element):
    '''
    Read directory information from XML directory entry.
    Includes reading variables and sub directories.
    '''
    for dir in data.findall('directory',namespaces=data.nsmap):
      self.dirs.append(XMLDirectory(name = dir.attrib['name'], data=dir, level=self.hierarchyLevel+1, path = self.path))
    for var in data.findall('variable',namespaces=data.nsmap):
      self.vars.append(XMLVar(var))
      
  def __str__(self):
    space = ""
    for i in range(self.hierarchyLevel):
      space = space + "\t"
    out = space + "Directory (level: {}): {} \n".format(self.hierarchyLevel,self.path)
    for dir in self.dirs:
      out = out + str(dir)
    for var in self.vars:
      out = out + space +  "\t" + str(var) + "\n"

    return out
  
  def generateMapEntries(self, root:ET.Element):
    for d in self.dirs:
      d.generateMapEntries(root)
    for var in self.vars:
      var.generateMapEntry(root, self.path)
    if self.newDescription or self.newName or self.newDestination:
      logging.debug("Adding xml entry for folder {}".format(self.path.removeprefix('/root')))
      folder = ET.SubElement(root, "folder")
      folder.set("sourceName", self.path.removeprefix('/root'))
      folder.set("copy", "False")
      if self.newDescription:
        dest = ET.SubElement(folder, "description")
        dest.text = self.newDescription
      if self.newName:
        name = ET.SubElement(folder, "name")
        name.text = self.newName
      if self.newDestination:
        dest = ET.SubElement(folder, "destination")
        dest.text = self.newDestination.removeprefix('/root')


      
  
class MapGenerator(Config, EncryptionSettings):
  def __init__(self, inputFile : str):
    super().__init__()
    self.inputFile = inputFile
    self.outputFile = None
    self.isChimeraTKXML = False
    self.applicationName = None
    self.nsmap = None
    self.dir = None
    self._openFile()
    
  def parseChimeraTK(self, data:ET.Element):
    '''
    Parse a ChiemraTK generated XML file using the XML generator of the application.
    '''
    logging.debug("ChimeraTK XML file.")
    self.dir = XMLDirectory(name='root', data=data, level=0)
    if logging.root.level == logging.DEBUG:
      print(self.dir) 
    
  def parseMapFile(self, data:ET.Element):
    '''
    Parse an existing map file.
    '''
    logging.debug("Parsing map file.")
  
  def _createConfig(self, root:ET.Element):
    '''
    Create server config XML section of the map file.
    
    @raise RuntimeError: If configuration is not complete.
    '''
    config = ET.SubElement(root,"config")
    if self.rootFolder:
      config.set("rootFolder", self.rootFolder)
    config.set("description", self.applicationDescription)
    server = ET.SubElement(config, "server")
    if self.applicationName:
      server.set("applicationName", self.applicationName)
    else:
      logging.info("Default application name will be used. It is the ChimeraTK application name.")
    if self.port:
      server.set("port", str(self.port))
    else:
      logging.info("Default port of 16664 will be used.")
    
    if self.enableLogin == True:
      login = ET.SubElement(config, "login")
      if self.username:
        login.set("username", self.username)
      else:
        raise RuntimeError("No username set!")
      if self.password:
        login.set("password", self.password)
      else:
        raise RuntimeError("No password set!")
      
    if self.encryptionEnabled:
      # raises in case of problems
      self.checkEncryptionSettings()
      security = ET.SubElement(config, "security")
      if self.addUnsecureEndpoint:
        security.set("unsecure", "True")
      else:
        security.set("unsecure", "False")
      security.set("privatekey", self.key)
      security.set("certificate", self.certificate)
      if self.trustList:
        security.set("trustlist", self.trustList)
      if self.issuerList:
        security.set("issuerlist", self.issuerList)
      if self.blockList:
        security.set("blocklist", self.blockList)
      
  def save(self):
    '''Write the map file.
    '''
    logging.info("Writing map file: {}".format(self.outputFile))
    root = ET.Element("uamapping", nsmap = {'csa': 'https://github.com/ChimeraTK/ControlSystemAdapter-OPC-UA-Adapter'})
    self._createConfig(root)
    self.dir.generateMapEntries(root)
    tree = ET.ElementTree()
    tree._setroot(root)
    uglyXml = ET.tounicode(tree, pretty_print = True)
    text_re = re.compile('>\n\s+([^<>\s].*?)\n\s+</', re.DOTALL)    
    prettyXml = text_re.sub('>\g<1></', uglyXml)
    with open(self.outputFile, "w") as text_file:
      text_file.write(prettyXml)
    
  def _openFile(self):
    '''
    Check if the given input file is a ChiemraTK XML file or an already 
    created map file.
    The ChimeraTK map file is identified by the root node called 'application'.
    Else assume it is a map file.
    '''
    logging.debug("analysing file {}".format(self.inputFile))
    data = ET.parse(self.inputFile).getroot()
    if data.tag == 'application' or data.tag.split('}')[1] == 'application':
      if 'name' in data.keys():
        self.applicationName = data.attrib['name']
        logging.info("Application name is: {}".format(self.applicationName))
      self.nsmap = data.nsmap
      self.parseChimeraTK(data)
    else:
      self.parseMapFile(data)
    
    
  def isChimeraTKXML(self) -> bool:
    '''
    The given input file could be an XML generated by the ChimeraTK
    XML generator of the application or an already created map file.
    
    @return (boolean): True if it is a ChimeraTK XML file.
    '''
    return self.isChimeraTKXML