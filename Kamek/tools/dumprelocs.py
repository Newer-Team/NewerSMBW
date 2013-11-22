import struct
up = struct.Struct('>II').unpack
u32 = struct.Struct('>I').unpack

data = open('NewerASM/pal_dlrelocs.bin', 'rb').read()
addr_offset = struct.unpack('>xxxxxxxxI', data[0:0xC])[0] - 0xC
data = data[0xC:]

reloc_count = addr_offset / 8

for i in xrange(reloc_count):
	entry = up(data[i*8:i*8+8])
	reltype = entry[0] >> 24
	addr_id = entry[0] & 0xFFFFFF
	offset = entry[1]

	offs = addr_offset+(addr_id*4)
	print "%2d: %08x => %08x" % (reltype, offset, u32(data[offs:offs+4])[0])

