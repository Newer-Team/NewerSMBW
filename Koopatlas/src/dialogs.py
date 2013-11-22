from common import *


class KPTilesetChooserDialog(QtGui.QDialog):
	def __init__(self, label='Choose a tileset', specials=None):
		QtGui.QDialog.__init__(self)

		self.label = QtGui.QLabel(label)
		self.label.setWordWrap(True)

		# can't be assed to create a model
		self.chooser = QtGui.QListWidget()

		self.nameList = KP.knownTilesets.keys()
		self.nameList.sort()

		# this piece of the API is kinda shitty but whatever
		self.specials = specials
		if specials:
			self.chooser.addItems(self.specials)
		self.chooser.addItems(self.nameList)

		self.chooser.currentRowChanged.connect(self.handleCurrentRowChanged)
		self.chooser.itemActivated.connect(self.handleItemActivated)

		self.buttons = QtGui.QDialogButtonBox(
				QtGui.QDialogButtonBox.Ok |
				QtGui.QDialogButtonBox.Cancel)

		self.okButton = self.buttons.button(QtGui.QDialogButtonBox.Ok)
		self.okButton.setEnabled(False)

		self.buttons.accepted.connect(self.accept)
		self.buttons.rejected.connect(self.reject)

		self.layout = QtGui.QVBoxLayout()
		self.layout.addWidget(self.label)
		self.layout.addWidget(self.chooser)
		self.layout.addWidget(self.buttons)
		self.setLayout(self.layout)
	
	def handleCurrentRowChanged(self, row):
		self.okButton.setEnabled(row != -1)
	
	def handleItemActivated(self, item):
		self.accept()

	def getChoice(self):
		item = self.chooser.currentItem()
		number = self.chooser.currentRow()

		if item is None:
			return None
		else:
			if self.specials is not None and number < len(self.specials):
				return number
			else:
				return str(item.text())


	@classmethod
	def run(cls, *args):
		dialog = cls(*args)
		result = dialog.exec_()

		if result == QtGui.QDialog.Accepted:
			return dialog.getChoice()
		else:
			return None




class KPAnimationPresetChooser(QtGui.QDialog):
	def __init__(self, label='Choose a preset to add:', specials=None):
		QtGui.QDialog.__init__(self)

		self.label = QtGui.QLabel(label)
		self.label.setWordWrap(True)

		# can't be assed to create a model
		self.chooser = QtGui.QListWidget()

		settings = KP.app.settings
		import mapfile

		if settings.contains('AnimationPresets'):
			self.presetList = mapfile.load(str(settings.value('AnimationPresets').toPyObject()))
			self.presets = mapfile.load(str(settings.value('AnimationPresetData').toPyObject()))

		else:
			self.presetList =  ["Circle", "Wiggle", "Drifting Cloud"]
			self.presets = [   [["Loop", "Sinusoidial", 200.0, "X Position", -200.0, 200.0, 0, 0],
								["Loop", "Cosinoidial", 200.0, "Y Position", -200.0, 200.0, 0, 0]],

							   [["Reversible Loop", "Sinusoidial", 50.0, "Y Scale", 100.0, 120.0, 0, 0],
							    ["Loop", "Cosinoidial", 50.0, "X Scale", 100.0, 90.0, 0, 0],
								["Reversible Loop", "Sinusoidial", 20.0, "Angle", 10.0, -10.0, 0, 0]],

							   [["Loop", "Sinusoidial", 5000.0, "X Position", -400.0, 400.0, 0, 0],
							    ["Loop", "Sinusoidial", 200.0, "Y Position", 10.0, -10.0, 0, 0],
								["Reversible Loop", "Linear", 500.0, "Opacity", 80.0, 40.0, 0, 0]]  ]

			settings.setValue('AnimationPresets', mapfile.dump(self.presetList))
			settings.setValue('AnimationPresetData', mapfile.dump(self.presets))


		self.chooser.addItems(self.presetList)

		self.chooser.currentRowChanged.connect(self.handleCurrentRowChanged)
		self.chooser.itemActivated.connect(self.handleItemActivated)

		self.buttons = QtGui.QDialogButtonBox(
				QtGui.QDialogButtonBox.Ok |
				QtGui.QDialogButtonBox.Cancel)

		self.okButton = self.buttons.button(QtGui.QDialogButtonBox.Ok)
		self.okButton.setEnabled(False)

		self.buttons.accepted.connect(self.accept)
		self.buttons.rejected.connect(self.reject)

		self.layout = QtGui.QVBoxLayout()
		self.layout.addWidget(self.label)
		self.layout.addWidget(self.chooser)
		self.layout.addWidget(self.buttons)
		self.setLayout(self.layout)
	
	def handleCurrentRowChanged(self, row):
		self.okButton.setEnabled(row != -1)
	
	def handleItemActivated(self, item):
		self.accept()

	def getChoice(self):
		item = self.chooser.currentItem()
		number = self.chooser.currentRow()

		if item is None:
			return None
		else:
			return self.presets[number]


	@classmethod
	def run(cls, *args):
		dialog = cls(*args)
		result = dialog.exec_()

		if result == QtGui.QDialog.Accepted:
			return dialog.getChoice()
		else:
			return None





def getTextDialog(title, label, existingText=''):

	text, ok = QtGui.QInputDialog.getText(KP.mainWindow, title, label, QtGui.QLineEdit.Normal, existingText)

	print text
	if ok and text != '':
		return text
	else:
		return None



