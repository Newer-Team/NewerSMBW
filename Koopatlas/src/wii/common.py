class WiiStringTableBuilder(object):
	def __init__(self):
		self.nextOffset = 0
		self.data = ''
		self.lookup = {}
	
	def add(self, string):
		if string in self.lookup:
			return self.lookup[string]

		offset = self.nextOffset
		self.lookup[string] = offset
		
		self.data = "%s%s\0" % (self.data, string.encode('Shift-JIS'))
		self.nextOffset = len(self.data)

		return offset


def alignUp(value, alignTo):
	return (value + alignTo - 1) & ~(alignTo - 1)

def alignDown(value, alignTo):
	return value & ~(alignTo - 1)

