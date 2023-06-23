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
  
  def createEncryption(self, security: ET.Element):
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

  def readEncryption(self, data: ET.Element):
    if 'unsecure' in data.attrib and data.attrib['unsecure'] == 'True':
      self.addUnsecureEndpoint = True
    if 'privatekey' in data.attrib:
      self.key = data.attrib['privatekey']
    if 'certificate' in data.attrib:
      self.certificate = data.attrib['certificate']
    if 'trustlist' in data.attrib:
      self.trustList = data.attrib['trustlist']
    if 'issuerlist' in data.attrib:
      self.issuerList = data.attrib['issuerlist']
    if 'blocklist' in data.attrib:
      self.blockList = data.attrib['blocklist']
    
class Config(EncryptionSettings):
  def __init__(self):
    super().__init__()
    self.username = None
    self.password = None
    self.applicationName = None
    self.rootFolder = None
    self.applicationDescription = ""
    self.enableLogin = False
    self.port = None
    
  def createConfig(self, root:ET.Element):
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
      self.createEncryption(ET.SubElement(config, "security"))
    
  def readConfig(self, data: ET.Element):
    config = data.find('config',namespaces=data.nsmap)
    if config != None:
      if 'rootFolder' in config.attrib:
        self.rootFolder = config.attrib['rootFolder']
      if 'rootFolder' in config.attrib:
        self.applicationDescription = config.attrib["description"]
      # server settings
      server = config.find('server')
      if server != None:
        if 'applicationName' in server.attrib:
          self.applicationName = server.attrib["applicationName"]
        if 'port' in server.attrib:
          self.port = int(server.attrib['port'])
        # security
      login = config.find('login')
      if login != None:
        self.enableLogin = True
        if 'username' in  login.attrib:
          self.username = login.attrib['username']
        if 'password' in  login.attrib:
          self.password = login.attrib['password']
          logging.info("Read password: {}".format(self.password))
      else:
        self.enableLogin = False  
        self.username = None
        self.password = None
      security = config.find("security")
      if security != None:
        self.encryptionEnabled = True
        self.readEncryption(security)
      else: 
        super().__init__()
      

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


      
  
class MapGenerator(Config):
  def __init__(self, inputFile : str):
    super().__init__()
    self.inputFile = inputFile
    self.outputFile = None
    self.isChimeraTKXML = False
    self.applicationName = None
    self.nsmap = None
    self.nsmapMapfile = None
    self.dir = None
    self.parseChimeraTK()
    
  def parseChimeraTK(self):
    '''
    Parse a ChiemraTK generated XML file using the XML generator of the application.
    @raise RuntimeError: If the given input file is not a ChiemraTK XML file. 
                         The ChimeraTK map file is identified by the root node called 'application'.
    '''
    logging.debug("ChimeraTK XML file.")
    data = self._openFile(self.inputFile)
    if data.tag == 'application' or data.tag.split('}')[1] == 'application':
      if 'name' in data.keys():
        self.applicationName = data.attrib['name']
        logging.info("Application name is: {}".format(self.applicationName))
      self.nsmap = data.nsmap
    else:
      raise RuntimeError("Failed to find application tag. Not an XML file created by ChimeraTK's XML generator.")
    self.dir = XMLDirectory(name='root', data=data, level=0)
    if logging.root.level == logging.DEBUG:
      print(self.dir) 
    
  def parseMapFile(self, inputFile:str):
    '''
    Parse an existing map file.
    @raise RuntimeError: If the given input file is not a mapping XML file. 
                         A mapping file is identified by the root node called 'uamapping'.

    '''
    logging.debug("Parsing map file.")
    data = self._openFile(inputFile)
    if data.tag == 'uamapping':
      nsmap = {'csa': 'https://github.com/ChimeraTK/ControlSystemAdapter-OPC-UA-Adapter'}
      if nsmap != data.nsmap:
        RuntimeError("Wrong name space ({}) used in mapping file.".format(data.nsmap))
      self.readConfig(data)
    else:
      raise RuntimeError("Failed to find uamapping tag. Not an ControlSystem-OPC-UA XML mapping file.")

  def save(self):
    '''Write the map file.
    '''
    logging.info("Writing map file: {}".format(self.outputFile))
    root = ET.Element("uamapping", nsmap = {'csa': 'https://github.com/ChimeraTK/ControlSystemAdapter-OPC-UA-Adapter'})
    self.createConfig(root)
    self.dir.generateMapEntries(root)
    tree = ET.ElementTree()
    tree._setroot(root)
    uglyXml = ET.tounicode(tree, pretty_print = True)
    text_re = re.compile('>\n\s+([^<>\s].*?)\n\s+</', re.DOTALL)    
    prettyXml = text_re.sub('>\g<1></', uglyXml)
    with open(self.outputFile, "w") as text_file:
      text_file.write(prettyXml)
    
  def _openFile(self, inputFile: str) -> ET.Element:
    '''
    Open an xml file.
    '''
    logging.debug("analysing file {}".format(inputFile))
    data = ET.parse(inputFile).getroot()
    return data
    
  def isChimeraTKXML(self) -> bool:
    '''
    The given input file could be an XML generated by the ChimeraTK
    XML generator of the application or an already created map file.
    
    @return (boolean): True if it is a ChimeraTK XML file.
    '''
    return self.isChimeraTKXML