#!/usr/bin/env python3

from pathlib import Path
import subprocess
import sys

import yaml


project_path = sys.argv[1]
project_name = project_path.rstrip('.yaml')

with open(project_path, 'r', encoding='utf-8') as f:
    project_data = yaml.safe_load(f)

if not isinstance(project_data, dict):
    raise ValueError('the project file is an invalid format (it should be a YAML mapping)')
if 'output_dir' not in project_data:
    raise ValueError('missing output_dir field in the project file')

asm_folder = Path(project_data['output_dir'])


if sys.platform == 'win32':
    # Running on Windows
    gccpath = Path('C:\\devkitPro\\devkitPPC\\bin\\')
    kamekopts = ['--gcc-append-exe']

else:
    proc_version = Path('/proc/version')
    if proc_version.is_file() and 'microsoft' in proc_version.read_text().lower():  # https://stackoverflow.com/a/38859331/4718769
        # Running on WSL
        gccpath = Path('/mnt/c/devkitPro/devkitPPC/bin/')
        kamekopts = ['--gcc-append-exe']
    else:
        # Running on Mac/Linux
        gccpath = Path('/opt/devkitpro/devkitPPC/bin/')
        kamekopts = ['--use-wine']

subprocess.run([sys.executable, 'tools/mapfile_tool.py', project_path])

res = subprocess.run([sys.executable, 'tools/kamek.py', project_path, '--show-cmd', '--no-rels', '--use-mw', f'--gcc-path={gccpath}', '--gcc-type=powerpc-eabi', '--mw-path=tools/cw', '--filt-path=tools/c++filt', '--fast-hack', *kamekopts])
if res.returncode == 0:
    print('compile successful')
else:
    exit(1)

build_folder = Path(f'Build_{project_name}')

def copy_files(version_name_1: str, version_name_2: str) -> None:
    (asm_folder / f'n_{version_name_1}_loader.bin').replace(build_folder / f'System{version_name_2}.bin')
    (asm_folder / f'n_{version_name_1}_dlcode.bin').replace(build_folder / f'DLCode{version_name_2}.bin')
    (asm_folder / f'n_{version_name_1}_dlrelocs.bin').replace(build_folder / f'DLRelocs{version_name_2}.bin')

build_folder.mkdir(exist_ok=True)
copy_files('pal', 'EU_1')
copy_files('pal2', 'EU_2')
copy_files('ntsc', 'US_1')
copy_files('ntsc2', 'US_2')
copy_files('jpn', 'JP_1')
copy_files('jpn2', 'JP_2')
copy_files('kor', 'KR_3')
copy_files('twn', 'TW_4')
# copy_files('chn', 'CN_5')

print('Built all!')

