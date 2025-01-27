# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'generator/encryptionUI.ui'
#
# Created by: PyQt5 UI code generator 5.15.6
#
# WARNING: Any manual changes made to this file will be lost when pyuic5 is
# run again.  Do not edit this file unless you know what you are doing.


from PyQt5 import QtCore, QtGui, QtWidgets


class Ui_EncryptionDialog(object):
    def setupUi(self, EncryptionDialog):
        EncryptionDialog.setObjectName("EncryptionDialog")
        EncryptionDialog.resize(837, 300)
        self.verticalLayout = QtWidgets.QVBoxLayout(EncryptionDialog)
        self.verticalLayout.setObjectName("verticalLayout")
        self.line = QtWidgets.QFrame(EncryptionDialog)
        self.line.setFrameShape(QtWidgets.QFrame.HLine)
        self.line.setFrameShadow(QtWidgets.QFrame.Sunken)
        self.line.setObjectName("line")
        self.verticalLayout.addWidget(self.line)
        self.label_6 = QtWidgets.QLabel(EncryptionDialog)
        self.label_6.setObjectName("label_6")
        self.verticalLayout.addWidget(self.label_6)
        self.horizontalLayout = QtWidgets.QHBoxLayout()
        self.horizontalLayout.setObjectName("horizontalLayout")
        self.label = QtWidgets.QLabel(EncryptionDialog)
        self.label.setObjectName("label")
        self.horizontalLayout.addWidget(self.label)
        spacerItem = QtWidgets.QSpacerItem(40, 20, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Minimum)
        self.horizontalLayout.addItem(spacerItem)
        self.userCert = QtWidgets.QLineEdit(EncryptionDialog)
        self.userCert.setEnabled(True)
        self.userCert.setMinimumSize(QtCore.QSize(500, 0))
        self.userCert.setMaximumSize(QtCore.QSize(16777215, 16777215))
        self.userCert.setReadOnly(True)
        self.userCert.setObjectName("userCert")
        self.horizontalLayout.addWidget(self.userCert)
        self.setUserCert = QtWidgets.QPushButton(EncryptionDialog)
        self.setUserCert.setObjectName("setUserCert")
        self.horizontalLayout.addWidget(self.setUserCert)
        self.verticalLayout.addLayout(self.horizontalLayout)
        self.horizontalLayout_2 = QtWidgets.QHBoxLayout()
        self.horizontalLayout_2.setObjectName("horizontalLayout_2")
        self.label_2 = QtWidgets.QLabel(EncryptionDialog)
        self.label_2.setObjectName("label_2")
        self.horizontalLayout_2.addWidget(self.label_2)
        spacerItem1 = QtWidgets.QSpacerItem(40, 20, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Minimum)
        self.horizontalLayout_2.addItem(spacerItem1)
        self.privateKey = QtWidgets.QLineEdit(EncryptionDialog)
        self.privateKey.setEnabled(True)
        self.privateKey.setMinimumSize(QtCore.QSize(500, 0))
        self.privateKey.setMaximumSize(QtCore.QSize(16777215, 16777215))
        self.privateKey.setReadOnly(True)
        self.privateKey.setObjectName("privateKey")
        self.horizontalLayout_2.addWidget(self.privateKey)
        self.setPrivateKey = QtWidgets.QPushButton(EncryptionDialog)
        self.setPrivateKey.setObjectName("setPrivateKey")
        self.horizontalLayout_2.addWidget(self.setPrivateKey)
        self.verticalLayout.addLayout(self.horizontalLayout_2)
        self.line_2 = QtWidgets.QFrame(EncryptionDialog)
        self.line_2.setFrameShape(QtWidgets.QFrame.HLine)
        self.line_2.setFrameShadow(QtWidgets.QFrame.Sunken)
        self.line_2.setObjectName("line_2")
        self.verticalLayout.addWidget(self.line_2)
        self.label_7 = QtWidgets.QLabel(EncryptionDialog)
        self.label_7.setObjectName("label_7")
        self.verticalLayout.addWidget(self.label_7)
        self.horizontalLayout_4 = QtWidgets.QHBoxLayout()
        self.horizontalLayout_4.setObjectName("horizontalLayout_4")
        self.label_4 = QtWidgets.QLabel(EncryptionDialog)
        self.label_4.setObjectName("label_4")
        self.horizontalLayout_4.addWidget(self.label_4)
        spacerItem2 = QtWidgets.QSpacerItem(40, 20, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Minimum)
        self.horizontalLayout_4.addItem(spacerItem2)
        self.trustList = QtWidgets.QLineEdit(EncryptionDialog)
        self.trustList.setEnabled(True)
        self.trustList.setMinimumSize(QtCore.QSize(500, 0))
        self.trustList.setReadOnly(True)
        self.trustList.setObjectName("trustList")
        self.horizontalLayout_4.addWidget(self.trustList)
        self.setTrustList = QtWidgets.QPushButton(EncryptionDialog)
        self.setTrustList.setObjectName("setTrustList")
        self.horizontalLayout_4.addWidget(self.setTrustList)
        self.verticalLayout.addLayout(self.horizontalLayout_4)
        self.horizontalLayout_3 = QtWidgets.QHBoxLayout()
        self.horizontalLayout_3.setObjectName("horizontalLayout_3")
        self.label_3 = QtWidgets.QLabel(EncryptionDialog)
        self.label_3.setObjectName("label_3")
        self.horizontalLayout_3.addWidget(self.label_3)
        spacerItem3 = QtWidgets.QSpacerItem(40, 20, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Minimum)
        self.horizontalLayout_3.addItem(spacerItem3)
        self.blockList = QtWidgets.QLineEdit(EncryptionDialog)
        self.blockList.setEnabled(True)
        self.blockList.setMinimumSize(QtCore.QSize(500, 0))
        self.blockList.setReadOnly(True)
        self.blockList.setObjectName("blockList")
        self.horizontalLayout_3.addWidget(self.blockList)
        self.setBlockList = QtWidgets.QPushButton(EncryptionDialog)
        self.setBlockList.setObjectName("setBlockList")
        self.horizontalLayout_3.addWidget(self.setBlockList)
        self.verticalLayout.addLayout(self.horizontalLayout_3)
        self.horizontalLayout_5 = QtWidgets.QHBoxLayout()
        self.horizontalLayout_5.setObjectName("horizontalLayout_5")
        self.label_5 = QtWidgets.QLabel(EncryptionDialog)
        self.label_5.setObjectName("label_5")
        self.horizontalLayout_5.addWidget(self.label_5)
        spacerItem4 = QtWidgets.QSpacerItem(40, 20, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Minimum)
        self.horizontalLayout_5.addItem(spacerItem4)
        self.issuerList = QtWidgets.QLineEdit(EncryptionDialog)
        self.issuerList.setEnabled(True)
        self.issuerList.setMinimumSize(QtCore.QSize(500, 0))
        self.issuerList.setReadOnly(True)
        self.issuerList.setObjectName("issuerList")
        self.horizontalLayout_5.addWidget(self.issuerList)
        self.setIssuerList = QtWidgets.QPushButton(EncryptionDialog)
        self.setIssuerList.setObjectName("setIssuerList")
        self.horizontalLayout_5.addWidget(self.setIssuerList)
        self.verticalLayout.addLayout(self.horizontalLayout_5)
        self.buttonBox = QtWidgets.QDialogButtonBox(EncryptionDialog)
        self.buttonBox.setOrientation(QtCore.Qt.Horizontal)
        self.buttonBox.setStandardButtons(QtWidgets.QDialogButtonBox.Cancel|QtWidgets.QDialogButtonBox.Ok)
        self.buttonBox.setObjectName("buttonBox")
        self.verticalLayout.addWidget(self.buttonBox)

        self.retranslateUi(EncryptionDialog)
        self.buttonBox.accepted.connect(EncryptionDialog.accept) # type: ignore
        self.buttonBox.rejected.connect(EncryptionDialog.reject) # type: ignore
        QtCore.QMetaObject.connectSlotsByName(EncryptionDialog)

    def retranslateUi(self, EncryptionDialog):
        _translate = QtCore.QCoreApplication.translate
        EncryptionDialog.setWindowTitle(_translate("EncryptionDialog", "Dialog"))
        self.label_6.setText(_translate("EncryptionDialog", "Required data:"))
        self.label.setText(_translate("EncryptionDialog", "User certificate:"))
        self.setUserCert.setText(_translate("EncryptionDialog", "Set"))
        self.label_2.setText(_translate("EncryptionDialog", "Private Key:"))
        self.setPrivateKey.setText(_translate("EncryptionDialog", "Set"))
        self.label_7.setText(_translate("EncryptionDialog", "Optional data (trust list and/or revocation list and issuer list):"))
        self.label_4.setText(_translate("EncryptionDialog", "Trust list directory:"))
        self.setTrustList.setToolTip(_translate("EncryptionDialog", "<html><head/><body><p>Set directory that includes certificates that should be trusted, i.e. self-signed certificates.</p><p>No certification authority is needed. </p></body></html>"))
        self.setTrustList.setText(_translate("EncryptionDialog", "Set"))
        self.label_3.setText(_translate("EncryptionDialog", "Revocation lists directory:"))
        self.setBlockList.setToolTip(_translate("EncryptionDialog", "<html><head/><body><p>Set directory that includes revocation lists for certificate authorities.</p></body></html>"))
        self.setBlockList.setText(_translate("EncryptionDialog", "Set"))
        self.label_5.setText(_translate("EncryptionDialog", "Issuer list directory:"))
        self.setIssuerList.setToolTip(_translate("EncryptionDialog", "<html><head/><body><p>Set directory that includes certificates of certificate authorities.</p></body></html>"))
        self.setIssuerList.setText(_translate("EncryptionDialog", "Set"))
