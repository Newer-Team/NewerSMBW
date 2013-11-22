from common import WiiStringTableBuilder, alignUp, alignDown
from filesystem import *
import struct
import os
import cStringIO

class WiiArchiveU8:
	class ReadInfo:
		# startPos, stringTable, currentNode
		pass
	
	class WriteInfo:
		# startPos, stringTableBuilder, currentRecursionLevel,
		# currentNode, nextDataOffset
		pass

	def __init__(self, handle=None):
		self.root = WiiDirectory()

		if handle:
			if not hasattr(handle, 'read'):
				if handle[0:4] == "U\xAA8\x2D":
					handle = cStringIO.StringIO(handle)
				else:
					handle = open(handle, 'rb')

			info = WiiArchiveU8.ReadInfo()
			info.startPos = handle.tell()

			magic = handle.read(4)
			if magic != "U\xAA8\x2D":
				print("WiiArchiveU8: tried to load an archive without the U8 magic")

			fstStart, fstSize, dataOffset = struct.unpack('>III', handle.read(12))

			# read the FST
			handle.seek(info.startPos + fstStart)

			# read the root node
			handle.seek(8, os.SEEK_CUR) # ignore type, nameOffset and dataOffset
			rootNodeLastChild = struct.unpack('>I', handle.read(4))[0]

			# but before we do this, read the string table
			savePos = handle.tell()
			handle.seek(savePos + ((rootNodeLastChild - 1) * 12))

			stringTableLength = fstSize - (rootNodeLastChild * 12)
			info.stringTable = handle.read(stringTableLength)
			
			# now read the root node
			handle.seek(savePos)

			info.currentNode = 1
			self._readDir(handle, self.root, rootNodeLastChild, info)
	
	def _readDir(self, handle, directory, lastChild, info):
		# read every node in this directory
		while info.currentNode < lastChild:
			info.currentNode += 1

			value, dataOffset, size = struct.unpack('>III', handle.read(12))

			nameOffset = value & 0xFFFFFF
			objType = value >> 24

			if objType == 0:
				newObj = WiiFile()
			elif objType == 1:
				newObj = WiiDirectory()
			else:
				raise "oh crap! unknown FS obj type %d" % objType

			nameEnd = info.stringTable.find("\0", nameOffset)
			newObj.name = info.stringTable[nameOffset:nameEnd]

			if newObj.isFile():
				savePos = handle.tell()
				handle.seek(info.startPos + dataOffset)
				newObj.data = handle.read(size)
				handle.seek(savePos)

			elif newObj.isDirectory():
				self._readDir(handle, newObj, size, info)

			directory.addChild(newObj)

	def pack(self, handle=None):
		returnData = False
		if handle is None:
			handle = cStringIO.StringIO()
			returnData = True

		info = WiiArchiveU8.WriteInfo()

		# first off, before we do anything else, create the string table
		info.stringTableBuilder = WiiStringTableBuilder()
		self._addNodeToStringTable(self.root, info.stringTableBuilder)

		stringTable = info.stringTableBuilder.data

		# calculate various fun offsets/sizes
		nodeCount = self._countNode(self.root)
		info.startPos = handle.tell()

		fstStart = 0x20
		nodeDataSize = nodeCount * 12
		stringTableSize = len(stringTable)
		fstSize = nodeDataSize + stringTableSize
		dataOffset = alignUp(fstStart + fstSize, 0x20)

		# now write the header
		handle.write("U\xAA8\x2D")
		handle.write(struct.pack('>III', fstStart, fstSize, dataOffset))
		handle.write("\0"*16)

		# write root node
		info.currentNode = 1
		info.currentRecursionLevel = 0
		info.nextDataOffset = dataOffset

		handle.write(struct.pack('>III',
			0x01000000 | info.stringTableBuilder.add(''),
			0, nodeCount))

		self._writeDir(handle, self.root, info)

		# write string table
		handle.write(stringTable)

		# write data (after padding)
		handle.write("\0" * (dataOffset - fstSize - fstStart))
		self._writeNodeData(handle, self.root)

		# looks like we are finally done
		if returnData:
			data = handle.getvalue()
			handle.close()
			return data

	def _addNodeToStringTable(self, node, table):
		table.add(node.name)

		if node.isDirectory():
			for obj in node.children:
				self._addNodeToStringTable(obj, table)

	def _countNode(self, node):
		if node.isDirectory():
			return 1 + sum(map(self._countNode, node.children))
		else:
			return 1

	def _writeDir(self, handle, directory, info):
		for obj in directory.children:
			info.currentNode += 1

			if obj.isDirectory():
				handle.write(struct.pack('>III',
					0x01000000 | info.stringTableBuilder.add(obj.name),
					info.currentRecursionLevel, 0))

				lastChildFieldPos = handle.tell() - 4

				info.currentRecursionLevel += 1
				self._writeDir(handle, obj, info)
				info.currentRecursionLevel -= 1

				# write lastChild field
				dirEndPos = handle.tell()

				handle.seek(lastChildFieldPos)
				handle.write(struct.pack('>I', info.currentNode))
				handle.seek(dirEndPos)

			elif obj.isFile():
				handle.write(struct.pack('>III',
					info.stringTableBuilder.add(obj.name),
					info.nextDataOffset, len(obj.data)))

				info.nextDataOffset = alignUp(info.nextDataOffset + len(obj.data), 0x20)


	def _writeNodeData(self, handle, node):
		if node.isDirectory():
			for obj in node.children:
				self._writeNodeData(handle, obj)
			
		elif node.isFile():
			size = len(node.data)
			handle.write(node.data)
			handle.write("\0" * (alignUp(size, 0x20) - size))
	

	def resolvePath(self, *args):
		return self.root.resolvePath(*args)

