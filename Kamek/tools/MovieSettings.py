import struct

class Settings(object):
	class Banner(object):
		def __init__(self, name):
			self.name = name
			self.commands = []

		def play_sound(self, delay, sound_id):
			self.commands.append((1, delay, sound_id))

		def play_looping_sound(self, handle_id, delay, sound_id):
			self.commands.append((2, handle_id, delay, sound_id))

		def stop_looping_sound(self, handle_id, delay, frame_count):
			self.commands.append((3, handle_id, delay, frame_count))

	def __init__(self, widescreen):
		self.banners = []
		self.widescreen = widescreen

	def add_banner(self, name):
		b = self.Banner(name)
		self.banners.append(b)
		return b
	
	def export(self):
		header_size = 8

		offset = header_size + (4 * len(self.banners))
		banner_offsets = []
		banner_data = []

		for b in self.banners:
			banner_offsets.append(offset)
			
			b_header1 = 'xxxx' # will be replaced later
			b_header2 = struct.pack('>bxxxI', 1 if self.widescreen else 0, len(b.commands))
			b_commands = []
			b_commands_size = 0

			for cmd in b.commands:
				ctype = cmd[0]
				if ctype == 1:
					# Play Sound
					b_commands.append(struct.pack('>III', 1, cmd[1], cmd[2]))
					b_commands_size += 12
				elif ctype == 2:
					# Play Looping Sound
					b_commands.append(struct.pack('>IIII', 2, cmd[1], cmd[2], cmd[3]))
					b_commands_size += 16
				elif ctype == 3:
					# Stop Looping Sound
					b_commands.append(struct.pack('>IIII', 3, cmd[1], cmd[2], cmd[3]))
					b_commands_size += 16

			offset += 12 + b_commands_size

			b_data = [b_header1, b_header2]
			b_data.extend(b_commands)
			banner_data.append(b_data)

		# now build a string table
		str_tab = []

		for b, b_data in zip(self.banners, banner_data):
			b_data[0] = struct.pack('>I', offset)
			offset += len(b.name) + 1
			str_tab.append(b.name)
			str_tab.append('\0')
	
		# whatever
		file_pieces = [
			'NWcs',
			struct.pack('>I', len(self.banners))
			]

		file_pieces.extend(map(lambda x: struct.pack('>I', x), banner_offsets))

		for b_data in banner_data:
			file_pieces.extend(b_data)
		
		file_pieces.extend(str_tab)

		return ''.join(file_pieces)




s = Settings(widescreen=False)

b = s.add_banner('/CS/Opening1.arc')
b.play_sound(delay=10, sound_id=740)
b.play_looping_sound(handle_id=0, delay=100, sound_id=500)
b.stop_looping_sound(handle_id=0, delay=1000, frame_count=15)
b = s.add_banner('/CS/Opening3.arc')
b = s.add_banner('/CS/Opening4.arc')
b = s.add_banner('/CS/Opening5.arc')
b = s.add_banner('/CS/Opening6.arc')
b = s.add_banner('/CS/Opening7.arc')
b = s.add_banner('/CS/Opening8.arc')
b = s.add_banner('/CS/Opening9.arc')
b = s.add_banner('/CS/Opening10.arc')
b = s.add_banner('/CS/Opening11.arc')


data = s.export()
open('Opening.cs', 'wb').write(data)

s = Settings(widescreen=True)

b = s.add_banner('/CS/Kamek1_2.arc')
b = s.add_banner('/CS/Kamek3_4.arc')
b = s.add_banner('/CS/Kamek5_6.arc')
b = s.add_banner('/CS/Kamek7_8.arc')
b = s.add_banner('/CS/Kamek9_10.arc')
b = s.add_banner('/CS/Kamek11.arc')
b = s.add_banner('/CS/Kamek12.arc')
b = s.add_banner('/CS/Kamek13_14.arc')
b = s.add_banner('/CS/Kamek15_16.arc')
b = s.add_banner('/CS/Kamek17.arc')


data = s.export()
open('Kamek.cs', 'wb').write(data)


