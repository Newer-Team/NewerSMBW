#!/usr/bin/env python

# Kamek - build tool for custom C++ code in New Super Mario Bros. Wii
# All rights reserved (c) Ninji 2010 - 2013
# Header files compiled by Ninji, Chronometrics and megazig

# Requires PyYAML and pyelftools

version_str = 'Kamek by Ninji'

import binascii
import os
import os.path
import shutil
import struct
import subprocess
import sys
import tempfile
import yaml

import elftools.elf.elffile

import hooks

u32 = struct.Struct('>I')

verbose = True
use_rels = True
use_mw = False
use_wine = False
mw_path = ''
filt_path = ''
gcc_path = ''
gcc_type = 'powerpc-eabi'
gcc_append_exe = False
show_cmd = False
delete_temp = True
override_config_file = None
only_build = None
fast_hack = False


def parse_cmd_options():
    global use_rels, use_mw, use_wine, show_cmd, delete_temp, only_build, fast_hack
    global override_config_file, gcc_type, gcc_path, gcc_append_exe, mw_path, filt_path

    if '--no-rels' in sys.argv:
        use_rels = False

    if '--use-mw' in sys.argv:
        use_mw = True

    if '--use-wine' in sys.argv:
        use_wine = True

    if '--show-cmd' in sys.argv:
        show_cmd = True

    if '--keep-temp' in sys.argv:
        delete_temp = False

    if '--fast-hack' in sys.argv:
        fast_hack = True

    if '--gcc-append-exe' in sys.argv:
        gcc_append_exe = True


    only_build = []

    for arg in sys.argv:
        if arg.startswith('--configs='):
            override_config_file = arg[10:]

        if arg.startswith('--build='):
            only_build.append(arg[8:])

        if arg.startswith('--gcc-type='):
            gcc_type = arg[11:]

        if arg.startswith('--gcc-path='):
            gcc_path = arg[11:] + '/'

        if arg.startswith('--mw-path='):
            mw_path = arg[10:] + '/'

        if arg.startswith('--filt-path='):
            filt_path = arg[12:] + '/'

    if len(only_build) == 0:
        only_build = None


def print_debug(s):
    if verbose: print('* '+str(s))


def read_configs(filename):
    with open(filename, 'r') as f:
        data = f.read()

    return yaml.safe_load(data)


current_unique_id = 0
def generate_unique_id():
    # this is used for temporary filenames, to ensure that .o files
    # do not overwrite each other
    global current_unique_id
    current_unique_id += 1
    return current_unique_id


def align_addr_up(addr, align):
    align -= 1
    return (addr + align) & ~align


def generate_riiv_mempatch(offset, data):
    return '<memory offset="0x%08X" value="%s" />' % (offset, binascii.hexlify(data))


def generate_ocarina_patch(destOffset, data):
    out = []
    count = len(data)

    sourceOffset = 0
    destOffset -= 0x80000000
    for i in range(count >> 2):
        out.append('%08X %s' % (destOffset | 0x4000000, binascii.hexlify(data[sourceOffset:sourceOffset+4])))
        sourceOffset += 4
        destOffset += 4

    # take care
    remainder = count % 4
    if remainder == 3:
        out.append('%08X 0000%s' % (destOffset | 0x2000000, binascii.hexlify(data[sourceOffset:sourceOffset+2])))
        out.append('%08X 000000%02x' % (destOffset, data[sourceOffset+2]))
    elif remainder == 2:
        out.append('%08X 0000%s' % (destOffset | 0x2000000, binascii.hexlify(data[sourceOffset:sourceOffset+2])))
    elif remainder == 1:
        out.append('%08X 000000%02x' % (destOffset, data[sourceOffset]))

    return '\n'.join(out)


def generate_kamek_patches(patchlist):
    kamekpatch = b''
    for patch in patchlist:
        if len(patch[1]) > 4:
            # block patch
            kamekpatch += u32.pack(align_addr_up(len(patch[1]), 4) // 4)
            kamekpatch += u32.pack(patch[0])
            kamekpatch += patch[1]
            # align it
            if len(patch[1]) % 4 != 0:
                kamekpatch += b'\0' * (4 - (len(patch[1]) % 4))
        else:
            # single patch
            kamekpatch += u32.pack(patch[0])
            kamekpatch += patch[1]

    kamekpatch += u32.pack(0xFFFFFFFF)
    return kamekpatch


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
        self.code = self.elf.get_section_by_name('.text').data()

        self._add_relocs(self.elf.get_section_by_name('.rela.text'))

    def _add_relocs(self, section):
        sym_values = {}
        sym_section = self.elf.get_section_by_name('.symtab')

        for reloc in section.iter_relocations():
            entry = reloc.entry
            #print(entry)

            sym_id = entry['r_info_sym']
            try:
                sym_value, sym_name = sym_values[sym_id]
            except KeyError:
                sym = sym_section.get_symbol(sym_id)
                sym_value = sym.entry['st_value']
                sym_name = sym.name
                sym_values[sym_id] = (sym_value, sym_name)
            #print(hex(sym_value))

            self.add_reloc(entry['r_info_type'], entry['r_offset'], sym_value+entry['r_addend'], sym_name)

    def add_reloc(self, reltype, addr, target, name="UNKNOWN NAME"):
        if reltype not in self.VALID_RELOCS:
            raise ValueError('Unknown/unsupported rel type: %d (%x => %x)' % (reltype, addr, target))

        try:
            target_id = self._target_lookups[target]
        except KeyError:
            target_id = len(self._targets)
            self._target_lookups[target] = target_id
            self._targets.append(target)
        if target <= 0:
            print("Warning: The following reloc (%x) points to %d: Is this right? %s" % (addr, target, name))

        self._relocs.append((reltype, addr, target_id))

    def build_reloc_data(self):
        header_struct = struct.Struct('>8sI')

        rel_struct_pack = struct.Struct('>II').pack
        target_struct_pack = struct.Struct('>I').pack

        rel_data = map(lambda x: rel_struct_pack((x[0] << 24) | x[2], x[1]), self._relocs)
        target_data = map(target_struct_pack, self._targets)

        header = header_struct.pack(b'NewerREL', 12 + (len(self._relocs) * 8))

        return header + b''.join(rel_data) + b''.join(target_data)



class KamekModule(object):
    _requiredFields = ['source_files']


    def __init__(self, filename):
        # load the module data
        self.modulePath = os.path.normpath(filename)
        self.moduleName = os.path.basename(self.modulePath)
        #self.moduleDir = os.path.dirname(self.modulePath)
        self.moduleDir = 'processed'

        with open(self.modulePath, 'r') as f:
            self.rawData = f.read()

        self.data = yaml.safe_load(self.rawData)
        if not isinstance(self.data, dict):
            raise ValueError('the module file %s is an invalid format (it should be a YAML mapping)' % self.moduleName)

        # verify it
        for field in self._requiredFields:
            if field not in self.data:
                raise ValueError('Missing field in the module file %s: %s' % (self.moduleName, field))



class KamekBuilder(object):
    def __init__(self, project, configs):
        self.project = project
        self.configs = configs


    def build(self):
        print_debug('Starting build')

        self._prepare_dirs()

        for config in self.configs:
            if only_build != None and config['short_name'] not in only_build:
                continue

            self._set_config(config)

            #self._configTempDir = tempfile.mkdtemp()
            self._configTempDir = 'tmp'
            if os.path.isdir('tmp'):
                shutil.rmtree('tmp')
            os.mkdir('tmp')
            print_debug('Temp files for this configuration are in: '+self._configTempDir)

            if 'dynamic_link' in self._config and self._config['dynamic_link']:
                self.dynamic_link_base = DyLinkCreator()
            else:
                self.dynamic_link_base = None

            self._builtCodeAddr = 0x80001800
            if 'code_address' in self.project.data:
                self._builtCodeAddr = self.project.data['code_address']

            # hook setup
            self._hook_contexts = {}
            for name, hookType in hooks.HookTypes.items():
                if hookType.has_context:
                    self._hook_contexts[hookType] = hookType.context_type()

            self._compile_modules()

            # figure out how many copies we need to build
            # this is a mess!
            if 'multi_build' in self._config:
                self._multi_build = self._config['multi_build']
            else:
                self._multi_build = {self._config['short_name']: self._config['linker_script']}

            for s_name, s_script in self._multi_build.items():
                self.current_build_name = s_name

                self._patches = []
                self._rel_patches = []
                self._hooks = []

                self._create_hooks()

                self._link(s_name, s_script)
                self._read_symbol_map()

                if self.dynamic_link_base:
                    self.dynamic_link = DyLinkCreator(self.dynamic_link_base)
                    self.dynamic_link.set_elf(open(self._currentOutFile, 'rb'))

                for hook in self._hooks:
                    hook.create_patches()

                self._create_patch(s_name)

            if delete_temp:
                shutil.rmtree(self._configTempDir)


    def _prepare_dirs(self):
        self._outDir = self.project.makeRelativePath(self.project.data['output_dir'])
        print_debug('Project will be built in: '+self._outDir)

        if not os.path.isdir(self._outDir):
            os.makedirs(self._outDir)
            print_debug('Created that directory')


    def _set_config(self, config):
        self._config = config
        print_debug('---')
        print_debug('Building for configuration: '+config['friendly_name'])

        self.config_short_name = config['short_name']
        if 'rel_area_start' in config:
            self._rel_area = (config['rel_area_start'], config['rel_area_end'])
        else:
            self._rel_area = (-50, -50)


    def _create_hooks(self):
        print_debug('---')
        print_debug('Creating hooks')

        for m in self.project.modules:
            if 'hooks' in m.data:
                for hookData in m.data['hooks']:
                    assert 'name' in hookData and 'type' in hookData

                    #print_debug('Hook: %s : %s' % (m.moduleName, hookData['name']))

                    if hookData['type'] in hooks.HookTypes:
                        hookType = hooks.HookTypes[hookData['type']]
                        hook = hookType(self, m, hookData)
                        self._hooks.append(hook)
                    else:
                        raise ValueError('Unknown hook type: %s' % hookData['type'])


    def _compile_modules(self):
        print_debug('---')
        print_debug('Compiling modules')

        if use_mw:
            # metrowerks setup
            cc_command = ['%smwcceppc.exe' % mw_path, '-I.', '-I-', '-I.', '-nostdinc', '-Cpp_exceptions', 'off', '-fp', 'hard', '-enum', 'int', '-sdata', '0', '-sdata2', '0', '-g', '-RTTI', 'off', '-use_lmw_stmw', 'on']
            as_command = ['%smwasmeppc.exe' % mw_path, '-I.', '-I-', '-I.', '-nostdinc', '-d', '__MWERKS__']

            for d in self._config.get('defines', []) + self.project.data.get('defines', []):
                cc_command.append('-d')
                cc_command.append(d)
                as_command.append('-d')
                as_command.append(d)
                print(f'defined: {d}')

            for i in self._config['include_dirs']:
                cc_command.append('-I%s' % i)
                #cc_command.append(i)
                as_command.append('-I%s' % i)
                #as_command.append(i)

            if use_wine:
                cc_command.insert(0, 'wine')
                as_command.insert(0, 'wine')

        else:
            # gcc setup
            cc_command = ['%s%s-g++' % (gcc_path, gcc_type), '-nodefaultlibs', '-I.', '-fno-builtin', '-Os', '-fno-exceptions', '-fno-rtti', '-mno-sdata']
            as_command = cc_command

            for d in self._config.get('defines', []) + self.project.data.get('defines', []):
                cc_command.append('-D%s' % d)

            for i in self._config['include_dirs']:
                cc_command.append('-I%s' % i)


        self._moduleFiles = []

        if fast_hack:
            fast_cpp_path = os.path.join(self._configTempDir, 'fasthack.cpp')
            fast_cpp = open(fast_cpp_path, 'w')

        for m in self.project.modules:
            for normal_sourcefile in m.data['source_files']:
                print_debug('Compiling %s : %s' % (m.moduleName, normal_sourcefile))

                objfile = os.path.join(self._configTempDir, '%d.o' % generate_unique_id())
                sourcefile = os.path.join(m.moduleDir, normal_sourcefile)

                if sourcefile.endswith('.o'):
                    new_command = ['cp', sourcefile, objfile]
                else:
                    # todo: better extension detection
                    if sourcefile.endswith('.s') or sourcefile.endswith('.S'):
                        command = as_command
                    elif sourcefile.endswith('.cpp') and fast_hack:
                        fast_cpp.write('//\n// %s\n//\n\n' % sourcefile)
                        with open(sourcefile, 'r') as sf:
                            fast_cpp.write(sf.read())
                        fast_cpp.write('\n')
                        continue
                    else:
                        command = cc_command

                    new_command = command + ['-c', '-o', objfile, sourcefile]

                    if 'cc_args' in m.data:
                        new_command += m.data['cc_args']

                if show_cmd:
                    print_debug(new_command)

                try:
                    errorVal = subprocess.call(new_command)
                except:
                    print('An error occured while calling the compiler. Please make sure CodeWarrior is installed correctly into the tools folder.')
                    sys.exit(1)
                
                if errorVal != 0:
                    print('BUILD FAILED!')
                    print('compiler returned %d - an error occurred while compiling %s' % (errorVal, sourcefile))
                    sys.exit(1)

                self._moduleFiles.append(objfile)

        if fast_hack:
            fast_cpp.close()

            print_debug('Fast compilation!!')
            objfile = os.path.join(self._configTempDir, 'fasthack.o')

            new_command = cc_command + ['-c', '-o', objfile, fast_cpp_path]
            if show_cmd:
                print_debug(new_command)

            errorVal = subprocess.call(new_command)
            if errorVal != 0:
                print('BUILD FAILED!')
                print('compiler returned %d - an error occurred while compiling the fast hack' % errorVal)
                sys.exit(1)

            self._moduleFiles.append(objfile)

        print_debug('Compilation complete')


    def _link(self, short_name, script_file):
        print_debug('---')
        print_debug('Linking %s (%s)...' % (short_name, script_file))

        nice_name = '%s_%s' % (self._config['short_name'], short_name)

        print_debug('---')

        self._currentMapFile = '%s/%s_linkmap.map' % (self._outDir, nice_name)
        outname = 'object.plf' if self.dynamic_link_base else 'object.bin'
        self._currentOutFile = '%s/%s_%s' % (self._outDir, nice_name, outname)

        exe = '.exe' if gcc_append_exe else ''
        ld_command = ['%s%s-ld%s' % (gcc_path, gcc_type, exe), '-L.']
        ld_command.append('-o')
        ld_command.append(self._currentOutFile)
        if self.dynamic_link_base:
            ld_command.append('-r')
            ld_command.append('--oformat=elf32-powerpc')
        else:
            ld_command.append('--oformat=binary')
            ld_command.append('-Ttext')
            ld_command.append('0x%08X' % self._builtCodeAddr)
        ld_command.append('-T')
        ld_command.append(script_file)
        ld_command.append('-Map')
        ld_command.append(self._currentMapFile)
        ld_command.append('--no-demangle') # for debugging
        #ld_command.append('--verbose')
        ld_command += self._moduleFiles

        if show_cmd:
            print_debug(ld_command)

        errorVal = subprocess.call(ld_command)
        if errorVal != 0:
            print('BUILD FAILED!')
            print('ld returned %d' % errorVal)
            sys.exit(1)

        print_debug('Successfully linked %s' % short_name)


    def _read_symbol_map(self):
        print_debug('---')
        print_debug('Reading symbol map')

        self._symbols = []

        with open(self._currentMapFile, 'r') as file:

            for line in file:
                if '__text_start' in line:
                    self._textSegStart = int(line.split()[0],0)
                    break

            # now read the individual symbols
            # this is probably a bad method to parse it, but whatever
            for line in file:
                if '__text_end' in line:
                    self._textSegEnd = int(line.split()[0],0)
                    break

                if not line.startswith('                '): continue

                sym = line.split()
                sym[0] = int(sym[0],0)
                self._symbols.append(sym)

            # we've found __text_end, so now we should be at the output section
            currentEndAddress = self._textSegEnd

            for line in file:
                if line[0] == '.':
                    # probably a segment
                    data = line.split()
                    if len(data) < 3: continue

                    segAddr = int(data[1],0)
                    segSize = int(data[2],0)

                    if segAddr+segSize > currentEndAddress:
                        currentEndAddress = segAddr+segSize

            self._codeStart = self._textSegStart
            self._codeEnd = currentEndAddress

        print_debug('Read, %d symbol(s) parsed' % len(self._symbols))


        # next up, run it through c++filt
        print_debug('Running c++filt')
        opsys = sys.platform
        if opsys == 'darwin': opsys = 'osx'
        print('%s/%s/%s-c++filt' % (filt_path, opsys, gcc_type))
        p = subprocess.Popen('%s/%s/%s-c++filt' % (filt_path, opsys, gcc_type), stdin=subprocess.PIPE, stdout=subprocess.PIPE)

        symbolNameList = [sym[1] for sym in self._symbols]
        filtResult = p.communicate('\n'.join(symbolNameList).encode('utf-8'))
        filteredSymbols = filtResult[0].decode('utf-8').split('\n')

        for sym, filt in zip(self._symbols, filteredSymbols):
            sym.append(filt.strip())

        print_debug('Done. All symbols complete.')
        print_debug('Generated code is at 0x%08X .. 0x%08X' % (self._codeStart, self._codeEnd - 4))


    def find_func_by_symbol(self, find_symbol):
        for sym in self._symbols:
            #if show_cmd:
            #   out = "0x%08x - %s - %s" % (sym[0], sym[1], sym[2])
            #   print_debug(out)
            if sym[2] == find_symbol:
                return sym[0]

        raise ValueError('Cannot find function: %s' % find_symbol)


    def add_patch(self, offset, data):
        if offset >= self._rel_area[0] and offset <= self._rel_area[1] and use_rels:
            self._rel_patches.append((offset, data))
        else:
            self._patches.append((offset, data))


    def _create_patch(self, short_name):
        print_debug('---')
        print_debug('Creating patch')

        nice_name = '%s_%s' % (self._config['short_name'], short_name)

        # convert the .rel patches to KamekPatcher format
        if len(self._rel_patches) > 0:
            kamekpatch = generate_kamek_patches(self._rel_patches)
            #self._patches.append((0x817F4800, kamekpatch))
            self._patches.append((0x80002F60, kamekpatch))

        if self.dynamic_link:
            # put together the dynamic link files
            with open('%s/%s_dlcode.bin' % (self._outDir, nice_name), 'wb') as dlcode:
                dlcode.write(self.dynamic_link.code)

            with open('%s/%s_dlrelocs.bin' % (self._outDir, nice_name), 'wb') as dlrelocs:
                dlrelocs.write(self.dynamic_link.build_reloc_data())

        else:
            # add the outfile as a patch if not using dynamic linking
            with open(self._currentOutFile, 'rb') as file:
                patch = (self._codeStart, file.read())

            self._patches.append(patch)

        # generate a Riivolution patch
        with open('%s/%s_riiv.xml' % (self._outDir, nice_name), 'w') as riiv:
            for patch in self._patches:
                riiv.write(generate_riiv_mempatch(*patch) + '\n')

        # generate an Ocarina patch
        with open('%s/%s_ocarina.txt' % (self._outDir, nice_name), 'w') as ocarina:
            for patch in self._patches:
                ocarina.write(generate_ocarina_patch(*patch) + '\n')

        # generate a KamekPatcher patch
        with open('%s/%s_loader.bin' % (self._outDir, nice_name), 'wb') as kpatch:
            kpatch.write(generate_kamek_patches(self._patches))

        print_debug('Patches generated')



class KamekProject(object):
    _requiredFields = ['output_dir', 'modules']


    def __init__(self, filename):
        # load the project data
        self.projectPath = os.path.abspath(filename)
        self.projectName = os.path.basename(self.projectPath)
        self.projectDir = '' #os.path.dirname(self.projectPath)

        with open(self.projectPath, 'r') as f:
            self.rawData = f.read()

        self.data = yaml.safe_load(self.rawData)
        if not isinstance(self.data, dict):
            raise ValueError('the project file is an invalid format (it should be a YAML mapping)')

        # verify it
        for field in self._requiredFields:
            if field not in self.data:
                raise ValueError('Missing field in the project file: %s' % field)

        # load each module
        self.modules = []
        for moduleName in self.data['modules']:
            modulePath = self.makeRelativePath(moduleName)
            self.modules.append(KamekModule(modulePath))


    def makeRelativePath(self, path):
        return os.path.normpath(os.path.join(self.projectDir, path))


    def build(self):
        # compile everything in the project
        builder = KamekBuilder(self, self.configs)
        builder.build()



def main():
    print(version_str)
    print('')

    if len(sys.argv) < 2:
        print('No input file specified')
        sys.exit()

    parse_cmd_options()

    project = KamekProject(os.path.normpath(sys.argv[1]))

    if override_config_file:
        project.configs = read_configs(override_config_file)
    else:
        project.configs = read_configs('kamek_configs.yaml')

    project.build()



if __name__ == '__main__':
    main()


