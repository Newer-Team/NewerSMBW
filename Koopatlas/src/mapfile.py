from common import *
import base64
import json

DUMPABLE_CLASSES_BY_NAME = {}
DUMPABLE_CLASS_NAMES = {}

DUMPABLE_PROXIES = {}
LOADABLE_PROXIES = {}

def dumpClassAs(cls, name=None):
	def __add_it(function):
		DUMPABLE_PROXIES[cls] = (name, function)
		return function
	return __add_it

def loadClassFrom(name):
	def __add_it(function):
		LOADABLE_PROXIES[name] = function
		return function
	return __add_it

def dumpable(name):
	def __add_it(cls):
		DUMPABLE_CLASSES_BY_NAME[name] = cls
		DUMPABLE_CLASS_NAMES[cls] = name
		return cls
	return __add_it

def dump(rootObj):
	def _dumpPiece(piece):
		try:
			clsObj = type(piece)
			clsName = DUMPABLE_CLASS_NAMES[clsObj]
		except KeyError:
			# let's give this one more shot with the dumpable proxies
			try:
				dumpName, dumpFunc = DUMPABLE_PROXIES[clsObj]
			except KeyError:
				return piece

			dest = dumpFunc(piece)
			if dumpName is not None:
				dest['_t'] = dumpName
			return dest


		dest = {'_t': clsName}

		for attrName in clsObj.__dump_attribs__:
			dest[attrName] = getattr(piece, attrName)

		if hasattr(piece, '_dump'):
			piece._dump(rootObj, dest)

		return dest

	return json.dumps(rootObj, default=_dumpPiece)



def load(string):
	needsSpecialCare = []

	def _loadObject(source):
		try:
			clsName = source['_t']
			clsObj = DUMPABLE_CLASSES_BY_NAME[clsName]
		except KeyError:
			# let's give this one more shot with the loadable proxies
			try:
				loadFunc = LOADABLE_PROXIES[clsName]
			except KeyError:
				return source
			
			return loadFunc(source)

		obj = clsObj()

		for attrName in clsObj.__dump_attribs__:
			try:
				setattr(obj, attrName, source[attrName])
			except:
				print "{0} was missing the attribute {1}".format(obj, attrName)
		
		if hasattr(obj, '_preload'):
			obj._preload(source)
		
		if hasattr(obj, '_load'):
			needsSpecialCare.append((obj, source))

		return obj
	
	root = json.loads(string, object_hook=_loadObject)

	for obj, source in needsSpecialCare:
		obj._load(root, source)
	
	return root




@dumpClassAs(QtGui.QPixmap, 'pixmap')
def dumpPixmap(pm):
	buf = QtCore.QBuffer()
	buf.open(buf.WriteOnly)
	pm.save(buf, 'PNG')
	data = str(buf.data())
	buf.close()
	return {'png': base64.b64encode(data)}

@loadClassFrom('pixmap')
def loadPixmap(source):
	pm = QtGui.QPixmap()
	pm.loadFromData(base64.b64decode(source['png']), 'PNG')
	return pm

@dumpClassAs(QtCore.QString)
def dumpQString(string):
	return str(string)

