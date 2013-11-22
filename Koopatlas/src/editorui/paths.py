from common import *
from editorcommon import *
import weakref

class KPEditorNode(KPEditorItem):
	SNAP_TO = (12,12)


	class ToggleButton(QtGui.QPushButton):
		stateToggled = QtCore.pyqtSignal(int)


		def __init__(self):
			QtGui.QPushButton.__init__(self)

			self.setIconSize(QtCore.QSize(24, 24))
			self.setFixedSize(24, 24)

			self.iconList = [KP.icon('Through'), KP.icon('Level'), KP.icon('Exit'), KP.icon('WorldChange')]

			self.state = 0

			if not hasattr(KPEditorNode.ToggleButton, 'PALETTE'):
				KPEditorNode.ToggleButton.PALETTE = QtGui.QPalette(Qt.transparent)

			self.setPalette(self.PALETTE)
 
			self.released.connect(self.toggle)


		def toggle(self):
			self.state += 1
			if self.state == 4:
				self.state = 0

			self.stateToggled.emit(self.state)


		def paintEvent(self, event):
			painter = QtGui.QPainter(self)

			if self.isDown():
				self.iconList[self.state].paint(painter, self.contentsRect(), Qt.AlignCenter, QtGui.QIcon.Disabled)
			else:
				self.iconList[self.state].paint(painter, self.contentsRect())

			painter.end()


	class HiddenProxy(QtGui.QGraphicsProxyWidget):
		def __init__(self, button, parent, x, y):
			QtGui.QGraphicsProxyWidget.__init__(self, parent)

			self.setWidget(button)
			self.setPos(x, y)
			self.setZValue(parent.zValue()+1000)
			self.hide()


	class LevelSlotSpinner(QtGui.QSpinBox):
		def __init__(self):
			QtGui.QSpinBox.__init__(self)

			self.setRange(1, 99)

			palette = self.palette()
			palette.setColor(QtGui.QPalette.ButtonText, Qt.black)
			palette.setColor(QtGui.QPalette.Window, Qt.transparent)

			self.setPalette(palette)


	class TransitionBox(QtGui.QComboBox):
		def __init__(self):
			QtGui.QComboBox.__init__(self)

			self.addItems(['Fade Out', 'Circle Wipe', 'Bowser Wipe', 'Goo Wipe Down',
						   'Mario Wipe', 'Circle Wipe Slow', 'Glitchgasm'])

			palette = self.palette()
			palette.setColor(QtGui.QPalette.ButtonText, Qt.black)
			palette.setColor(QtGui.QPalette.Window, Qt.transparent)

			self.setPalette(palette)


	class SecretBox(QtGui.QCheckBox):
		def __init__(self):
			QtGui.QCheckBox.__init__(self)

			palette = self.palette()
			palette.setColor(QtGui.QPalette.ButtonText, Qt.black)
			palette.setColor(QtGui.QPalette.Window, Qt.transparent)

			self.setPalette(palette)


	class mapArcEdit(QtGui.QLineEdit):
		def __init__(self):
			QtGui.QLineEdit.__init__(self)

			self.setText('None.arc')

			palette = self.palette()
			palette.setColor(QtGui.QPalette.ButtonText, Qt.black)
			palette.setColor(QtGui.QPalette.Window, Qt.transparent)

			self.setPalette(palette)


	def __init__(self, node):
		KPEditorItem.__init__(self)

		node.qtItem = self
		self._nodeRef = weakref.ref(node)

		self.setZValue(101)

		self._boundingRect = QtCore.QRectF(-24, -24, 48, 48)
		self._levelRect = self._boundingRect
		self._stopRect = QtCore.QRectF(-12, -12, 24, 24)
		self._worldChangeRect = QtCore.QRectF(-16, -16, 32, 32)
		self._tinyRect = QtCore.QRectF(-6, -6, 12, 12)
		self.isLayerSelected = False

		if not hasattr(KPEditorNode, 'SELECTION_PEN'):
			KPEditorNode.SELECTION_PEN = QtGui.QPen(Qt.blue, 1, Qt.DotLine)

		self.button = self.ToggleButton()
		self.buttonProxy = self.HiddenProxy(self.button, self, 12, -24)
		self.button.stateToggled.connect(self.stateChange)


		self.world = self.LevelSlotSpinner()
		self.worldProxy = self.HiddenProxy(self.world, self, -42, 24)
		self.world.valueChanged.connect(self.worldChange)

		self.stage = self.LevelSlotSpinner()
		self.stageProxy = self.HiddenProxy(self.stage, self, 6, 24)
		self.stage.valueChanged.connect(self.stageChange)

		self.secret = self.SecretBox()
		self.secretProxy = self.HiddenProxy(self.secret, self, -60, 26)
		self.secret.stateChanged.connect(self.secretChange)

		if node.level != None:
			self.world.setValue(node.level[0])
			self.stage.setValue(node.level[1])

		if node.hasSecret != None:
			self.secret.setChecked(node.hasSecret)


		self.foreignID = self.LevelSlotSpinner()
		self.foreignID.setRange(0,255)
		self.foreignIDProxy = self.HiddenProxy(self.foreignID, self, 60, 24)
		self.foreignID.valueChanged.connect(self.foreignIDChange)

		self.mapChange = self.mapArcEdit()
		self.mapChangeProxy = self.HiddenProxy(self.mapChange, self, -100, 60)
		self.mapChange.textEdited.connect(self.mapChangeChange)

		self.transition = self.TransitionBox()
		self.transitionProxy = self.HiddenProxy(self.transition, self, -102, 24)
		self.transition.currentIndexChanged.connect(self.transitionChange)

		self.worldDefID = self.LevelSlotSpinner()
		self.worldDefID.setRange(0,255)
		self.worldDefIDProxy = self.HiddenProxy(self.worldDefID, self, 60, 24)
		self.worldDefID.valueChanged.connect(self.worldDefIDChange)

		if node.foreignID != None:
			self.foreignID.setValue(node.foreignID)

		if node.mapChange != None:
			self.mapChange.setText(node.mapChange)

		if node.worldDefID != None:
			self.worldDefID.setValue(node.worldDefID)

		self.transition.setCurrentIndex(node.transition)


		self._updatePosition()


	@QtCore.pyqtSlot(int)
	def stateChange(self, state):

		node = self._nodeRef()

		node.transition = 0
		node.mapChange = None
		node.mapID = None
		node.foreignID = None
		node.level = None
		node.worldDefID = None

		if state == 1:
			node.level = [1, 1]
			self.world.setValue(node.level[0])
			self.stage.setValue(node.level[1])

		elif state == 2:
			node.transition = 0
			node.mapChange = 'None.arc'
			node.foreignID = 0

			usedIDs = []
			for nodesPicker in KP.map.pathLayer.nodes:
				usedIDs.append(nodesPicker.mapID)

			i = 1
			while i in usedIDs:
				i += 1

			node.mapID = i

			self.foreignID.setValue(1)
			self.mapChange.setText('None.arc')
			self.transition.setCurrentIndex(0)

		elif state == 3:
			node.worldDefID = 0


		self.update()
		KP.mainWindow.pathNodeList.update()


	@QtCore.pyqtSlot(int)
	def worldChange(self, world):

		node = self._nodeRef()
		node.level[0] = world

		KP.mainWindow.pathNodeList.update()


	@QtCore.pyqtSlot(int)
	def stageChange(self, stage):

		node = self._nodeRef()
		node.level[1] = stage

		KP.mainWindow.pathNodeList.update()


	@QtCore.pyqtSlot(int)
	def secretChange(self, secret):

		node = self._nodeRef()
		node.hasSecret = secret

		KP.mainWindow.pathNodeList.update()


	@QtCore.pyqtSlot(int)
	def foreignIDChange(self, ID):

		node = self._nodeRef()
		node.foreignID = ID

		KP.mainWindow.pathNodeList.update()


	@QtCore.pyqtSlot(int)
	def worldDefIDChange(self, ID):
		node = self._nodeRef()
		node.worldDefID = ID


	@QtCore.pyqtSlot(int)
	def transitionChange(self, index):

		node = self._nodeRef()
		node.transition = index

		KP.mainWindow.pathNodeList.update()


	@QtCore.pyqtSlot(str)
	def mapChangeChange(self, mapname):

		node = self._nodeRef()
		node.mapChange = mapname

		KP.mainWindow.pathNodeList.update()


	def _updatePosition(self):
		self.ignoreMovement = True

		node = self._nodeRef()
		x, y = node.position
		self.setPos(x+12, y+12)

		self.ignoreMovement = False


	def _itemMoved(self, oldX, oldY, newX, newY):
		node = self._nodeRef()
		node.position = (newX-12, newY-12)

		for exit in node.exits:
			exit.qtItem.updatePosition()


	def setLayerSelected(self, selected):
		self.isLayerSelected = selected
		self.update()


	def paint(self, painter, option, widget):

		painter.setRenderHint(QtGui.QPainter.Antialiasing)
		node = self._nodeRef()

		selectionRect = None

		if node.level:
			painter.setBrush(QtGui.QColor(0, 0, 0, 0))
			painter.setPen(QtGui.QColor(0, 0, 0, 0))
			painter.drawPixmap(self._boundingRect.topLeft(), QtGui.QPixmap("Resources/BlackLevel.png"))
			selectionRect = self._boundingRect.adjusted(1,5,-1,-5)

		elif node.mapChange != None:
			painter.setBrush(QtGui.QColor(0, 0, 0, 0))
			painter.setPen(QtGui.QColor(0, 0, 0, 0))
			painter.drawPixmap(self._boundingRect.topLeft(), QtGui.QPixmap("Resources/ExitWorld.png"))

			textPath = QtGui.QPainterPath()
			font = QtGui.QFont("Times", 22)
			font.setStyleStrategy(QtGui.QFont.ForceOutline)
			textPath.addText(-6, 3, font, str(node.mapID))

			painter.setBrush(QtGui.QColor(140, 140, 255))
			pen = QtGui.QPen(QtGui.QColor(80, 80, 255))
			pen.setWidth(1)
			pen.setStyle(Qt.SolidLine)
			painter.setPen(pen)
			painter.drawPath(textPath)

			selectionRect = self._boundingRect.adjusted(1,5,-1,-5)

		elif node.worldDefID != None:
			painter.setBrush(QtGui.QColor(0, 0, 0, 0))
			painter.setPen(QtGui.QColor(0, 0, 0, 0))
			painter.drawPixmap(self._worldChangeRect.topLeft(), QtGui.QPixmap("Resources/WorldChange.png"))
			selectionRect = self._worldChangeRect

		elif len(node.exits) != 2:
			if self.isLayerSelected:
				brush = QtGui.QBrush(QtGui.QColor(255, 40, 40))
			else:
				brush = QtGui.QBrush(QtGui.QColor(255, 220, 220))
			painter.setPen(QtGui.QColor(255, 255, 255))
			painter.setBrush(brush)
			painter.drawEllipse(self._stopRect)
			selectionRect = self._stopRect.adjusted(-1,-1,1,1)

		else:
			if self.isLayerSelected:
				brush = QtGui.QBrush(QtGui.QColor(255, 40, 40))
			else:
				brush = QtGui.QBrush(QtGui.QColor(255, 255, 255))
			painter.setPen(QtGui.QColor(255, 255, 255))
			painter.setBrush(brush)
			painter.drawEllipse(self._tinyRect)
			selectionRect = self._tinyRect.adjusted(-1,-1,1,1)


		if self.isSelected():
			painter.setPen(self.SELECTION_PEN)
			painter.setBrush(QtGui.QColor(0,0,0,0))
			painter.drawEllipse(selectionRect)
	
			# WHAT THE FUCK SINCE WHEN DO YOU SHOW/HIDE WIDGETS IN A PAINT EVENT
			# oh well, I don't feel like refactoring this
			self.buttonProxy.show()

			if node.level:
				self.worldProxy.show()
				self.stageProxy.show()
				self.secretProxy.show()

			else:
				self.worldProxy.hide()
				self.stageProxy.hide()
				self.secretProxy.hide()

			if node.mapChange != None:
				self.foreignID.show()
				self.transition.show()
				self.mapChange.show()

			else:
				self.foreignID.hide()
				self.transition.hide()
				self.mapChange.hide()

			if node.worldDefID != None:
				self.worldDefID.show()
			else:
				self.worldDefID.hide()

		else:
			self.buttonProxy.hide()
			self.worldProxy.hide()
			self.stageProxy.hide()
			self.secretProxy.hide()
			self.foreignID.hide()
			self.transition.hide()
			self.mapChange.hide()
			self.worldDefID.hide()
		

	def remove(self, withItem=False):
		node = self._nodeRef()
		layer = KP.map.pathLayer

		try:
			layer.nodes.remove(node)
		except ValueError:
			pass

		KP.mainWindow.pathNodeList.removeLayer(node)

		if len(node.exits) == 2:
			# let's try to join the two!
			pathOne, pathTwo = node.exits

			start1, end1 = pathOne._startNodeRef(), pathOne._endNodeRef()
			start2, end2 = pathTwo._startNodeRef(), pathTwo._endNodeRef()

			if start1 == node:
				start = end1
			else:
				start = start1

			if start2 == node:
				end = end2
			else:
				end = start2

			# make sure no path already exists between these nodes
			nope = False

			for pathToCheck in start.exits:
				if pathToCheck._startNodeRef() == end:
					nope = True
				elif pathToCheck._endNodeRef() == end:
					nope = True

			if not nope:
				joinedPath = KPPath(start, end, pathOne)
				KP.mainWindow.pathNodeList.addLayer(joinedPath, False)
				layer.paths.append(joinedPath)
				item = KPEditorPath(joinedPath)
				self.scene().addItem(item)

			for path in (pathOne, pathTwo):
				path.qtItem.remove(True)
		else:
			# we can't join them so just nuke them
			for exit in node.exits[:]:
				exit.qtItem.remove(True)

		if withItem:
			self.scene().removeItem(self)


class KPEditorPath(QtGui.QGraphicsLineItem):


	class PathOptionsMenuButton(QtGui.QPushButton):


		class PathOptionsWidget(QtGui.QWidget):
			def __init__(self, pathRef):
				QtGui.QWidget.__init__(self)

				self._pathRef = pathRef

				TopLayout = QtGui.QGridLayout()
				Layout = QtGui.QGridLayout()

				# Make an exclusive button group for our animations.
				AnimationList = ["Walk", "WalkSand", "WalkSnow", "WalkWater",
								 "Jump", "JumpSand", "JumpSnow", "JumpWater",
								 "Ladder", "LadderLeft", "LadderRight", "Fall",
								 "Swim", "Run", "Pipe", "Door",
								 'TJumped', 'Enter Cave Up', 'Reserved 18', 'Invisible']

				self.ExclusiveButtons = QtGui.QButtonGroup()

				i = 0
				j = 1
				id = 0
				for anim in AnimationList:
					if id < 16:
						newButton = QtGui.QPushButton(QtGui.QIcon("Resources/Anm/" + anim), "")
					else:
						newButton = QtGui.QPushButton(anim)
					newButton.setCheckable(True)
					newButton.setIconSize(QtCore.QSize(38, 38))
					newButton.setToolTip(anim)
					self.ExclusiveButtons.addButton(newButton, id)

					Layout.addWidget(newButton, j, i)

					if id == 0:
						newButton.setChecked(True)

					id += 1
					i += 1
					if i == 4:
						i = 0
						j += 1
				

				
				# Movement Speed Spin Box
				self.moveSpeedSpinner = QtGui.QDoubleSpinBox()
				self.moveSpeedSpinner.setMinimum(0.0)
				self.moveSpeedSpinner.setMaximum(256.0)
				self.moveSpeedSpinner.setDecimals(2)
				self.moveSpeedSpinner.setSingleStep(0.05)
				self.moveSpeedSpinner.setValue(1.0)

				TopLayout.addWidget(self.moveSpeedSpinner, 1, 0)


				# Layer Combo Box
				# self.linkedLayer = QtGui.QComboBox(self)
				# self.linkedLayer.setModel(KP.map.layerModel)

				# TopLayout.addWidget(self.linkedLayer, 1, 1, 1, 3)


				# Connections

				# regular connect doesn't work for some reason...
				#self.ExclusiveButtons.buttonReleased.connect(self.updatePathAnim)
				QtCore.QObject.connect(
						self.ExclusiveButtons,
						QtCore.SIGNAL('buttonReleased(int)'),
						self.updatePathAnim)

				self.moveSpeedSpinner.valueChanged.connect(self.updateMoveSpeed)
				# self.linkedLayer.currentIndexChanged.connect(self.updateLinkLayer)

				# Layout
				TopLayout.addWidget(QtGui.QLabel("Speed:"), 0, 0)
				TopLayout.addWidget(QtGui.QLabel("Layer Shown on Unlock:"), 0, 1, 1, 3)
				Layout.addLayout(TopLayout, 0, 0, 1, 4)
				self.setLayout(Layout)

				
			@QtCore.pyqtSlot(float)
			def updateMoveSpeed(self, speed):
				path = self._pathRef()

				path.movementSpeed = speed
				path.qtItem.update()


			@QtCore.pyqtSlot(int)
			def updatePathAnim(self, buttonID):
				path = self._pathRef()

				path.animation = buttonID
				path.qtItem.update()


			@QtCore.pyqtSlot(int)
			def updateLinkLayer(self, layerIndex):
				path = self._pathRef()

				path.linkedLayer = KP.map.layers[layerIndex]


		def __init__(self, pathRef):
			QtGui.QPushButton.__init__(self)

			self.setText("Options")

			self.menu = QtGui.QMenu(self)
			layout = QtGui.QVBoxLayout()
			self.bgroupWidget = self.PathOptionsWidget(pathRef)
			layout.addWidget(self.bgroupWidget)

			self.menu.setLayout(layout)

			menuPalette = self.menu.palette()
			menuPalette.setColor(QtGui.QPalette.Window, Qt.black)
			self.menu.setPalette(menuPalette)

			# dropShadow = QtGui.QGraphicsDropShadowEffect()
			# self.menu.setGraphicsEffect(dropShadow)
			self.setMenu(self.menu)

			palette = self.palette()
			palette.setColor(QtGui.QPalette.ButtonText, Qt.black)
			palette.setColor(QtGui.QPalette.Window, Qt.transparent)

			self.setPalette(palette)


	class HiddenProxy(QtGui.QGraphicsProxyWidget):
		def __init__(self, button, parent, x, y):
			QtGui.QGraphicsProxyWidget.__init__(self, parent)

			self.setWidget(button)
			self.setPos(x, y)
			self.setZValue(parent.zValue()+1000)
			self.hide()


	def __init__(self, path):
		QtGui.QGraphicsLineItem.__init__(self)

		self.setFlag(self.ItemIsSelectable, True)

		self.setZValue(100)

		startNode = path._startNodeRef().qtItem
		endNode = path._endNodeRef().qtItem

		startNode.update()
		endNode.update()

		self._startNodeRef = weakref.ref(startNode)
		self._endNodeRef = weakref.ref(endNode)
		self._pathRef = weakref.ref(path)
		self.isLayerSelected = False

		path.qtItem = self

		if not hasattr(KPEditorPath, 'PEN'):
			KPEditorPath.BRUSH = QtGui.QBrush(QtGui.QColor(255, 255, 255, 140))
			KPEditorPath.PEN = QtGui.QPen(KPEditorPath.BRUSH, 8, Qt.SolidLine, Qt.RoundCap)
		self.setPen(KPEditorPath.PEN)
	
		if not hasattr(KPEditorPath, 'SELECTION_PEN'):
			KPEditorPath.SELECTION_PEN = QtGui.QPen(Qt.blue, 1, Qt.DotLine)

		self.options = self.PathOptionsMenuButton(self._pathRef)
		self.optionsProxy = self.HiddenProxy(self.options, self, -54, +24)

		self.options.bgroupWidget.ExclusiveButtons.button(path.animation).setChecked(True)
		self.options.bgroupWidget.moveSpeedSpinner.setValue(path.movementSpeed)

		self.updatePosition()

	def mousePressEvent(self, event):
		if event.button() != Qt.LeftButton:
			return
		if QtGui.QApplication.keyboardModifiers() != QtCore.Qt.ControlModifier:
			return

		# modify the unlock settings
		from unlock import KPUnlockSpecDialog
		
		dlg = KPUnlockSpecDialog('path', 'unlocked')

		if hasattr(self._pathRef(), 'unlockSpec'):
			dlg.setSpec(self._pathRef().unlockSpec)

		result = dlg.exec_()
		if result == QtGui.QDialog.Accepted:
			print "New spec:", dlg.spec
			self._pathRef().unlockSpec = dlg.spec


	def updatePosition(self):
		path = self._pathRef()

		sn = path._startNodeRef()
		en = path._endNodeRef()
		if sn is None or en is None:
			return

		x1, y1 = path._startNodeRef().position
		x2, y2 = path._endNodeRef().position

		originLine = QtCore.QLineF(x1+12, y1+12, x2+12, y2+12)
		self.setPos(originLine.pointAt(0.5))
		dy = originLine.dy()/2
		dx = originLine.dx()/2

		self.setLine(QtCore.QLineF(-dx, -dy, dx, dy))
	

	def setLayerSelected(self, selected):
		self.isLayerSelected = selected
		self.update()


	def paint(self, painter, option, widget):

		painter.setRenderHint(QtGui.QPainter.Antialiasing)

		if self.isLayerSelected:
			brush = QtGui.QBrush(QtGui.QColor(255, 40, 40, 200))
			pen = QtGui.QPen(brush, 12, Qt.SolidLine, Qt.RoundCap)
			painter.setPen(pen)
			painter.setBrush(brush)
		else:
			painter.setPen(KPEditorPath.PEN)

		painter.drawLine(self.line())

		if self.isSelected():
			painter.setPen(self.SELECTION_PEN)
			painter.setBrush(QtGui.QColor(0,0,0,0))
			painter.drawPath(self.shape())
			self.optionsProxy.show()

		else:
			self.optionsProxy.hide()


	def remove(self, withItem=False):
		if hasattr(self, 'hasBeenRemovedAlready'):
			return

		self.hasBeenRemovedAlready = True

		path = self._pathRef()
		layer = KP.map.pathLayer
		KP.mainWindow.pathNodeList.removeLayer(path)

		layer.paths.remove(path)

		for ref in (self._startNodeRef, self._endNodeRef):
			node = ref()._nodeRef()
			try:
				node.exits.remove(path)
				node.qtItem.update()
			except ValueError:
				pass

		if withItem:
			self.scene().removeItem(self)


 
