from common import *
from editorcommon import *
import weakref

class KPEditorObject(KPEditorItem):
	SNAP_TO = (24,24)

	def __init__(self, obj, layer):
		KPEditorItem.__init__(self)
		obj.qtItem = self
		self._objRef = weakref.ref(obj)
		self._layerRef = weakref.ref(layer)
		self._updatePosition()
		self._updateSize()

		self.setAcceptHoverEvents(True)

		self.resizing = None

		if not hasattr(KPEditorObject, 'SELECTION_PEN'):
			KPEditorObject.SELECTION_PEN = QtGui.QPen(Qt.green, 1, Qt.DotLine)

		# I don't bother setting the ZValue because it doesn't quite matter:
		# only one layer's objects are ever clickable, and drawBackground takes
		# care of the layered drawing
	
	def _updatePosition(self):
		self.ignoreMovement = True

		x,y = self._objRef().position
		self.setPos(x*24, y*24)

		self.ignoreMovement = False
	
	def _updateSize(self):
		self.prepareGeometryChange()

		obj = self._objRef()
		w,h = obj.size

		self._boundingRect = QtCore.QRectF(0, 0, w*24, h*24)
		self._selectionRect = QtCore.QRectF(0, 0, w*24-1, h*24-1)

		self._resizerEndXY = (w*24-5, h*24-5)
	

	def paint(self, painter, option, widget):
		if self.isSelected():
			painter.setPen(self.SELECTION_PEN)
			painter.drawRect(self._selectionRect)
	

	def hoverMoveEvent(self, event):
		if self._layerRef() != KP.mapScene.currentLayer:
			self.setCursor(Qt.ArrowCursor)
			return

		pos = event.pos()
		bit = self.resizerPortionAt(pos.x(), pos.y())

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

				x, xSide, y, ySide = False, None, False, None
				
				if bit == 1 or bit == 7 or bit == 3:
					x, xSide = True, 1
				elif bit == 2 or bit == 4 or bit == 8:
					x, xSide = True, 0

				if bit == 1 or bit == 2 or bit == 5:
					y, ySide = True, 1
				elif bit == 3 or bit == 4 or bit == 6:
					y, ySide = True, 0

				self.resizing = (x, xSide, y, ySide)
				return

		KPEditorItem.mousePressEvent(self, event)
	

	def _tryAndResize(self, obj, axisIndex, mousePosition, stationarySide):
		objPosition = obj.position[axisIndex]
		objSize = obj.size[axisIndex]

		if stationarySide == 0:
			# Resize the right/bottom side
			relativeMousePosition = mousePosition - objPosition
			newSize = relativeMousePosition + 1
			if newSize == objSize or newSize < 1:
				return False

			if axisIndex == 1:
				obj.size = (obj.size[0], newSize)
			else:
				obj.size = (newSize, obj.size[1])

		else:
			# Resize the left/top side
			rightSide = objPosition + objSize - 1
			newLeftSide = mousePosition

			newPosition = newLeftSide
			newSize = rightSide - newLeftSide + 1

			if newSize < 1:
				return False
			if newPosition == objPosition and newSize == objSize:
				return False

			if axisIndex == 1:
				obj.position = (obj.position[0], newPosition)
				obj.size = (obj.size[0], newSize)
			else:
				obj.position = (newPosition, obj.position[1])
				obj.size = (newSize, obj.size[1])

		return True


	def mouseMoveEvent(self, event):
		if self.resizing:
			obj = self._objRef()
			scenePos = event.scenePos()

			hasChanged = False
			resizeX, xSide, resizeY, ySide = self.resizing

			if resizeX:
				hasChanged |= self._tryAndResize(obj, 0, int(scenePos.x() / 24), xSide)
			if resizeY:
				hasChanged |= self._tryAndResize(obj, 1, int(scenePos.y() / 24), ySide)

			if hasChanged:
				obj.updateCache()
				self._layerRef().updateCache()
				self._updatePosition()
				self._updateSize()

		else:
			KPEditorItem.mouseMoveEvent(self, event)
	

	def mouseReleaseEvent(self, event):
		if self.resizing and event.button() == Qt.LeftButton:
			self.resizing = None
		else:
			KPEditorItem.mouseReleaseEvent(self, event)
	

	def _itemMoved(self, oldX, oldY, newX, newY):
		obj = self._objRef()
		obj.position = (newX/24, newY/24)
		self._layerRef().updateCache()


	def remove(self, withItem=False):
		obj = self._objRef()
		layer = self._layerRef()

		layer.objects.remove(obj)
		layer.updateCache()

		if withItem:
			self.scene().removeItem(self)


