import binascii
import struct

u32 = struct.Struct('>I')

def make_branch_insn(fromAddr, toAddr, branchType):
	branchTypes = ['b', 'bl', 'ba', 'bla']
	if branchType not in branchTypes:
		raise ValueError, 'invalid branch type: %s' % branchType
	
	extra = branchTypes.index(branchType)
	
	distance = toAddr - fromAddr
	if distance >= 0x2000000 or distance <= -0x2000000:
		raise ValueError, 'branching too far'
	
	return (distance & 0x3FFFFFC) | 0x48000000 | extra



class HookContext(object):
	"""Object which can be used by each hook type to hold data."""
	
	def __init__(self):
		self.hooks = []



class Hook(object):
	"""Generic hook class"""
	
	has_context = False
	required_data = []
	
	def __init__(self, builder, module, data):
		"""Sets up a hook"""
		self.builder = builder
		self.module = module
		self.data = data
		
		if self.has_context:
			hookType = type(self)
			self.context = builder._hook_contexts[hookType]
			self.context.hooks.append(self)
		
		# validate the hook's data
		current_config_name = builder._config_short_name
		
		for field in self.required_data:
			field = field.replace('%CONFIG%', current_config_name)
			if field not in data:
				raise ValueError, 'hook %s : %s is missing the field %s' % (module.moduleName, data['name'], field)
	
	def create_patches(self):
		pass



class BasicPatchHook(Hook):
	"""Hook that simply patches data to an address"""
	
	required_data = ['addr_%CONFIG%', 'data']
	
	def __init__(self, builder, module, data):
		Hook.__init__(self, builder, module, data)
	
	def create_patches(self):
		addr = self.data['addr_%s' % self.builder._config_short_name]
		
		hex_data = self.data['data']
		
		whitespace = ' \n\r\t'
		for char in whitespace:
			hex_data = hex_data.replace(char, '')
		
		patch = binascii.unhexlify(hex_data)
		
		self.builder._add_patch(addr, patch)



class BranchInsnHook(Hook):
	"""Hook that replaces the instruction at a specific address with a branch"""
	
	required_data = ['branch_type', 'src_addr_%CONFIG%']
	
	def __init__(self, builder, module, data):
		Hook.__init__(self, builder, module, data)
	
	def create_patches(self):
		try:
			target_func = self.data['target_func']
		except KeyError:
			target_func = self.data['target_func_%s' % self.builder._config_short_name]
		
		if isinstance(target_func, str):
			target_func = self.builder._find_func_by_symbol(target_func)
		else:
			# assume it's an address
			pass
		
		
		src_addr = self.data['src_addr_%s' % self.builder._config_short_name]
		branch_insn = make_branch_insn(src_addr, target_func, self.data['branch_type'])
		
		self.builder._add_patch(src_addr, u32.pack(branch_insn))



class AddFunctionPointerHook(Hook):
	"""Hook that places a function pointer at an address"""
	
	required_data = ['src_addr_%CONFIG%']
	
	def __init__(self, builder, module, data):
		Hook.__init__(self, builder, module, data)
	
	def create_patches(self):
		try:
			target_func = self.data['target_func']
		except KeyError:
			target_func = self.data['target_func_%s' % self.builder._config_short_name]
		
		if isinstance(target_func, str):
			target_func = self.builder._find_func_by_symbol(target_func)
		else:
			# assume it's an address
			pass
		
		
		src_addr = self.data['src_addr_%s' % self.builder._config_short_name]
		
		self.builder._add_patch(src_addr, u32.pack(target_func))



class NopInsnHook(Hook):
	"""Hook that NOPs out the instruction(s) at an address"""
	
	required_data = ['area_%CONFIG%']
	
	def __init__(self, builder, module, data):
		Hook.__init__(self, builder, module, data)
	
	def create_patches(self):
		area = self.data['area_%s' % self.builder._config_short_name]
		
		if isinstance(area, list):
			addr, end = area
			count = (end + 4 - addr) / 4
			nop_patch = '\x60\x00\x00\x00' * count
		else:
			addr = area
			nop_patch = '\x60\x00\x00\x00'
		
		self.builder._add_patch(addr, nop_patch)



HookTypes = {
	'patch': BasicPatchHook,
	'branch_insn': BranchInsnHook,
	'add_func_pointer': AddFunctionPointerHook,
	'nop_insn': NopInsnHook,
}

