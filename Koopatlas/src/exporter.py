from common import *
import array
import sys
import math
from ctypes import create_string_buffer

# Useful Stuff
u32 = struct.Struct('>I')
u16 = struct.Struct('>H')
zero32 = u32.pack(0)

def RGBA8Encode(tex):
	tex = tex.toImage()
	w, h = tex.width(), tex.height()
	padW = (w + 3) & ~3
	padH = (h + 3) & ~3

	destBuffer = create_string_buffer(padW * padH * 4)

	shortstruct = struct.Struct('>H')
	sspack = shortstruct.pack_into
	offset = 0

	for ytile in xrange(0, padH, 4):
		for xtile in xrange(0, padW, 4):
			for ypixel in xrange(ytile, ytile + 4):
				for xpixel in xrange(xtile, xtile + 4):
					
					if xpixel >= w or ypixel >= h:
						sspack(destBuffer, offset, 0)
						sspack(destBuffer, offset+32, 0)
					else:
						pixel = tex.pixel(xpixel, ypixel)
						
						sspack(destBuffer, offset, pixel>>16)
						sspack(destBuffer, offset+32, pixel&0xFFFF)
					offset += 2
			offset += 32

	return destBuffer.raw

def RGB5A3Encode(tex):
	tex = tex.toImage()
	w, h = tex.width(), tex.height()
	padW = (w + 3) & ~3
	padH = (h + 3) & ~3

	destBuffer = create_string_buffer(padW * padH * 2)

	shortstruct = struct.Struct('>H')
	sspack = shortstruct.pack_into
	offset = 0

	for ytile in xrange(0, padH, 4):
		for xtile in xrange(0, padW, 4):
			for ypixel in xrange(ytile, ytile + 4):
				for xpixel in xrange(xtile, xtile + 4):

					if xpixel >= w or ypixel >= h:
						rgbDAT = 0x7FFF
					else:
						pixel = tex.pixel(xpixel, ypixel)

						a = pixel >> 24
						r = (pixel >> 16) & 0xFF
						g = (pixel >> 8) & 0xFF
						b = pixel & 0xFF

						if a < 245: #RGB4A3
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

					sspack(destBuffer, offset, rgbDAT)
					offset += 2

	return destBuffer.raw

class KPMapExporter:
	class LayerExporter:
		def __init__(self, layer):
			self.layer = layer
	
	class TileLayerExporter(LayerExporter):
		def buildSectors(self, sectors, indices):
			# we'll use the cache held by the layer: why reinvent the wheel?
			layer = self.layer
			layer.updateCache()
			cache = layer.cache

			# first off, get all the info and figure out the sector bounds
			layerX, layerY = layer.cacheBasePos
			layerWidth, layerHeight = layer.cacheSize

			sectorLeft = layerX / 16
			sectorTop = layerY / 16
			sectorRight = (layerX + layerWidth - 1) / 16
			sectorBottom = (layerY + layerHeight - 1) / 16

			rawSectors = []
			for i in xrange(sectorBottom - sectorTop + 1):
				rawSectors.append([None for j in xrange(sectorRight - sectorLeft + 1)])

			tileset = KP.tileset(layer.tileset)
			optMappings = tileset.optMappings

			# copy every tile index over
			for srcY in xrange(layerHeight):
				srcRow = cache[srcY]
				worldY = srcY + layerY
				sectorY = worldY / 16
				destY = worldY % 16

				destRow = rawSectors[sectorY - sectorTop]

				for srcX in xrange(layerWidth):
					worldX = srcX + layerX
					sectorX = worldX / 16
					destX = worldX % 16

					tile = srcRow[srcX]
					if tile == -1: continue
					tile = optMappings[tile]
					if tile == -1: continue

					destSector = destRow[sectorX - sectorLeft]
					if destSector is None:
						destSector = [[-1 for j in xrange(16)] for i in xrange(16)]
						destRow[sectorX - sectorLeft] = destSector

					destSector[destY][destX] = tile

			# now add the created sectors to the data
			count = reduce(lambda x,y: x+len(y), rawSectors, 0)
			sectorMap = [0xFFFF for i in xrange(count)]
			destIdx = 0

			for srcRow in rawSectors:
				for sector in srcRow:
					if sector is not None:
						# see if it's a duplicate or not
						sectorKey = '|'.join(map(lambda x: ','.join(map(str, x)), sector))

						try:
							sectorMap[destIdx] = indices[sectorKey]
						except KeyError:
							indices[sectorKey] = len(sectors)
							sectorMap[destIdx] = len(sectors)
							sectors.append(sector)

					destIdx += 1

			self.sectorBounds = (sectorLeft, sectorTop, sectorRight, sectorBottom)
			self.realBounds = (layerX, layerY, layerX+layerWidth-1, layerY+layerHeight-1)
			self.sectorMap = sectorMap

	class DoodadLayerExporter(LayerExporter):
		pass

	class PathLayerExporter(LayerExporter):
		pass



	def __init__(self, mapObj):
		self.map = mapObj

		self.tileAssociates = {}
		self.doodadAssociates = {}

		output = []
		for layer in self.map.layers:
			if isinstance(layer, KPTileLayer) and len(layer.objects) > 0:
				output.append(KPMapExporter.TileLayerExporter(layer))

			elif isinstance(layer, KPDoodadLayer) and len(layer.objects) > 0:
				output.append(KPMapExporter.DoodadLayerExporter(layer))

			elif isinstance(layer, KPPathLayer):
				output.append(KPMapExporter.PathLayerExporter(layer))

				for iLayer in self.map.associateLayers:
					if len(iLayer.objects) > 0:
						tl = KPMapExporter.TileLayerExporter(iLayer)
						self.tileAssociates[iLayer.associate] = tl
						output.append(tl)

					if len(iLayer.doodads) > 0:
						dl = KPMapExporter.DoodadLayerExporter(iLayer)
						self.doodadAssociates[iLayer.associate] = dl
						output.append(dl)

		self.layers = output

	def build(self):
		requiredFixUps = []
		stringsToAdd = set()
		textures = set()
		texInfo = set()
		tilesets = set()
		offsets = {None: 0xFFFFFFFF}

		# first off, build the sectors
		sectors = []
		sectorIndices = {}

		for layer in self.layers:
			if isinstance(layer, self.TileLayerExporter):
				layer.buildSectors(sectors, sectorIndices)

		sectorData = self._packSectorData(sectors)

		# now that we've got that, we can pack the first part of the file
		version = 2
		headerSize = 0x2C
		tsInfoOffsetInHeader = 0x10
		data = bytearray(struct.pack('>4sIIIIIIIIII', 'KP_m', version, len(self.layers), headerSize + len(sectorData), 0, 0, 0, headerSize, 0, 0, len(self.map.worlds)))
		requiredFixUps.append((0x18, 'UnlockBytecode'))
		requiredFixUps.append((0x20, self.map.bgName))
		requiredFixUps.append((0x24, '_WorldDefList'))
		stringsToAdd.add(self.map.bgName)

		# list of layer pointers goes here.. or will, later
		data += sectorData

		for layer in self.layers:
			requiredFixUps.append((len(data), layer))
			data += zero32

		# now build the layers
		for eLayer in self.layers:
			layer = eLayer.layer

			offsets[eLayer] = len(data)
			offsets[layer] = len(data)

			if isinstance(eLayer, self.TileLayerExporter):
				data += u32.pack(0)
				data += u32.pack(0xFF000000)

				# tileset name
				tileset = '/Maps/Texture/%s.bin' % layer.tileset
				tilesets.add(tileset)
				stringsToAdd.add(tileset)
				requiredFixUps.append((len(data), ('tileset', tileset)))
				data += zero32

				# sector info
				data += struct.pack('>IIII', *eLayer.sectorBounds)
				data += struct.pack('>IIII', *eLayer.realBounds)
				data += ''.join(map(u16.pack, eLayer.sectorMap))

				pad = (4 - (len(data) & 3)) % 4
				data += ('\0' * pad)

			elif isinstance(eLayer, self.DoodadLayerExporter):
				data += u32.pack(1)
				data += u32.pack(0xFF000000)

				# doodad list
				try:
					doodadList = layer.doodads
				except AttributeError:
					doodadList = layer.objects

				data += u32.pack(len(doodadList))
				for doodad in doodadList:
					requiredFixUps.append((len(data), doodad))
					data += zero32

				# now pack them ...
				for doodad in doodadList:
					offsets[doodad] = len(data)

					x, y = doodad.position
					w, h = doodad.size
					data += struct.pack('>fffffii', x, y, w, h, doodad.angle, 0, len(doodad.animations))

					is_rgba8 = doodad.source[0].startswith('Cloud') or \
							doodad.source[0].startswith('Tiling_Cloud')

					texInfo.add((doodad.source[0], doodad.source[1].height() * doodad.source[1].width() * (4 if is_rgba8 else 2)))
					texture = doodad.source[1]
					textures.add((is_rgba8, texture))
					requiredFixUps.append((len(data) - 8, texture))

					for anim in doodad.animations:
						rLoop, rCurve, rFrames, rType, rStart, rEnd, rDelay, rDelayOffset = anim

						loopid = self.ANIM_LOOPS.index(rLoop)
						curveid = self.ANIM_CURVES.index(rCurve)
						typeid = self.ANIM_TYPES.index(rType)
						data += struct.pack('>iiiiiiiiii', loopid, curveid, rFrames, typeid, rStart, rEnd, rDelay, rDelayOffset, 0, 0)

			elif isinstance(eLayer, self.PathLayerExporter):
				data += u32.pack(2)
				data += zero32

				# before we do anything, build the list of secret levels
				# we'll need that
				levelsWithSecrets = set()

				for path in layer.paths:
					if hasattr(path, 'unlockSpec') and path.unlockSpec is not None:
						self._checkSpecForSecrets(path.unlockSpec, levelsWithSecrets)

				# lists
				current = len(data)
				nodeArray = current + 16
				pathArray = nodeArray + (len(layer.nodes) * 4)

				data += struct.pack('>IIII', len(layer.nodes), nodeArray, len(layer.paths), pathArray)

				for node in layer.nodes:
					requiredFixUps.append((len(data), node))
					data += zero32
				for path in layer.paths:
					requiredFixUps.append((len(data), path))
					data += zero32

				# now do the actual structs
				for node in layer.nodes:
					offsets[node] = len(data)

					x, y = node.position
					current = len(data)
					data += struct.pack('>hhiiiiii', x+12, y+12, 0, 0, 0, 0, 0, 0)

					# figure out the exits by direction
					exits = [None, None, None, None]
					left, right, up, down = 0, 1, 2, 3

					for exit in node.exits:
						start, end = exit._startNodeRef(), exit._endNodeRef()
						opposite = end if (start == node) else start

						oX, oY = opposite.position
						deltaX, deltaY = oX-x, oY-y
						angle = math.degrees(math.atan2(deltaX, deltaY)) % 360
						print "Here: %d,%d Opposite %d,%d Delta: %d,%d Angle: %d" % (x,y,oX,oY,deltaX,deltaY,angle)

						# Left = 270, Right = 90, Up = 180, Down = 0
						if angle >= 225 and angle <= 315:
							direction = left
						elif angle >= 45 and angle <= 135:
							direction = right
						elif angle > 135 and angle < 225:
							direction = up
						elif angle > 315 or angle < 45:
							direction = down

						if exits[direction]:
							print "Conflicting directions!"
							while exits[direction]:
								direction = (direction + 1) % 4

						exits[direction] = exit

					requiredFixUps.append((current+4, exits[0]))
					requiredFixUps.append((current+8, exits[1]))
					requiredFixUps.append((current+12, exits[2]))
					requiredFixUps.append((current+16, exits[3]))

					if node in self.tileAssociates:
						requiredFixUps.append((current+20, self.tileAssociates[node]))
					if node in self.doodadAssociates:
						requiredFixUps.append((current+24, self.doodadAssociates[node]))

					if node.isStop():
						if node.level:
							level1, level2 = node.level
							hasSecret = (1 if ((level1,level2) in levelsWithSecrets) else 0)
							# i i i b b b b: node type, isNew, Extra pointer, world, level, hasSecret, padding
							data += struct.pack('>iiibbbb', 2, 0, 0, level1, level2, hasSecret, 0)

						elif node.mapChange:
							data += u32.pack(3) # node type

							destMap = node.mapChange
							requiredFixUps.append((len(data)+8, destMap))
							stringsToAdd.add(destMap)

							# i i i b b b b: isNew, Extra pointer, dest map, map ID, foreign ID, transition, padding
							data += struct.pack('>iiibbbb', 0, 0, 0, node.mapID, node.foreignID, node.transition, 0)

						else:
							data += u32.pack(1) # node type
							data += zero32 # isNew
							data += zero32 # Extra pointer

					elif node.worldDefID != None:
						# i i i b b b b: node type, isNew, Extra pointer, world def ID, padding
						data += struct.pack('>iiibbbb', 4, 0, 0, node.worldDefID, 0, 0, 0)

					else:
						data += zero32 # node type
						data += zero32 # isNew
						data += zero32 # Extra pointer

				pathIndices = {}

				for i, path in enumerate(layer.paths):
					pathIndices[path] = i
					offsets[path] = len(data)

					start = path._startNodeRef()
					end = path._endNodeRef()
					current = len(data)

					requiredFixUps.append((current, start))
					requiredFixUps.append((current+4, end))
					if path in self.tileAssociates:
						requiredFixUps.append((current+8, self.tileAssociates[path]))
					if path in self.doodadAssociates:
						requiredFixUps.append((current+12, self.doodadAssociates[path]))

					data += (zero32 * 4)

					available = 0
					if (not hasattr(path, 'unlockSpec')) or path.unlockSpec is None:
						available = 3

					data += struct.pack('>bbbbfi', available, 0, 0, 0, path.movementSpeed, path.animation)

		# align it to 4 bytes before we write the world defs
		padding = ((len(data) + 4) & ~4) - len(data)
		data += ('\0' * padding)

		offsets['_WorldDefList'] = len(data)
		for world in self.map.worlds:
			requiredFixUps.append((len(data), world.name))
			stringsToAdd.add(world.name)
			data += zero32

			fst1,fst2 = world.fsTextColours
			fsh1,fsh2 = world.fsHintColours
			ht1,ht2 = world.hudTextColours
			htf = world.hudHintTransform

			try:
				convertedWorldID = int(world.worldID)
			except ValueError:
				convertedWorldID = ord(world.worldID) - ord('A') + 10

			parseCrap = world.titleScreenID.split('-')
			tsW = int(parseCrap[0])
			tsL = int(parseCrap[1])

			data += struct.pack('>BBBB BBBB BBBB BBBB BBBB BBBB hbb BBB BB BBB',
					fst1[0],fst1[1],fst1[2],fst1[3],
					fst2[0],fst2[1],fst2[2],fst2[3],
					fsh1[0],fsh1[1],fsh1[2],fsh1[3],
					fsh2[0],fsh2[1],fsh2[2],fsh2[3],
					ht1[0],ht1[1],ht1[2],ht1[3],
					ht2[0],ht2[1],ht2[2],ht2[3],
					htf[0],htf[1],htf[2],
					world.uniqueKey, world.musicTrackID,
					convertedWorldID,
					tsW - 1, tsL - 1,
					0, 0, 0
					)

		# now that we're almost done... pack the strings
		for string in stringsToAdd:
			offsets[string] = len(data)
			data += str(string)
			data += '\0'

		# textures
		texA = sorted(texInfo, key=lambda x: x[1])

		s = 0
		d = 0
		for texItem in texA:
			print texItem[1]/1000, "kb:", texItem[0]
			s += texItem[1]
			d += 1
			if d == 10:
				d = 0
				print "So far:", s/1000, "kb"

		print "Total:", s/1000, "kb"

		texPadding = ((len(data) + 0x1F) & ~0x1F) - len(data)
		data += ('\0' * texPadding)

		texHeaderStartOffset = len(data)
		texDataStartOffset = texHeaderStartOffset + ((len(textures) + len(tilesets)) * 0x20)

		currentTexOffset = texDataStartOffset
		
		imageData = []

		struct.pack_into('>ii', data, tsInfoOffsetInHeader, len(tilesets), len(data))
		for setname in tilesets:
			offsets[('tileset', setname)] = len(data)
			if 'RGBA8' in setname:
				data += self._buildGXTexObjRGBA8(896, 448, offsets[setname])
			else:
				data += self._buildGXTexObjRGB5A3(896, 448, offsets[setname])

		for is_rgba8, tex in textures:
			offsets[tex] = len(data)

			if is_rgba8:
				data += self._buildGXTexObjRGBA8(tex.width(), tex.height(), currentTexOffset)
				converted = RGBA8Encode(tex)
			else:
				data += self._buildGXTexObjRGB5A3(tex.width(), tex.height(), currentTexOffset)
				converted = RGB5A3Encode(tex)
			imageData.append(converted)
			currentTexOffset += len(converted)

		for piece in imageData:
			data += piece

		# at the end comes the unlock bytecode
		offsets['UnlockBytecode'] = len(data)

		# first off, build a map of unlocks
		unlockLists = {}
		
		from unlock import stringifyUnlockData

		for path in self.map.pathLayer.paths:
			if not hasattr(path, 'unlockSpec'):
				continue
			spec = path.unlockSpec
			if spec is None:
				continue

			# we stringify it first because the specs become lists when
			# imported from the kpmap (not tuples) and those can't be
			# used as dict keys
			spec = stringifyUnlockData(spec)
			try:
				lst = unlockLists[spec]
			except KeyError:
				lst = []
				unlockLists[spec] = lst
			lst.append(path)

		# now produce the thing
		from unlock import parseUnlockText, packUnlockSpec

		for spec, lst in unlockLists.iteritems():
			data += packUnlockSpec(parseUnlockText(spec))
			data += chr(len(lst))
			for p in lst:
				data += u16.pack(pathIndices[p])
		data += chr(0)

		# to finish up, correct every offset
		for offset, target in requiredFixUps:
			u32.pack_into(data, offset, offsets[target])

		return data

	ANIM_LOOPS = ['Contiguous', 'Loop', 'Reversible Loop']
	ANIM_CURVES = ['Linear', 'Sinusoidial', 'Cosinoidial']
	ANIM_TYPES = ['X Position', 'Y Position', 'Angle', 'X Scale', 'Y Scale', 'Opacity']

	def _checkSpecForSecrets(self, spec, levelSet):
		kind = spec[0]

		if kind == 'level':
			k, one, two, secret = spec
			if secret:
				levelSet.add((one, two))
		elif kind == 'and' or kind == 'or':
			for term in spec[1]:
				self._checkSpecForSecrets(term, levelSet)

	def _buildGXTexObjRGB5A3(self, width, height, imgOffset):
		# Format: RGB5A3 (5)
		# Wrap: CLAMP (0)
		return struct.pack('>IIIIIIIHH',
				0x90, 0,
				(0x500000 | ((height - 1) << 10) | (width - 1)),
				0x10000000 + imgOffset, # (imgptr >> 5)
				0, 0, 0,
				(((width + 3) / 4) * ((height + 3) / 4)) & 0x7FFF,
				0x0202
				)

	def _buildGXTexObjRGBA8(self, width, height, imgOffset):
		# Format: RGBA8 (6)
		# Wrap: CLAMP (0)
		return struct.pack('>IIIIIIIHH',
				0x90, 0,
				(0x600000 | ((height - 1) << 10) | (width - 1)),
				0x10000000 + imgOffset, # (imgptr >> 5)
				0, 0, 0,
				(((width + 3) / 4) * ((height + 3) / 4)) & 0x7FFF,
				0x0202
				)


	def _packSectorData(self, sectors):
		rowStruct = struct.Struct('>16h')
		output = []

		for sector in sectors:
			for row in sector:
				output.append(rowStruct.pack(*row))

		return ''.join(output)


