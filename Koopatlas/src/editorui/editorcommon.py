from common import *

class KPEditorItem(QtGui.QGraphicsItem):
	def __init__(self):
		QtGui.QGraphicsItem.__init__(self)
		self.setFlags(
				self.ItemSendsGeometryChanges |
				self.ItemIsSelectable |
				self.ItemIsMovable
				)

		self.ignoreMovement = False
		self.overrideSnap = False
	
	def itemChange(self, change, value):
		if change == self.ItemPositionChange and not self.ignoreMovement:
			currentX, currentY = self.x(), self.y()

			newpos = value.toPyObject()

			x, y = newpos.x(), newpos.y()

			if self.overrideSnap:
				snapX, snapY = 1, 1
			else:
				# snap the item
				snapX, snapY = self.SNAP_TO
				x = int((x + (snapX/2)) / snapX) * snapX
				y = int((y + (snapY/2)) / snapY) * snapY

			if x < 0: x = 0
			if x >= (12288+snapX): x = (12288+snapX-1)
			if y < 0: y = 0
			if y >= (12288+snapY): y = (12288+snapY-1)

			if x != currentX or y != currentY:
				self._itemMoved(currentX, currentY, x, y)

			newpos.setX(x)
			newpos.setY(y)
			return newpos

		return QtGui.QGraphicsItem.itemChange(self, change, value)

	def boundingRect(self):
		return self._boundingRect


	def resizerPortionAt(self, x, y, originX=0, originY=0):
		try:
			leftBound, topBound = originX+5, originY+5
			rightBound, bottomBound = self._resizerEndXY
		except AttributeError:
			rect = self._boundingRect
			leftBound, topBound = rect.x() + 5, rect.y() + 5
			rightBound, bottomBound = rect.right() - 5, rect.bottom() - 5

		if y < topBound:
			if x < leftBound:		return 1 # TOP_LEFT
			elif x >= rightBound:	return 2 # TOP_RIGHT
			else:					return 5 # TOP

		elif y >= bottomBound:
			if x < leftBound:		return 3 # BOTTOM_LEFT
			elif x >= rightBound:	return 4 # BOTTOM_RIGHT
			else:					return 6 # BOTTOM

		else:
			if x < leftBound:		return 7 # LEFT
			elif x >= rightBound:	return 8 # RIGHT
			else:					return None


	def _itemMoved(self, oldX, oldY, newX, newY):
		pass


