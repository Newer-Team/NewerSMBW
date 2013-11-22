import os.path, struct, sys


class StructType(tuple):
	def __getitem__(self, value):
		return [self] * value
	def __call__(self, value, endian='<'):
		if isinstance(value, str):
			return struct.unpack(endian + tuple.__getitem__(self, 0), value[:tuple.__getitem__(self, 1)])[0]
		else:
			return struct.pack(endian + tuple.__getitem__(self, 0), value)

class StructException(Exception):
	pass

class Struct(object):
	__slots__ = ('__attrs__', '__baked__', '__defs__', '__endian__', '__next__', '__sizes__', '__values__')
	int8 = StructType(('b', 1))
	uint8 = StructType(('B', 1))
	
	int16 = StructType(('h', 2))
	uint16 = StructType(('H', 2))
	
	int32 = StructType(('l', 4))
	uint32 = StructType(('L', 4))
	
	int64 = StructType(('q', 8))
	uint64 = StructType(('Q', 8))
	
	float = StructType(('f', 4))

	def string(cls, len, offset=0, encoding=None, stripNulls=False, value=''):
		return StructType(('string', (len, offset, encoding, stripNulls, value)))
	string = classmethod(string)
	
	LE = '<'
	BE = '>'
	__endian__ = '<'
	
	def __init__(self, func=None, unpack=None, **kwargs):
		self.__defs__ = []
		self.__sizes__ = []
		self.__attrs__ = []
		self.__values__ = {}
		self.__next__ = True
		self.__baked__ = False
		
		if func == None:
			self.__format__()
		else:
			sys.settrace(self.__trace__)
			func()
			for name in func.func_code.co_varnames:
				value = self.__frame__.f_locals[name]
				self.__setattr__(name, value)
		
		self.__baked__ = True
		
		if unpack != None:
			if isinstance(unpack, tuple):
				self.unpack(*unpack)
			else:
				self.unpack(unpack)
		
		if len(kwargs):
			for name in kwargs:
				self.__values__[name] = kwargs[name]
	
	def __trace__(self, frame, event, arg):
		self.__frame__ = frame
		sys.settrace(None)
	
	def __setattr__(self, name, value):
		if name in self.__slots__:
			return object.__setattr__(self, name, value)
		
		if self.__baked__ == False:
			if not isinstance(value, list):
				value = [value]
				attrname = name
			else:
				attrname = '*' + name
			
			self.__values__[name] = None
			
			for sub in value:
				if isinstance(sub, Struct):
					sub = sub.__class__
				try:
					if issubclass(sub, Struct):
						sub = ('struct', sub)
				except TypeError:
					pass
				type_, size = tuple(sub)
				if type_ == 'string':
					self.__defs__.append(Struct.string)
					self.__sizes__.append(size)
					self.__attrs__.append(attrname)
					self.__next__ = True
					
					if attrname[0] != '*':
						self.__values__[name] = size[3]
					elif self.__values__[name] == None:
						self.__values__[name] = [size[3] for val in value]
				elif type_ == 'struct':
					self.__defs__.append(Struct)
					self.__sizes__.append(size)
					self.__attrs__.append(attrname)
					self.__next__ = True
					
					if attrname[0] != '*':
						self.__values__[name] = size()
					elif self.__values__[name] == None:
						self.__values__[name] = [size() for val in value]
				else:
					if self.__next__:
						self.__defs__.append('')
						self.__sizes__.append(0)
						self.__attrs__.append([])
						self.__next__ = False
					
					self.__defs__[-1] += type_
					self.__sizes__[-1] += size
					self.__attrs__[-1].append(attrname)
					
					if attrname[0] != '*':
						self.__values__[name] = 0
					elif self.__values__[name] == None:
						self.__values__[name] = [0 for val in value]
		else:
			try:
				self.__values__[name] = value
			except KeyError:
				raise AttributeError(name)
	
	def __getattr__(self, name):
		if self.__baked__ == False:
			return name
		else:
			try:
				return self.__values__[name]
			except KeyError:
				raise AttributeError(name)
	
	def __len__(self):
		ret = 0
		arraypos, arrayname = None, None
		
		for i in range(len(self.__defs__)):
			sdef, size, attrs = self.__defs__[i], self.__sizes__[i], self.__attrs__[i]
			
			if sdef == Struct.string:
				size, offset, encoding, stripNulls, value = size
				if isinstance(size, str):
					size = self.__values__[size] + offset
			elif sdef == Struct:
				if attrs[0] == '*':
					if arrayname != attrs:
						arrayname = attrs
						arraypos = 0
					size = len(self.__values__[attrs[1:]][arraypos])
				size = len(self.__values__[attrs])
			
			ret += size
		
		return ret
	
	def unpack(self, data, pos=0):
		for name in self.__values__:
			if not isinstance(self.__values__[name], Struct):
				self.__values__[name] = None
			elif self.__values__[name].__class__ == list and len(self.__values__[name]) != 0:
				if not isinstance(self.__values__[name][0], Struct):
					self.__values__[name] = None
		
		arraypos, arrayname = None, None
		
		for i in range(len(self.__defs__)):
			sdef, size, attrs = self.__defs__[i], self.__sizes__[i], self.__attrs__[i]
			
			if sdef == Struct.string:
				size, offset, encoding, stripNulls, value = size
				if isinstance(size, str):
					size = self.__values__[size] + offset
				
				temp = data[pos:pos+size]
				if len(temp) != size:
					raise StructException('Expected %i byte string, got %i' % (size, len(temp)))
				
				if encoding != None:
					temp = temp.decode(encoding)
				
				if stripNulls:
					temp = temp.rstrip('\0')
				
				if attrs[0] == '*':
					name = attrs[1:]
					if self.__values__[name] == None:
						self.__values__[name] = []
					self.__values__[name].append(temp)
				else:
					self.__values__[attrs] = temp
				pos += size
			elif sdef == Struct:
				if attrs[0] == '*':
					if arrayname != attrs:
						arrayname = attrs
						arraypos = 0
					name = attrs[1:]
					self.__values__[attrs][arraypos].unpack(data, pos)
					pos += len(self.__values__[attrs][arraypos])
					arraypos += 1
				else:
					self.__values__[attrs].unpack(data, pos)
					pos += len(self.__values__[attrs])
			else:
				values = struct.unpack(self.__endian__+sdef, data[pos:pos+size])
				pos += size
				j = 0
				for name in attrs:
					if name[0] == '*':
						name = name[1:]
						if self.__values__[name] == None:
							self.__values__[name] = []
						self.__values__[name].append(values[j])
					else:
						self.__values__[name] = values[j]
					j += 1
		
		return self
	
	def pack(self):
		arraypos, arrayname = None, None
		
		ret = ''
		for i in range(len(self.__defs__)):
			sdef, size, attrs = self.__defs__[i], self.__sizes__[i], self.__attrs__[i]
			
			if sdef == Struct.string:
				size, offset, encoding, stripNulls, value = size
				if isinstance(size, str):
					size = self.__values__[size]+offset
				
				if attrs[0] == '*':
					if arrayname != attrs:
						arraypos = 0
						arrayname = attrs
					temp = self.__values__[attrs[1:]][arraypos]
					arraypos += 1
				else:
					temp = self.__values__[attrs]
				
				if encoding != None:
					temp = temp.encode(encoding)
				
				temp = temp[:size]
				ret += temp + ('\0' * (size - len(temp)))
			elif sdef == Struct:
				if attrs[0] == '*':
					if arrayname != attrs:
						arraypos = 0
						arrayname = attrs
					ret += self.__values__[attrs[1:]][arraypos].pack()
					arraypos += 1
				else:
					ret += self.__values__[attrs].pack()
			else:
				values = []
				for name in attrs:
					if name[0] == '*':
						if arrayname != name:
							arraypos = 0
							arrayname = name
						values.append(self.__values__[name[1:]][arraypos])
						arraypos += 1
					else:
						values.append(self.__values__[name])
				
				ret += struct.pack(self.__endian__+sdef, *values)
		return ret
	
	def __getitem__(self, value):
		return [('struct', self.__class__)] * value


class WiiObject(object):
	def load(cls, data, *args, **kwargs):
		self = cls()
		self._load(data, *args, **kwargs)
		return self
	load = classmethod(load)

	def loadFile(cls, filename, *args, **kwargs):
		return cls.load(open(filename, "rb").read(), *args, **kwargs)
	loadFile = classmethod(loadFile)

	def dump(self, *args, **kwargs):
		return self._dump(*args, **kwargs)
	def dumpFile(self, filename, *args, **kwargs):
		open(filename, "wb").write(self.dump(*args, **kwargs))
		return filename


class WiiArchive(WiiObject):
	def loadDir(cls, dirname):
		self = cls()
		self._loadDir(dirname)
		return self
	loadDir = classmethod(loadDir)
		
	def dumpDir(self, dirname):
		if(not os.path.isdir(dirname)):
			os.mkdir(dirname)
		self._dumpDir(dirname)
		return dirname


class WiiHeader(object):
	def __init__(self, data):
		self.data = data
	def addFile(self, filename):
		open(filename, "wb").write(self.add())
	def removeFile(self, filename):
		open(filename, "wb").write(self.remove())
	def loadFile(cls, filename, *args, **kwargs):
		return cls(open(filename, "rb").read(), *args, **kwargs)
	loadFile = classmethod(loadFile)



def align(x, boundary):
	while x % boundary != 0:
		x += 1
	return x
	
def clamp(var, min, max):
	if var < min: var = min
	if var > max: var = max
	return var

def abs(var):
	if var < 0:
		var = var + (2 * var)
	return var

def hexdump(s, sep=" "): # just dumps hex values
	return sep.join(map(lambda x: "%02x" % ord(x), s))
		
def hexdump2(src, length = 16): # dumps to a "hex editor" style output
	result = []
	for i in xrange(0, len(src), length):
		s = src[i:i + length]
		if(len(s) % 4 == 0):
			mod = 0 
		else: 
			mod = 1 
		hexa = ''
		for j in range((len(s) / 4) + mod):
			hexa += ' '.join(["%02X" % ord(x) for x in s[j * 4:j * 4 + 4]])
			if(j != ((len(s) / 4) + mod) - 1):
				hexa += '  '
		printable = s.translate(''.join([(len(repr(chr(x))) == 3) and chr(x) or '.' for x in range(256)]))
		result.append("0x%04X   %-*s   %s\n" % (i, (length * 3) + 2, hexa, printable))
	return ''.join(result)
