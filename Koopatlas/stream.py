import struct

BIG_ENDIAN, LITTLE_ENDIAN = range(2)

def _create(s):
	return (struct.Struct('>'+s), struct.Struct('<'+s))

u64 = _create('Q')
u32 = _create('I')
u16 = _create('H')
u8 = _create('B')

s64 = _create('q')
s32 = _create('i')
s16 = _create('h')
s8 = _create('b')

f32 = _create('f')
f64 = _create('d')

del _create




class DataInputStream:
	def __init__(self, data, endian=BIG_ENDIAN):
		self.data = data
		self.length = len(data)
		self.pos = 0
		self.endian = endian
	
	def seek(self, pos):
		assert pos >= 0 and pos <= self.length
		self.pos = pos
	
	def skip(self, count):
		new_pos = self.pos + count
		assert new_pos >= 0 and new_pos <= self.length
		self.pos = new_pos
	
	def read_bytes(self, count):
		new_pos = self.pos + count
		assert new_pos >= 0 and new_pos <= self.length
		data = self.data[self.pos:new_pos]
		self.pos = new_pos
		return data
	
	def read_u8(self):
		return u8[self.endian].unpack(self.read_bytes(1))[0]
	
	def read_u16(self):
		return u16[self.endian].unpack(self.read_bytes(2))[0]
	
	def read_u32(self):
		return u32[self.endian].unpack(self.read_bytes(4))[0]
	
	def read_s64(self):
		return u64[self.endian].unpack(self.read_bytes(8))[0]
	
	def read_s8(self):
		return s8[self.endian].unpack(self.read_bytes(1))[0]
	
	def read_s16(self):
		return s16[self.endian].unpack(self.read_bytes(2))[0]
	
	def read_s32(self):
		return s32[self.endian].unpack(self.read_bytes(4))[0]
	
	def read_s64(self):
		return s64[self.endian].unpack(self.read_bytes(8))[0]
	
	def read_float(self):
		return f32[self.endian].unpack(self.read_bytes(4))[0]
	
	def read_double(self):
		return f64[self.endian].unpack(self.read_bytes(8))[0]
	
	
	def at(self, pos):
		copy = DataInputStream(self.data, self.endian)
		copy.seek(pos)
		return copy

