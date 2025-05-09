import lxml.etree as ET
#import xml.etree.ElementTree as ET
import logging
import re
from typing import List


class MapOption():
  def __init__(self):
    # True/False in case it should be mapped
    self.exclude:bool|None = None
    # Name of the history setting in case history is enabled
    self.historizing:str|None = None

class HistorySetting():
  def __init__(self, name:str):
    self.name:str = name
    self.entriesPerResponse:int = 1000
    self.bufferLength:int = 100
    self.interval:int = 1000
    
  def checkSettings(self):
    if self.entriesPerResponse == None:
      raise RuntimeError("Number of entries per response not set.")
    if self.interval == None:
      raise RuntimeError("Sampling interval not set.")
    if self.bufferLength == None:
      raise RuntimeError("Buffer length not set.")
  
  def createHistory(self, history: ET._Element):
    history.set("name", self.name)
    history.set("entries_per_response", str(self.entriesPerResponse))
    history.set("buffer_length", str(self.bufferLength))
    history.set("interval", str(self.interval))
    
  def readHistory(self, data: ET._Element):
    if 'name' in data.attrib:
      self.name = str(data.attrib['name'])
    if 'entries_per_response' in data.attrib:
      self.entriesPerResponse = int(data.attrib['entries_per_response'])
    if 'buffer_length' in data.attrib:
      self.bufferLength = int(data.attrib['buffer_length'])
    if 'interval' in data.attrib:
      self.interval = int(data.attrib['interval'])
      
  def writeSetting(self, element:ET._Element):
    element.set("name", self.name)
    element.set("entries_per_response",str(self.entriesPerResponse))
    element.set("buffer_length",str(self.bufferLength))
    element.set("interval",str(self.interval))
    
      
class EncryptionSettings():
  def __init__(self):
    self.encryptionEnabled:bool = False
    self.addUnsecureEndpoint:bool = False
    self.certificate:str|None = None
    self.key:str|None = None
    self.blockList:str|None = None
    self.trustList:str|None = None
    self.issuerList:str|None = None
    
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
  
  def createEncryption(self, security: ET._Element):
    if self.addUnsecureEndpoint:
      security.set("unsecure", "True")
    else:
      security.set("unsecure", "False")
    if self.key:
      security.set("privatekey", self.key)
    if self.certificate:
      security.set("certificate", self.certificate)
    if self.trustList:
      security.set("trustlist", self.trustList)
    if self.issuerList:
      security.set("issuerlist", self.issuerList)
    if self.blockList:
      security.set("blocklist", self.blockList)  

  def readEncryption(self, data: ET._Element):
    if 'unsecure' in data.attrib and data.attrib['unsecure'] == 'True':
      self.addUnsecureEndpoint = True
    if 'privatekey' in data.attrib:
      self.key = str(data.attrib['privatekey'])
    if 'certificate' in data.attrib:
      self.certificate = str(data.attrib['certificate'])
    if 'trustlist' in data.attrib:
      self.trustList = str(data.attrib['trustlist'])
    if 'issuerlist' in data.attrib:
      self.issuerList = str(data.attrib['issuerlist'])
    if 'blocklist' in data.attrib:
      self.blockList = str(data.attrib['blocklist'])
    
class Config(EncryptionSettings):
  def __init__(self):
    super().__init__()
    self.username: str|None = None
    self.password: str|None = None
    self.applicationName: str|None = None
    self.rootFolder: str|None = None
    self.applicationDescription = ""
    self.enableLogin = False
    self.port: int|None = None
    self.historySettings: List[HistorySetting] = []
    self.logLevel: str = "INFO"
  
  def createConfig(self, root:ET._Element):
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
      
    server.set("logLevel", self.logLevel)
    
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
      
    if len(self.historySettings) > 0:
      logging.info("Writing {} history settings.".format(len(self.historySettings)))
      historizing = ET.SubElement(config, "historizing")
      for setting in self.historySettings:
        settingElement = ET.SubElement(historizing,"setup")
        setting.writeSetting(settingElement)
    else:
      logging.info("No history settings found.")
    
  def readConfig(self, data: ET._Element):
    config = data.find('config',namespaces=data.nsmap)
    if config is not None:
      if 'rootFolder' in config.attrib:
        self.rootFolder = str(config.attrib['rootFolder'])
      if 'rootFolder' in config.attrib:
        self.applicationDescription = config.attrib["description"]
      # server settings
      server = config.find('server')
      if server is not None:
        if 'applicationName' in server.attrib:
          self.applicationName = str(server.attrib["applicationName"])
        if 'port' in server.attrib:
          self.port = int(server.attrib['port'])
        if 'logLevel' in server.attrib:
          tmp = str(server.attrib["logLevel"]).upper()
          if tmp in ['TRACE', 'DEBUG', 'INFO', 'WARNING','ERROR','FATAL']:
            self.logLevel = str(server.attrib["logLevel"]).upper()
          else:
            logging.warning("Failed to read log level. {} is not a valid logLevel. Using INFO instead.".format(tmp))
        # security
      login = config.find('login')
      if login is not None:
        self.enableLogin = True
        if 'username' in  login.attrib:
          self.username = str(login.attrib['username'])
        if 'password' in  login.attrib:
          self.password = str(login.attrib['password'])
      else:
        self.enableLogin = False  
        self.username = None
        self.password = None
      security = config.find("security")
      if security is not None:
        self.encryptionEnabled = True
        self.readEncryption(security)
      else: 
        super().__init__()
      historizing = config.find('historizing')
      if historizing is not None:
        for setup in historizing.findall('setup', namespaces=data.nsmap):
          h = HistorySetting('')
          h.readHistory(setup)
          foundSetting = next((x for x in self.historySettings if x.name == h.name), None)
          if foundSetting == None:
            self.historySettings.append(h)
          else:
            logging.warning("Found history settings dublicate with name {}. Will not it again.".format(h.name))

class XMLVar(MapOption):
  '''
  Class used to store all relevant information of process variables.
  '''
  def __init__(self, var:ET._Element, path:str):
    super().__init__()
    self.element = var
    self.name:str = str(var.attrib['name'])
    # This name is set if the directory should be renamed
    self.newName: str|None = None    
    self.valueType:str = var.find('value_type', namespaces=var.nsmap).text
    self.numberOfElements:int = int(var.find('numberOfElements', namespaces=var.nsmap).text)
    self.unit:str = var.find('unit', namespaces=var.nsmap).text
    self.newUnit: str|None = None
    self.direction:str = var.find('direction', namespaces=var.nsmap).text
    self.description:str = var.find('description', namespaces=var.nsmap).text
    self.newDescription: str|None = None
    self.newDestination: str|None = None
    self.fullName = path + "/" + self.name

  def __str__(self) -> str:
    return "Variable ({}, {}): {}".format(self.valueType, self.numberOfElements,self.name)
  
  def __eq__(self, other: object) -> bool:
    if isinstance(other, str):
      return other == self.fullName
    elif isinstance(other, XMLVar):
      return other.fullName == self.fullName 
    else:
      return NotImplemented
  
  def generateMapEntry(self, root:ET._Element, path:str, historizingActive:bool = False, excludeActive:bool = False):
    if(self.exclude and not excludeActive):
      exclude = ET.SubElement(root, "exclude")
      exclude.set("sourceName", self.fullName.removeprefix('/root/'))
    if self.newName or self.newDescription or self.newUnit or self.newDestination or (self.historizing and not historizingActive):
      logging.debug("Adding xml entry for process_variable for {}/{}".format(path.removeprefix('/root/'),self.name))
      pv = ET.SubElement(root, "process_variable")
      if path.removeprefix('/root/') == '':
        pv.set("sourceName", self.name)
      else:
        pv.set("sourceName", path.removeprefix('/root/') + '/' + self.name)
      if self.historizing and not historizingActive:
        pv.set("history", self.historizing)
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
        if self.newDestination == "/root":
          # PV is moved to root -> No destination needs to be set in that case
          # If no new name is assigned the original name needs to be set 
          if not self.newName:
            name = ET.SubElement(pv, "name")
            name.text = self.name
        else:
          dest.text = self.newDestination.removeprefix('/root/')
      if self.newName:
        pv.set("copy", "True")
      else:
        pv.set("copy", "False")
        
  def reset(self):
    '''
    Resets all information related to updated directory and varaiable information, e.g.  set newName, newDescription ect. to None.
    '''
    self.newName = None    
    self.newUnit = None
    self.newDescription = None
    self.newDestination = None

class XMLDirectory(MapOption):
  '''
  Class used to store all relevant information of CS directories.
  It includes sub directories and process variables.
  '''
  def __init__(self, name: str,  data:ET._Element, level: int, path:str = ''):
    super().__init__()
    self.dirs: List[XMLDirectory] = []
    self.vars: List[XMLVar] = []
    self.element = data
    self.name = name
    self.path = path + "/" + name
    # This name is set if the directory should be renamed
    self.newName: str|None = None
    self.newDescription: str|None = None
    self.newDestination: str|None = None
    self.hierarchyLevel = level
    self.parseDir(data)
    
  def __eq__(self, other: object) -> bool:
    if isinstance(other, str):
      return other == self.path
    elif isinstance(dir, XMLDirectory):
      return other.path == self.path
    else:
      return NotImplemented
      
  def findDir(self, directory: str):
    if self == directory:
      return self
    for d in self.dirs:
      if d == directory:
        return d
      ret = d.findDir(directory)
      if ret != None:
        return ret
    return None
  
  def findVar(self, var: str):
    for v in self.vars:
      if v == var:
        return v
    for d in self.dirs:
      ret = d.findVar(var)
      if ret != None:
        return ret
    return None

  def parseDir(self, data:ET._Element):
    '''
    Read directory information from XML directory entry.
    Includes reading variables and sub directories.
    '''
    for directory in data.findall('directory',namespaces=data.nsmap):
      self.dirs.append(XMLDirectory(name = str(directory.attrib['name']), data=directory, level=self.hierarchyLevel+1, path = self.path))
    for var in data.findall('variable',namespaces=data.nsmap):
      self.vars.append(XMLVar(var, self.path))
      
  def __str__(self):
    space = self.hierarchyLevel*"\t"
    out = space + "Directory (level: {}): {} \n".format(self.hierarchyLevel,self.path)
    for directory in self.dirs:
      out = out + str(directory)
    for var in self.vars:
      out = out + space +  "\t" + str(var) + "\n"

    return out
  
  def generateMapEntries(self, root:ET._Element, historiszingActive:bool = False, excludeActive = False):
    for d in self.dirs:
      d.generateMapEntries(root, self.historizing is not None, self.exclude is not None)
    for var in self.vars:
      var.generateMapEntry(root, self.path, self.historizing is not None, self.exclude is not None)
    if self.exclude and not excludeActive:
      exclude = ET.SubElement(root, "exclude")
      exclude.set("sourceName", self.path.removeprefix('/root/') + '/*')
    destText = self.path.removeprefix('/root').removesuffix(self.name)
    # special case: If only the description is to be changed no sourceName has to be given put name and destination
    # If a history is assigned and a description is given two entries have to be created!
    #<folder sourceName="watchdog/logging" copy="False" history="short_history">
    #  <name>logging</name>
    #  <destination>watchdog</destination>
    #</folder>
    #<folder>
    #  <description>Test</description>
    #  <name>logging</name>
    #  <destination>watchdog</destination>
    #</folder>
    descriptionAlreadySet = False
    if self.newDescription and not self.newName and not self.newDestination:
      logging.debug("Adding xml entry for folder {} that only changes the description of an existing folder".format(self.path.removeprefix('/root/')))
      folder = ET.SubElement(root, "folder")
      description = ET.SubElement(folder, "description")
      description.text = self.newDescription
      name = ET.SubElement(folder, "name")
      name.text = self.name
      if not destText == '':
        dest = ET.SubElement(folder, "destination")
        dest.text = destText.removeprefix('/').removesuffix('/')
      descriptionAlreadySet = True
      
    if self.newName or self.newDestination or (self.historizing and not historiszingActive):
      logging.debug("Adding xml entry for folder {}".format(self.path.removeprefix('/root/')))
      folder = ET.SubElement(root, "folder")
      folder.set("sourceName", self.path.removeprefix('/root/'))
      folder.set("copy", "False")
      if self.historizing and not historiszingActive:
        folder.set("history", self.historizing)
      if self.newDescription and not descriptionAlreadySet:
        dest = ET.SubElement(folder, "description")
        dest.text = self.newDescription
      if self.newName:
        name = ET.SubElement(folder, "name")
        name.text = self.newName
      else:
        name = ET.SubElement(folder, "name")
        name.text = self.name
      dest = ET.SubElement(folder, "destination")
      if self.newDestination:
        # If folder is moved to root the new destination is '/root' -> that is why the '/' is removed in an extra call
        dest.text = self.newDestination.removeprefix('/root').removeprefix('/')
      else:
        dest.text = destText.removeprefix('/').removesuffix('/')

  def reset(self):
    '''
    Resets all information related to updated directory and varaiable information, e.g.  set newName, newDescription ect. to None.
    '''
    self.newName = None
    self.newDescription = None
    self.newDestination = None
    for directory in self.dirs:
      directory.reset()
    for var in self.vars:
      var.reset()      
  
class MapGenerator(Config):
  def __init__(self, inputFile : str|None):
    super().__init__()
    self.inputFile: str|None = inputFile
    self.outputFile: str|None = None
    self.applicationName: str|None = None
    self.nsmap: Dict|None = None
    self.dir: XMLDirectory|None = None
    if self.inputFile:
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
    
  def parseMapFile(self, inputFile:str) -> List[int]:
    '''
    Parse an existing map file.
    @param inputFile: The map file name.
    @return: list that includes the number of not found directories (index 0), pvs (index 1) and excludes (index 2). 
             In the map file the sourceName is given and it can happen that the source
             given in the map file does not correspond to a PV or directory from the original
             variable tree.
    @raise RuntimeError: If the given input file is not a mapping XML file. 
                         A mapping file is identified by the root node called 'uamapping'.
    '''
    logging.debug("Parsing map file.")
    if self.dir:
      self.dir.reset()
    data = self._openFile(inputFile)
    if data.tag == 'uamapping':
      nsmap = {'csa': 'https://github.com/ChimeraTK/ControlSystemAdapter-OPC-UA-Adapter'}
      if nsmap != data.nsmap:
        RuntimeError("Wrong name space ({}) used in mapping file.".format(data.nsmap))
      self.readConfig(data)
      
      nSkipped = [0,0,0]
      if self.dir == None:
        # No xml input was given so we do not parse any further
        return nSkipped
      # read folder information
      for folder in data.findall('folder', namespaces=data.nsmap):
        if "sourceName" in  folder.attrib:
          directory = self.dir.findDir("/root/" + str(folder.attrib["sourceName"]))
          if directory != None:
            # construct test path from name and destination to compare to the directory path 
            # if both match only the history is added
            # else the folder is moved
            tmpPath = "/root/"
            if 'history' in folder.attrib:
              directory.historizing = str(folder.attrib["history"])
            if folder.find('description', namespaces=folder.nsmap) != None and folder.find('description', namespaces=folder.nsmap).text != None:
              directory.newDescription = folder.find('description', namespaces=folder.nsmap).text 
            if folder.find('destination', namespaces=folder.nsmap) != None and folder.find('destination', namespaces=folder.nsmap).text != None:
              tmpDestination = directory.path.removeprefix("/root").removeprefix("/").removesuffix(directory.name).removesuffix("/")
              tmpPath = tmpPath + folder.find('destination', namespaces=folder.nsmap).text
              if tmpDestination != folder.find('destination', namespaces=folder.nsmap).text:
                directory.newDestination = "/root/"+folder.find('destination', namespaces=folder.nsmap).text
            if folder.find('name', namespaces=folder.nsmap) != None and directory.name != folder.find('name', namespaces=folder.nsmap).text:
              directory.newName = folder.find('name', namespaces=folder.nsmap).text
            # first remove "/" avoids resulting "//" when no destination was added
            tmpPath = tmpPath.removesuffix("/") + "/" + folder.find('name', namespaces=folder.nsmap).text
            if directory.newDestination == None and tmpPath != directory.path:
              directory.newDestination = "/root"
          else:
            logging.warning("Failed to find source folder path {} in the application variable tree!".format("/root/" + str(folder.attrib["sourceName"])))
            nSkipped[0] =  nSkipped[0] + 1
        else:
          tmpDestination = None
          tmpName = None
          if folder.find('destination', namespaces=folder.nsmap) != None:
            tmpDestination = folder.find('destination', namespaces=folder.nsmap).text
          if folder.find('name', namespaces=folder.nsmap) != None:
            tmpName = folder.find('name', namespaces=folder.nsmap).text
          if tmpName == None:
            logging.warning("For folder entry in the map file with no sourceName no name is given. Will be skipped.")
          tmpSourceName = "/root/"
          if tmpDestination == None:
            tmpSourceName = tmpSourceName + tmpName
          else:
            tmpSourceName = tmpSourceName + tmpDestination + "/" + tmpName
          directory = self.dir.findDir(tmpSourceName)
          if directory != None:
            if folder.find('description', namespaces=folder.nsmap) != None:
              directory.newDescription = folder.find('description', namespaces=folder.nsmap).text 
          else:
            logging.warning("Failed to find source folder path {} in the application variable tree!".format(tmpSourceName))
            nSkipped[0] =  nSkipped[0] + 1
      # read process_variable information
      for pv in data.findall('process_variable', namespaces=data.nsmap):
        if "sourceName" in  pv.attrib:
          var = self.dir.findVar("/root/" + str(pv.attrib["sourceName"]))
          if var != None:
            if 'history' in pv.attrib:
              var.historizing = str(pv.attrib["history"])
            if pv.find('description', namespaces=pv.nsmap) != None:
              var.newDescription = pv.find('description', namespaces=pv.nsmap).text 
            if pv.find('unit', namespaces=pv.nsmap) != None:
              var.newUnit = pv.find('unit', namespaces=pv.nsmap).text
            if pv.find('name', namespaces=pv.nsmap) != None:
              var.newName = pv.find('name', namespaces=pv.nsmap).text  
            if pv.find('destination', namespaces=pv.nsmap) != None:
              var.newDestination = pv.find('destination', namespaces=pv.nsmap).text 
            if (var.newDestination == "" or var.newDestination == None) and var.name == var.newName:
              var.newDestination = "/root"
              var.newName = None
          else:
            logging.warning("Failed to find source pv path {} in the application variable tree!".format("/root/" + str(pv.attrib["sourceName"])))
            nSkipped[1] =  nSkipped[1] + 1
      for exclude in data.findall('exclude', namespaces=data.nsmap):
        if "sourceName" in exclude.attrib:
          if exclude.attrib["sourceName"].endswith("/*"):
            directory = self.dir.findDir("/root/" + exclude.attrib["sourceName"][0:-2])
            if directory != None:
              directory.exclude = True
            else: 
              logging.warning("Failed to find source exclude path {} in the application variable tree!".format("/root/" + str(exclude.attrib["sourceName"][0:-2])))
              nSkipped[2] =  nSkipped[2] + 1
          else:
            var = self.dir.findVar("/root/" + exclude.attrib["sourceName"])
            if var != None:
              var.exclude = True
            else: 
              logging.warning("Failed to find source exclude path {} in the application variable tree!".format("/root/" + str(exclude.attrib["sourceName"])))
              nSkipped[2] =  nSkipped[2] + 1
      return nSkipped
    else:
      raise RuntimeError("Failed to find uamapping tag. Not an ControlSystem-OPC-UA XML mapping file.")

  def save(self):
    '''Write the map file.
    '''
    logging.info("Writing map file: {}".format(self.outputFile))
    root = ET.Element("uamapping", nsmap = {'csa': 'https://github.com/ChimeraTK/ControlSystemAdapter-OPC-UA-Adapter'})
    self.createConfig(root)
    if self.dir:
      self.dir.generateMapEntries(root)
    tree = ET.ElementTree()
    tree._setroot(root)
    uglyXml = ET.tounicode(tree, pretty_print = True)
    text_re = re.compile('>\n\s+([^<>\s].*?)\n\s+</', re.DOTALL)    
    prettyXml = text_re.sub('>\g<1></', uglyXml)
    with open(self.outputFile, "w") as text_file:
      text_file.write(prettyXml)
    
  def _openFile(self, inputFile: str) -> ET._Element:
    '''
    Open an xml file.
    '''
    logging.debug("analysing file {}".format(inputFile))
    data = ET.parse(inputFile).getroot()
    return data
