class RandTileGenerator
	Types = [:none, :horz, :vert, :both]
	Specials = [nil, :vdouble_top, :vdouble_bottom]

	def initialize
		@sections = {}
	end

	def section(*namelist)
		raise "what" unless namelist.all?{|x| x.is_a?(String)}
		@current_section = (@sections[namelist] ||= {entries: []})
		yield
	end

	def random(range, type=:both, numbers=nil)
		case range
		when Range
			# Regular handling
			numbers = range if numbers.nil?
			@current_section[:entries] << {range: range, type: type, numbers: numbers.to_a}
		when Numeric
			# One number
			random(range..range, type, numbers)
		when Enumerable
			# An array or something else similar
			numbers = range if numbers.nil?
			range.each { |r| random(r, type, numbers) }
		end
	end

	def pack
		# first, work out an offset for every section and entry
		# also, collect the data for each individual entry into an Array
		current_offset = 8 + (@sections.count * 4)
		all_entry_data = []

		@sections.each_pair do |name, section|
			section[:offset] = current_offset
			current_offset += 8

			section[:entries].each do |entry|
				entry[:offset] = current_offset
				all_entry_data << entry[:numbers]
				current_offset += 8
			end
		end

		# assign an offset to each section name list
		namelist_offsets = {}
		@sections.each_key do |namelist|
			namelist_offsets[namelist] = current_offset
			current_offset += 4 + (4 * namelist.size)
		end

		# assign an offset to each piece of entry data
		data_offsets = {}
		all_entry_data.uniq!

		all_entry_data.each do |data|
			data_offsets[data] = current_offset
			current_offset += data.size
		end

		# assign an offset to each section name
		name_offsets = {}
		@sections.each_key do |namelist|
			namelist.each do |name|
				name_offsets[name] = current_offset
				current_offset += name.size + 1
			end
		end

		# now pack it all together
		header = ['NwRT', @sections.count].pack('a4 N')
		offsets = @sections.each_value.map{|s| s[:offset]}.pack('N*')

		section_data = @sections.each_pair.map do |namelist, section|
			namelist_offset = namelist_offsets[namelist] - section[:offset]
			entry_count = section[:entries].count

			entry_data = section[:entries].map do |entry|
				lower_bound = entry[:range].min
				upper_bound = entry[:range].max

				count = entry[:numbers].count

				type_sym, special_sym = entry[:type].to_s.split('_', 2).map(&:to_sym)
				type_id = Types.index(type_sym)
				special_id = Specials.index(special_sym)
				type = type_id | (special_id << 2)

				num_offset = data_offsets[entry[:numbers]] - entry[:offset]

				[lower_bound, upper_bound, count, type, num_offset].pack('CCCC N')
			end

			[namelist_offset, entry_count].pack('NN') + entry_data.join
		end

		namelist_data = @sections.each_key.map do |namelist|
			puts "Writing list: #{namelist.inspect}"
			count = namelist.size
			c_offsets = namelist.map{|n| name_offsets[n] - namelist_offsets[namelist]}
			puts "Offsets: #{c_offsets.inspect}"

			[count].pack('N') + c_offsets.pack('N*')
		end

		output = [header, offsets]
		output += section_data
		output += namelist_data
		output += all_entry_data.map{|data| data.pack('C*')}
		output << @sections.keys.flatten.join("\0")
		output << "\0"
		output.join
	end


	def regular_terrain
		# Left Side
		random([0x10, 0x20, 0x30, 0x40], :vert)
		# Right Side
		random([0x11, 0x21, 0x31, 0x41], :vert)
		# Top Side
		random(2..7, :horz)
		# Bottom Side
		random(0x22..0x27, :horz)
		# Middle
		random(0x12..0x17)
	end

	def sub_terrain
		# Left Side
		random([0x18, 0x28, 0x38, 0x48], :vert)
		# Right Side
		random([0x19, 0x29, 0x39, 0x49], :vert)
		# Top Side
		random(0xA..0xF, :horz)
		# Bottom Side
		random(0x2A..0x2F, :horz)
		# Middle
		random(0x1A..0x1F)
	end
end


g = RandTileGenerator.new
g.section('TestTileset') do
	g.random(1..20)
	g.random(21..24, :none)
	g.random(250..255, :vert, 0..5)
end

regular_ts1 = %w(chika setsugen shiro suichu daishizen sabaku_chika)
regular_ts1 += %w(nohara2 kurayami_chika shiro_yogan koopa_out shiro_koopa gake_yougan)
regular_ts2 = %w(doukutu doukutu2 doukutu3 doukutu4 doukutu5 doukutu6 doukutu8)
newer = %w(ghostrocks Pa1_daishizenkuri Pa1_darkmtmush Pa1_e3setsugen Pa2_darkcave)
newer += %w(Pa3_autumnbg Pa2_volcanobg Pa1_mushcastle Pa1_pumpkin)

regular_ts1.map!{ |x| "Pa1_#{x}" }
regular_ts2.map!{ |x| "Pa2_#{x}" }
g.section(*regular_ts1, *regular_ts2, *newer) do
	g.regular_terrain
end

nohara_clones = %w(nohara cracks springwater space chika2)
g.section(*nohara_clones.map{ |x| "Pa1_#{x}"}) do
	g.regular_terrain
	g.sub_terrain
end

g.section('Pa1_gake', 'Pa1_gake_nocoll', 'Pa1_Mountains') do
	g.regular_terrain
	g.sub_terrain
	g.random([0xAA, 0xBA, 0xCA, 0xDA], :vert)
	g.random([0xAB, 0xBB, 0xCB, 0xDB], :vert)
	g.random([0x8E, 0x9E, 0xAE, 0xBE], :vert)
	g.random([0x8F, 0x9F, 0xAF, 0xBF], :vert)
	g.random(0xC4..0xC9, :horz)
	g.random(0xD4..0xD9, :horz)
	g.random(0xE4..0xE9, :horz)
end

g.section('Pa1_kaigan', 'Pa1_kaiganplus') do
	g.regular_terrain
	g.random(0x18..0x1B)
	g.random(0x28..0x2A)
	g.random(0x3A..0x3D)
end

g.section('Pa1_obake_soto') do
	g.random(0xA..0xF, :horz)
	g.random(0x1A..0x1F)
	g.random(0x2A..0x2F, :horz)
	g.random([0x4E, 0x5E, 0x6E, 0x7E], :vert)
	g.random([0x4F, 0x5F, 0x6F, 0x7F], :vert)
end

g.section('Pa1_korichika') do
	g.regular_terrain
	g.random(0x6A..0x6F, :horz)
	g.random(0x7A..0x7F)
	g.random(0x8A..0x8F, :horz)
	g.random([0x1E, 0x2E, 0x3E, 0x4E], :vert)
	g.random([0x1F, 0x2F, 0x3F, 0x4F], :vert)
	g.random(0x6A..0x6F, :horz)
	g.random(0x7A..0x7F)
	g.random(0x8A..0x8F, :horz)
	g.random([0xB8, 0xC8, 0xD8, 0xE8], :vert)
	g.random([0xB9, 0xC9, 0xD9, 0xE9], :vert)
end

g.section('Pa1_shiro_boss1') do
	g.regular_terrain
	g.random(0x60..0x65, :horz)
end

g.section('Pa2_kori', 'Pa2_e3kori') do
	g.regular_terrain
	g.random(0xA0..0xA5, :horz)
	g.random(0xB0..0xB5, :horz)
end


g.section('Cloudscape', 'pa2_clouds') do
	g.random(0x20..0x23, :horz)
	g.random(0x30..0x33)
	g.random(0x24..0x27, :horz)
	g.random([0x8, 0x18, 0x28, 0x38], :vert)
	g.random([0x9, 0x19, 0x29, 0x39], :vert)
end

g.section('Pa1_autumncastle', 'Pa3_autumncastle', 'Pa1_snowfort', 'Pa2_ghostcastle', 'Pa2_crystalcastle') do
	g.random([0xB0, 0xC0, 0xD0, 0xE0], :vert)
	g.random([0xB1, 0xC1, 0xD1, 0xE1], :vert)
	g.random(0xB2..0xB7, :horz)
	g.random(0xC2..0xC7)
	g.random(0xD2..0xD7, :horz)
end

g.section('CrackedMega') do
	g.random([0x20, 0x30, 0x40], :vert)
	g.random([0x21, 0x31, 0x41], :vert)
	g.random(0x22..0x27, :horz)
	g.random(0x2A..0x2F, :horz)
	g.random([0x5E, 0x6E, 0x7E], :vert)
	g.random([0x5F, 0x6F, 0x7F], :vert)
	g.random(0xB0..0xDF)
	g.random(0x2..0x7, :horz_vdouble_top)
	g.random(0x12..0x17, :horz_vdouble_bottom)
	g.random(0xA..0xF, :horz_vdouble_top)
	g.random(0x1A..0x1F, :horz_vdouble_bottom)
end

g.section('Pa1_dessert') do
	g.regular_terrain
	g.random(0xC..0xE, :horz)
	g.random(0x1C..0x1E, :horz)
end

g.section('Pa1_freezeflame') do
	g.random(0x1A..0x1F, :horz)
	g.random(0x2A..0x2F, :horz)
	g.random([0x10, 0x20, 0x30, 0x40], :vert)
	g.random([0x11, 0x21, 0x31, 0x41], :vert)
	g.random([0x18, 0x28, 0x38, 0x48], :vert)
	g.random([0x19, 0x29, 0x39, 0x49], :vert)
	g.random(0x2..0x7, :horz)
	g.random(0x12..0x17)
	g.random(0x22..0x27, :horz)
	g.random(0x6A..0x6F)
	g.random(0x52..0x57, :horz)
	g.random([0x96, 0xA6, 0xB6, 0xC6], :vert)
	g.random([0x97, 0xA7, 0xB7, 0xC7], :vert)
	g.random([0xBE, 0xCE, 0xDE, 0xEE], :vert)
	g.random([0xBF, 0xCF, 0xDF, 0xEF], :vert)
	g.random(0xB8..0xBD, :horz)
	g.random(0xC8..0xCD)
	g.random(0xD8..0xDD, :horz)
	g.random(0xE8..0xED)
	g.random(0xF8..0xFD, :horz)
end

g.section(*%w(Pa1_magicsnow Pa2_magicsnow Pa3_magicsnow)) do
	g.random(0xB4..0xB9, :horz)
end

g.section('Pa1_graveyard', 'Pa3_graveyars', 'Pa3_graveyard') do
	g.regular_terrain
	g.random(0x19..0x1E, :horz)
end

g.section('Pa3_ghostship') do
	g.random(0xA..0xF, :horz)
	g.random(0x1A..0x1F)
	g.random(0x2A..0x2F, :horz)
	g.random([0x4E, 0x5E, 0x6E, 0x7E], :vert)
	g.random([0x4F, 0x5F, 0x6F, 0x7F], :vert)
	g.random(0xA0..0xA3, :horz)
end

g.section('Pa1_lavaglow') do
	g.random([0x20, 0x30, 0x40, 0x50], :vert)
	g.random([0x21, 0x31, 0x41, 0x51], :vert)
	g.random(0x2..0x7, :horz_vdouble_top)
	g.random(0x12..0x17, :horz_vdouble_bottom)
	g.random(0x22..0x27)
	g.random(0x32..0x37, :horz)
	g.random(0x28..0x2D, :horz)
end

g.section('Pa1_StarRoad') do
	g.random(0x11, :both, [0x11, 0x4C, 0x4D, 0x4E, 0x4F, 0x5C, 0x5D, 0x5E, 0x5F])
end

g.section('Pa2_forestobake') do
	g.random(0..5, :horz)
	g.random(0x10..0x15)
	g.random(0x20..0x25, :horz)
	g.random([0x40, 0x50, 0x60, 0x70], :vert)
	g.random([0x41, 0x51, 0x61, 0x71], :vert)
end

g.section('Pa1_newnohara') do
	g.sub_terrain

	g.random(2..7, :horz)
	g.random(0x22..0x27, :horz)
	g.random([0x10, 0x20, 0x30, 0x40], :vert)
	g.random([0x11, 0x21, 0x31, 0x41], :vert)
	random_set = (0xAA..0xAF).to_a + (0xBA..0xBF).to_a + (0xCA..0xCF).to_a
	random_set += (0xDA..0xDF).to_a + (0xEA..0xEF).to_a + (0xFA..0xFF).to_a
	g.random(0x12..0x17, :both, random_set)
	g.random(0xAA..0xAF, :both, random_set)
	g.random(0xBA..0xBF, :both, random_set)
	g.random(0xCA..0xCF, :both, random_set)
	g.random(0xDA..0xDF, :both, random_set)
	g.random(0xEA..0xEF, :both, random_set)
	g.random(0xFA..0xFF, :both, random_set)
end

g.section('Pa1_mtmush') do
	g.sub_terrain

	g.random(2..7, :horz)
	g.random(0x22..0x27, :horz)
	g.random([0x10, 0x20, 0x30, 0x40], :vert)
	g.random([0x11, 0x21, 0x31, 0x41], :vert)
	random_set = (0xDA..0xDF).to_a + (0xEA..0xEF).to_a + (0xFA..0xFF).to_a
	g.random(0x12..0x17, :both, random_set)
	g.random(0xDA..0xDF, :both, random_set)
	g.random(0xEA..0xEF, :both, random_set)
	g.random(0xFA..0xFF, :both, random_set)
end

g.section('Pa1_darkmtmush') do
	g.sub_terrain

	g.random(2..7, :horz)
	g.random(0x22..0x27, :horz)
	g.random([0x10, 0x20, 0x30, 0x40], :vert)
	g.random([0x11, 0x21, 0x31, 0x41], :vert)
	random_set = (0xCC..0xCF).to_a + (0xDC..0xDF).to_a + (0xEC..0xEF).to_a + (0xFC..0xFF).to_a
	g.random(0x12..0x17, :both, random_set)
	g.random(0xCC..0xCF, :both, random_set)
	g.random(0xDC..0xDF, :both, random_set)
	g.random(0xEC..0xEF, :both, random_set)
	g.random(0xFC..0xFF, :both, random_set)
end

g.section('Pa1_risingsun') do
	g.random([0x19, 0x1A, 0x29, 0x2A, 0xA3, 0xA6], :both)
end



File.open('/home/me/Games/Newer/DolphinPatch/NewerRes/RandTiles.bin', 'wb') do |f|
	f.write g.pack
end

