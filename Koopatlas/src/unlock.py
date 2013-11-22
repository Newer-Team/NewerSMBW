import re

LEVEL_RE = re.compile(r'^([0-9]{1,2})-([0-9]{1,2})( secret)?$')
COINS_RE = re.compile(r'^(unspent |total )?star coins (<|>|==|!=) ([0-9]{1,3})$')
COMBINER_RE = re.compile(r'[ ]*(and|or)[ ]*')

class UnlockParseError(ValueError):
	# todo: is this the proper way to make an Error?
	pass

def parseUnlockText(text):
	parsed = _parseUnlockBit(text.lower())
	if parsed == ('always',):
		return None
	else:
		return parsed

def _parseUnlockBit(text):
	# blank criterion
	if text == '':
		return ('always',)

	# is this a simple one...?
	m = LEVEL_RE.match(text)
	if m:
		one, two, secret = m.groups()
		w = int(one)
		l = int(two)
		if w < 1 or w > 10:
			raise UnlockParseError('world must be between 1 to 10 inclusive; not %s' % w)
		return ('level', w, l, (secret != None))

	# ... or star coins?
	m = COINS_RE.match(text)
	if m:
		is_unspent, op, count_s = m.groups()
		if is_unspent == 'unspent ':
			mode = 'unspent'
		else:
			mode = 'total'
		return ('star_coins', op, int(count_s), mode)

	# OK, let's parse parentheses
	pLevel = 0
	endAt = len(text) - 1

	# this could be either AND or OR or nothing at all
	# we won't know it until we finish parsing!
	whatCombiner = None

	subTerms = []
	currentSubTermStart = None

	skip = 0

	for index, char in enumerate(text):
		if skip > 0:
			skip -= 1
			continue

		if char == '(':
			if pLevel == 0:
				currentSubTermStart = index
			pLevel += 1
		elif char == ')':
			pLevel -= 1
			if pLevel < 0:
				raise UnlockParseError('close parenthesis without a matching open')
			elif pLevel == 0:
				subTerms.append((currentSubTermStart, index, text[currentSubTermStart+1:index]))
				if len(subTerms) > 64:
					raise UnlockParseError('no more than 64 subterms in one %s condition' % whatCombiner.upper())

				# are we expecting to see something else?
				if index == endAt: break

				m = COMBINER_RE.match(text, index + 1)
				if not m:
					raise UnlockParseError('something unexpected at position %d' % (index+1))

				# what is it?
				nextCombiner = m.group(1)
				if whatCombiner is not None and nextCombiner != whatCombiner:
					raise UnlockParseError('mixed %s and %s in one term. use more parentheses!' % (whatCombiner,nextCombiner))
				whatCombiner = nextCombiner

				# go right past this, to the next subterm
				skip = len(m.group(0))
				if (index + skip) == endAt:
					raise UnlockParseError('%s what?!' % (whatCombiner.upper()))
		else:
			if pLevel == 0:
				if index == 0:
					raise UnlockParseError('that\'s not right')
				else:
					raise UnlockParseError('something unexpected at position %d' % index)
	
	if pLevel > 0:
		raise UnlockParseError('unclosed parenthesis')
	
	# now that we're here, we must have parsed these subterms
	# do we have a combiner?
	if whatCombiner is None:
		if len(subTerms) != 1:
			raise UnlockParseError('unclosed parenthesis')

		return _parseUnlockBit(subTerms[0][2])
	else:
		return (whatCombiner, map(lambda x: _parseUnlockBit(x[2]), subTerms))


def stringifyUnlockData(data):
	if data == None:
		return ''

	kind = data[0]

	if kind == 'always':
		return ''
	elif kind == 'level':
		return '%02d-%02d%s' % (data[1], data[2], (' secret' if data[3] else ''))
	elif kind == 'star_coins':
		return '%s star coins %s %d' % (data[3], data[1], data[2])
	elif kind == 'and' or kind == 'or':
		return (' %s ' % kind).join(map(lambda x: '(%s)' % stringifyUnlockData(x), data[1]))


def packUnlockSpec(data):
	kind = data[0]

	if kind == 'always':
		return '\x0F'

	elif kind == 'level':
		k, world, level, secret = data

		one = (1 << 6) | (0x10 if secret else 0) | (world - 1)

		return chr(one) + chr(level - 1)

	elif kind == 'star_coins':
		k, op, count, mode = data
		ops = ('==', '!=', '<', '>')
		one = (0 << 6) | ops.index(op)
		if mode == 'unspent':
			modeID = 0x80
		else:
			modeID = 0
		return chr(one) + chr(modeID | (count >> 8)) + chr(count & 0xFF)

	elif kind == 'and' or kind == 'or':
		terms = data[1]
		cond = 2 if (kind == 'and') else 3
		one = (cond << 6) | (len(terms) - 1)

		return chr(one) + ''.join(map(packUnlockSpec, terms))


if __name__ == '__main__':
	p1 = parseUnlockText('((01-01 secret) and (01-02)) or (02-99 secret) or (01-01)')
	p2 = parseUnlockText('(1-1 secret) or ((1-2) and (1-3 secret)) or (2-1)')
	p3 = parseUnlockText('(star coins > 200) or ((unspent star coins == 300) and (total star coins != 400))')
	
	print
	print repr(p1)
	print
	print stringifyUnlockData(p1)
	print
	print repr(p2)
	print
	print stringifyUnlockData(p2)
	print
	print repr(p3)
	print
	print stringifyUnlockData(p3)

	from sys import exit
	exit()





from common import *




class KPUnlockSpecDialog(QtGui.QDialog):
	def __init__(self, forWhat, unlockAdjective):
		QtGui.QDialog.__init__(self)

		self.setWindowTitle('Set Unlock Criteria')

		text = """You may enter various criteria that must be fulfilled for this {0} to be {1}.<br>
			<br>
			Here are some examples of what you can use:
			<ul>
				<li>01-01 - <i>a single criterion</i></li>
				<li>01-01 secret - <i>secret exits</i></li>
				<li>(01-01 secret) and (01-02) - <i>combine two criteria</i></li>
				<li>((01-01 secret) or (01-02)) and (01-04) - <i>nested criteria</i></li>
			</ul>
			Each criterion used on the sides of AND and OR must be surrounded by parentheses.
			You may use more than one, for example: <i>(01-01) or (02-02) or (03-03)</i><br>
			<br>
			To leave this {0} permanently unlocked, leave the box blank.
			""".format(forWhat, unlockAdjective)

		self.label = QtGui.QLabel(text)
		self.label.setWordWrap(True)

		self.textBox = QtGui.QLineEdit()
		self.textBox.textChanged.connect(self.checkInputValidity)

		self.statusLabel = QtGui.QLabel()
		self.statusLabel.setWordWrap(True)

		self.buttons = QtGui.QDialogButtonBox(
				QtGui.QDialogButtonBox.Ok | QtGui.QDialogButtonBox.Cancel)

		self.buttons.accepted.connect(self.accept)
		self.buttons.rejected.connect(self.reject)

		self.layout = QtGui.QVBoxLayout()
		self.layout.addWidget(self.label)
		self.layout.addWidget(self.textBox)
		self.layout.addWidget(self.statusLabel)
		self.layout.addWidget(self.buttons)
		self.setLayout(self.layout)

		self.spec = None
	
	def setSpec(self, spec):
		self.textBox.setText(stringifyUnlockData(spec))
	
	def checkInputValidity(self, text):
		valid = True
		try:
			self.spec = parseUnlockText(str(text))
		except UnlockParseError as e:
			valid = False
			error = str(e)
			self.spec = None

		self.buttons.button(QtGui.QDialogButtonBox.Ok).setEnabled(valid)

		if valid:
			self.statusLabel.setText('Your input is valid.')
		else:
			self.statusLabel.setText('[!] %s' % error)

