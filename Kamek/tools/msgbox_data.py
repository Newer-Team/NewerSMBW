messages = [
		# Message 0
		(0x100, 'A Test Message',
			'This is a test message.\nWith some lines.\nAnd more lines.\nAnd a really long line to see if nw4r::lyt can handle wrapping or not...\nFinal line.'
			),
		]

import struct, sys, os.path, codecs

if len(sys.argv) > 1:
	target = sys.argv[1]
elif os.path.exists('/home/me/Games/Newer'):
	target = '/home/me/Games/Newer/ISO/files/NewerRes/Messages.bin'
else:
	target = 'Messages.bin'

messageCount = len(messages)

stringOffset = (messageCount * 0xC) + 4
infoStruct = struct.Struct('>III')

headerData = bytearray(struct.pack('>I', messageCount))
stringData = bytearray()

for msgID, title, msg in messages:
	titleOffset = stringOffset + len(stringData)
	stringData += codecs.utf_16_be_encode(title)[0] + '\0\0'
	msgOffset = stringOffset + len(stringData)
	stringData += codecs.utf_16_be_encode(msg)[0] + '\0\0'

	headerData += infoStruct.pack(msgID, titleOffset, msgOffset)

with open(target, 'wb') as out:
	out.write(headerData)
	out.write(stringData)

