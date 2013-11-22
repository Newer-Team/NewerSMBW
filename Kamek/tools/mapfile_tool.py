# half assed

# d_profile: same place
# d_bases: same place
# d_enemies and part of d_bases: offset by 0x20
# d_en_boss: offset by 0x20

def read_num(idx, s):
	num = 0

	while True:
		num = (num * 10) + int(s[idx])
		idx += 1
		if not s[idx].isdigit():
			return idx,num

def read_vl_string(idx, name):
	idx, slen = read_num(idx, name)
	s = name[idx:idx+slen]
	idx += slen
	return idx, s

def read_class_name(idx, name):
	if name[idx] == 'Q':
		ns_elem_ct = int(name[idx + 1])
		ns_elems = []
		idx += 2
		for i in xrange(ns_elem_ct):
			idx, n = read_vl_string(idx, name)
			ns_elems.append(n)
		return idx, ns_elems
	else:
		idx, n = read_vl_string(idx, name)
		return idx, [n]

class BuiltInType(object):
	no_compress = True
	def __init__(self, char, name):
		self.char = char
		self.name = name
	def output(self):
		output.append(self.char)
	def __str__(self):
		return self.name

def Char(): return BuiltInType('c', 'char')
def UChar(): return BuiltInType('h', 'unsigned char')
def Short(): return BuiltInType('s', 'short')
def UShort(): return BuiltInType('t', 'unsigned short')
def Int(): return BuiltInType('i', 'int')
def UInt(): return BuiltInType('j', 'unsigned int')
def Long(): return BuiltInType('l', 'long')
def ULong(): return BuiltInType('m', 'unsigned long')
def Float(): return BuiltInType('f', 'float')
def Double(): return BuiltInType('d', 'double')
def WChar(): return BuiltInType('w', 'wchar_t')
def Void(): return BuiltInType('v', 'void')
def Bool(): return BuiltInType('b', 'bool')
def Ellipsis(): return BuiltInType('z', '...')

class Reference(object):
	def __init__(self, thing):
		self.thing = thing
	def output(self):
		if compress(self): return
		output.append('R')
		self.thing.output()
		add_compress(self.thing)
	def __str__(self):
		return '%s&' % self.thing

class Pointer(object):
	def __init__(self, thing):
		self.thing = thing
	def output(self):
		if compress(self): return
		output.append('P')
		self.thing.output()
		add_compress(self.thing)
	def __str__(self):
		return '%s*' % self.thing

SIMPLE_NAMES = {
	'__ct': 'C1',
	'__dt': 'D1',
	'__nw': 'nw',
	'__dl': 'dl',
	'__nwa': 'na',
	'__dla': 'da',
}

class Name(object):
	def __init__(self, name):
		self.name = name
	def output(self, trash_me=False, include_const=False):
		if include_const: raise 'OH NO'
		if compress(self): return

		try:
			output.append(SIMPLE_NAMES[self.name])
		except KeyError:
			output.append(str(len(self.name)) + self.name)

	def __str__(self):
		return self.name

class NestedName(object):
	def __init__(self, *bits):
		if len(bits) > 2:
			self.parent = NestedName(*bits[:-1])
		else:
			self.parent = bits[0]
		self.name = bits[-1]
	def output(self, nested=False, include_const=False):
		if isinstance(self.name, Name) and self.name.name == '__vt':
			# a hack.
			output.append('TV')
			self.parent.output()
			return

		if compress(self): return

		if not nested:
			output.append('N')
			if include_const:
				output.append('K')

		self.parent.output(True)
		self.name.output()

		if not nested:
			output.append('E')
		add_compress(self.parent)

	def __str__(self):
		return str(self.parent) + '::' + str(self.name)

class Function(object):
	def __init__(self, name, params, is_const=False):
		self.name = name
		self.params = params
		self.is_const = is_const
	def output(self):
		self.name.output(include_const=self.is_const)
		for param in self.params:
			param.output()
			add_compress(param)
	def __str__(self):
		param_str = ', '.join(map(str, self.params))
		const_str = ' const' if self.is_const else ''
		return '%s(%s)%s' % (self.name, param_str, const_str)

class GenericFunction(object):
	def __init__(self, ret_type, params):
		self.ret_type = ret_type
		self.params = params
	def output(self):
		if compress(self): return
		output.append('F')

		self.ret_type.output()
		add_compress(self.ret_type)

		for param in self.params:
			param.output()
			add_compress(param)

		output.append('E')
	def __str__(self):
		param_str = ', '.join(map(str, self.params))
		return '%s ()(%s)' % (self.ret_type, param_str)

class Array(object):
	def __init__(self, elem_type, size):
		self.elem_type = elem_type
		self.size = size
	def output(self):
		if compress(self): return
		output.append('A')
		output.append(str(self.size))
		output.append('_')
		self.elem_type.output()
		add_compress(self.elem_type)
	def __str__(self):
		return '%s[%d]' % (self.elem_type, self.size)

class Const(object):
	def __init__(self, thing):
		self.thing = thing
	def output(self):
		if compress(self): return
		output.append('K')
		self.thing.output()
		add_compress(self.thing)
	def __str__(self):
		return '%s const' % self.thing

def canonicalise_compress(obj):
	if isinstance(obj, str):
		return obj
	else:
		return str(obj)
def add_compress(piece):
	if hasattr(piece, 'no_compress'):
		return
	piece = canonicalise_compress(piece)
	if piece not in comp_bits:
		comp_bits.append(piece)

compress_ids = '0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ'
def pack_compress_idx(idx):
	if idx == 0:
		return 'S_'
	else:
		return 'S%s_' % compress_ids[idx - 1]

def compress(piece):
	piece = canonicalise_compress(piece)
	try:
		idx = comp_bits.index(piece)
		output.append(pack_compress_idx(idx))
		return True
	except ValueError:
		# not found
		return False

def mangle(thing):
	global comp_bits, output
	comp_bits = []
	output = []
	thing.output()
	return '_Z' + ''.join(output)

# fname = NestedName(Name('ActivePhysics'), Name('InitWithStruct'))
# info = NestedName(Name('ActivePhysics'), Name('Info'))
# actor = Name('dActor_c')
# params = [Pointer(Const(info)), Pointer(actor)]
# func = Function(fname, params)
# print mangle(func)

# fname = NestedName(Name('mMtx'), Name('applyRotationYXZ'))
# ps = Pointer(Short())
# func = Function(fname, [ps,ps,ps])
# print mangle(func)

# fname = NestedName(Name('a'), Name('b'), Name('c'))
# print mangle(fname)

# import sys
# sys.exit()


def read_type(idx, name, unsigned=False):
	char = name[idx]
	idx += 1

	if char == 'U':
		return read_type(idx, name, True)
	elif char.isdigit() or char == 'Q':
		idx -= 1
		idx, t = read_class_name(idx, name)
		if len(t) == 1:
			return idx, Name(t[0])
		else:
			return idx, NestedName(*map(Name, t))
	elif char == 'P':
		idx, t = read_type(idx, name)
		return idx, Pointer(t)
	elif char == 'R':
		idx, t = read_type(idx, name)
		return idx, Reference(t)
	elif char == 'C' or char == 'K':
		idx, t = read_type(idx, name)
		return idx, Const(t)
	elif char == 'v':
		return idx, Void()
	elif char == 'b':
		return idx, Bool()
	elif char == 'f':
		return idx, Float()
	elif char == 'd':
		return idx, Double()
	elif char == 'w':
		return idx, WChar()
	elif char == 'c':
		return idx, (UChar if unsigned else Char)()
	elif char == 's':
		return idx, (UShort if unsigned else Short)()
	elif char == 'i':
		return idx, (UInt if unsigned else Int)()
	elif char == 'l':
		return idx, (ULong if unsigned else Long)()
	elif char == 'e':
		return idx, Ellipsis()
	elif char == 'F':
		idx -= 1
		idx, params = read_func_params(idx, name)
		idx, ret_type = read_type(idx, name)
		return idx, GenericFunction(ret_type, params)
	elif char == 'A':
		idx, size = read_num(idx, name)
		assert name[idx] == '_'
		idx += 1
		idx, atype = read_type(idx, name)
		return idx, Array(atype, size)
	else:
		print 'Unknown: ' + char + ' @ ' + name
		return idx, None

def read_func_params(idx, name):
	assert name[idx] == 'F'
	idx += 1
	params = []

	while idx < len(name):
		if name[idx] == '_':
			idx += 1
			break
		idx, t = read_type(idx, name)
		params.append(t)

	return idx, params

def from_cw(name):
	split_offs = name.rfind('__')
	if split_offs > 0:
		global comp_bits
		comp_bits = []

		is_const = False

		# print 'IN:  ' + name
		base_name = name[:split_offs]
		idx = split_offs + 2
		if name[idx] == 'F':
			# No class, just a function
			ptype = []
		else:
			# Got class
			idx, ptype = read_class_name(idx, name)

		if not ptype:
			f_name = Name(base_name)
		else:
			f_name = NestedName(*map(Name, ptype + [base_name]))

		if idx == len(name):
			# This must be a var
			return f_name

		if name[idx] == 'C':
			idx += 1
			is_const = True

		f_params = []

		idx, f_params = read_func_params(idx, name)
		return Function(f_name, f_params, is_const)
	return None


def make_hex_offset(offs):
	return '0x%08X' % offs

def fix_offs_pal_v1(offs):
	return offs

def fix_offs_pal_v2(offs):
	if offs >= 0x800CF6E8 and offs <= 0x800CF90F:
		return offs + 8

	if offs >= 0x807685A0 and offs <= 0x807AAA70:
		return offs + 0x40

	if offs >= 0x807AAA74 and offs <= 0x809907FF:
		return offs + 0x10

	if offs >= 0x80990800:
		return offs + 0x20

	return offs

def fix_offs_ntsc_v1(offs):
	# .text section
	if offs >= 0x800B4604 and offs <= 0x800C8E4C:
		return offs - 0x50

	if offs >= 0x800C8E50 and offs <= 0x800E4D70:
		return offs - 0xF0

	if offs >= 0x800E4EC0 and offs <= 0x8010F200:
		return offs - 0x110

	if offs >= 0x8010F430 and offs <= 0x802BB6BC:
		return offs - 0x140

	if offs >= 0x802BB6C0 and offs <= 0x802BB74C:
		return offs - 0x150

	if offs >= 0x802BB860 and offs <= 0x802BBBFC:
		return offs - 0x260

	if offs >= 0x802BBC90 and offs <= 0x802EDCC0:
		return offs - 0x2F0

	# .ctors, .dtors, .rodata, part of .data section
	if offs >= 0x802EDCE0 and offs <= 0x80317734:
		return offs - 0x300

	# .data section
	if offs >= 0x80317750 and offs <= 0x80322FE0:
		return offs - 0x318

	if offs >= 0x80323118 and offs <= 0x8032E77C:
		return offs - 0x348

	if offs >= 0x8032E780 and offs <= 0x8035197C:
		return offs - 0x340

	# .sdata section, part of .sbss
	if offs >= 0x80351980 and offs <= 0x80427E87:
		return offs - 0x300

	# .sbss, .sdata2, .sbss2 sections
	if offs >= 0x80427E88 and offs <= 0x80429563:
		return offs - 0x310

	if offs >= 0x80429564 and offs <= 0x80429D7F:
		return offs - 0x2F8

	if offs >= 0x80429D80 and offs <= 0x807684BF:
		return offs - 0x2E0

	# part of d_basesNP, d_enemiesNP, d_en_bossNP
	if offs >= 0x8098A43C:
		return offs + 0x20

	return offs


def fix_offs_ntsc_v2(offs):
	offs = fix_offs_ntsc_v1(offs)

	if offs >= 0x800CF5F8 and offs <= 0x800CF81F:
		return offs + 8

	if offs >= 0x807685A0 and offs <= 0x807AAA70:
		return offs + 0x40

	if offs >= 0x807AAA74 and offs <= 0x8099081C:
		return offs + 0x10

	if offs >= 0x80990820:
		return offs + 0x20

	return offs


def fix_offs_jpn_v1(offs):
	# .text section
	if offs >= 0x800B4604 and offs <= 0x800B475C:
		return offs - 0x50

	if offs >= 0x800B4760 and offs <= 0x800C8DAC:
		return offs - 0xD0

	if offs >= 0x800C8E50 and offs <= 0x800E4D6C:
		return offs - 0x170

	if offs >= 0x800E4D94 and offs <= 0x800E4EB4:
		return offs - 0x194

	if offs >= 0x800E4EB8 and offs <= 0x8010F1D0:
		return offs - 0x190

	if offs >= 0x8010F430 and offs <= 0x802BB6BC:
		return offs - 0x330

	if offs >= 0x802BB6D0 and offs <= 0x802BB74C:
		return offs - 0x340

	if offs >= 0x802BB860 and offs <= 0x802BBBFC:
		return offs - 0x450

	if offs >= 0x802BBC90 and offs <= 0x802EDCC0:
		return offs - 0x4E0

	# .ctors, .dtors, .rodata, part of .data section
	if offs >= 0x802EDCE0 and offs <= 0x80317734:
		return offs - 0x4E0

	# .data section
	if offs >= 0x80317750 and offs <= 0x80322FDC:
		return offs - 0x4F8

	if offs >= 0x80323118 and offs <= 0x8035197C:
		return offs - 0x5E0

	# .sdata, part of .sbss section
	if offs >= 0x80351980 and offs <= 0x80427E5F:
		return offs - 0x580

	if offs >= 0x80427E88 and offs <= 0x8042954B:
		return offs - 0x5A8

	if offs >= 0x80429570 and offs <= 0x80429D7F:
		return offs - 0x5C8

	# part of .sdata2, .sbss2 section
	if offs >= 0x80429D80 and offs <= 0x807684BF: # end offset is right before d_profileNP header
		return offs - 0x5C0

	# d_profileNP and d_basesNP
	# "no change" gap ends at 8779ABC
	if offs >= 0x80779C70 and offs <= 0x8078891F:
		return offs - 0x130

	if offs >= 0x80788AD0 and offs <= 0x80789EEF:
		return offs - 0x260

	if offs >= 0x80789F00 and offs <= 0x808D3B87:
		return offs - 0x270

	if offs >= 0x808D3BD4 and offs <= 0x808D3C1F:
		return offs - 0x2B4

	if offs >= 0x808D3C20 and offs <= 0x80940C47:
		return offs - 0x2C0

	if offs >= 0x80940F58 and offs <= 0x80943167:
		return offs - 0x4E8

	if offs >= 0x809431F8 and offs <= 0x8094329F:
		return offs - 0x4F8

	if offs >= 0x809432C0 and offs <= 0x80944E77:
		return offs - 0x500

	if offs >= 0x80945144 and offs <= 0x80945153:
		return offs - 0x714

	if offs >= 0x80945158 and offs <= 0x8098A36B:
		return offs - 0x718

	if offs >= 0x8098A478 and offs <= 0x8098FF18:
		return offs - 0x6F8

	# d_enemiesNP
	# this offset starts at the .rel header
	if offs >= 0x80990800 and offs <= 0xDEADBEEF:
		return offs - 0x700

	#if offs >= 0x8010F430 and offs <= 0xDEADBEEF:
	#	return offs - 0x330

	#if offs >= 0x8010F430 and offs <= 0xDEADBEEF:
	#	return offs - 0x330

	return offs


def fix_offs_jpn_v2(offs):
	offs = fix_offs_jpn_v1(offs)

	if offs >= 0x800CF578 and offs <= 0x800CF79F:
		return offs + 8

	if offs >= 0x807685A0 and offs <= 0x807AA7FF:
		return offs + 0x40

	if offs >= 0x807AA800 and offs <= 0x809900FF:
		return offs + 0x10

	if offs >= 0x80990100:
		return offs + 0x20

	return offs


import yaml

original = 'pal'
fix_for = {
	'pal2': fix_offs_pal_v2,
	'ntsc': fix_offs_ntsc_v1,
	'ntsc2': fix_offs_ntsc_v2,
	'jpn': fix_offs_jpn_v1,
	'jpn2': fix_offs_jpn_v2
}

fixmes = {
	'setMatrix__21dPlayerModelHandler_cFPA4_f':
	'_ZN21dPlayerModelHandler_c9setMatrixEPA4_f',

	'SetCameraMtxDirectly__Q34nw4r3g3d6CameraFRA3_A4_Cf':
	'_ZN4nw4r3g3d6Camera20SetCameraMtxDirectlyERA3_A4_Kf',

	'GetCameraMtx__Q34nw4r3g3d6CameraCFPA3_A4_f':
	'_ZKN4nw4r3g3d6Camera12GetCameraMtxPA3_A4_f',

	'getViewMatrix__Q23m3d9scnLeaf_cFPA3_A4_f':
	'_ZN3m3d9scnLeaf_c13getViewMatrixEPA3_A4_f',

	'setDrawMatrix__Q23m3d5mdl_cFPA4_Cf':
	'_ZN3m3d5mdl_c13setDrawMatrixEPA4_Kf',

	'getMatrixForNode__Q23m3d5mdl_cFUiPA4_f':
	'_ZN3m3d5mdl_c16getMatrixForNodeEjPA4_f',
}

def do_mapfile(src, dest, fix_offset):
	map = [x.strip() for x in open(src, 'r')]
	new = []

	for line in map:
		if ' = 0x80' in line:
			spos = line.find(' ')
			pos = line.find('0x80')
			name = line[:spos]
			if name.startswith('_Z') and '/*addAnyway*/' not in line:
				continue #already mangled for Itanium
			old_name = name
			if '/*mi*/' not in line:
				f = from_cw(name)
				if f:
					name = mangle(f)
					if 'FIXME' in name:
						try:
							name = fixmes[old_name]
						except KeyError:
							print "FIX ME!"
							print old_name
				# print 'OUT: ' + name

			offs = int(line[pos:pos+10], 0)
			offs = fix_offset(offs)
			after = line[pos+10:]
			line = name + ' = ' + make_hex_offset(offs) + after
			if name != old_name:
				new.append(line + '\n')
				line = old_name + ' = ' + make_hex_offset(offs) + after

		new.append(line + '\n')

	open(dest, 'w').writelines(new)


def work_on_hook(hook, id, func):
	
	try:
		t = hook['type']

		if t == 'patch':
			error = 'missing addr'
			
			hook['addr_%s' % id] = func(hook['addr_%s' % original])

		elif t == 'branch_insn' or t == 'add_func_pointer':
			error = 'missing src_addr'

			hook['src_addr_%s' % id] = func(hook['src_addr_%s' % original])

			if 'target_func_%s' % original in hook:
				error = 'missing target_func'

				hook['target_func_%s' % id] = func(hook['target_func_%s' % original])

		elif t == 'nop_insn':
			error = 'missing area'

			area = hook['area_%s' % original]
			if isinstance(area, list):
				start = func(area[0])
				new_area = [start, start + (area[1] - area[0])]
			else:
				new_area = func(area)

			hook['area_%s' % id] = new_area

	except:
		print 'Key Error %s for %s (%s) for %s' % (error, hook['name'], hook['type'], id)

	#if hook['name'] == 'WM_onCreate': print hook


def do_module(src, dest):
	m = yaml.safe_load(open(src, 'r').read())

	if 'hooks' in m:
		for id, func in fix_for.iteritems():
			for hook in m['hooks']:
				work_on_hook(hook, id, func)

	open(dest, 'w').write(yaml.dump(m))


def do_project(f, already_done):
	proj = yaml.safe_load(open(f, 'r').read())

	for m in proj['modules']:
		if m not in already_done:
			already_done.add(m)
			do_module(m.replace('processed/', ''), m)


def main():
	do_mapfile('kamek_base.x', 'kamek_pal.x', fix_offs_pal_v1)
	do_mapfile('kamek_base.x', 'kamek_pal2.x', fix_offs_pal_v2)
	do_mapfile('kamek_base.x', 'kamek_ntsc.x', fix_offs_ntsc_v1)
	do_mapfile('kamek_base.x', 'kamek_ntsc2.x', fix_offs_ntsc_v2)
	do_mapfile('kamek_base.x', 'kamek_jpn.x', fix_offs_jpn_v1)
	do_mapfile('kamek_base.x', 'kamek_jpn2.x', fix_offs_jpn_v2)

	already_done = set()
	do_project('NewerProject.yaml', already_done)
	do_project('NewerProjectKP.yaml', already_done)
	do_project('SummerSun.yaml', already_done)

	do_module('anotherhax.yaml', 'processed/anotherhax.yaml')

if __name__ == '__main__':
	main()

