import elftools.elf.elffile
import struct

class Symbol(object):
	__slots__ = ('is_valid', 'name', 'is_external', 'address')
	def __init__(self, name):
		self.is_valid = False
		self.name = name

class DyLinkCreator(object):
	R_PPC_ADDR32 = 1
	R_PPC_ADDR16_LO = 4
	R_PPC_ADDR16_HI = 5
	R_PPC_ADDR16_HA = 6
	R_PPC_REL24 = 10

	VALID_RELOCS = set([1, 4, 5, 6, 10])

	def __init__(self, other=None):
		if other:
			self._relocs = other._relocs[:]

			self._targets = other._targets[:]
			self._target_lookups = other._target_lookups.copy()
		else:
			self._relocs = []

			self._targets = []
			self._target_lookups = {}

		self.elf = None
	
	def set_elf(self, stream):
		if self.elf != None:
			raise ValueError('ELF already set')

		self.elf = elftools.elf.elffile.ELFFile(stream)

		self._sections = list(self.elf.iter_sections())

		# Take all the sections we're gonna use and assemble them into a blob,
		# and store the base addresses
		self._build_blob()

		# Parse the symbol table into set addresses
		self._process_symbols()

		# Process all relocs
		self._process_relocs()

	def _build_blob(self):
		code_bits = []
		code_size = 0

		for sect in self._sections:
			if sect.name.startswith('.text') or sect.name.startswith('.data'):
				data = sect.data()

				sect.in_blob = True
				sect.blob_base = code_size
				sect.rela_sect = self.elf.get_section_by_name('.rela' + sect.name)

				code_bits.append(data)
				code_size += len(data)
				aligned_size = (code_size + 3) & ~3

				if aligned_size > code_size:
					code_bits.append('\0' * (aligned_size - code_size))
			else:
				sect.in_blob = False

		self.code = ''.join(code_bits)

	def _process_symbols(self):
		sym_section = self.elf.get_section_by_name('.symtab')
		syms = [None] * sym_section.num_symbols()

		for i, r_sym in enumerate(sym_section.iter_symbols()):
			entry = r_sym.entry
			value = entry['st_value']
			shndx = entry['st_shndx']

			sym = Symbol(r_sym.name)
			syms[i] = sym

			if shndx == 'SHN_ABS':
				sym.is_valid = True
				sym.is_external = True
				sym.address = value
			elif shndx == 'SHN_UNDEF':
				print 'Unknown symbol => ' + sym.name + ' (v:' + str(value) + ')'
			else:
				# print '%s + 0x%x => %s' % (self._sections[shndx].name, value, sym.name)
				section = self._sections[shndx]
				if section.in_blob:
					sym.is_valid = True
					sym.is_external = False
					sym.address = section.blob_base + value
				else:
					if section.name != '.group':
						print 'Section not included in blob: %s (required for %s)' % (section.name, sym.name)

		self._symbols = syms


	def _process_relocs(self):
		for sect in self._sections:
			if not sect.in_blob: continue

			rela_sect = sect.rela_sect
			if not rela_sect: continue

			for reloc in rela_sect.iter_relocations():
				entry = reloc.entry

				symbol = self._symbols[entry['r_info_sym']]
				if symbol.is_valid:
					self.add_reloc(
						entry['r_info_type'],
						sect.blob_base + entry['r_offset'],
						symbol.address + entry['r_addend'],
						symbol.name)
				else:
					print 'Failed relocation: %s+0x%x => %s+0x%x' % (sect.name, entry['r_offset'], symbol.name, entry['r_addend'])



	def add_reloc(self, reltype, addr, target, name="UNKNOWN NAME"):
		if reltype not in self.VALID_RELOCS:
			raise ValueError('Unknown/unsupported rel type: %d (%x => %x)' % (reltype, addr, target))

		try:
			target_id = self._target_lookups[target]
		except KeyError:
			target_id = len(self._targets)
			self._target_lookups[target] = target_id
			self._targets.append(target)

		self._relocs.append((reltype, addr, target_id))
	
	def build_reloc_data(self):
		header_struct = struct.Struct('>8sI')

		rel_struct_pack = struct.Struct('>II').pack
		target_struct_pack = struct.Struct('>I').pack

		rel_data = map(lambda x: rel_struct_pack((x[0] << 24) | x[2], x[1]), self._relocs)
		target_data = map(target_struct_pack, self._targets)

		header = header_struct.pack('NewerREL', 12 + (len(self._relocs) * 8))

		return header + ''.join(rel_data) + ''.join(target_data)



if __name__ == '__main__':
	dlc = DyLinkCreator()
	dlc.set_elf(open('NewerASM/n_jpn_object.plf', 'rb'))
