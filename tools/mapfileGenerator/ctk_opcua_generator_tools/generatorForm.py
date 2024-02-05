from PyQt5.QtWidgets import QFileDialog, QDialog, QMainWindow, QTreeWidgetItem, QCheckBox, QMessageBox, QDialogButtonBox, QComboBox
from PyQt5.QtCore import Qt
from PyQt5.QtGui import QBrush, QColor
import os
from ctk_opcua_generator_tools.generatorUI import Ui_MainWindow
from ctk_opcua_generator_tools.encryptionUI import Ui_EncryptionDialog
from ctk_opcua_generator_tools.historyUI import Ui_HistoryDialog
from ctk_opcua_generator_tools.generatorClass import MapGenerator, XMLDirectory, XMLVar, EncryptionSettings, HistorySetting
import logging
from typing import Optional, List



class EncryptionDialog(QDialog, Ui_EncryptionDialog):
  def _getFileDialog(self):
    dlg = QFileDialog(self)
    dlg.setFileMode(QFileDialog.ExistingFile)
    dlg.setViewMode( QFileDialog.Detail )
    dlg.setNameFilters( [self.tr('Certificate file (*.der)'), self.tr('All Files (*)')] )
    dlg.setDefaultSuffix('der')
    return dlg

  def openUserCert(self):
    dlg = self._getFileDialog()
    dlg.setWindowTitle('Set application certificate file')
    # show dialog, save only if user did not cancel
    if dlg.exec_() :
        self.settings.certificate = str(dlg.selectedFiles()[0])
        self.userCert.setText(self.settings.certificate)

  def openPrivateKey(self):
    dlg = self._getFileDialog()
    dlg.setWindowTitle('Set appliction private key file')
    # show dialog, save only if user did not cancel
    if dlg.exec_() :
        self.settings.key = str(dlg.selectedFiles()[0])
        self.privateKey.setText(self.settings.key)
        
  def _openDirectory(self) -> Optional[str]:
    dlg = QFileDialog(self)
    dlg.setFileMode(QFileDialog.Directory)
    dlg.setOption(QFileDialog.ShowDirsOnly)
    if dlg.exec() :
      return str(dlg.selectedFiles()[0])
    else:
      return None
    
  def openTrustList(self):
    directory = self._openDirectory()
    if directory:
      self.settings.trustList = directory
      self.trustList.setText(directory)
      
  def openBlockList(self):
    directory = self._openDirectory()
    if directory:
      self.settings.blockList = directory
      self.blockList.setText(directory)

  def openIssuerList(self):
    directory = self._openDirectory()
    if directory:
      self.settings.issuerList = directory
      self.issuerList.setText(directory)
    
  
  def __init__(self, data:EncryptionSettings, parent=None):
    super(EncryptionDialog, self).__init__(parent)
    self.setupUi(self)
    
    self.settings = data
    if data.certificate:
      self.userCert.setText(data.certificate)
    if data.key:
      self.privateKey.setText(data.key)
    if data.trustList:
      self.trustList.setText(data.trustList)
    if data.blockList:
      self.blockList.setText(data.blockList)
    if data.issuerList:
      self.issuerList.setText(data.issuerList)
    
    self.setUserCert.clicked.connect(self.openUserCert)
    self.setPrivateKey.clicked.connect(self.openPrivateKey)
    self.setBlockList.clicked.connect(self.openBlockList)
    self.setTrustList.clicked.connect(self.openTrustList)
    self.setIssuerList.clicked.connect(self.openIssuerList)
    
class HistorySettingsDialog(QDialog, Ui_HistoryDialog):
  def __init__(self, data:QComboBox, histories: List[HistorySetting], edit:bool, parent=None):
    '''
    @param data: The setting to be added.
    @param histories: The list of all settings. Is used to ensure that names are unique!
    '''
    super(HistorySettingsDialog,self).__init__(parent)
    self.setupUi(self)
    self.data = data.currentData()
    self.historyName.setText(self.data.name)
    self.entriesPerResponse.setValue(self.data.entriesPerResponse)
    self.bufferLength.setValue(self.data.bufferLength)
    self.samplingInterval.setValue(self.data.interval)
    self.histories = histories
    self.edit = edit
    # Save current text -> if edit is true and the name is changed we need to check again if it already exists
    self.combo = data
    
    self.historyName.editingFinished.connect(self.checkNameExists)
    self.samplingInterval.valueChanged.connect(self.updateData)
    self.entriesPerResponse.valueChanged.connect(self.updateData)
    self.bufferLength.valueChanged.connect(self.updateData)
    
  def checkNameExists(self):
    # check if is new or name was changed
    if self.edit == False or self.combo.currentText() != self.historyName.text():
      found = next((x for x in self.histories if x.name == self.historyName.text()),None)
      if found != None:
        self.buttonBox.button( QDialogButtonBox.Ok ).setEnabled( False );
      else:
        self.buttonBox.button( QDialogButtonBox.Ok ).setEnabled( True );
        self.data.name = self.historyName.text()
        self.combo.setItemText(self.combo.currentIndex(),self.historyName.text())
        
        
      
  def updateData(self):
    self.data.entriesPerResponse = self.entriesPerResponse.value()
    self.data.interval = self.samplingInterval.value()
    self.data.bufferLength = self.bufferLength.value()
      
class MapGeneratorForm(QMainWindow, Ui_MainWindow):
  def _createMapGenerator(self, fileName: str):
    try:    
      self.MapGenerator: MapGenerator|None = MapGenerator(fileName)
      self.statusbar.showMessage(fileName)
      self.fillTree()
    except RuntimeError as error:
      QMessageBox.critical(self, "Map file generator", 
                           "Could not parse xml file. Is that a file generated by the XML generator of your application?\n Error: {}".format(error) )
      self.treeWidget.clear()
      self.MapGenerator = None
    except:
      QMessageBox.critical(self, "Map file generator", 
                           "Could not parse xml file. Is that a file generated by the XML generator of your application?" )
      self.treeWidget.clear()
      self.MapGenerator = None
    
  def openFile(self):
    '''
    Read all information from the given XML file.
    Data is stored in the MapGenerator object.
    '''
    name = QFileDialog.getOpenFileName(self, caption='Set input file', filter="XML files (*.xml)", options=QFileDialog.DontResolveSymlinks)
    if name[0]:
      self._createMapGenerator(name[0])
  
  def fillTree(self):
    '''
    Fill the tree by passing the root node read from the XML file.
    Filling is done recursively.
    '''
    self.treeWidget.clear()
    # block widget while filling in order to avoid calling _updateItem
    self.treeWidget.blockSignals(True)
    self._createDirectoryNode(self.treeWidget, self.MapGenerator.dir, True)
    self.treeWidget.blockSignals(False)
    # Attention the following fields are set in the GUI but not set in the mapping file as long as they 
    # are not changed by the user!
    # Simply show what will be used, because root folder and application name default to the
    # application name if not set in the mapping file!
    self.applicationName.setText(self.MapGenerator.applicationName)
    self.rootFolder.setText(self.MapGenerator.applicationName)
    
  def _getCheckBox(self, item:XMLDirectory | XMLVar, text:str, node:QTreeWidgetItem):
    '''
    Create a checkbox.
    
    @param item: The XML items that corresponds to the checkbox.
    @param text: The check box label.
    @param node: The widget item where the checkbox will be added. It is assumed it is a QTreeWidgetItem with multiple columns.
    '''
    checkbox = QCheckBox(parent=self.treeWidget, text=text)
    if isinstance(item, XMLDirectory) == True:
      checkbox.stateChanged.connect(lambda state: self._mapDirectory(state,item, node))
    else:
      checkbox.stateChanged.connect(lambda state: self._mapItem(state,item, node))
    return checkbox
  
  def _getComboBox(self, item:XMLDirectory | XMLVar, node:QTreeWidgetItem):
    '''
    Create a combo box.
    
    @param item: The XML items that corresponds to the checkbox.
    @param node: The widget item where the checkbox will be added. It is assumed it is a QTreeWidgetItem with multiple columns.
    '''
    combobox = QComboBox(parent=self.treeWidget)
    combobox.addItem('No history', None)
    combobox.currentIndexChanged.connect(lambda histIndex: self._setHistoryItem(histIndex, item, combobox, node))
    return combobox
  
  def _updateItem(self, item:QTreeWidgetItem, index:int):
    self.treeWidget.blockSignals(True)
    data = item.data(0, Qt.UserRole)
    if index == 0:
      # Name
      data.newName = item.text(index)
      item.setText(index, item.text(index) + " (Orig.: " + data.name + ")")
      item.setForeground(index,QBrush(QColor("#FF0000")))
    elif index == 3:
      # Unit
      if isinstance(data,XMLVar):
        data.newUnit = item.text(index)
        item.setText(index, item.text(index) + " (Orig.: " + str(data.unit or '') + ")")
        item.setForeground(index,QBrush(QColor("#FF0000")))
      else:
        item.setText(index, '')
        msg = "Adding unit to a directory does not make sense!"
        QMessageBox.warning(self, "Map file generator", msg )
    elif index == 4:
      # Description
      data.newDescription = item.text(index)
      item.setText(index, item.text(index) +  " (Added descr.)")
      item.setForeground(index,QBrush(QColor("#FF0000")))
    elif index == 5:
      # additional Location
      data.newDestination = item.text(index)
      item.setText(5,data.newDestination)
      item.setForeground(5,QBrush(QColor("#FF0000")))
    self.treeWidget.blockSignals(False)
    
  def _setupRow(self, item:QTreeWidgetItem, data: XMLDirectory|XMLVar):
    item.setData(0, Qt.UserRole, data)
    self.treeWidget.setItemWidget(item, 1, self._getCheckBox(data, "exclude", item))
    self.treeWidget.setItemWidget(item, 2, self._getComboBox(data, item))
    if isinstance(data, XMLDirectory):
      item.setFlags(Qt.ItemIsEditable | Qt.ItemIsEnabled | Qt.ItemIsDragEnabled | Qt.ItemIsSelectable | Qt.ItemIsDropEnabled)
    else:
      item.setFlags(Qt.ItemIsEditable | Qt.ItemIsEnabled | Qt.ItemIsDragEnabled | Qt.ItemIsSelectable)
  
  def _setState(self, state:int, item: XMLDirectory|XMLVar):
    '''
    Set data in XMLDirectory or XMLVar.
    This is used to enable mapping.
    '''
    if state == Qt.Checked:
      item.exclude = True
      logging.debug("Excluding item {}.".format(item.name))
    else:
      item.exclude = False
      logging.debug("Including item {}.".format(item.name))

  def _createVariableNode(self, parent:QTreeWidgetItem, v:XMLVar) -> QTreeWidgetItem:
    '''
    Create variable entry in the tree.
    Reads description and unit. 
    Add CheckBox to enable/disable mapping of that variable.
    '''
    name = v.name
    if v.newName != None:
      name = "{} (Orig.: {})".format(v.newName, v.name)
    description = v.description
    if v.newDescription != None:
      description = "{} (Orig.: {})".format(v.newDescription, v.description)
    unit = v.unit
    if v.newUnit != None:
      unit = "{} (Orig.: {})".format(v.newUnit, v.unit)
    node = QTreeWidgetItem(parent, [name]+[""]*2 + [unit] + [description]+[""])
    if v.newName != None:
      node.setForeground(0,QBrush(QColor("#FF0000")))
    if v.newUnit != None:
      node.setForeground(3,QBrush(QColor("#FF0000")))
    if v.newDescription != None:
      node.setForeground(4,QBrush(QColor("#FF0000")))
    if v.newDestination != None:
      node.setForeground(5,QBrush(QColor("#FF0000")))
    self._setupRow(node, v)
    return node
  
  def _createDirectoryNode(self, parent:QTreeWidgetItem, directory:XMLDirectory, isRootNode:bool = False) -> QTreeWidgetItem:
    '''
    Create a directory entry in the Tree.
    This will recursively add sub directories and variables in the directories.
    '''
    name = directory.name
    if directory.newName != None:
      name = "{} (Orig.: {})".format(directory.newName, directory.name)
    description = ''
    if directory.newDescription != None:
      description = "{} (Added descr.)".format(directory.newDescription)
    mainNode = QTreeWidgetItem(parent, [name] + [""]*3 + [description] + [directory.newDestination or ''])
    if directory.newName != None:
      mainNode.setForeground(0,QBrush(QColor("#FF0000")))
    if directory.newDescription != None:
      mainNode.setForeground(4,QBrush(QColor("#FF0000")))
    if directory.newDestination != None:
      mainNode.setForeground(5,QBrush(QColor("#FF0000")))

    self._setupRow(mainNode, directory)
    if isRootNode == True:
      mainNode.setExpanded(True)
    for d in directory.dirs:
      self._createDirectoryNode(mainNode, d)
    for v in directory.vars:
      self._createVariableNode(mainNode, v)
    return mainNode
  
  def _mapItem(self, state:int, var:XMLVar, node:QTreeWidgetItem):
    '''
    Called when a variable is to be mapped. 
    The corresponding check box is edited.
    '''
    self._setState(state, var)
    logging.debug("Childs: {}".format(node.childCount()))
    
  def _mapDirectory(self, state:int, directory:XMLDirectory, node:QTreeWidgetItem):
    '''
    Called when a directory is to be mapped. 
    The corresponding check box is edited.
    This will change the map status of all variables in the directory.
    '''
    logging.debug("Mapping directory.")
    for chId in range(node.childCount()):
      ch = node.child(chId)
      self.treeWidget.itemWidget(ch, 1).setCheckState(state)
      self._setState(state, directory)
      
  def _setHistoryItem(self, histIndex:int, var:XMLVar|XMLDirectory, combo:QComboBox, node:QTreeWidgetItem):
    '''
    Called when historizing setting is choosen.
    @param histIndex: The index of the history setting.
    '''
    if combo.itemText(histIndex) != 'No history':
      var.historizing = combo.itemText(histIndex)
    else:
      var.historizing = None
      
    if isinstance(var, XMLDirectory):
      for chId in range(node.childCount()):
        ch = node.child(chId)
        self.treeWidget.itemWidget(ch, 2).setCurrentText(combo.itemText(histIndex))
        if var.historizing:
          self.treeWidget.itemWidget(ch, 2).setEnabled(False)
        else:
          self.treeWidget.itemWidget(ch, 2).setEnabled(True)
      
  def closeEvent(self, event):
    msg = "Are you sure you want to close the editor?"
    reply = QMessageBox.question(self, 'ChimeraTK Logical Name Mapping editor', msg, QMessageBox.Yes, QMessageBox.No)
    if reply != QMessageBox.Yes:
      event.ignore()
    else:
      event.accept()
      
  def saveFile(self):
    '''
    Save file. If file not yet set open dialog with saveFileAs()
    '''
    try:
      if self.MapGenerator.outputFile == None:
        self.saveFileAs()
      else:
        self.MapGenerator.save()
    except RuntimeError as error:
      msg = "The following error was reported: " + str(error)
      QMessageBox.critical(self, "Map file generator", msg )
      
  def saveFileAs(self):
    # create file-save dialog
    dlg = QFileDialog(self, directory=self.MapGenerator.applicationName + "_mapping.xml")
    dlg.setAcceptMode(QFileDialog.AcceptSave)
    dlg.setWindowTitle('Save XML mapping file')
    dlg.setViewMode( QFileDialog.Detail )
    dlg.setNameFilters( [self.tr('XML Mapping Files (*.xml)'), self.tr('All Files (*)')] )
    dlg.setDefaultSuffix('xml')
    if self.MapGenerator.outputFile != None:
        dlg.setDirectory( os.path.dirname(self.MapGenerator.outputFile) )
    
    # show dialog, save only if user did not cancel
    if dlg.exec_() :
        # file name must be converted into standard python string
        self.MapGenerator.outputFile = str(dlg.selectedFiles()[0])
        # save the file
        self.saveFile()
    self.actionSave.setEnabled(True)

  def enableLogin(self, state):
    if state == Qt.Checked:
      self.userName.setEnabled(True)
      self.password.setEnabled(True)
      self.MapGenerator.enableLogin = True
    else: 
      self.userName.setEnabled(False)
      self.password.setEnabled(False)
      self.MapGenerator.enableLogin = False
      
  def updateConfig(self):
    self.MapGenerator.username = self.userName.text()
    self.MapGenerator.password = self.password.text()
    self.MapGenerator.applicationName = self.applicationName.text()
    self.MapGenerator.rootFolder = self.rootFolder.text()
    self.MapGenerator.applicationDescription = self.applicationDescription.text()
    self.MapGenerator.port = self.port.value()
    
    
  def dropEvent(self, event):
    # get item where to drop
    item = self.treeWidget.itemAt(event.pos())
    if isinstance(item.data(0,Qt.UserRole), XMLVar):
      msg = "Variables or directories can only be moved to other directories."
      QMessageBox.critical(self, "Map file generator", msg )
      return
    # get item that is dropped
    current = self.treeWidget.currentItem()
    # get data that is dropped
    currentData = current.data(0,Qt.UserRole)
    currentData.newDestination = item.data(0,Qt.UserRole).path
    current.setText(5,currentData.newDestination)
    current.setForeground(5,QBrush(QColor("#FF0000")))
    # create new item in the dropped place
    # self._createVariableNode(item, currentData)
    # remove original item -> leave original PV/directory
    # parent = current.parent()
    # parent.removeChild(current)
  
  def dragEnterEvent(self, e)->None:
    e.accept()
    
  def configureEncryption(self):
    dlg = EncryptionDialog(parent=self, data=self.MapGenerator)
    dlg.exec()
    
  def updateEncryptionConfiguration(self, state):
    states = [self.enableEncryptionButton.isChecked(),
              self.addUnsecureEndpoint.isChecked()]
    self.addUnsecureEndpoint.setEnabled(states[0])
    self.configureEncryptionButton.setEnabled(states[0])
    self.MapGenerator.encryptionEnabled = states[0]
    self.MapGenerator.addUnsecureEndpoint = states[1]
  
  def addHistorySetting(self):
    histName = 'historySetting'
    if self.histories.findText(histName) != -1:
      i = 0
      histName = 'historySetting_{}'.format(i)
      while(self.histories.findText(histName) != -1):
        histName = 'historySetting_{}'.format(i)
        i = i+1
    setting = HistorySetting(name=histName)
    # add item to the historizing combo box
    self.histories.addItem(setting.name, setting)
    self.histories.setCurrentText(setting.name)
    dlg = HistorySettingsDialog(parent=self, data=self.histories, histories=self.MapGenerator.historySettings, edit=False)
    dlg.exec()
    logging.info("Adding historizing setting with name: {}".format(setting.name))
    # add HistorySetting to the settings of the MapGenerator
    self.MapGenerator.historySettings.append(setting)
    # add history settings to the combo boxes in the TreeWidget (recursively)
    self.addComboEntry(self.treeWidget.itemAt(0,0), setting)
    self.editHistorySettingButton.setEnabled(True)

  def addComboEntry(self, item:QTreeWidgetItem, setting:HistorySetting):
    self.treeWidget.itemWidget(item, 2).addItem(setting.name, setting)
    for chId in range(item.childCount()):
      self.addComboEntry(item.child(chId), setting)
      
  def editHistorySetting(self):
    dlg = HistorySettingsDialog(parent=self, data=self.histories,histories=self.MapGenerator.historySettings, edit=True)
    dlg.exec()
    
  def addHistoryForInputs(self, node:QTreeWidgetItem):
    data = node.data(0, Qt.UserRole)
    if isinstance(data, XMLVar) and data.direction == 'control_system_to_application':
      self.treeWidget.itemWidget(node, 2).setCurrentText(self.histories.currentText())
    else:
      for chId in range(node.childCount()):
        self.addHistoryForInputs(node.child(chId))
  
  def _blockAndSetTextBox(self, value: str, control):
    '''
    Set a text box without updating the configuration,
    which would override parameters of the MapGenerator.
    '''
    control.blockSignals(True)
    if value != None:
      control.setText(value)
    else:
      control.setText(value)
    control.blockSignals(False)
    
  def _blockAndSetCheckbox(self, value: bool, control):
    '''
    Set a check box without updating the configuration,
    which would override parameters of the MapGenerator.
    '''
    control.blockSignals(True)
    if value == True:
      control.setChecked(True)
    else:
      control.setChecked(False)
    control.blockSignals(False)
    
  def loadMapFile(self):
    if self.MapGenerator == None:
      msg = "You have to open a xml file first."
      QMessageBox.critical(self, "Map file generator", msg )
    else:
      name = QFileDialog.getOpenFileName(self, caption='Load existing mapping file', filter="XML files (*.xml)", options=QFileDialog.DontResolveSymlinks)
      if name[0]:
        try:    
          missed = self.MapGenerator.parseMapFile(name[0])
          if missed[0] != 0 or missed[1] != 0:
            QMessageBox.warning(self, "Map file generator", 
                           "{} directories and {} PV could not be found by their source name in the original variable tree.\n Probably"
                           " the mapping file does not correspong to the application XML file or the application was changed in the mean time.".format(missed[0], missed[1]))
          self._blockAndSetCheckbox(self.MapGenerator.addUnsecureEndpoint, self.addUnsecureEndpoint)
          self._blockAndSetCheckbox(self.MapGenerator.encryptionEnabled, self.enableEncryptionButton)
          self._blockAndSetCheckbox(self.MapGenerator.enableLogin, self.enableLoginSwitch)
          self.configureEncryptionButton.setEnabled(self.MapGenerator.encryptionEnabled)
          self.addUnsecureEndpoint.setEnabled(self.MapGenerator.encryptionEnabled)
          self.userName.setEnabled(self.MapGenerator.enableLogin)
          self.password.setEnabled(self.MapGenerator.enableLogin)
          self._blockAndSetTextBox(self.MapGenerator.username, self.userName)
          self._blockAndSetTextBox(self.MapGenerator.password, self.password)
          # update the tree
          self.fillTree()
          # update historizing settings
          for setting in self.MapGenerator.historySettings:
            if self.histories.findText(setting.name) == -1:
              self.histories.addItem(setting.name,setting)
            else:
              logging.warning("Setting with name {} already exist. Not loading parameters from mapping file.".format(setting.name))
        except RuntimeError as error:
          QMessageBox.critical(self, "Map file generator", 
                               "Failed to load map file: {}\n Error: {}".format(name[0], error) )
          
  def __init__(self, args, parent=None):
    super(MapGeneratorForm, self).__init__(parent)
    self.setupUi(self)
    

    self.MapGenerator = None
    if args.input:
      self._createMapGenerator(args.input)
    else:
      self.openFile()
  
    self.treeWidget.itemChanged.connect(self._updateItem)
    
    self.actionOpen_File.triggered.connect(self.openFile)
    self.actionSave_as.triggered.connect(self.saveFileAs)
    self.actionSave.triggered.connect(self.saveFile)
    self.actionLoad_mapping_file.triggered.connect(self.loadMapFile)
    self.actionQuit.triggered.connect(self.close)
    
    self.enableLoginSwitch.stateChanged.connect(self.enableLogin)
    self.password.textChanged.connect(self.updateConfig)
    self.userName.textChanged.connect(self.updateConfig)
    self.applicationName.textChanged.connect(self.updateConfig)
    self.rootFolder.textChanged.connect(self.updateConfig)
    self.applicationDescription.textChanged.connect(self.updateConfig)
    self.enableEncryptionButton.stateChanged.connect(self.updateEncryptionConfiguration)
    self.addUnsecureEndpoint.stateChanged.connect(self.updateEncryptionConfiguration)
    self.configureEncryptionButton.clicked.connect(self.configureEncryption)
    self.editHistorySettingButton.clicked.connect(self.editHistorySetting)
    self.addHistorySettingButton.clicked.connect(self.addHistorySetting)
    self.setHistoryForInputsButton.clicked.connect(lambda: self.addHistoryForInputs(self.treeWidget.itemAt(0,0)))

    
    # Allow to move items 
    self.treeWidget.dropEvent = self.dropEvent
    self.treeWidget.dragEnterEvent = self.dragEnterEvent
