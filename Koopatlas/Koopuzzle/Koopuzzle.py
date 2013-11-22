#!/usr/bin/env python

import archive
import os.path
import struct
import sys
import cPickle

from ctypes import create_string_buffer
from PyQt4 import QtCore, QtGui



########################################################
# To Do:
#
#   - Object List does not retain indices on loading a
#     Tileset, but Groups do, causing a desync after
#     save/open if at least one object has been deleted
#
#   - Properly refresh the group list icons on load
#
#   - Do something about slopes
#
########################################################


Tileset = None

#############################################################################################
########################## Tileset Class and Tile/Object Subclasses #########################

class TilesetClass():
	'''Contains Tileset data. Inits itself to a blank tileset.
	Methods: addTile, removeTile, addObject, removeObject, clear'''

	class Tile():
		def __init__(self, image):
			'''Tile Constructor'''
						
			self.image = image


	class Object():
	
		def __init__(self, tilelist, height, width, wrapmode):
			'''Tile Constructor'''
			
			self.height = height
			self.width = width
			
			self.tiles = tilelist

			self.wrap = wrapmode

				# --- Wrapmodes ---
				# 'Repeat'
				# 'Stretch Center'
				# 'Stretch X'
				# 'Stretch Y'
				# 'Repeat Bottom'
				# 'Repeat Top'
				# 'Repeat Left'
				# 'Repeat Right'
				# 'Upward slope' (LL to UR)
				# 'Downward slope' (UL to LR)
						

	def __init__(self):
		'''Constructor'''
		
		self.tiles = []
		self.objects = []
		
		self.slot = 0


	def addTile(self, image):
		'''Adds an tile class to the tile list with the passed image or parameters'''

		self.tiles.append(self.Tile(image))
		

	def addObject(self, tilelist = [[0xFFFF]], height = 1, width = 1, wrapmode = 0):
		'''Adds a new object'''
		
		global Tileset

		# Initialize trusim power! This is required to work, due to python's single default parameter initialization
		if tilelist == [[0xFFFF]]:
			tilelist = [[0xFFFF]]
			
		self.objects.append(self.Object(tilelist, height, width, wrapmode))
		
		
	def removeObject(self, index):
		'''Removes an Object by Index number. Don't use this much, because we want objects to preserve their ID.'''
		
		self.objects.pop(index)
	   
	
	def clear(self):
		'''Clears the tileset for a new file'''
		
		self.tiles = []
		self.objects = []
		
		
	def clearObjects(self):
		'''Clears the object data'''
		
		self.objects = []
				 

#############################################################################################
##################### Object List Widget and Model Setup with Painter #######################


class objectList(QtGui.QListView):
		
	def __init__(self, parent=None):
		super(objectList, self).__init__(parent)


		self.setFlow(QtGui.QListView.TopToBottom)
		# self.setViewMode(QtGui.QListView.ListMode)
		self.setIconSize(QtCore.QSize(96,96))
		self.setGridSize(QtCore.QSize(100,100))
		self.setMovement(QtGui.QListView.Static)
		self.setBackgroundRole(QtGui.QPalette.BrightText)
		self.setEditTriggers(QtGui.QAbstractItemView.NoEditTriggers)
		self.setWrapping(False)

		

def SetupObjectModel(self, objects, tiles, treecko, group):
	global Tileset
	self.clear()
	
	count = 0
	for object in objects:
		tex = QtGui.QPixmap(object.width * 24, object.height * 24)
		tex.fill(QtCore.Qt.transparent)
		painter = QtGui.QPainter(tex)
		
		Xoffset = 0
		Yoffset = 0
		
		for i in range(len(object.tiles)):
			for tile in object.tiles[i]:
				if (tile != 0xFFFF):
					painter.drawPixmap(Xoffset, Yoffset, tiles[tile].image)
				Xoffset += 24
			Xoffset = 0
			Yoffset += 24
						
		painter.end()

		self.appendRow(QtGui.QStandardItem(QtGui.QIcon(tex), 'Object {0}'.format(count)))
	
		count += 1

	if group != None:
		grovyle = cPickle.loads(group)
		makeTree(grovyle, treecko)


def makeTree(grovyle, treecko):
		
	for razorLeaf in grovyle:

		if (type(razorLeaf) is str) and (razorLeaf[:6] == "Object"):

			pix = QtGui.QPixmap(24, 24)
			pix.fill(QtCore.Qt.transparent)
			painter = QtGui.QPainter(pix)
			painter.drawPixmap(0, 0, pix)
			painter.end()

			a = QtGui.QTreeWidgetItem(treecko)
			a.setText(0, razorLeaf)
			a.setFlags(QtCore.Qt.ItemFlags(0x25))
			a.setIcon(1, QtGui.QIcon(pix))

		else:

			a = QtGui.QTreeWidgetItem(treecko)
			a.setText(0, razorLeaf[0])
			a.setFlags(QtCore.Qt.ItemFlags(0x2F))
			a.setChildIndicatorPolicy(QtGui.QTreeWidgetItem.ShowIndicator)
			a.setExpanded(True)


			makeTree(razorLeaf[1], a)




@QtCore.pyqtSlot(QtGui.QTreeWidgetItem, int)
def connectToTileWidget(tree, column):

	row = tree.text(0)
	if row[:7] == "Object ":

		newrow = int(row[7:])
		index = window.objmodel.index(newrow, 0)
		
		window.objectList.setCurrentIndex(index) 

		window.tileWidget.setObject(index)



#############################################################################################
######################## List Widget with custom painter/MouseEvent #########################


class displayWidget(QtGui.QListView):
		
	def __init__(self, parent=None):
		super(displayWidget, self).__init__(parent)

		self.setMinimumWidth(828)
		self.setMaximumWidth(828)
		self.setMinimumHeight(404)
		self.setMaximumHeight(404)
		self.setDragEnabled(True)
		self.setViewMode(QtGui.QListView.IconMode)
		self.setIconSize(QtCore.QSize(24,24))
		self.setGridSize(QtCore.QSize(25,25))
		self.setMovement(QtGui.QListView.Static)
		self.setAcceptDrops(False)
		self.setDropIndicatorShown(True)
		self.setResizeMode(QtGui.QListView.Adjust)
		self.setUniformItemSizes(True)
		self.setBackgroundRole(QtGui.QPalette.BrightText)
		self.setMouseTracking(True)
		self.setSelectionMode(QtGui.QAbstractItemView.ExtendedSelection)

		self.setItemDelegate(self.TileItemDelegate())

				
				
	class TileItemDelegate(QtGui.QAbstractItemDelegate):
		"""Handles tiles and their rendering"""
		
		def __init__(self):
			"""Initialises the delegate"""
			QtGui.QAbstractItemDelegate.__init__(self)
		
		def paint(self, painter, option, index):
			"""Paints an object"""

			global Tileset
			p = index.model().data(index, QtCore.Qt.DecorationRole)
			painter.drawPixmap(option.rect.x(), option.rect.y(), p.pixmap(24,24))

			x = option.rect.x()
			y = option.rect.y()
			

			# Highlight stuff. 
			colour = QtGui.QColor(option.palette.highlight())
			colour.setAlpha(80)

			if option.state & QtGui.QStyle.State_Selected:
				painter.fillRect(option.rect, colour)
			
		
		def sizeHint(self, option, index):
			"""Returns the size for the object"""
			return QtCore.QSize(24,24)
		
		
		
#############################################################################################
############################ Tile widget for drag n'drop Objects ############################


class tileOverlord(QtGui.QWidget):

	def __init__(self):
		super(tileOverlord, self).__init__()

		# Setup Widgets
		self.tiles = tileWidget()

		self.addObject = QtGui.QPushButton('Add')
		self.removeObject = QtGui.QPushButton('Remove')

		self.addRow = QtGui.QPushButton('+')
		self.removeRow = QtGui.QPushButton('-')
	
		self.addColumn = QtGui.QPushButton('+')
		self.removeColumn = QtGui.QPushButton('-')

		self.tilingMethod = QtGui.QComboBox()

		self.tilingMethod.addItems(['Repeat', 
									'Stretch Center',
									'Stretch X',
									'Stretch Y',
									'Repeat Bottom',
									'Repeat Top',
									'Repeat Left',
									'Repeat Right',
									'Upward slope',
									'Downward slope'])


		# Connections
		self.addObject.released.connect(self.addObj)
		self.removeObject.released.connect(self.removeObj)
		self.addRow.released.connect(self.tiles.addRow)
		self.removeRow.released.connect(self.tiles.removeRow)
		self.addColumn.released.connect(self.tiles.addColumn)
		self.removeColumn.released.connect(self.tiles.removeColumn)

		self.tilingMethod.activated.connect(self.setTiling)


		# Layout
		layout = QtGui.QGridLayout()        
		
		layout.addWidget(self.tilingMethod, 0, 0, 1, 3)

		layout.addWidget(self.addObject, 0, 6, 1, 1)
		layout.addWidget(self.removeObject, 0, 7, 1, 1)
		
		layout.setRowMinimumHeight(1, 40)
		
		layout.setRowStretch(1, 1)
		layout.setRowStretch(2, 5)
		layout.setRowStretch(5, 5)
		layout.addWidget(self.tiles, 2, 1, 4, 6)
		
		layout.addWidget(self.addColumn, 3, 7, 1, 1)
		layout.addWidget(self.removeColumn, 4, 7, 1, 1)
		layout.addWidget(self.addRow, 6, 3, 1, 1)
		layout.addWidget(self.removeRow, 6, 4, 1, 1)
		
		self.setLayout(layout)
	   

	def addObj(self):
		global Tileset
		
		Tileset.addObject()
		
		pix = QtGui.QPixmap(24, 24)
		pix.fill(QtCore.Qt.transparent)
		painter = QtGui.QPainter(pix)
		painter.drawPixmap(0, 0, pix)
		painter.end()
					
		count = len(Tileset.objects)
		window.objmodel.appendRow(QtGui.QStandardItem(QtGui.QIcon(pix), 'Object {0}'.format(count-1)))
		a = QtGui.QTreeWidgetItem(window.treeki)
		a.setText(0, 'Object {0}'.format(count-1))
		a.setFlags(QtCore.Qt.ItemFlags(0x25))
		a.setIcon(1, QtGui.QIcon(pix))

		index = window.objectList.currentIndex()
		window.objectList.setCurrentIndex(index)
		self.setObject(index)

		window.objectList.update()
		self.update()
		

	def removeObj(self):
		global Tileset

		index = window.objectList.currentIndex()

		Tileset.removeObject(index.row())
		window.objmodel.removeRow(index.row())
		self.tiles.clear()

		matchList = window.treeki.findItems("Object {0}".format(index.row()), QtCore.Qt.MatchExactly | QtCore.Qt.MatchRecursive | QtCore.Qt.MatchWrap)
		for x in matchList:
			index = window.treeki.indexFromItem(x, 0)
			realx = index.row()
			if x.parent():
				y = x.parent().takeChild(realx)
				del y
			else:
				y =window.treeki.takeTopLevelItem(realx)
				del y


		window.objectList.update()
		self.update()


	def setObject(self, index):
		global Tileset
		object = Tileset.objects[index.row()]
	 
		self.tilingMethod.setCurrentIndex(object.wrap)           
				
		self.tiles.setObject(object)

	
	@QtCore.pyqtSlot(int)
	def setTiling(self, listindex):
		global Tileset
		
		index = window.objectList.currentIndex()
		object = Tileset.objects[index.row()]
		
		object.wrap = listindex
			
		self.tiles.update()
	   

class tileWidget(QtGui.QWidget):
	
	def __init__(self):
		super(tileWidget, self).__init__()

		self.tiles = []

		self.size = [1, 1]
		self.setMinimumSize(120, 120)

		self.slope = 0

		self.highlightedRect = QtCore.QRect()

		self.setAcceptDrops(True)
		self.object = 0


	def clear(self):
		self.tiles = []
		self.size = [1, 1] # [width, height]
		
		self.slope = 0
		self.highlightedRect = QtCore.QRect()

		self.update()

		return


	def addColumn(self):
		global Tileset
		
		if self.size[0] >= 24:
			return
			
		if len(Tileset.objects) == 0:
			window.tileWidget.addObj()

		self.size[0] += 1
		self.setMinimumSize(self.size[0]*24, self.size[1]*24)

		pix = QtGui.QPixmap(24,24)
		pix.fill(QtGui.QColor(205, 205, 255))

		for y in xrange(self.size[1]):
			self.tiles.insert(((y+1) * self.size[0]) -1, [self.size[0]-1, y, pix])

 
		curObj = Tileset.objects[self.object]
		curObj.width += 1

		for row in curObj.tiles:
			row.append(0xFFFF)
			
		self.update()
		self.updateList()

   
	def removeColumn(self):
		global Tileset

		if self.size[0] == 1:
			return

		if len(Tileset.objects) == 0:
			window.tileWidget.addObj()

		for y in xrange(self.size[1]):
			self.tiles.pop(((y+1) * self.size[0])-(y+1))

		self.size[0] = self.size[0] - 1
		self.setMinimumSize(self.size[0]*24, self.size[1]*24)


		curObj = Tileset.objects[self.object]
		curObj.width -= 1

		for row in curObj.tiles:
			row.pop()

		self.update()
		self.updateList()


	def addRow(self):
		global Tileset

		if len(Tileset.objects) == 0:
			window.tileWidget.addObj()

		if self.size[1] >= 24:
			return
		
		self.size[1] += 1
		self.setMinimumSize(self.size[0]*24, self.size[1]*24)

		pix = QtGui.QPixmap(24,24)
		pix.fill(QtGui.QColor(205, 205, 255))

		for x in xrange(self.size[0]):
			self.tiles.append([x, self.size[1]-1, pix])

		curObj = Tileset.objects[self.object]
		curObj.height += 1

		curObj.tiles.append([])
		for i in xrange(0, curObj.width):
			curObj.tiles[len(curObj.tiles)-1].append(0xFFFF)

		self.update()
		self.updateList()

	
	def removeRow(self):
		global Tileset

		if self.size[1] == 1:
			return

		if len(Tileset.objects) == 0:
			window.tileWidget.addObj()

		for x in xrange(self.size[0]):
			self.tiles.pop()
		
		self.size[1] -= 1
		self.setMinimumSize(self.size[0]*24, self.size[1]*24)

		curObj = Tileset.objects[self.object]
		curObj.height -= 1

		curObj.tiles.pop()

		self.update()
		self.updateList()


	def setObject(self, object):
		self.clear()
			
		global Tileset
			
		self.size = [object.width, object.height]
		
		if object.wrap > 7:
			if (object.wrap == 8) or (object.wrap == 10):
				self.slope = 1
			else:
				self.slope = -1

		x = 0
		y = 0
		for row in object.tiles:
			for tile in row:

				if (tile == 0xFFFF):
					pix = QtGui.QPixmap(24,24)
					pix.fill(QtGui.QColor(205, 205, 255))
					self.tiles.append([x, y, pix])
				else:
					self.tiles.append([x, y, Tileset.tiles[tile].image])

				x += 1
			y += 1
			x = 0
		   
		   
		self.object = window.objectList.currentIndex().row()    
		self.update()
		self.updateList()
			   

	def contextMenuEvent(self, event):
	
		TileMenu = QtGui.QMenu(self)
		self.contX = event.x()
		self.contY = event.y()
		
		TileMenu.addAction('Set tile...', self.setTile)

		TileMenu.exec_(event.globalPos())


	def mousePressEvent(self, event):
		global Tileset

		if event.button() == 2:
			return

		if window.tileDisplay.selectedIndexes() == []:
			return

		currentSelected = window.tileDisplay.selectedIndexes()
		
		ix = 0
		iy = 0
		for modelItem in currentSelected:
			# Update yourself!
			centerPoint = self.contentsRect().center()
	
			tile = modelItem.row()
			upperLeftX = centerPoint.x() - self.size[0]*12
			upperLeftY = centerPoint.y() - self.size[1]*12
	
			lowerRightX = centerPoint.x() + self.size[0]*12
			lowerRightY = centerPoint.y() + self.size[1]*12
	
	
			x = (event.x() - upperLeftX)/24 + ix
			y = (event.y() - upperLeftY)/24 + iy
	
			if event.x() < upperLeftX or event.y() < upperLeftY or event.x() > lowerRightX or event.y() > lowerRightY:
				return
					
			self.tiles[(y * self.size[0]) + x][2] = Tileset.tiles[tile].image
					
			Tileset.objects[self.object].tiles[y][x] = (tile)

			ix += 1
			if self.size[0]-1 < ix:
				ix = 0
				iy += 1
			if iy > self.size[1]-1:
				break
			
			
		self.update()
		
		self.updateList()
		

	def updateList(self):        
		# Update the list >.>
		object = window.objmodel.itemFromIndex(window.objectList.currentIndex())
		matchList = window.treeki.findItems("Object {0}".format(window.objectList.currentIndex().row()), QtCore.Qt.MatchExactly | QtCore.Qt.MatchRecursive | QtCore.Qt.MatchWrap)
		

		tex = QtGui.QPixmap(self.size[0] * 24, self.size[1] * 24)
		tex.fill(QtCore.Qt.transparent)
		painter = QtGui.QPainter(tex)
		
		Xoffset = 0
		Yoffset = 0
		
		for tile in self.tiles:
			painter.drawPixmap(tile[0]*24, tile[1]*24, tile[2])
						
		painter.end()

		try:
			object.setIcon(QtGui.QIcon(tex))
			matchList[0].setIcon(1, QtGui.QIcon(tex))
		except:
			pass

		window.objectList.update()
	
			
		
	def setTile(self):
		global Tileset
		
		dlg = self.setTileDialog()
		if dlg.exec_() == QtGui.QDialog.Accepted:
			# Do stuff
			centerPoint = self.contentsRect().center()

			upperLeftX = centerPoint.x() - self.size[0]*12
			upperLeftY = centerPoint.y() - self.size[1]*12

			tile = dlg.tile.value()
	
			x = (self.contX - upperLeftX)/24
			y = (self.contY - upperLeftY)/24


			Tileset.objects[self.object].tiles[y][x] = tile
			
			self.update()
			self.updateList()


	class setTileDialog(QtGui.QDialog):
	
		def __init__(self):
			QtGui.QDialog.__init__(self)
		
			self.setWindowTitle('Set tiles')
				
			self.tile = QtGui.QSpinBox()                
			self.tile.setRange(0, 512)             
			
			self.buttons = QtGui.QDialogButtonBox(QtGui.QDialogButtonBox.Ok | QtGui.QDialogButtonBox.Cancel)
			self.buttons.accepted.connect(self.accept)
			self.buttons.rejected.connect(self.reject)
			
			self.layout = QtGui.QGridLayout()
			self.layout.addWidget(QtGui.QLabel('Tile:'), 0,3,1,1, QtCore.Qt.AlignLeft)
			self.layout.addWidget(self.tile, 1, 3, 1, 3)
			self.layout.addWidget(self.buttons, 2, 3)
			self.setLayout(self.layout)

						   

	def paintEvent(self, event):
		painter = QtGui.QPainter()
		painter.begin(self)
		
		centerPoint = self.contentsRect().center()
		upperLeftX = centerPoint.x() - self.size[0]*12
		lowerRightX = centerPoint.x() + self.size[0]*12

		upperLeftY = centerPoint.y() - self.size[1]*12
		lowerRightY = centerPoint.y() + self.size[1]*12


		painter.fillRect(upperLeftX, upperLeftY, self.size[0] * 24, self.size[1]*24, QtGui.QColor(205, 205, 255))

		for x, y, pix in self.tiles:
			painter.drawPixmap(upperLeftX + (x * 24), upperLeftY + (y * 24), pix)

		if not self.slope == 0:
			pen = QtGui.QPen()
			# pen.setStyle(QtCore.Qt.QDashLine)
			pen.setWidth(1)
			pen.setColor(QtCore.Qt.blue)
			painter.setPen(QtGui.QPen(pen))
			painter.drawLine(upperLeftX, upperLeftY + (abs(self.slope) * 24), lowerRightX, upperLeftY + (abs(self.slope) * 24))
			
			if self.slope > 0:
				main = 'Main'
				sub = 'Sub'
			elif self.slope < 0:
				main = 'Sub'
				sub = 'Main'

			font = painter.font()
			font.setPixelSize(8)
			font.setFamily('Monaco')
			painter.setFont(font)

			painter.drawText(upperLeftX+1, upperLeftY+10, main)
			painter.drawText(upperLeftX+1, upperLeftY + (abs(self.slope) * 24) + 9, sub)

		painter.end()



#############################################################################################
############################ Subclassed one dimension Item Model ############################


class PiecesModel(QtCore.QAbstractListModel):
	def __init__(self, parent=None):
		super(PiecesModel, self).__init__(parent)

		self.pixmaps = []
		self.setSupportedDragActions(QtCore.Qt.CopyAction | QtCore.Qt.MoveAction | QtCore.Qt.LinkAction)

	def data(self, index, role=QtCore.Qt.DisplayRole):
		if not index.isValid():
			return None

		if role == QtCore.Qt.DecorationRole:
			return QtGui.QIcon(self.pixmaps[index.row()])

		if role == QtCore.Qt.UserRole:
			return self.pixmaps[index.row()]

		return None

	def addPieces(self, pixmap):
		row = len(self.pixmaps)

		self.beginInsertRows(QtCore.QModelIndex(), row, row)
		self.pixmaps.insert(row, pixmap)
		self.endInsertRows()
		
	def flags(self,index):
		if index.isValid():
			return (QtCore.Qt.ItemIsEnabled | QtCore.Qt.ItemIsSelectable |
					QtCore.Qt.ItemIsDragEnabled)

	def clear(self):
		row = len(self.pixmaps)

		del self.pixmaps[:]


	def mimeTypes(self):
		return ['image/x-tile-piece']


	def mimeData(self, indexes):
		mimeData = QtCore.QMimeData()
		encodedData = QtCore.QByteArray()

		stream = QtCore.QDataStream(encodedData, QtCore.QIODevice.WriteOnly)

		for index in indexes:
			if index.isValid():
				pixmap = QtGui.QPixmap(self.data(index, QtCore.Qt.UserRole))
				stream << pixmap

		mimeData.setData('image/x-tile-piece', encodedData)
		return mimeData


	def rowCount(self, parent):
		if parent.isValid():
			return 0
		else:
			return len(self.pixmaps)

	def supportedDragActions(self):
		return QtCore.Qt.CopyAction | QtCore.Qt.MoveAction



#############################################################################################
################## Python-based RGB5a3 Decoding code from my BRFNT program ##################


def RGB5A3Decode(tex):
	dest = QtGui.QImage(896,448,QtGui.QImage.Format_ARGB32)
	dest.fill(QtCore.Qt.transparent)
	
	i = 0
	height = min(len(tex) / 1792, 448)
	for ytile in xrange(0, 448, 4):
		for xtile in xrange(0, 896, 4):
			for ypixel in xrange(ytile, ytile + 4):
				for xpixel in xrange(xtile, xtile + 4):
					
					if(xpixel >= 896 or ypixel >= height):
						continue

					newpixel = struct.unpack_from('>H', tex, i)[0]
					# newpixel = (int(tex[i]) << 8) | int(tex[i+1])
					
	
					if(newpixel >= 0x8000): # Check if it's RGB555
						red = ((newpixel >> 10) & 0x1F) * 255 / 0x1F
						green = ((newpixel >> 5) & 0x1F) * 255 / 0x1F
						blue = (newpixel & 0x1F) * 255 / 0x1F
						alpha = 0xFF
					else: # If not, it's RGB4A3
						alpha = ((newpixel & 0x7000) >> 12) * 255 / 0x7
						red = ((newpixel & 0xF00) >> 8) * 255 / 0xF
						green = ((newpixel & 0xF0) >> 4) * 255 / 0xF
						blue = (newpixel & 0xF) * 255 / 0xF
	
					argb = (blue) | (green << 8) | (red << 16) | (alpha << 24)
					dest.setPixel(xpixel, ypixel, argb)
					i += 2
	return dest


def RGB5A3Encode(tex, force4A3=True):
	destBuffer = create_string_buffer(896*tex.height()*2)

	shortstruct = struct.Struct('>H')
	offset = 0

	for ytile in xrange(0, tex.height(), 4):
		for xtile in xrange(0, 896, 4):
			for ypixel in xrange(ytile, ytile + 4):
				for xpixel in xrange(xtile, xtile + 4):
					
					pixel = tex.pixel(xpixel, ypixel)
					
					a = pixel >> 24
					r = (pixel >> 16) & 0xFF
					g = (pixel >> 8) & 0xFF
					b = pixel & 0xFF
					
					if a < 245 or force4A3: #RGB4A3
						alpha = a/32
						red = r/16
						green = g/16
						blue = b/16
						rgbDAT = (blue) | (green << 4) | (red << 8) | (alpha << 12)
					else: # RGB555
						red = r/8
						green = g/8
						blue = b/8
						rgbDAT = (blue) | (green << 5) | (red << 10) | (0x8000) # 0rrrrrgggggbbbbb

					shortstruct.pack_into(destBuffer, offset, rgbDAT)
					offset += 2
					
	return destBuffer.raw


def RGBA8Encode(tex):
	destBuffer = create_string_buffer(896*tex.height()*4)

	shortstruct = struct.Struct('>H')
	offset = 0

	for ytile in xrange(0, tex.height(), 4):
		for xtile in xrange(0, 896, 4):
			for ypixel in xrange(ytile, ytile + 4):
				for xpixel in xrange(xtile, xtile + 4):
					
					pixel = tex.pixel(xpixel, ypixel)
					
					shortstruct.pack_into(destBuffer, offset, pixel>>16)
					shortstruct.pack_into(destBuffer, offset+32, pixel&0xFFFF)
					offset += 2
			offset += 32

	return destBuffer.raw


#############################################################################################
############ Main Window Class. Takes care of menu functions and widget creation ############


class MainWindow(QtGui.QMainWindow):
	def __init__(self, parent=None):
		super(MainWindow, self).__init__(parent)

		self.tileImage = QtGui.QPixmap()
		self.alpha = True
		
		global Tileset
		Tileset = TilesetClass()

		self.name = ''

		self.setupMenus()
		self.setupWidgets()

		self.setuptile()

		self.newTileset()

		self.setSizePolicy(QtGui.QSizePolicy(QtGui.QSizePolicy.Fixed,
				QtGui.QSizePolicy.Fixed))
		self.setWindowTitle("New Tileset")


	def setuptile(self):
		self.tileWidget.tiles.clear()
		self.model.clear()

		if self.alpha == True:
			for tile in Tileset.tiles:
				self.model.addPieces(tile.image)
		else:
			for tile in Tileset.tiles:
				self.model.addPieces(tile.noalpha)


	def newTileset(self):
		'''Creates a new, blank tileset'''
		
		global Tileset
		Tileset.clear()
		self.treeki.clear()
		self.objmodel.clear()

		Tileset = TilesetClass()
		
		EmptyPix = QtGui.QPixmap(24, 24)
		EmptyPix.fill(QtCore.Qt.black)
		
		for i in range(512):
			Tileset.addTile(EmptyPix)

		self.setuptile()
		self.setWindowTitle('New Tileset')
		
		
	def openTileset(self):
		'''Opens a Koopatlas tileset arc and parses the heck out of it.'''
		
		global Tileset

		path = str(QtGui.QFileDialog.getOpenFileName(self, "Open Koopatlas Tileset", '',
					"Image Files (*.arc)"))
					
		if path:
			self.setWindowTitle(os.path.basename(path))
			Tileset.clear()
			self.treeki.clear()
			self.objmodel.clear()

			name = path[str(path).rfind('/')+1:-4]
		
			file = open(path,'rb')
			data = file.read()
			file.close()
			
			arc = archive.U8()
			arc._load(data)
			
			Image = None
			behaviourdata = None
			objstrings = None
			metadata = None
			
			for key, value in arc.files:
				if value == None:
					pass
				print key
				if (key.startswith('BG_tex/') and key.endswith('_tex.bin')) or key == '/texture.bin':
					Image = arc[key]
				if (key.startswith('BG_grp/') and key.endswith('_grp.bin')) or key == '/groups.bin':
					group = arc[key]
				if key.startswith('BG_unt/') or key.startswith('/objects'):
					if key.endswith('_hd.bin') or key.endswith('Meta.bin'):
						metadata = arc[key]
					elif key.endswith('.bin'):
						objstrings = arc[key]

			if (Image == None) or (group == None) or (objstrings == None) or (metadata == None):
				QtGui.QMessageBox.warning(None, 'Error',  'Error - the necessary files were not found.\n\nNot a valid Koopatlas tileset, sadly.')
				return
			
			dest = RGB5A3Decode(Image)
			
			self.tileImage = QtGui.QPixmap.fromImage(dest)
			
			# Makes us some nice Tile Classes!
			Xoffset = 2
			Yoffset = 2
			for i in range(512):
				Tileset.addTile(self.tileImage.copy(Xoffset,Yoffset,24,24))
				Xoffset += 28
				if Xoffset >= 896:
					Xoffset = 2
					Yoffset += 28
			
			# Load Objects
			
			meta = []
			for i in xrange(len(metadata)/5):
				meta.append(struct.unpack_from('>H3B', metadata, i * 5))                                    
				
			tilelist = []
			rowlist = []
			
			for entry in meta:  
				offset = entry[0]
				row = 0
				
				for tiles in xrange(entry[2]):
					for tiles in xrange(entry[1]):
						untile = struct.unpack_from('>H', objstrings, offset)[0]

						rowlist.append(untile)
						offset += 2

					tilelist.append(rowlist)
					rowlist = []

				Tileset.addObject(tilelist, entry[2], entry[1], entry[3])

				tilelist = []

			self.setuptile()
			SetupObjectModel(self.objmodel, Tileset.objects, Tileset.tiles, self.treeki, group)

			self.tileWidget.tiles.updateList()

		self.name = path


	def openImage(self):
		'''Opens an Image from png, and creates a new tileset from it.'''

		path = QtGui.QFileDialog.getOpenFileName(self, "Open Image", '',
					"Image Files (*.png)")
					
		if path:
			newImage = QtGui.QPixmap()
			self.tileImage = newImage

			if not newImage.load(path):
				QtGui.QMessageBox.warning(self, "Open Image",
						"The image file could not be loaded.",
						QtGui.QMessageBox.Cancel)
				return

			if ((newImage.width() == 768) & (newImage.height() == 384)):
				x = 0
				y = 0
				for i in range(512):
					Tileset.tiles[i].image = self.tileImage.copy(x*24,y*24,24,24)
					x += 1
					if (x * 24) >= 768:
						y += 1
						x = 0

			else: 
				QtGui.QMessageBox.warning(self, "Open Image",
						"The image was not the proper dimensions."
						"Please resize the image to 768x384 pixels.",
						QtGui.QMessageBox.Cancel)
				return


			self.setuptile()


	def saveImage(self):
			
		fn = QtGui.QFileDialog.getSaveFileName(self, 'Choose a new filename', '', '.png (*.png)')
		if fn == '': return
		
		tex = QtGui.QPixmap(768, 384)
		tex.fill(QtCore.Qt.transparent)
		painter = QtGui.QPainter(tex)
		
		Xoffset = 0
		Yoffset = 0
		
		for tile in Tileset.tiles:
			painter.drawPixmap(Xoffset, Yoffset, tile.image)
			Xoffset += 24
			if Xoffset >= 768:
				Xoffset = 0
				Yoffset += 24
						
		painter.end()

		tex.save(fn)
			   
	def saveTileset(self):
		if self.name == '':
			self.saveTilesetAs()
			return
			
		
		outdata = self.saving(os.path.basename(unicode(self.name))[:-4])
		
		fn = unicode(self.name)
		f = open(fn, 'wb')
		f.write(outdata)
		f.close()
				
		reply = QtGui.QMessageBox.information(self, "Save Complete", "Tileset saved as {0}.".format(os.path.basename(unicode(self.name))[:-4]))
	   
	def saveTilesetAs(self):
		
		fn = QtGui.QFileDialog.getSaveFileName(self, 'Choose a new filename', '', '.arc (*.arc)')
		if fn == '': return

		self.name = fn
		self.setWindowTitle(os.path.basename(unicode(fn)))
		
		outdata = self.saving(os.path.basename(unicode(fn))[:-4])
		f = open(fn, 'wb')
		f.write(outdata)
		f.close()

		reply = QtGui.QMessageBox.information(self, "Save Complete", "Tileset saved as {0}.".format(os.path.basename(unicode(self.name))[:-4]))


	def saving(self, name):

		# Prepare tiles, objects, object metadata, and textures and stuff into buffers.

		self.usesRGBA8 = ('RGBA8' in name)

		textureBuffer = self.PackTexture()
		optTextureBuffer, optMappings = self.PackOptimisedTexture()
		objectBuffers = self.PackObjects()
		objectBuffer = objectBuffers[0]
		objectMetaBuffer = objectBuffers[1]
		groupBuffer = self.PackGroups()

				
		# Make an arc and pack up the files!
		arc = archive.U8()
		arc['texture.bin'.format(name)] = textureBuffer
		arc['optimisedTexture.bin'.format(name)] = optTextureBuffer
		arc['optimisedTileMappings.txt'] = optMappings

		arc['objects.bin'] = objectBuffer
		arc['objectsMeta.bin'] = objectMetaBuffer
		
		arc['groups.bin'] = groupBuffer

		return arc._dump()

	def PackOptimisedTexture(self):
		sortedTiles = []
		mapping = [-1 for x in xrange(512)]

		first_fuckin_image = Tileset.tiles[0].image.toImage()
		transparent = QtGui.QImage(32, 32, first_fuckin_image.format())
		transparent.fill(QtCore.Qt.transparent)

		for i, tile in enumerate(Tileset.tiles):
			img = tile.image.toImage()
			if tile == transparent:
				continue

			for j, otherImg in enumerate(sortedTiles):
				if img == otherImg:
					mapping[i] = j
					break
			else:
				mapping[i] = len(sortedTiles)
				sortedTiles.append(img)

		tex = self.PackTexture(sortedTiles, 'rgba8' if self.usesRGBA8 else 'real_rgb5a3')
		maps = ','.join(map(str, mapping))
		return tex, maps

	def PackTexture(self, imageList=None, mode='standard'):
		if imageList is None:
			imageList = map(lambda x: x.image, Tileset.tiles)
		usesPixmap = isinstance(imageList[0], QtGui.QPixmap)

		from math import ceil
		height = int(ceil(len(imageList) / 32.0))
		tex = QtGui.QImage(896, height*28, QtGui.QImage.Format_ARGB32)
		tex.fill(QtCore.Qt.transparent)
		painter = QtGui.QPainter(tex)
		
		Xoffset = 0
		Yoffset = 0

		for img in imageList:
			minitex = QtGui.QImage(28, 28, QtGui.QImage.Format_ARGB32)
			minitex.fill(QtCore.Qt.transparent)
			minipainter = QtGui.QPainter(minitex)
			
			if usesPixmap:
				minipainter.drawPixmap(2, 2, img)
			else:
				minipainter.drawImage(2, 2, img)
			minipainter.end()
			
			# Read colours and DESTROY THEM (or copy them to the edges, w/e)
			for i in xrange(2,26):
				
				# Top Clamp
				colour = minitex.pixel(i, 2)
				for p in xrange(0,2):
					minitex.setPixel(i, p, colour)
				
				# Left Clamp
				colour = minitex.pixel(2, i)
				for p in xrange(0,2):
					minitex.setPixel(p, i, colour)
				
				# Right Clamp
				colour = minitex.pixel(i, 25)
				for p in xrange(26,28):
					minitex.setPixel(i, p, colour)
				
				# Bottom Clamp
				colour = minitex.pixel(25, i)
				for p in xrange(26,28):
					minitex.setPixel(p, i, colour)

			# UpperLeft Corner Clamp
			colour = minitex.pixel(2, 2)
			for x in xrange(0,2):
				for y in xrange(0,2):
					minitex.setPixel(x, y, colour)

			# UpperRight Corner Clamp
			colour = minitex.pixel(25, 2)
			for x in xrange(26,28):
				for y in xrange(0,2):
					minitex.setPixel(x, y, colour)

			# LowerLeft Corner Clamp
			colour = minitex.pixel(2, 25)
			for x in xrange(0,2):
				for y in xrange(26,28):
					minitex.setPixel(x, y, colour)

			# LowerRight Corner Clamp
			colour = minitex.pixel(25, 25)
			for x in xrange(26,28):
				for y in xrange(26,28):
					minitex.setPixel(x, y, colour)

					
			painter.drawImage(Xoffset, Yoffset, minitex)
			
			Xoffset += 28
			
			if Xoffset >= 896:
				Xoffset = 0
				Yoffset += 28
									
		painter.end()
		tex.save('beef.png')

		if mode == 'rgba8':
			return RGBA8Encode(tex)
		elif mode == 'real_rgb5a3':
			return RGB5A3Encode(tex, False)
		else:
			return RGB5A3Encode(tex)



	def PackObjects(self):
		objectStrings = []
		
		o = 0
		for object in Tileset.objects:
				 
			a = ''
			
			for tilerow in object.tiles:
				for tile in tilerow:
					a = a + struct.pack('>H', tile)

					print tile
								
			a = a + '\xff'
			
			objectStrings.append(a)
			
			o += 1
			
		Objbuffer = ''
		Metabuffer = ''
		i = 0
		for a in objectStrings:
			Metabuffer = Metabuffer + struct.pack('>H3B', len(Objbuffer), Tileset.objects[i].width, Tileset.objects[i].height, Tileset.objects[i].wrap)
			Objbuffer = Objbuffer + a
			
			i += 1
		
		return (Objbuffer, Metabuffer)


	
	def PackGroups(self):

		groupList = []

		# Walk through all top level items
		for kiddy in xrange(self.treeki.topLevelItemCount()):
			groupList.append(self.walkTree(self.treeki.topLevelItem(kiddy)))

		return cPickle.dumps(groupList)


	def walkTree(self, treeItem):
		
		# If item has kids, walk through each kid. If not, just add the string.

		name = str(treeItem.text(0))

		alist = []

		if treeItem.childCount() > 0:

			for kiddy in xrange(treeItem.childCount()):
				alist.append(self.walkTree(treeItem.child(kiddy)))

			return (name, alist)

		else:
			return name


	def setupMenus(self):
		fileMenu = self.menuBar().addMenu("&File")

		pixmap = QtGui.QPixmap(60,60)
		pixmap.fill(QtCore.Qt.black)
		icon = QtGui.QIcon(pixmap)

		self.action = fileMenu.addAction(icon, "New", self.newTileset, QtGui.QKeySequence.New)
		fileMenu.addAction("Open...", self.openTileset, QtGui.QKeySequence.Open)
		fileMenu.addAction("Import Image...", self.openImage, QtGui.QKeySequence('Ctrl+I'))
		fileMenu.addAction("Export Image...", self.saveImage, QtGui.QKeySequence('Ctrl+E'))
		fileMenu.addAction("Save", self.saveTileset, QtGui.QKeySequence.Save)
		fileMenu.addAction("Save as...", self.saveTilesetAs, QtGui.QKeySequence.SaveAs)
		fileMenu.addAction("Quit", self.close, QtGui.QKeySequence('Ctrl-Q'))

		taskMenu = self.menuBar().addMenu("&Tasks")

		taskMenu.addAction("Clear Object Data", Tileset.clearObjects, QtGui.QKeySequence('Ctrl+Alt+Backspace'))
		

	def TriggerNewGroup(self):

		a = QtGui.QTreeWidgetItem(self.treeki)
		a.setText(0, 'Double Click to Rename')
		a.setFlags(QtCore.Qt.ItemFlags(0x2F))
		a.setChildIndicatorPolicy(QtGui.QTreeWidgetItem.ShowIndicator)
		a.setExpanded(True)


	def TriggerDelGroup(self):
		
		treecko = self.treeki.currentItem()
		if str(treecko.text(0)).find("Object"):
			eggs = treecko.takeChildren()

			self.treeki.addTopLevelItems(eggs)

			index = window.treeki.indexFromItem(treecko, 0)
			realx = index.row()

			if treecko.parent():
				y = treecko.parent().takeChild(realx)
			else:
				y = self.treeki.takeTopLevelItem(realx)

			del y



	def setupWidgets(self):
		frame = QtGui.QFrame()
		frameLayout = QtGui.QHBoxLayout(frame)

		# Displays the tiles
		self.tileDisplay = displayWidget()
				
		# Sets up the model for the tile pieces
		self.model = PiecesModel(self)
		self.tileDisplay.setModel(self.model)

		# Object List
		self.objectList = objectList()
		self.objmodel = QtGui.QStandardItemModel()
		self.objectList.setModel(self.objmodel)

		self.tileWidget = tileOverlord()

		# Vertical Container A
		self.container = QtGui.QWidget()
		layout = QtGui.QVBoxLayout()
		layout.addWidget(self.tileDisplay)
		layout.addWidget(self.tileWidget)
		self.container.setLayout(layout)


		# Create the Group Tree
		self.treeki = QtGui.QTreeWidget()
		self.treeki.setSelectionMode(QtGui.QAbstractItemView.ExtendedSelection)
		self.treeki.setDragEnabled(True)
		self.treeki.setDragDropMode(QtGui.QAbstractItemView.InternalMove)
		self.treeki.setAcceptDrops(True)
		self.treeki.setDropIndicatorShown(True)
		self.treeki.setEditTriggers(QtGui.QAbstractItemView.DoubleClicked)
		self.treeki.setColumnCount(2)
		self.treeki.setColumnWidth(0, 200)

		self.treeki.setColumnWidth(1, 40)


		# Vertical Container B
		self.treeBario = QtGui.QWidget()
		tlayout = QtGui.QVBoxLayout()
		tlayout.addWidget(self.treeki)

		self.groupbar = QtGui.QToolBar()

		self.newGroup = QtGui.QAction('New Group', self.groupbar)
		self.newGroup.triggered.connect(self.TriggerNewGroup)
		self.newGroup.setShortcut(QtGui.QKeySequence('Ctrl+Shift+N'))

		self.delGroup = QtGui.QAction('Delete Group', self.groupbar)
		self.delGroup.triggered.connect(self.TriggerDelGroup)
		self.delGroup.setShortcut(QtGui.QKeySequence('Ctrl+Shift+Del'))

		self.groupbar.addAction(self.newGroup)
		self.groupbar.addAction(self.delGroup)

		self.groupbar.setFloatable(False)

		tlayout.addWidget(self.groupbar)
		self.treeBario.setLayout(tlayout)


		# Creates the Tab Widget for behaviours and objects
		self.tabWidget = QtGui.QTabWidget()

		# Sets the Tabs
		self.tabWidget.addTab(self.objectList, 'Object List')
		self.tabWidget.addTab(self.treeBario, 'Object Groups')
	


		# Connections do things!
		self.objectList.clicked.connect(self.tileWidget.setObject)
		self.treeki.itemClicked.connect(connectToTileWidget)


		SetupObjectModel(self.objmodel, Tileset.objects, Tileset.tiles, self.treeki, None)


		frameLayout.addWidget(self.container)
		frameLayout.addWidget(self.tabWidget)
		

		self.setCentralWidget(frame)
				



#############################################################################################
####################################### Main Function #######################################


if __name__ == '__main__':

	import sys

	app = QtGui.QApplication(sys.argv)
	window = MainWindow()
	window.show()
	sys.exit(app.exec_())
	app.deleteLater()
