from distutils.core import setup
from py2exe.build_exe import py2exe
import os, os.path, shutil, sys

upxFlag = False
if '-upx' in sys.argv:
    sys.argv.remove('-upx')
    upxFlag = True

dir = 'distrib/windows'

print '[[ Running Koopatlas Through py2exe! ]]'
print '>> Destination directory: %s' % dir
sys.argv.append('py2exe')

if os.path.isdir(dir): shutil.rmtree(dir)
os.makedirs(dir)

# exclude QtWebKit to save space, plus Python stuff we don't use
excludes = ['encodings', 'doctest', 'pdb', 'unittest', 'difflib', 'inspect',
    'os2emxpath', 'posixpath', 'optpath', 'locale', 'calendar',
    'threading', 'select', 'socket', 'hashlib', 'multiprocessing', 'ssl',
    'PyQt4.QtWebKit', 'PyQt4.QtNetwork']

# set it up
setup(
    name='Koopatlas',
    version='0.2',
    description="Koopatlas - Newer's Fantastic World Map Editor",
    windows=[
        {'script': 'koopatlas.py',
         }
    ],
    options={'py2exe':{
        'includes': ['sip', 'encodings', 'encodings.hex_codec', 'encodings.utf_8', 'hashlib'],
        'compressed': 1,
        'optimize': 2,
        'excludes': excludes,
        'bundle_files': 3,
        'dist_dir': dir
    }}
)

print '>> Built frozen executable!'

# now that it's built, configure everything
os.unlink(dir + '/w9xpopen.exe') # not needed

if upxFlag:
    if os.path.isfile('upx.exe'):
        print '>> Found UPX, using it to compress the executables!'
        files = os.listdir(dir)
        upx = []
        for f in files:
            if f.endswith('.exe') or f.endswith('.dll') or f.endswith('.pyd'):
                upx.append('"%s/%s"' % (dir,f))
        os.system('upx -9 ' + ' '.join(upx))
        print '>> Compression complete.'
    else:
        print '>> UPX not found, binaries can\'t be compressed.'
        print '>> In order to build Koopuzzle! with UPX, place the upx.exe file into '\
              'this folder.'


print '>> Attempting to copy VC++2008 libraries...'
if os.path.isdir('Microsoft.VC90.CRT'):
    shutil.copytree('Microsoft.VC90.CRT', dir + '/Microsoft.VC90.CRT')
    print '>> Copied libraries!'
else:
    print '>> Libraries not found! The frozen executable will require the '\
          'Visual C++ 2008 runtimes to be installed in order to work.'
    print '>> In order to automatically include the runtimes, place the '\
          'Microsoft.VC90.CRT folder into this folder.'

print '>> Koopatlas has been frozen to %s!' % dir
