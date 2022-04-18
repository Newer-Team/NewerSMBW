import dataclasses
import os
import sys
import re
import typing
import yaml


BASE_VERSION = 'pal'
VERSION_IDS = {
    # kamek_***.x ID, versions-nsmbw.txt ID
    # (don't list pal v1 here)
    'pal2': 'P2',
    'ntsc': 'E1',
    'ntsc2': 'E2',
    'jpn': 'J1',
    'jpn2': 'J2',
    'kor': 'K',
    'twn': 'W',
    #'chn': 'C',
}


class AddressMapper:
    base: 'AddressMapper' = None

    @dataclasses.dataclass
    class Mapping:
        start: int = None
        end: int = None
        delta: int = None

        def overlaps(self, other: 'Mapping'):
            return (self.end >= other.start) and (self.start <= other.end)

        def __str__(self):
            return f'{self.start:08X}-{self.end:08X}: {"+" if self.delta >= 0 else "-"}0x{abs(self.delta)}'

        def __repr__(self):
            return f'<mapping {self!s}>'

    def __init__(self, base=None):
        self.base = base
        self.mappings = []

    def add_mapping(self, start: int, end: int, delta: int):
        if start > end:
            raise ValueError(f'cannot map {start:08X}-{end:08X} as start is higher than end')

        new_mapping = self.Mapping(start, end, delta)

        for mapping in self.mappings:
            if mapping.overlaps(new_mapping):
                raise ValueError(f'mapping "{new_mapping}" overlaps with earlier mapping "{mapping}"')

        self.mappings.append(new_mapping)

    def remap(self, input: int):
        if self.base is not None:
            input = self.base.remap(input)

        for mapping in self.mappings:
            if mapping.start <= input <= mapping.end:
                return input + mapping.delta

        return input


def read_version_info(f: typing.TextIO):
    mappers = {'default': AddressMapper()}

    comment_regex = re.compile(r'^\s*#')
    empty_line_regex = re.compile(r'^\s*$')
    section_regex = re.compile(r'^\s*\[([a-zA-Z0-9_.]+)\]$')
    extend_regex = re.compile(r'^\s*extend ([a-zA-Z0-9_.]+)\s*(#.*)?$')
    mapping_regex = re.compile(r'^\s*([a-fA-F0-9]{8})-((?:[a-fA-F0-9]{8})|\*)\s*:\s*([-+])0x([a-fA-F0-9]+)\s*(#.*)?$')
    current_version_name = None
    current_version = None

    for line in f:
        line = line.rstrip('\n')

        if empty_line_regex.match(line):
            continue
        if comment_regex.match(line):
            continue

        match = section_regex.match(line)
        if match:
            # New version
            current_version_name = match.group(1)
            if current_version_name in mappers:
                raise ValueError(f'versions file contains duplicate version name {current_version_name}')

            current_version = AddressMapper()
            mappers[current_version_name] = current_version
            continue

        if current_version is not None:
            # Try to associate something with the current version
            match = extend_regex.match(line)
            if match:
                base_name = match.group(1)
                if base_name not in mappers:
                    raise ValueError(f'version {current_version_name} extends unknown version {base_name}')
                if current_version.base is not None:
                    raise ValueError(f'version {current_version_name} already extends a version')

                current_version.base = mappers[base_name]
                continue

            match = mapping_regex.match(line)
            if match:
                start_address = int(match.group(1), 16)
                if match.group(2) == '*':
                    end_address = 0xFFFFFFFF
                else:
                    end_address = int(match.group(2), 16)

                delta = int(match.group(4), 16)
                if match.group(3) == '-':
                    delta = -delta

                current_version.add_mapping(start_address, end_address, delta)
                continue

        print(f'unrecognised line in versions file: {line}')

    return mappers


def do_mapfile(src, dest, mapper):
    new = []
    with open(src) as f:
        mapfile = [x.rstrip() for x in f]

    for line in mapfile:
        pos = line.find('= 0x80')
        if pos != -1:
            oldoffs = line[pos+2:pos+12]
            newoffs = mapper.remap(int(oldoffs, 16))
            line = line.replace(oldoffs, str(newoffs))

        new.append(line + '\n')

    with open(dest, 'w') as f:
        f.writelines(new)


def work_on_hook(hook, name, mapper):
    error = 'Missing hook type'
    try:
        t = hook['type']

        if t == 'patch':
            error = 'Missing address'
            hook[f'addr_{name}'] = mapper.remap(hook[f'addr_{BASE_VERSION}'])

        elif t == 'branch_insn' or t == 'add_func_pointer':
            error = 'Missing source address'
            hook[f'src_addr_{name}'] = mapper.remap(hook[f'src_addr_{BASE_VERSION}'])

            if f'target_func_{BASE_VERSION}' in hook:
                error = 'Missing target function'
                hook[f'target_func_{name}'] = mapper.remap(hook[f'target_func_{BASE_VERSION}'])

        elif t == 'nop_insn':
            error = 'Missing area'
            area = hook[f'area_{BASE_VERSION}']

            if isinstance(area, list):
                start = mapper.remap(area[0])
                new_area = [start, start + (area[1] - area[0])]
            else:
                new_area = mapper.remap(area)

            hook[f'area_{name}'] = new_area

    except KeyError:
        print(f'Key Error {error} in {name}')


def do_module(src, dest, mappers):
    with open(src) as f:
        m = yaml.safe_load(f.read())

    for x_id, txt_id in VERSION_IDS.items():
        mapper = mappers[txt_id]
        if 'hooks' in m:
            for hook in m['hooks']:
                work_on_hook(hook, x_id, mapper)

    with open(dest, 'w') as f:
        f.write(yaml.dump(m))


def do_project(f, already_done, mappers):
    with open(f) as f:
        proj = yaml.safe_load(f.read())

    if 'modules' in proj:
        for m in proj['modules']:
            if m not in already_done:
                already_done.add(m)
                do_module(m.replace('processed/', ''), m, mappers)


def main():
    project_path = sys.argv[1]

    with open('tools/versions-nsmbw.txt', 'r', encoding='utf-8') as f:
        mappers = read_version_info(f)

    if not os.path.isdir('processed'):
        os.mkdir('processed')

    for x_id, txt_id in VERSION_IDS.items():
        do_mapfile(f'kamek_{BASE_VERSION}.x', f'kamek_{x_id}.x', mappers[txt_id])

    already_done = set()
    do_project(project_path, already_done, mappers)


if __name__ == '__main__':
    main()