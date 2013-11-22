from common import *
from math import floor, ceil

from objects import *
from doodads import *
from paths import *

class KPMapScene(QtGui.QGraphicsScene):
	def __init__(self):
		QtGui.QGraphicsScene.__init__(self, 0, 0, 512*24, 512*24)
		
		# todo: handle selectionChanged
		# todo: look up why I used setItemIndexMethod(self.NoIndex) in Reggie

		self.currentLayer = None
		KP.mapScene = self
		self.playing = False
		self.timeLines = []
		self.ticker = QtCore.QTimeLine(100000)
		self.ticker.setLoopCount(0)
		self.ticker.setCurveShape(4)
		self.ticker.setFrameRange(0,100000)
		self.ticker.valueChanged.connect(self.viewportUpdateProxy)
		self.ticker.setUpdateInterval(16.6666666666667)

		self.grid = False

		# create an item for everything in the map
		for layer in KP.map.layers:
			if isinstance(layer, KPTileLayer):
				for obj in layer.objects:
					self.addItem(KPEditorObject(obj, layer))
			elif isinstance(layer, KPDoodadLayer):
				for obj in layer.objects:
					self.addItem(KPEditorDoodad(obj, layer))
			elif isinstance(layer, KPPathLayer):
				for inLayer in KP.map.associateLayers:
					for obj in inLayer.objects:
						self.addItem(KPEditorObject(obj, inLayer))
					for obj in inLayer.doodads:
						self.addItem(KPEditorDoodad(obj, inLayer))

				for node in layer.nodes:
					self.addItem(KPEditorNode(node))

				for path in layer.paths:
					self.addItem(KPEditorPath(path))

			layer.setActivated(False)


	def playPause(self):
		if self.playing == False:
			self.playing = True
			self.views()[0].setViewportUpdateMode(0)
			self.ticker.start()

			for timeline in self.timeLines:
				timeline.start()
			self.views()[0].viewport().update()
		
		else:
			self.playing = False
			self.views()[0].setViewportUpdateMode(1)
			self.ticker.stop()

			for timeline in self.timeLines:
				timeline.stop()
			self.views()[0].viewport().update()


	@QtCore.pyqtSlot()
	def viewportUpdateProxy(self):
		self.views()[0].viewport().update()


	def drawForeground(self, painter, rect):
		if not self.grid: return
		
		Zoom = KP.mainWindow.ZoomLevel
		drawLine = painter.drawLine
		
		if Zoom >= 4:
			startx = rect.x()
			startx -= (startx % 24)
			endx = startx + rect.width() + 24
			
			starty = rect.y()
			starty -= (starty % 24)
			endy = starty + rect.height() + 24
			
			painter.setPen(QtGui.QPen(QtGui.QColor.fromRgb(255,255,255,100), 1, QtCore.Qt.DotLine))
			
			x = startx
			y1 = rect.top()
			y2 = rect.bottom()
			while x <= endx:
				drawLine(x, starty, x, endy)
				x += 24
			
			y = starty
			x1 = rect.left()
			x2 = rect.right()
			while y <= endy:
				drawLine(startx, y, endx, y)
				y += 24
		
		
		if Zoom >= 2:
			startx = rect.x()
			startx -= (startx % 96)
			endx = startx + rect.width() + 96
			
			starty = rect.y()
			starty -= (starty % 96)
			endy = starty + rect.height() + 96
			
			painter.setPen(QtGui.QPen(QtGui.QColor.fromRgb(255,255,255,100), 1, QtCore.Qt.DashLine))
			
			x = startx
			y1 = rect.top()
			y2 = rect.bottom()
			while x <= endx:
				drawLine(x, starty, x, endy)
				x += 96
			
			y = starty
			x1 = rect.left()
			x2 = rect.right()
			while y <= endy:
				drawLine(startx, y, endx, y)
				y += 96
		
		
		startx = rect.x()
		startx -= (startx % 192)
		endx = startx + rect.width() + 192
		
		starty = rect.y()
		starty -= (starty % 192)
		endy = starty + rect.height() + 192
		
		painter.setPen(QtGui.QPen(QtGui.QColor.fromRgb(255,255,255,100), 2, QtCore.Qt.DashLine))
		
		x = startx
		y1 = rect.top()
		y2 = rect.bottom()
		while x <= endx:
			drawLine(x, starty, x, endy)
			x += 192
		
		y = starty
		x1 = rect.left()
		x2 = rect.right()
		while y <= endy:
			drawLine(startx, y, endx, y)
			y += 192


	def drawBackground(self, painter, rect):
		painter.fillRect(rect, QtGui.QColor(209, 218, 236))

		areaLeft, areaTop = rect.x(), rect.y()
		areaWidth, areaHeight = rect.width(), rect.height()
		areaRight, areaBottom = areaLeft+areaWidth, areaTop+areaHeight

		areaLeftT = floor(areaLeft / 24)
		areaTopT = floor(areaTop / 24)
		areaRightT = ceil(areaRight / 24)
		areaBottomT = ceil(areaBottom / 24)

		# compile a list of doodads
		visibleDoodadsByLayer = {}

		for obj in self.items(rect):
			if not isinstance(obj, KPEditorDoodad): continue

			layer = obj._layerRef()

			try:
				doodadList = visibleDoodadsByLayer[layer]
			except KeyError:
				doodadList = []
				visibleDoodadsByLayer[layer] = doodadList

			doodadList.append(obj)

		# now draw everything!
		for layer in reversed(KP.map.layers):
			if not layer.visible: continue

			if isinstance(layer, KPDoodadLayer):
				try:
					toDraw = visibleDoodadsByLayer[layer]
				except KeyError:
					continue

				for item in reversed(toDraw):
					
					painter.save()

					if self.playing == False:
						painter.setWorldTransform(item.sceneTransform(), True)
						p = item._boundingRect
						painter.drawPixmap(p.x(), p.y(), p.width(), p.height(), item.pixmap)

					else:
						self.animateDoodad(painter, item)
					painter.restore()


			elif isinstance(layer, KPTileLayer):
				left, top = layer.cacheBasePos
				width, height = layer.cacheSize
				right, bottom = left+width, top+height

				if width == 0 and height == 0: continue

				if right < areaLeftT: continue
				if left > areaRightT: continue

				if bottom < areaTopT: continue
				if top > areaBottomT: continue

				# decide how much of the layer we'll actually draw
				drawLeft = int(max(areaLeftT, left))
				drawRight = int(min(areaRightT, right))

				drawTop = int(max(areaTopT, top))
				drawBottom = int(min(areaBottomT, bottom))

				srcY = drawTop - top
				destY = drawTop * 24

				baseSrcX = drawLeft - left
				baseDestX = drawLeft * 24

				rows = layer.cache
				tileset = KP.tileset(layer.tileset)
				tileList = tileset.tiles

				for y in xrange(drawTop, drawBottom):
					srcX = baseSrcX
					destX = baseDestX
					row = rows[srcY]

					for x in xrange(drawLeft, drawRight):
						tile = row[srcX]
						if tile != -1:
							painter.drawPixmap(destX, destY, tileList[tile])
						
						srcX += 1
						destX += 24

					srcY += 1
					destY += 24
	
			
			elif isinstance(layer, KPPathLayer):
				for pnLayer in reversed(KP.mainWindow.pathNodeList.getLayers()):
					if not pnLayer.visible: continue

					# Render Tiles
					left, top = pnLayer.cacheBasePos
					width, height = pnLayer.cacheSize
					right, bottom = left+width, top+height

					if not (width == 0) or (height == 0) or (right < areaLeftT) or (left > areaRightT) or (bottom < areaTopT) or (top > areaBottomT):

						drawLeft = int(max(areaLeftT, left))
						drawRight = int(min(areaRightT, right))

						drawTop = int(max(areaTopT, top))
						drawBottom = int(min(areaBottomT, bottom))

						srcY = drawTop - top
						destY = drawTop * 24

						baseSrcX = drawLeft - left
						baseDestX = drawLeft * 24

						rows = pnLayer.cache
						tileset = KP.tileset(pnLayer.tileset)
						tileList = tileset.tiles

						for y in xrange(drawTop, drawBottom):
							srcX = baseSrcX
							destX = baseDestX
							row = rows[srcY]

							for x in xrange(drawLeft, drawRight):
								tile = row[srcX]
								if tile != -1:
									painter.drawPixmap(destX, destY, tileList[tile])
								
								srcX += 1
								destX += 24

							srcY += 1
							destY += 24

					# Render Doodads
					try:
						toDraw = visibleDoodadsByLayer[pnLayer]
					except KeyError:
						continue

					for item in reversed(toDraw):

						painter.save()

						if self.playing == False:
							painter.setWorldTransform(item.sceneTransform(), True)
							p = item._boundingRect
							painter.drawPixmap(p.x(), p.y(), p.width(), p.height(), item.pixmap)

						else:
							self.animateDoodad(painter, item)
						painter.restore()


	def animateDoodad(self, painter, item):

		doodad = item._doodadRef()
		animations = doodad.animations

		transform = item.sceneTransform()
		posRect = item._boundingRect.adjusted(0,0,0,0)

		# Anm indexes are Looping, Interpolation, Frame Len, Type, Start Value, End Value
		#
		# Anm Loops are Contiguous, Loop, Reversible Loop
		# Anm Interpolations are Linear, Sinusoidial, Cosinoidial
		# Anm Types are X Position, Y Position, Angle, X Scale, Y Scale, Opacity

		if len(animations) > 0:
			for anm, Timeline in zip(animations, doodad.timelines):

				Type = anm[3]
			
				modifier = Timeline.currentFrame()

				if Type == "X Position":
					posRect.adjust(modifier, 0, modifier, 0)

				elif Type == "Y Position":
					posRect.adjust(0, modifier, 0, modifier)

				elif Type == "Angle":
					transform.rotate(modifier)

				elif Type == "X Scale":
					posRect.setWidth(posRect.width()*modifier/100.0)
			
				elif Type == "Y Scale":
					h = posRect.height()
					posRect.setHeight(h*modifier/100.0)

					new = h - posRect.height()
					posRect.adjust(0, new, 0, new)

				elif Type == "Opacity":
					painter.setOpacity(modifier/100.0)

		painter.setWorldTransform(transform, True)
		painter.drawPixmap(posRect.x(), posRect.y(), posRect.width(), posRect.height(), item.pixmap)


	def setCurrentLayer(self, layer):
		if self.currentLayer is not None:
			self.currentLayer.setActivated(False)

		self.currentLayer = layer
		self.currentLayer.setActivated(True)


class KPEditorWidget(QtGui.QGraphicsView):
	def __init__(self, scene, parent=None):
		QtGui.QGraphicsView.__init__(self, scene, parent)

		self.setRenderHints(QtGui.QPainter.Antialiasing)
		self.setViewportUpdateMode(self.FullViewportUpdate)
		self.grid = False

		self.setAlignment(Qt.AlignLeft | Qt.AlignTop)
		self.setDragMode(self.RubberBandDrag)
		
		self.xScrollBar = QtGui.QScrollBar(Qt.Horizontal, parent)
		self.setHorizontalScrollBar(self.xScrollBar)
		
		self.yScrollBar = QtGui.QScrollBar(Qt.Vertical, parent)
		self.setVerticalScrollBar(self.yScrollBar)

		self.assignNewScene(scene)

	def drawForeground(self, painter, rect):

		QtGui.QGraphicsView.drawForeground(self, painter, rect)

		if self.grid:
			painter.setPen(Qt.red)
			painter.setBrush(Qt.transparent)

			c = rect.center()
			x = c.x()
			y = c.y()

			mx = 880.0
			my = 660.0

			m2x = 1180.0
			m2y = 660.0

			# mx = 1792.0
			# my = 1344.0
			newRect = QtCore.QRectF(x-(mx/2.0), y-(my/2.0), mx, my)
			painter.drawRect(newRect)

			newRect2 = QtCore.QRectF(x-(m2x/2.0), y-(m2y/2.0), m2x, m2y)
			painter.drawRect(newRect2)

			painter.drawLine(x, y-36, x, y+36)
			painter.drawLine(x-36, y, x+36, y)

			newRect3 = QtCore.QRectF(x-24, y-24, 48, 48)
			painter.drawRect(newRect3)


	def assignNewScene(self, scene):
		self.setScene(scene)
		self.centerOn(0,0)

		# set up stuff for painting
		self.objectToPaint = None
		self.objectIDToPaint = None
		self.doodadToPaint = None
		self.typeToPaint = None

		self._resetPaintVars()
	
	def _resetPaintVars(self):
		self.painting = None
		self.paintingItem = None
		self.paintBeginPosition = None
	
	def _tryToPaint(self, event):
		'''Called when a paint attempt is initiated'''

		layer = self.scene().currentLayer
		if not layer.visible: return

		if isinstance(layer, KPTileLayer):
			paint = self.objectToPaint
			if paint is None: return

			clicked = self.mapToScene(event.x(), event.y())
			x, y = clicked.x(), clicked.y()
			if x < 0: x = 0
			if y < 0: y = 0

			x = int(x / 24)
			y = int(y / 24)

			obj = KPObject()
			obj.position = (x,y)
			obj.size = (1,1)
			obj.tileset = layer.tileset
			obj.kind = self.objectIDToPaint
			obj.kindObj = paint
			obj.updateCache()
			layer.objects.append(obj)
			layer.updateCache()

			item = KPEditorObject(obj, layer)
			self.scene().addItem(item)

			self.painting = obj
			self.paintingItem = item
			self.paintBeginPosition = (x, y)

		elif isinstance(layer, KPDoodadLayer):
			paint = self.doodadToPaint
			if paint is None: return

			clicked = self.mapToScene(event.x(), event.y())
			x, y = clicked.x(), clicked.y()
			if x < 0: x = 0
			if y < 0: y = 0

			obj = KPDoodad()
			obj.position = [x,y]
			obj.source = paint
			obj.setDefaultSize()
			layer.objects.append(obj)

			item = KPEditorDoodad(obj, layer)
			self.scene().addItem(item)

			self.painting = obj
			self.paintingItem = item
			self.paintBeginPosition = (x, y)

		elif isinstance(layer, KPPathLayer):
			# decide what's under the mouse
			clicked = self.mapToScene(event.x(), event.y())
			x, y = clicked.x(), clicked.y()
			itemsUnder = self.scene().items(clicked)

			if event.modifiers() & Qt.AltModifier:
				dialog = True
			else:
				dialog = False

			for item in itemsUnder:
				if isinstance(item, KPEditorNode):
					# Paint a path to this node (if one is selected)
					sourceItem, sourceNode = None, None
					selected = self.scene().selectedItems()

					for selItem in selected:
						if isinstance(item, KPEditorNode) and selItem != item:
							sourceItem = selItem
							sourceNode = selItem._nodeRef()
							break

					if sourceItem is None: return

					# Make sure that no path already exists between these nodes
					destNode = item._nodeRef()

					for pathToCheck in sourceNode.exits:
						if pathToCheck._startNodeRef() == destNode:
							return
						if pathToCheck._endNodeRef() == destNode:
							return

					# No node can have more than four paths, because there are only
					# four directions registered by a Wiimote DPad.

					if len(sourceNode.exits) > 3:
						return

					if len(destNode.exits) > 3:
						return

					path = KPPath(sourceNode, destNode)

					KP.map.pathLayer.paths.append(path)
					KP.mainWindow.pathNodeList.addLayer(path, dialog)

					item = KPEditorPath(path)
					self.scene().addItem(item)

					return

				elif isinstance(item, KPEditorPath):
					# Split this path into two... at this point

					origPath = item._pathRef()

					node = KPNode()
					node.position = (x - 12, y - 12)
					KP.map.pathLayer.nodes.append(node)
					KP.mainWindow.pathNodeList.addLayer(node, dialog)

					# Start node => Original path => New node => New path => End node

					endNode = origPath._endNodeRef()

					origPath.setEnd(node)
					
					nodeItem = KPEditorNode(node)
					self.scene().addItem(nodeItem)

					# TODO: fix this ugly bit of code
					item._endNodeRef = weakref.ref(nodeItem)
					item.updatePosition()

					self.painting = node
					self.paintingItem = item
					self.paintBeginPosition = (x - 12, y - 12)

					newPath = KPPath(node, endNode, origPath)
					KP.map.pathLayer.paths.append(newPath)
					KP.mainWindow.pathNodeList.addLayer(newPath, dialog)

					pathItem = KPEditorPath(newPath)
					self.scene().addItem(pathItem)

					return

			# Paint a new node
			node = KPNode()
			node.position = (x - 12, y - 12)
			KP.map.pathLayer.nodes.append(node)
			KP.mainWindow.pathNodeList.addLayer(node, dialog)

			item = KPEditorNode(node)
			self.scene().addItem(item)

			# Paint a path to this node (if one is selected)
			sourceItem, sourceNode = None, None
			selected = self.scene().selectedItems()

			for selItem in selected:
				if isinstance(item, KPEditorNode) and selItem != item:
					sourceItem = selItem
					sourceNode = selItem._nodeRef()
					break

			# No node can have more than four paths, because there are only
			# four directions registered by a Wiimote DPad.

			if not sourceItem is None:
				if len(sourceNode.exits) > 3:
					return

				# There, now you can draw paths easily in a row.
				path = KPPath(sourceNode, node)

				KP.map.pathLayer.paths.append(path)
				KP.mainWindow.pathNodeList.addLayer(path, dialog)

				pathItem = KPEditorPath(path)
				self.scene().addItem(pathItem)


			# Switch the selection to the recently drawn node, so you can keep on rolling.
			self.scene().clearSelection()
			item.setSelected(True)

			self.painting = node
			self.paintingItem = item
			self.paintBeginPosition = (x - 12, y - 12)

		elif isinstance(layer, KPPathTileLayer):
			if self.typeToPaint == 'object':
				paint = self.objectToPaint
				if paint is None: return

				clicked = self.mapToScene(event.x(), event.y())
				x, y = clicked.x(), clicked.y()
				if x < 0: x = 0
				if y < 0: y = 0

				x = int(x / 24)
				y = int(y / 24)

				obj = KPObject()
				obj.position = (x,y)
				obj.size = (1,1)
				obj.tileset = layer.tileset
				obj.kind = self.objectIDToPaint
				obj.kindObj = paint
				obj.updateCache()
				layer.objects.append(obj)
				layer.updateCache()

				item = KPEditorObject(obj, layer)
				self.scene().addItem(item)

				self.painting = obj
				self.paintingItem = item
				self.paintBeginPosition = (x, y)

			elif self.typeToPaint == 'doodad':

				paint = self.doodadToPaint
				if paint is None: return

				clicked = self.mapToScene(event.x(), event.y())
				x, y = clicked.x(), clicked.y()
				if x < 0: x = 0
				if y < 0: y = 0

				obj = KPDoodad()
				obj.position = [x,y]
				obj.source = paint
				obj.setDefaultSize()
				layer.doodads.append(obj)

				item = KPEditorDoodad(obj, layer)
				self.scene().addItem(item)

				self.painting = obj
				self.paintingItem = item
				self.paintBeginPosition = (x, y)

	
	def _movedWhilePainting(self, event):
		'''Called when the mouse is moved while painting something'''

		obj = self.painting
		item = self.paintingItem

		if isinstance(obj, KPObject):
			clicked = self.mapToScene(event.x(), event.y())
			x, y = clicked.x(), clicked.y()
			if x < 0: x = 0
			if y < 0: y = 0

			x = int(x / 24)
			y = int(y / 24)

			beginX, beginY = self.paintBeginPosition

			if x >= beginX:
				objX = beginX
				width = x - beginX + 1
			else:
				objX = x
				width = beginX - x + 1

			if y >= beginY:
				objY = beginY
				height = y - beginY + 1
			else:
				objY = y
				height = beginY - y + 1

			currentX, currentY = obj.position
			currentWidth, currentHeight = obj.size

			# update everything if changed
			changed = False

			if currentX != objX or currentY != objY:
				obj.position = (objX, objY)
				item._updatePosition()
				changed = True
			
			if currentWidth != width or currentHeight != height:
				obj.size = (width, height)
				obj.updateCache()
				item._updateSize()
				changed = True

			if not changed: return

			item._layerRef().updateCache()
	

	def mousePressEvent(self, event):

		if event.button() == Qt.RightButton:
			self._tryToPaint(event)
			event.accept()

		elif event.modifiers() & Qt.ControlModifier:
			if isinstance(self.scene().currentLayer, KPPathLayer):
				QtGui.QGraphicsView.mousePressEvent(self, event)
				return

			itemsUnder = self.scene().items(self.mapToScene(event.pos()), Qt.IntersectsItemShape, Qt.AscendingOrder)

			if itemsUnder:

				self.scene().clearSelection()

				kLayer = itemsUnder[0]._layerRef()
				if isinstance(kLayer, (KPPathTileLayer, KPPathLayer)):
					QtGui.QGraphicsView.mousePressEvent(self, event)
					return

				KP.mainWindow.handleSelectedLayerChanged(kLayer)
				index = KP.map.refLayer(kLayer)
				KP.mainWindow.layerList.selectLayer(index)

				itemsUnder[0].setSelected(True)

		else:
			QtGui.QGraphicsView.mousePressEvent(self, event)
	

	def mouseMoveEvent(self, event):
		if event.buttons() == Qt.RightButton and self.painting:
			self._movedWhilePainting(event)
			event.accept()

		else:
			QtGui.QGraphicsView.mouseMoveEvent(self, event)


	def mouseReleaseEvent(self, event):
		QtGui.QGraphicsView.mouseReleaseEvent(self, event)
	# 	try:
	# 		self.scene().mouseGrabberItem().ungrabMouse()
	# 	except:
	# 		pass
		self.userClick.emit()


	def keyPressEvent(self, event):
		if event.key() == QtCore.Qt.Key_Delete or event.key() == QtCore.Qt.Key_Backspace:
			scene = self.scene()

			selection = scene.selectedItems()
			if len(selection) > 0:
				for obj in selection:
					obj.setSelected(False)
					obj.remove(True)
				scene.update()
				self.userClick.emit()
				self.update()
				return

		else:
			QtGui.QGraphicsView.keyPressEvent(self, event)

	userClick = QtCore.pyqtSignal()



