import unittest
import logging
from ctk_opcua_generator_tools.generatorClass import MapGenerator, XMLDirectory, XMLVar, EncryptionSettings

class Test(unittest.TestCase):
  def testFindDir(self):
    generator = MapGenerator("test.xml")
    ret = generator.dir.findDir("/root/Configuration")
    self.assertEqual(ret.name, "Configuration", "Found wrong directory name")
    ret = generator.dir.findDir("/root/NotExisting")
    self.assertEqual(ret, None, "Found non existing directory")
    
  def testFindVar(self):
    generator = MapGenerator("test.xml")
    ret = generator.dir.findVar("/root/Configuration/test")
    self.assertEqual(ret.name, "test", "Wrong pv name")
    self.assertEqual(ret.fullName, "/root/Configuration/test", "Wrong full name")
    ret = generator.dir.findVar("/root/NotExisting")
    self.assertEqual(ret, None, "Found non existing PV")
    
  def testReadXML(self):
    generator = MapGenerator("test.xml")
    self.assertEqual(generator.dir.dirs[0].name, "Configuration", "Read wrong directory name")
    self.assertEqual(generator.dir.dirs[0].newDestination, None, "Read wrong destination")
    self.assertEqual(generator.dir.dirs[0].newDescription, None, "Read wrong description")
    self.assertEqual(generator.dir.dirs[0].vars[0].newName, None, "Read wrong pv name")
    self.assertEqual(generator.dir.dirs[0].vars[0].newDestination, None, "Read wrong pv destination")
    self.assertEqual(generator.dir.dirs[0].vars[0].newDescription, None, "Read wrong pv description")
    self.assertEqual(generator.dir.dirs[0].vars[0].newUnit, None, "Read wrong pv unit")
    if generator.certificate:
      logging.info("ok")
    generator.parseMapFile("test_mapping.xml")
    self.assertEqual(generator.rootFolder, "Test", "Root folder not read from mapping file")
    self.assertEqual(generator.applicationName, "Test", "Application name not read from mapping file.")
    self.assertEqual(generator.port, 16664, "Port not read from mapping file")
    self.assertEqual(generator.blockList, "certs/blocklists", "Block lists not read from mapping file")
    self.assertEqual(generator.trustList, "certs/trust", "Trust lists not read from mapping file")
    self.assertEqual(generator.issuerList, "certs/issuer", "Issuer lists not read from mapping file")
    self.assertEqual(generator.certificate, "certs/server_cert.der", "Certificate not read from mapping file")
    self.assertEqual(generator.key, "certs/server_key.der", "Key not read from mapping file")
    self.assertEqual(generator.addUnsecureEndpoint, True, "Unsecure endpoint information not read from mapping file")
    self.assertEqual(generator.username, "user", "User name not read from mapping file")
    self.assertEqual(generator.password, "test", "Password not read from mapping file")
    self.assertEqual(generator.enableLogin, True, "Wrong login state detected.")
    self.assertEqual(generator.encryptionEnabled, True, "Encryption state detected is wrong.")
    self.assertEqual(generator.dir.dirs[0].newDestination, "Test", "Read wrong destination")
    self.assertEqual(generator.dir.dirs[0].newDescription, "newDescription" "Read wrong description")
    self.assertEqual(generator.dir.dirs[0].vars[0].newName, "newName", "Read wrong pv name")
    self.assertEqual(generator.dir.dirs[0].vars[0].newDestination, "Test", "Read wrong pv destination")
    self.assertEqual(generator.dir.dirs[0].vars[0].newDescription, "newDescription", "Read wrong pv description")
    self.assertEqual(generator.dir.dirs[0].vars[0].newUnit, "newUnit", "Read wrong pv unit")


    generator.parseMapFile("test_mapping_minimal.xml")
    self.assertEqual(generator.rootFolder, "Test1", "Root folder not read from mapping file")
    self.assertEqual(generator.applicationName, "Test1", "Application name not read from mapping file.")
    self.assertEqual(generator.port, 16665, "Port not read from mapping file")
    self.assertEqual(generator.blockList, None, "Block lists not read from mapping file")
    self.assertEqual(generator.trustList, None, "Trust lists not read from mapping file")
    self.assertEqual(generator.issuerList, None, "Issuer lists not read from mapping file")
    self.assertEqual(generator.certificate, None, "Certificate not read from mapping file")
    self.assertEqual(generator.key, None, "Key not read from mapping file")
    self.assertEqual(generator.addUnsecureEndpoint, False, "Unsecure endpoint information not read from mapping file")
    self.assertEqual(generator.username, None, "User name not read from mapping file")
    self.assertEqual(generator.password, None, "Password not read from mapping file")
    self.assertEqual(generator.enableLogin, False, "Wrong login state detected.")
    self.assertEqual(generator.encryptionEnabled, False, "Encryption state detected is wrong.")
    self.assertEqual(generator.dir.dirs[0].newDestination, None, "Read wrong destination")
    self.assertEqual(generator.dir.dirs[0].newDescription, None, "Read wrong description")
    self.assertEqual(generator.dir.dirs[0].vars[0].newName, None, "Read wrong pv name")
    self.assertEqual(generator.dir.dirs[0].vars[0].newDestination, None, "Read wrong pv destination")
    self.assertEqual(generator.dir.dirs[0].vars[0].newDescription, None, "Read wrong pv description")
    self.assertEqual(generator.dir.dirs[0].vars[0].newUnit, None, "Read wrong pv unit")
  
if __name__ == "__main__":
  logging.basicConfig(format='[%(levelname)s]: %(message)s', level=logging.DEBUG)
  unittest.main()    