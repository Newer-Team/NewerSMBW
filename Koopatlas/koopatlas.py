#!/usr/bin/env python2

# Koopatlas
# A project by Treeki and Tempus
# Started 2nd November 2011

import os.path, sys

if hasattr(sys, 'frozen'):
	sys.path.append(os.path.join(os.path.dirname(sys.executable), 'src'))
else:
	sys.path.append(os.path.join(os.path.dirname(__file__), 'src'))



from common import *

if __name__ == '__main__':
	KP.run()

