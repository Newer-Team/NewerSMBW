from common import *

class KP:
	@staticmethod
	def run():
		KP.app = QtGui.QApplication(sys.argv)

		KP.app.settings = QtCore.QSettings('Koopatlas', 'Newer Team')

		from mapdata import KPMap
		KP.map = KPMap()

		from ui import KPMainWindow
		
		KP.mainWindow = KPMainWindow()
		KP.mainWindow.show()

		KP.enumerateTilesets()

		KP.app.exec_()


	@classmethod
	def icon(cls, name):
		try:
			cache = cls.iconCache
		except AttributeError:
			cache = {}
			cls.iconCache = cache

		try:
			return cache[name]
		except KeyError:
			icon = QtGui.QIcon('Resources/%s.png' % name)
			cache[name] = icon
			return icon


	@classmethod
	def enumerateTilesets(cls):
		try:
			registry = cls.knownTilesets
		except AttributeError:
			registry = {}
			cls.knownTilesets = registry
			cls.loadedTilesets = {}

		path = os.path.join(os.getcwd(), 'Tilesets')
		if not os.path.exists(path):
			os.mkdir(path)

		for file in os.listdir(path):
			name = file[:-4]

			if file.endswith('.arc'):
				filepath = os.path.join(path, file)
				registry[name] = {'path': filepath}


	@classmethod
	def loadTileset(cls, name):
		from hashlib import sha256 as sha

		if name in cls.loadedTilesets:
			return

		filepath = cls.knownTilesets[name]['path']
		data = open(filepath, 'rb').read()

		tsInfo = cls.knownTilesets[name]
		newHash = sha(data).hexdigest()
		if 'hash' in tsInfo and tsInfo['hash'] == newHash:
			# file hasn't changed
			return

		tsInfo['hash'] = newHash

		from tileset import KPTileset

		import time
		b = time.clock()

		cls.loadedTilesets[name] = KPTileset.loadFromArc(data)

		e = time.clock()
		print "Loading set: {0} in {1}".format(name, e-b)


	@classmethod
	def tileset(cls, name):
		cache = cls.loadedTilesets

		try:
			return cache[name]
		except KeyError:
			cls.loadTileset(name)
			return cache[name]

