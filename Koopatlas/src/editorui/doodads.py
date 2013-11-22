from common import *
from editorcommon import *
import weakref
from math import floor, ceil
import math


class KPEditorDoodad(KPEditorItem):
	SNAP_TO = (12,12)


	class DoodadAnmButton(QtGui.QPushButton):


		class AnmOptionsWidget(QtGui.QWidget):


			class AnmDelegate(QtGui.QStyledItemDelegate):

				def createEditor(self, parent, option, index):

					loop = ["Contiguous", "Loop", "Reversible Loop"]
					interp = ["Linear", "Sinusoidial", "Cosinoidial"]
					anmType = ["X Position", "Y Position", "Angle", "X Scale", "Y Scale", "Opacity"]

					thing = index.data(Qt.DisplayRole)
					thong = index.data(Qt.EditRole).toFloat()[0]

					if thing in loop:
						editWidget = QtGui.QComboBox(parent)
						editWidget.addItems(loop)

						return editWidget

					elif thing in interp:
						editWidget = QtGui.QComboBox(parent)
						editWidget.addItems(interp)

						return editWidget

					elif thing in anmType:
						editWidget = QtGui.QComboBox(parent)
						editWidget.addItems(anmType)

						return editWidget

					elif isinstance(thong, float):
						editWidget = QtGui.QDoubleSpinBox(parent)
						editWidget.setSingleStep(0.05)
						editWidget.setDecimals(2)
						editWidget.setRange(-99999.0, 99999.0)
						return editWidget

					else:
						print "Thing was something else!"
						print thong
						print type(thong)

				
				def setEditorData(self, editor, index):
					

					if isinstance(editor, QtGui.QDoubleSpinBox):
						thing = index.data(Qt.EditRole).toFloat()[0]

						editor.setValue(thing)

					elif isinstance(editor, QtGui.QComboBox):
						thing = index.data(Qt.DisplayRole).toString()

						editor.setCurrentIndex(editor.findText(thing))

					else:
						print "editor is something else!"
						print editor
					

				def setModelData(self, editor, model, index):
					
					if isinstance(editor, QtGui.QDoubleSpinBox):
						editor.interpretText()
						value = editor.value()

						model.setData(index, value, QtCore.Qt.EditRole)

					elif isinstance(editor, QtGui.QComboBox):
						value = editor.currentText()

						model.setData(index, value, QtCore.Qt.EditRole)

					else:
						print "editor is something else!"
						print editor


				def updateEditorGeometry(self, editor, option, index):
					editor.setGeometry(option.rect)


			def __init__(self, doodadRef, parent):
				QtGui.QWidget.__init__(self)

				self._doodadRef = doodadRef
				self.parent = parent

				# Setup Layout
				BottomLayout = QtGui.QGridLayout()


				# Time for the Table View, model and Delegate
				self.model = QtGui.QStandardItemModel(0, 6)
				self.anmTable = QtGui.QTableView()
				self.anmTable.setModel(self.model)

				delegate = self.AnmDelegate()
				self.anmTable.setItemDelegate(delegate)

				self.model.setHorizontalHeaderLabels(["Looping", "Interpolation", "Frame Len", "Type", "Start Value", "End Value"])
				self.anmTable.setColumnWidth(0, 150)
				self.anmTable.setColumnWidth(1, 100)
				self.anmTable.setColumnWidth(2, 65)
				self.anmTable.setColumnWidth(3, 120)
				self.anmTable.setColumnWidth(4, 65)
				self.anmTable.setColumnWidth(5, 65)

				self.anmTable.horizontalHeader().setVisible(True)
				self.anmTable.verticalHeader().setVisible(False)

				BottomLayout.addWidget(self.anmTable, 0, 0, 1, 6)


				# Add/Remove Animation Buttons
				addbutton = QtGui.QPushButton(QtGui.QIcon("Resources/Plus.png"), "")
				rembutton = QtGui.QPushButton(QtGui.QIcon("Resources/Minus.png"), "")
				presetbutton = QtGui.QPushButton(QtGui.QIcon("Resources/AddPreset.png"), "Add Preset")
				newpbutton = QtGui.QPushButton(QtGui.QIcon("Resources/NewPreset.png"), "New Preset")
				# savebutton = QtGui.QPushButton(QtGui.QIcon("Resources/SavePreset.png"), "Save")
				# loadbutton = QtGui.QPushButton(QtGui.QIcon("Resources/LoadPreset.png"), "Load")
				# clearbutton = QtGui.QPushButton(QtGui.QIcon("Resources/ClearPreset.png"), "Clear")
				BottomLayout.addWidget(addbutton, 1, 0, 1, 1)
				BottomLayout.addWidget(rembutton, 1, 1, 1, 1)
				BottomLayout.addWidget(QtGui.QLabel(""), 1, 2, 1, 2)
				BottomLayout.addWidget(presetbutton, 1, 4, 1, 1)
				BottomLayout.addWidget(newpbutton, 1, 5, 1, 1)
				# BottomLayout.addWidget(savebutton, 1, 6, 1, 1)
				# BottomLayout.addWidget(loadbutton, 1, 7, 1, 1)
				# BottomLayout.addWidget(clearbutton, 1, 8, 1, 1)


				# Annnnndddd we're spent.
				self.setLayout(BottomLayout)

				addbutton.released.connect(self.addAnmItem)
				rembutton.released.connect(self.remAnmItem)

				presetbutton.released.connect(self.selectPreset)
				newpbutton.released.connect(self.addToPreset)
				# savebutton.released.connect(KP.mainWindow.saveAnimPresets)
				# loadbutton.released.connect(KP.mainWindow.loadAnimPresets)
				# clearbutton.released.connect(KP.mainWindow.clearAnimPresets)

				self.setupAnms()


			def setupAnms(self):
				doodad = self._doodadRef()
				
				for row in doodad.animations:

					itemA = QtGui.QStandardItem()
					itemB = QtGui.QStandardItem()
					itemC = QtGui.QStandardItem()

					itemA.setData(row[2], QtCore.Qt.EditRole)
					itemB.setData(row[4], QtCore.Qt.EditRole)
					itemC.setData(row[5], QtCore.Qt.EditRole)

					self.model.appendRow([QtGui.QStandardItem(row[0]), QtGui.QStandardItem(row[1]), 
									  	  itemA, QtGui.QStandardItem(row[3]), itemB, itemC])

				self.update()


			def sizeHint(self):
				return QtCore.QSize(591,300)


			def addAnmItem(self):

				itemA = QtGui.QStandardItem()
				itemB = QtGui.QStandardItem()
				itemC = QtGui.QStandardItem()

				itemA.setData(1, QtCore.Qt.EditRole)
				itemB.setData(0.0, QtCore.Qt.EditRole)
				itemC.setData(0.0, QtCore.Qt.EditRole)

				self.model.appendRow([QtGui.QStandardItem("Contiguous"), QtGui.QStandardItem("Linear"), 
									  itemA, QtGui.QStandardItem("X Position"), 
									  itemB, itemC])


			def remAnmItem(self):

				if self.model.rowCount() == 0:
					return

				rowNum, ok = QtGui.QInputDialog.getInteger(self,
						"Select A Row", "Delete This Row:", 0, 0, self.model.rowCount(), 1)
				if ok:
					self.model.removeRows(rowNum, 1)


			def selectPreset(self):
				from dialogs import KPAnimationPresetChooser
				presetList = KPAnimationPresetChooser.run()
				
				if presetList is None: return
				if presetList is []: return
				if presetList is [[]]: return

				q = QtGui.QStandardItem

				for row in presetList:
					a = q(row[0])
					b = q(row[1])
					c = q()
					c.setData(row[2], Qt.EditRole)
					d = q(row[3])
					e = q()
					e.setData(row[4], Qt.EditRole)
					f = q()
					f.setData(row[5], Qt.EditRole)

					self.model.appendRow([a,b,c,d,e,f])

				self.parent.resolveAnmList()

			def addToPreset(self):
				from dialogs import getTextDialog

				name = getTextDialog('Choose Preset Name', 'Preset name:')
				if name == None: 
					print 'Returning'
					return

				print 'Adding.'
				preset = []
				for row in xrange(self.model.rowCount()):
					listrow = []
					for column in xrange(6):
						item = self.model.item(row, column)
						if (column == 0) or (column == 1) or (column == 2):
							data = str(item.data(Qt.EditRole).toString())
						else:
							data = item.data(Qt.EditRole).toFloat()[0]
							
						listrow.append(data)

					preset.append(listrow)

				settings = KP.app.settings
				import mapfile

				if settings.contains('AnimationPresets'):

					presetList = mapfile.load(str(settings.value('AnimationPresets').toPyObject()))
					presets = mapfile.load(str(settings.value('AnimationPresetData').toPyObject()))

				else:

					presetList = []
					presets = []

				presetList.append(name)
				presets.append(preset)

				settings.setValue('AnimationPresets', mapfile.dump(presetList))
				settings.setValue('AnimationPresetData', mapfile.dump(presets))


		def __init__(self, doodadRef):
			QtGui.QPushButton.__init__(self)

			self._doodadRef = doodadRef
			self.setText("Animate")

			self.menu = QtGui.QMenu(self)
			self.menuWidget = self.AnmOptionsWidget(doodadRef, self)
			self.menuWidgetAction = QtGui.QWidgetAction(self)
			self.menuWidgetAction.setDefaultWidget(self.menuWidget)
			self.menu.addAction(self.menuWidgetAction)


			menuPalette = self.menu.palette()
			menuPalette.setColor(QtGui.QPalette.Window, Qt.black)
			self.menu.setPalette(menuPalette)

			self.setMenu(self.menu)

			palette = self.palette()
			palette.setColor(QtGui.QPalette.ButtonText, Qt.black)
			palette.setColor(QtGui.QPalette.Window, Qt.transparent)

			self.setPalette(palette)

			self.menu.aboutToHide.connect(self.resolveAnmList)


		def resolveAnmList(self):
			doodad = self._doodadRef()
			
			anmList = []
			model = self.menuWidget.model
			rows = model.rowCount()

			for x in xrange(rows):
				rowList = []

				for item in xrange(6):
					index = model.index(x, item)
					data = model.data(index, Qt.EditRole).toString()
					if data.toFloat()[1]:
						data = data.toFloat()[0]
					else:
						data = str(data)

					rowList.append(data)

				anmList.append(rowList)

			doodad.animations = anmList
			doodad.setupAnimations()

			self.update()


	class HiddenProxy(QtGui.QGraphicsProxyWidget):
		def __init__(self, button, parent, x, y):
			QtGui.QGraphicsProxyWidget.__init__(self, parent)

			self.setWidget(button)
			self.setPos(x, y)
			self.setZValue(parent.zValue()+1000)
			self.hide()


	def __init__(self, doodad, layer):
		KPEditorItem.__init__(self)

		doodad.qtItem = self
		self._doodadRef = weakref.ref(doodad)
		self._layerRef = weakref.ref(layer)

		self.resizing = None
		self.rotating = None

		self.source = doodad.source

		self._updatePixmap()
		self._updatePosition()
		self._updateSize()
		self._updateTransform()

		# self.anmButton = self.DoodadAnmButton(self._doodadRef)
		# self.anmProxy = self.HiddenProxy(self.anmButton, self, self.boundingRect().right() - 101, self.boundingRect().bottom() - 25)

		self.setAcceptHoverEvents(True)

		if len(doodad.animations) > 0:
			doodad.setupAnimations()

		if not hasattr(KPEditorDoodad, 'SELECTION_PEN'):
			KPEditorDoodad.SELECTION_PEN = QtGui.QPen(Qt.red, 1, Qt.DotLine)


	def _updatePixmap(self):
		pixmap = self.source[1]

		self.prepareGeometryChange()
		w, h = pixmap.width(), pixmap.height()

		self.pixmap = pixmap


	def _updatePosition(self):
		# NOTE: EditorDoodads originate at the centre, not the top left like the others
		self.ignoreMovement = True

		doodad = self._doodadRef()
		x,y = doodad.position
		w,h = doodad.size
		self.setPos(x+floor(w/2.0), y+floor(h/2.0))

		self.ignoreMovement = False


	def _updateSize(self):
		self.prepareGeometryChange()

		w,h = self._doodadRef().size

		self._boundingRect = QtCore.QRectF(-w/2, -h/2, w, h)
		self._selectionRect = self._boundingRect.adjusted(0, 0, -1, -1)


	def _updateTransform(self):
		doodad = self._doodadRef()

		self.setRotation(doodad.angle)


	def paint(self, painter, option, widget):
		if self.isSelected():
			painter.setPen(self.SELECTION_PEN)
			painter.drawRect(self._selectionRect)
	

	def _itemMoved(self, oldX, oldY, newX, newY):
		doodad = self._doodadRef()
		w,h = doodad.size
		doodad.position = [newX-floor(w/2.0), newY-floor(h/2.0)]


	def hoverMoveEvent(self, event):
		if self._layerRef() != KP.mapScene.currentLayer:
			self.setCursor(Qt.ArrowCursor)
			return

		pos = event.pos()
		bit = self.resizerPortionAt(pos.x(), pos.y())


		if (event.modifiers() == Qt.ShiftModifier):

			if bit:
				self.setCursor(Qt.OpenHandCursor)
			else:
				self.setCursor(Qt.ArrowCursor)

		else:

			if bit == 1 or bit == 4:
				self.setCursor(Qt.SizeFDiagCursor)
			elif bit == 2 or bit == 3:
				self.setCursor(Qt.SizeBDiagCursor)
			elif bit == 7 or bit == 8:
				self.setCursor(Qt.SizeHorCursor)
			elif bit == 5 or bit == 6:
				self.setCursor(Qt.SizeVerCursor)
			else:
				self.setCursor(Qt.ArrowCursor)
	

	def mousePressEvent(self, event):
		if event.button() == Qt.LeftButton:
			pos = event.pos()
			bit = self.resizerPortionAt(pos.x(), pos.y())

			if self._layerRef() == KP.mapScene.currentLayer and bit:
			# if bit:
				event.accept()

				if (event.modifiers() & Qt.ShiftModifier):
					self.rotating = self.mapToScene(pos), self._doodadRef().angle
					self.setCursor(Qt.ClosedHandCursor)
					return
				
				else:
					x, xSide, y, ySide = False, None, False, None
			
					if bit == 1 or bit == 7 or bit == 3: # left
						x, xSide = True, 1

					elif bit == 2 or bit == 4 or bit == 8: # right
						x, xSide = True, 0

					if bit == 1 or bit == 2 or bit == 5: # top
						y, ySide = True, 1

					elif bit == 3 or bit == 4 or bit == 6: # bottom
						y, ySide = True, 0

					self._updateSize()
					self.resizing = (x, xSide, y, ySide)
					return


		KPEditorItem.mousePressEvent(self, event)
	

	def _tryAndResize(self, obj, axisIndex, mousePosition, stationarySide):

		newSize = abs(mousePosition) * 2

		if newSize < 10:
			return False
	
		obj.size[axisIndex] = newSize
		
		return True


	def _tryAndRotate(self, obj, mouseX, mouseY, originalPos, oldAngle, modifiers):
		center = self.mapToScene(self.boundingRect().center())

		objX = center.x()
		objY = center.y()


		origX = originalPos.x()
		origY = originalPos.y()

		dy = origY - objY
		dx = origX - objX
		rads = math.atan2(dy, dx)

		origAngle = math.degrees(rads)

		dy = mouseY - objY
		dx = mouseX - objX
		rads = math.atan2(dy, dx)

		angle = math.degrees(rads)

		
		# Move this to ItemChange() or something at some point.
		finalAngle = angle - origAngle + oldAngle

		if (modifiers & Qt.ControlModifier):
			finalAngle = int(finalAngle / 45.0) * 45.0

		return True, finalAngle


	def mouseMoveEvent(self, event):
		if self.resizing:
			obj = self._doodadRef()

			hasChanged = False
			resizeX, xSide, resizeY, ySide = self.resizing

			if resizeX:
				hasChanged |= self._tryAndResize(obj, 0, event.pos().x(), xSide)
			if resizeY:
				hasChanged |= self._tryAndResize(obj, 1, event.pos().y(), ySide)

			if hasChanged:
				# Doodads aren't supposed to snap, they're all free flowing like the wind.
				self._updateSize()


		elif self.rotating:
			obj = self._doodadRef()
			scenePos = event.scenePos()
			self.setTransformOriginPoint(self.boundingRect().center())

			hasChanged = False

			hasChanged, angle = self._tryAndRotate(obj, scenePos.x(), scenePos.y(), self.rotating[0], self.rotating[1], event.modifiers())

			if hasChanged:
				obj.angle = angle
				self._updateTransform()


		else:
			KPEditorItem.mouseMoveEvent(self, event)
	

	def mouseReleaseEvent(self, event):
		if self.resizing and event.button() == Qt.LeftButton:
			self.resizing = None
			# self._doodadRef().position = [self.x(), self.y()]

		elif self.rotating and event.button() == Qt.LeftButton:
			self.rotating = None

		else:
			KPEditorItem.mouseReleaseEvent(self, event)


	def remove(self, withItem=False):
		doodad = self._doodadRef()
		layer = self._layerRef()

		if isinstance(layer, KPPathTileLayer):
			layer.doodads.remove(doodad)
		else:
			layer.objects.remove(doodad)
		doodad.cleanUpAnimations()

		if withItem:
			self.scene().removeItem(self)



