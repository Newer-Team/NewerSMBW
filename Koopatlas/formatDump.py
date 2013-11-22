from stream import DataInputStream

LAYER_TYPES = ('objects', 'doodads', 'paths')
ANIM_LOOP_TYPES = ('contiguous', 'loop', 'reverse loop')
ANIM_CURVE_TYPES = ('linear', 'sin', 'cos')
ANIM_TYPES = ('X pos', 'Y pos', 'angle', 'X scale', 'Y scale', 'opacity')
NODE_TYPES = ('passthrough', 'stop', 'level', 'change', 'world change')

def readTexObj(st):
	st.skip(8)
	bit1 = st.read_u32()
	ptr = st.read_u32()
	st.skip(12)
	bit2 = st.read_u16()
	st.skip(2)
	if (ptr & 0x10000000) != 0:
		offsBit = 'file->0x%x' % (ptr & 0xFFFFFFF)
	else:
		offsBit = 'ram->0x%x' % (ptr << 5)
	return 'GXTexObj(%s, 0x%x, 0x%x)' % (offsBit,bit1,bit2)

def examine(data):
	st = DataInputStream(data)

	magic = st.read_u32()
	version = st.read_s32()
	yield('Magic:%08x / Version:%d' % (magic,version))

	layerCount = st.read_s32()
	layerOffs = st.read_u32()
	tilesetCount = st.read_s32()
	tilesetOffs = st.read_u32()
	yield('Layers: %d @ 0x%x, Tilesets: %d @ 0x%x' % (layerCount,layerOffs,tilesetCount,tilesetOffs))

	unlockOffs = st.read_u32()
	sectorOffs = st.read_u32()
	bgNameOffs = st.read_u32()
	worldOffs = st.read_u32()
	worldCount = st.read_s32()
	yield('Unlocks @ 0x%x, Sectors @ 0x%x, BGName @ 0x%x, Worlds: %d at 0x%x' % (unlockOffs,sectorOffs,bgNameOffs,worldCount,worldOffs))

	st.seek(layerOffs)
	for i in range(layerCount):
		tOffs = st.read_u32()
		ls = st.at(tOffs)
		lType = ls.read_u32()
		alpha = ls.read_u8()
		ls.skip(3)
		yield('* Layer %d at 0x%x: %s; alpha %d' % (i,tOffs,LAYER_TYPES[lType],alpha))

		if lType == 0: #OBJECTS
			yield('  - Tileset: %s' % readTexObj(ls.at(ls.read_u32())))
			sectorBounds = tuple([ls.read_s32() for x in range(4)])
			realBounds = tuple([ls.read_s32() for x in range(4)])
			yield('  - Sector Bounds: Left:%d, Top:%d, Right:%d, Bottom:%d' % sectorBounds)
			yield('  -   Real Bounds: Left:%d, Top:%d, Right:%d, Bottom:%d' % realBounds)
			w = sectorBounds[2] - sectorBounds[0] + 1
			h = sectorBounds[3] - sectorBounds[1] + 1
			count = w*h
			indices = [str(ls.read_u16()) for x in range(count)]
			yield('  - Sector Indices: %s' % (','.join(indices)))

		elif lType == 1: #DOODADS
			doodCount = ls.read_s32()
			yield('  - %d doodad%s' % (doodCount, 's' if doodCount != 1 else ''))

			for j in range(doodCount):
				dOffs = ls.read_u32()
				ds = ls.at(dOffs)

				x = ds.read_float()
				y = ds.read_float()
				w = ds.read_float()
				h = ds.read_float()
				angle = ds.read_float()
				yield('    - (%f,%f) (%f x %f) angle:%f' % (x,y,w,h,angle))

				texOffs = ds.read_u32()
				yield('    - %s' % readTexObj(ds.at(texOffs)))

				animCount = ds.read_s32()
				if animCount > 0:
					yield('    - %d animation%s' % (animCount, 's' if animCount != 1 else ''))
					for l in range(animCount):
						loop = ds.read_u32()
						curve = ds.read_u32()
						frameCount = ds.read_u32()
						type = ds.read_u32()
						start = ds.read_u32()
						end = ds.read_u32()
						delay = ds.read_u32()
						delayOffset = ds.read_u32()
						ds.skip(8)
						vars = (ANIM_LOOP_TYPES[loop], ANIM_CURVE_TYPES[curve],
								ANIM_TYPES[type], frameCount, start, end,
								delay, delayOffset)
						yield('      - %s, %s, %s | count:%d | range:%d - %d | delay: %d, offset %d' % vars)
		elif lType == 2: #PATHS
			nodeCount = ls.read_s32()
			nodesOffs = ls.read_u32()
			pathCount = ls.read_s32()
			pathsOffs = ls.read_u32()
			yield('  - Nodes: %d @ 0x%x; Paths: %d @ 0x%x' % (nodeCount, nodesOffs, pathCount, pathsOffs))

			ls.seek(nodesOffs)
			nodeOffsets = [ls.read_u32() for x in range(nodeCount)]
			ls.seek(pathsOffs)
			pathOffsets = [ls.read_u32() for x in range(pathCount)]

			for i, nodeOffs in enumerate(nodeOffsets):
				ls.seek(nodeOffs)
				x = ls.read_s16()
				y = ls.read_s16()
				exits = []
				for _ in range(4):
					exit = ls.read_u32()
					if exit == 0xFFFFFFFF:
						exits.append(None)
					else:
						exits.append(pathOffsets.index(exit))

				tileLayer = ls.read_u32()
				doodLayer = ls.read_u32()
				ls.skip(3)
				type = ls.read_u8()
				ls.skip(8)

				extra = NODE_TYPES[type]
				if type == 2: # LEVEL
					w = ls.read_u8()
					l = ls.read_u8()
					hasSecret = ls.read_u8()
					secretStr = ' (secret)' if hasSecret else ''
					extra = '%s: %d-%d%s' % (extra,w,l,secretStr)
				elif type == 3: # CHANGE
					dmOffs = ls.read_u32()
					thisID = ls.read_u8()
					foreignID = ls.read_u8()
					transition = ls.read_u8()
					extra = '%s: destMap@0x%x thisID:%d foreignID:%d transition:%d' % (extra,dmOffs,thisID,foreignID,transition)
				elif type == 4: # WORLD CHANGE
					worldID = ls.read_u8()
					extra = '%s: %d' % (extra,worldID)

				vars = (i, x, y,
						exits[0], exits[1], exits[2], exits[3],
						tileLayer, doodLayer, extra)
				yield('    - %d => (%d,%d) | L:%s R:%s U:%s D:%s | Tiles:0x%x Doodads:0x%x | %s' % vars)

			for i, pathOffs in enumerate(pathOffsets):
				ls.seek(pathOffs)

				start = nodeOffsets.index(ls.read_u32())
				end = nodeOffsets.index(ls.read_u32())

				tileLayer = ls.read_u32()
				doodLayer = ls.read_u32()

				isAvailable = ls.read_u8()
				isSecret = ls.read_u8()
				ls.skip(2)
				speed = ls.read_float()
				animation = ls.read_u32()

				vars = (i, start, end, tileLayer, doodLayer,
						isAvailable, isSecret, speed, animation)
				yield('    - %d => %d to %d | Tiles:0x%x Doodads:0x%x | Available:%d, Secret:%d | Speed: %f | Animation: %d' % vars)


	st.seek(tilesetOffs)
	for i in range(tilesetCount):
		yield('Tileset %d: %s' % (i, readTexObj(st)))



with open('old.txt', 'w') as f:
	for line in examine(open('UNCOMP_OLD.bin', 'rb').read()):
		f.write(line)
		f.write('\n')
with open('new.txt', 'w') as f:
	for line in examine(open('UNCOMP_NEW.bin', 'rb').read()):
		f.write(line)
		f.write('\n')

