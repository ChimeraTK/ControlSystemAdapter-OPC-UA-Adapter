import unittest
import logging
from ctk_opcua_generator_tools.generatorClass import MapGenerator, XMLDirectory, XMLVar, EncryptionSettings

class Test(unittest.TestCase):
  def testReadXML(self):
    generator = MapGenerator("test.xml")
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

if __name__ == "__main__":
  logging.basicConfig(format='[%(levelname)s]: %(message)s', level=logging.DEBUG)
  unittest.main()    