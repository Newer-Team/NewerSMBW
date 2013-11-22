from common import *
from wii.u8archive import WiiArchiveU8
import struct
import cPickle



class KPTileObject(object):
	def __init__(self, tilelist, height, width, wrapmode, icon):

		# A list of lists of tile indices
		self.tiles = tilelist

		# Dimensions
		self.height = height
		self.width = width

		# QPixmap of Rendered 1:1 Object
		self.icon = icon

		# Wrapmode
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

		self.itemsize = QtCore.QSize(self.icon.width() * 24 + 4, self.icon.height() * 24 + 4)



	def render(self, size):
		'''Returns a tilemap of the object at a certain size as a list of lists.'''

		if self.wrap > 7:
			self._renderSlope(size)


		# size is a tuple of (width, height)

		buf = []

		beforeRepeat = []
		inRepeat = self.tiles[:]
		afterRepeat = []

		if (self.wrap == 1) or (self.wrap == 3) or (self.wrap == 5):
			beforeRepeat = inRepeat.pop(0)

		if (self.wrap == 1) or (self.wrap == 3) or (self.wrap == 4):
			afterRepeat = inRepeat.pop()	

		bC, iC, aC = len(beforeRepeat), len(inRepeat), len(afterRepeat)

		if iC == 0:
			for y in xrange(size[1]):
				buf.append(self._renderRow(beforeRepeat[y % bC], size[0]))
		else:
			middleUntil = size[1] - aC

			for y in xrange(size[1]):
				if y < bC:
					buf.append(self._renderRow(beforeRepeat[y], size[0]))
				elif y < middleUntil:
					buf.append(self._renderRow(inRepeat[(y - bC) % iC], size[0]))
				else:
					buf.append(self._renderRow(afterRepeat[y - bC - iC], size[0]))

		return buf

	def _renderRow(self, row, width):
		buf = [-1 for i in xrange(width)]

		beforeRepeat = []
		inRepeat = row
		afterRepeat = []

		if (self.wrap == 1) or (self.wrap == 2) or (self.wrap == 6):
			beforeRepeat = inRepeat.pop(0)

		if (self.wrap == 1) or (self.wrap == 2) or (self.wrap == 7):
			afterRepeat = inRepeat.pop()	

		bC, iC, aC = len(beforeRepeat), len(inRepeat), len(afterRepeat)

		if iC == 0:
			for x in xrange(width):
				buf[x] = beforeRepeat[x % bC]
		else:
			middleUntil = width - aC

			for x in xrange(width):
				if x < bC:
					buf[x] = beforeRepeat[x]
				elif x < middleUntil:
					buf[x] = inRepeat[(x - bC) % iC]
				else:
					buf[x] = afterRepeat[x - bC - iC]

		return buf


	def _renderSlope(self, size):
		# Slopes are annoying

		buf = []
		w = xrange(size[0])
		h = xrange(size[1])

		# Koopuzzle really only does slopes that are two blocks tall.
		# Suck it, multi height slopes.

		mainblock = self.tiles[0]
		subblock = self.tiles[1]

		if self.wrap == 8:		# Upward (LL to UR)

			# List Comprehension to make a palette
			buf = [[-1 for x in w] for x in h]

			offset = 0

			# Paint the mainblock
			for row in h:
				for tile in w:
					if (tile == ((size[1] - row - 1) * self.width) + offset):
						buf[size[1] - row - 2][tile] = mainblock[offset]
					offset += 1
				offset = 0

			# Paint the subblock
			for row in h:
				for tile in w:
					if (tile == ((size[1] - row)  * self.width) + offset):
						buf[size[1] - row - 1][tile] = mainblock[offset]
					offset += 1
				offset = 0


		elif self.wrap == 9:	# Downward (UL to LR)

			# List Comprehension to make a palette
			buf = [[-1 for x in w] for x in h]

			offset = 0

			# Paint the mainblock
			for row in h:
				for tile in w:
					if (tile == (row * self.width) + offset) and offset < self.width:
						buf[row][tile] = mainblock[offset]
					offset += 1
				offset = 0

			# Paint the subblock
			for row in h:
				for tile in w:
					if (tile == ((row - 1) * self.width) + offset) and offset < self.width:
						buf[row][tile] = mainblock[offset]
					offset += 1
				offset = 0






class KPGroupModel(QtCore.QAbstractListModel):
	"""Model containing all the grouped objects in a tileset"""

	def __init__(self, groupItem):
		self.container = groupItem
		self.view = None

		QtCore.QAbstractListModel.__init__(self)


	def rowCount(self, parent=None):
		return self.container.objectCount()


	def groupItem(self):
		"""Returns the group item"""

		return self.container


	def flags(self, index):
		# item = QtCore.QAbstractItemModel.flags(self, index)

		item, depth = self.container.getItem(index.row())

		if isinstance(item, KPGroupItem):
			return Qt.NoItemFlags
		else:
			return Qt.ItemIsEnabled | Qt.ItemIsSelectable


	def data(self, index, role=Qt.DisplayRole):
		# Should return the contents of a row when asked for the index 
		#
		# Can be optimized by only dealing with the roles we need prior
		# to lookup: Role order is 13, 6, 7, 9, 10, 1, 0, 8

		if ((role > 1) and (role < 6)):
			return None

		elif (role == Qt.ForegroundRole):
			return QtGui.QBrush(Qt.black)

		elif role == Qt.TextAlignmentRole:
			return Qt.AlignCenter


		if not index.isValid(): return None
		n = index.row()

		if n < 0: return None
		if n >= self.container.objectCount(): return None

		item, depth = self.container.getItem(n)

		if role == Qt.DecorationRole:
			if isinstance(item, KPTileObject):
				return item.icon

		elif role == Qt.DisplayRole:
			if isinstance(item, KPGroupItem):
				return item.name

		elif (role == Qt.SizeHintRole):
			if isinstance(item, KPGroupItem):
				return QtCore.QSize(self.view.viewport().width(), (28 - (depth * 2)))

		elif role == Qt.BackgroundRole:
			if isinstance(item, KPGroupItem):

				colour = 165 + (depth * 15)

				if colour > 255:
					colour = 255

				brush = QtGui.QBrush(QtGui.QColor(colour, colour, colour), Qt.Dense4Pattern)

				return brush

		elif (role == Qt.FontRole):
			font = QtGui.QFont()
			font.setPixelSize(20 - (depth * 2))
			font.setBold(True)

			return font

		return None




class KPGroupItem(object):
	"""Hierarchal object for making a 2D hierarchy recursively using classes"""

	def __init__(self, name):

		self.objects = []
		self.groups = []
		self.startIndex = 0
		self.endIndex = 0

		self.name = name
		self.alignment = Qt.AlignCenter


	def getGroupList(self, returnList=[], depth=0):
		"""Gets a list of group names and indices for the sorter menu"""

		returnList.append((('   ' * depth) + self.name, self.startIndex, self.endIndex))

		depth += 1

		for group in self.groups:
			group.getGroupList(returnList, depth)

		return returnList


	def objectCount(self):
		''' Retrieves the total number of items in this group and all it's children '''

		objCount = 0

		objCount += len(self.objects)
		objCount += len(self.groups)
		for group in self.groups:
			objCount += group.objectCount()

		return objCount


	def getItem(self, index, depth=0):
		''' Retrieves an item of a specific index. The index is already checked for validity '''

		if index == self.startIndex:
			return self, depth

		if (index <= self.startIndex + len(self.objects)):
			return self.objects[index - self.startIndex - 1], depth

		else:
			depth += 1

			for group in self.groups:
				if (group.startIndex <= index) and (index <= group.endIndex):
					return group.getItem(index, depth)


	def calculateIndices(self, index):		
		self.startIndex = index
		self.endIndex = self.objectCount() + index

		start = self.startIndex + len(self.objects) + 1

		for group in self.groups:

			group.calculateIndices(start)

			start = group.endIndex + 1




class KPTileset(object):
	@classmethod
	def loadFromArc(cls, handleOrPath):
		arc = WiiArchiveU8(handleOrPath)

		img = arc.resolvePath('/texture.bin').data
		grp = arc.resolvePath('/groups.bin').data

		obj = arc.resolvePath('/objects.bin').data
		meta = arc.resolvePath('/objectsMeta.bin').data

		tmaps = arc.resolvePath('/optimisedTileMappings.txt').data

		return cls(img, obj, meta, grp, tmaps)


	def __init__(self, imageBuffer, objectBuffer, objectMetadata, groupBuffer, optMapBuffer):
		'''A Koopatlas Tileset class. To initialize, pass it image data,
		object data, and group data as read from a Koopatlas tileset file.

		tiles -> has a list of all 512 tiles.
		objects -> has a list of objects in the tileset
		groupModel -> has a model containing groups with items for all objects
					  and groups in the tileset.


		Methods of Note:

			getTile(TileIndex)
				# Returns a tile image as a QPixmap

			getObject(ObjectIndex)
				# Returns a KPTileObject

			getObjectIcon(ObjectIndex)
			getObjectIcon(KPTileObject)
				# Returns a QPixmap for the Object

			getObjectIcon(ObjectIndex, (width, height))
			getObjectIcon(KPTileObject, (width, height))
				# Returns a render map for the Object at the given size

			getModel()
				# Returns the tileset's groupModel, which handles groups

			overrideTile(Tile Index, QPixmap)
				# Takes a 24x24 QPixmap and a tile index

					  '''


		self.tiles = []
		self.objects = []
		self.groupItem = KPGroupItem("All Groups")

		self.processImage(imageBuffer)
		self.processObjects(objectBuffer, objectMetadata)
		self.processGroup(groupBuffer)

		self.groupItem.calculateIndices(0)
		self.groupModel = KPGroupModel(self.groupItem)

		self.optMappings = map(int, optMapBuffer.split(','))


	def processImage(self, imageBuffer):
		'''Takes an imageBuffer from a Koopatlas Tileset and converts it into 24x24
		tiles, which get put into KPTileset.tiles.'''

		dest = self.RGB4A3Decode(imageBuffer)

		self.tileImage = QtGui.QPixmap.fromImage(dest)

		# Makes us some nice Tiles!
		Xoffset = 2
		Yoffset = 2
		for i in range(512):
			self.tiles.append(self.tileImage.copy(Xoffset,Yoffset,24,24))
			Xoffset += 28
			if Xoffset >= 896:
				Xoffset = 2
				Yoffset += 28



	@staticmethod
	def RGB4A3Decode(tex):

		out = bytearray(896*448*4)
		dest = QtGui.QImage(896,448,QtGui.QImage.Format_ARGB32)
		dest.fill(Qt.transparent)

		# Some fairly ugly code, in an attempt to make it run faster
		dest_setPixel = dest.setPixel
		_xrange = xrange
		
		tex = bytearray(tex)

		i = 0

		xtile_range = _xrange(0, 896, 4)
		ytile_range = _xrange(0, 448, 4)

		for ytile in ytile_range:
			ypixel_range = _xrange(ytile, ytile + 4)

			for xtile in xtile_range:
				xpixel_range = _xrange(xtile, xtile + 4)

				for ypixel in ypixel_range:
					for xpixel in xpixel_range:

						bit1 = tex[i]
						bit2 = tex[i+1]

						dest_setPixel(xpixel, ypixel,
								((((bit1 & 0x70) >> 4) * 255 / 7) << 24) |
								(((bit1 & 0xF) * 255 / 15) << 16) |
								((((bit2 & 0xF0) >> 4) * 255 / 15) << 8) |
								((bit2 & 0xF) * 255 / 15)
								)
						i += 2

		return dest


	def processObjects(self, objstrings, metadata):
		'''Takes the object files from a Koopatlas Tileset and converts it into
		KPTileObject classes, which get put into KPTileset.objects.'''

		# Load Objects

		meta = []
		for i in xrange(len(metadata)/5):
			meta.append(struct.unpack_from('>H3B', metadata, i * 5))                                    

		tilelist = []
		rowlist = []

		for entry in meta:  
			offset = entry[0]
			row = 0

			tex = QtGui.QPixmap(entry[1] * 24, entry[2] * 24)
			tex.fill(Qt.transparent)
			painter = QtGui.QPainter(tex)


			for tilesA in xrange(entry[2]):
				for tilesB in xrange(entry[1]):
					untile = struct.unpack_from('>h', objstrings, offset)[0]

					if untile != -1:
						painter.drawPixmap(tilesB*24, tilesA*24, self.tiles[untile])

					rowlist.append(untile)
					offset += 2

				tilelist.append(rowlist)
				rowlist = []

			painter.end()


			self.objects.append(KPTileObject(tilelist, entry[2], entry[1], entry[3], tex))

			tilelist = []




	def processGroup(self, groupBuffer):

		grovyle = cPickle.loads(groupBuffer)
		self.makeTree(grovyle, self.groupItem)




	def makeTree(self, grovyle, treecko):


		for razorLeaf in grovyle:

			if (type(razorLeaf) is str) and (razorLeaf[:6] == "Object"):

				objnum = int(razorLeaf[7:])
				obj = self.objects[objnum]

				treecko.objects.append(obj)

			else:

				a = KPGroupItem(razorLeaf[0])
				treecko.groups.append(a)

				self.makeTree(razorLeaf[1], a)




	def getTile(self, index):
		'''Takes a tile index and returns a tile image as a QPixmap, or -1 if failed.'''

		if index > 511:
			return False

		if index < 0:
			return False

		return self.tiles[index]


	def getObject(self, index):
		'''Takes an object index and returns a KPTileObject, or False if failed.'''

		if index < 0:
			return False

		return self.tiles[index]


	def getObjectIcon(self, index):
		'''Takes an object index or a KPTileObject and returns a QPixmap for the 
		object, or False if failed.'''

		if hasattr(index, 'icon'):
			return index.icon

		try:
			return self.objects[index].icon
		except:
			pass

		return False


	def getObjectRender(self, index, size):
		'''Takes an object index or a KPTileObject and returns a render map for the 
		object, or False if failed.'''

		if hasattr(index, 'render'):
			return index.render(size)

		try:
			return self.objects[index].render(size)
		except:
			pass

		return False


	def getModel(self):
		'''Returns the Group Model'''

		return self.groupModel



	def overrideTile(self, index, pixmap):
		'''Takes a 24x24 QPixmap and a tile index, and returns True if it succeeds.'''

		if index > 511:
			return False

		if index < 0:
			return False

		if not isinstance(pixmap, QtGui.QPixmap):
			return False

		if (pixmap.height() != 24) or (pixmap.width() != 24):
			return False


		self.tiles[index] = pixmap

		return True


