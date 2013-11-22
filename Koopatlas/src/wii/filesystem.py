import weakref

class WiiFSObject(object):
	def __init__(self):
		self._parent_ref = None
		self._name = ''
		self._low_name = ''
	
	@property
	def parent(self):
		ref = self._parent_ref
		return (ref() if ref else None)

	@parent.setter
	def parent(self, value):
		self._parent_ref = weakref.ref(value)
	
	@parent.deleter
	def parent(self):
		self._parent_ref = None
	

	@property
	def name(self):
		return self._name

	@name.setter
	def name(self, value):
		self._name = value
		self._low_name = value.lower()

	
	def unlinkFromParent(self):
		if self.parent == None:
			return

		if self.parent.isDirectory():
			self.parent.children.remove(self)

		del self.parent

	def isFile(self): return False
	def isDirectory(self): return False


class WiiFile(WiiFSObject):
	def __init__(self):
		WiiFSObject.__init__(self)
		self.data = None
	
	def isFile(self): return True


class WiiDirectory(WiiFSObject):
	def __init__(self):
		WiiFSObject.__init__(self)
		self.children = []
	
	def isDirectory(self): return True

	def findByName(self, name, recursive):
		return self._findByName(name.lower(), recursive)

	def _findByName(self, lowercase_name, recursive):
		for obj in self.children:
			if obj._low_name == lowercase_name:
				return obj

			if recursive and obj.isDirectory():
				tryThis = obj._findByName(lowercase_name, recursive)
				if tryThis:
					return tryThis

		return None

	def resolvePath(self, path, createIfNotExists=False):
		components = path.split('/')
		currentDir = self

		# special case: handle absolute paths
		if components[0] == '':
			while currentDir.parent:
				currentDir = currentDir.parent

			del components[0]

		while len(components) > 0:
			bit = components.pop(0)
			nextObj = None

			if bit == '.':
				nextObj = currentDir
			elif bit == '..':
				nextObj = currentDir.parent
			else:
				nextObj = currentDir.findByName(bit, False)

			if nextObj is None:
				if not createIfNotExists:
					print("failed to resolve path %s: missing component %s" % (path, bit))
					return None
				elif bit != '..':
					# we must create it!
					if len(components) == 0:
						nextObj = WiiFile()
					else:
						nextObj = WiiDirectory()

					nextObj.name = bit
					currentDir.addChild(nextObj)

			if len(components) == 0:
				return nextObj

			if not nextObj.isDirectory():
				print("failed to resolve path %s: component %s is not a directory" % (path, bit))
				return None

			# has to be a directory so just continue
			currentDir = nextObj

		print("wtf? %s" % path)
		return None

	def addChild(self, obj):
		if self.findByName(obj.name, False):
			return False

		obj.unlinkFromParent()
		self.children.append(obj)
		obj.parent = self

		return True

