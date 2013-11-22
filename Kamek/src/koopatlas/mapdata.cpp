#include "koopatlas/mapdata.h"


// HELPER FUNCTIONS
dKPPath_s *dKPNode_s::getOppositeExitTo(dKPPath_s *path, bool mustBeAvailable) {
	for (int i = 0; i < 4; i++) {
		dKPPath_s *check = exits[i];

		if (check != 0 && check != path) {
			if (mustBeAvailable && !check->isAvailable)
				continue;

			return check;
		}
	}

	return 0;
}

int dKPNode_s::getExitCount(bool mustBeAvailable) {
	int ct = 0;

	for (int i = 0; i < 4; i++)
		if (exits[i] && (mustBeAvailable ? exits[i]->isAvailable : true))
			ct++;

	return ct;
}

bool dKPNode_s::isUnlocked() {
	for (int i = 0; i < 4; i++)
		if (exits[i]) 
			if (exits[i]->isAvailable)
				return true; 
	return false;
}

void dKPNode_s::setupNodeExtra() {
	int world = this->levelNumber[0];
	int level = this->levelNumber[1];

	SaveBlock *save = GetSaveFile()->GetBlock(-1);
	u32 conds = save->GetLevelCondition(world-1, level-1);

	bool isUnlocked = this->isUnlocked();
	bool exitComplete = false;
	bool secretComplete = false;

	if (conds & 0x10)
		exitComplete = true;
	if (conds & 0x20)
		secretComplete = true;

	const char *colour;

	//OSReport("Level %d-%d, isUnlocked: %d, exitComplete: %d", world, level, isUnlocked, exitComplete);

	// default: non-unlocked levels AND completed one-time levels
	colour = "g3d/black.brres";

	// one-time levels
	if ((level >= 30) && (level <= 37)) {
		if (isUnlocked && !exitComplete)
			colour = "g3d/red.brres";
	}
	// the shop
	else if (level == 99)
		colour = "g3d/shop.brres";

	else if (isUnlocked) {
		if (hasSecret) {
			if (exitComplete && secretComplete)
				colour = "g3d/blue.brres";
			else if (exitComplete || secretComplete)
				colour = "g3d/purple.brres";
			else
				colour = "g3d/red.brres";
		} else {
			if (exitComplete)
				colour = "g3d/blue.brres";
			else
				colour = "g3d/red.brres";
		}
	}

	// model time
	this->extra->mallocator.link(-1, GameHeaps[0], 0, 0x20);

	nw4r::g3d::ResFile rg(getResource("cobCourse", colour));
	this->extra->model.setup(rg.GetResMdl("cobCourse"), &this->extra->mallocator, 0x224, 1, 0);
	this->extra->matrix.identity();
	SetupTextures_MapObj(&this->extra->model, 0);

	this->extra->mallocator.unlink();
}

void dKPNode_s::setLayerAlpha(u8 alpha) {
	if (tileLayer)
		tileLayer->alpha = alpha;
	if (doodadLayer)
		doodadLayer->alpha = alpha;
}

void dKPPath_s::setLayerAlpha(u8 alpha) {
	if (tileLayer)
		tileLayer->alpha = alpha;
	if (doodadLayer)
		doodadLayer->alpha = alpha;
}

int dKPLayer_s::findNodeID(dKPNode_s *node) {
	for (int i = 0; i < nodeCount; i++)
		if (nodes[i] == node)
			return i;

	return -1;
}

dKPMapData_c::dKPMapData_c() {
	data = 0;
	fixedUp = false;
	levelNodeExtraArray = 0;
	tilesetLoaders = 0;
	tilesetsLoaded = false;
}

dKPMapData_c::~dKPMapData_c() {
	if (levelNodeExtraArray)
		delete[] levelNodeExtraArray;

	unloadTilesets();

	m_fileLoader.unload();
}

bool dKPMapData_c::load(const char *filename) {
	data = (dKPMapFile_s*)m_fileLoader.load(filename, 0, mHeap::archiveHeap);

	if (data == 0)
		return false;

	if (!fixedUp)
		fixup();

	bool didLoadTilesets = loadTilesets();
	bool didLoadBG = (bgLoader.load(data->backgroundName) != 0);

	return didLoadTilesets && didLoadBG;
}

bool dKPMapData_c::loadTilesets() {
	if (tilesetsLoaded)
		return true;

	if (tilesetLoaders == 0)
		tilesetLoaders = new dDvdLoader_c[data->tilesetCount];

	bool result = true;

	for (int i = 0; i < data->tilesetCount; i++) {
		char *filename = ((char*)data) + (data->tilesets[i].val[3] - 0x10000000);
		void *heap = (i < 9) ? mHeap::gameHeaps[2] : mHeap::archiveHeap;
		result &= (tilesetLoaders[i].load(filename, 0, heap) != 0);
	}

	if (result) {
		tilesetsLoaded = true;

		for (int i = 0; i < data->tilesetCount; i++) {
			data->tilesets[i].val[3] = (((u32)tilesetLoaders[i].buffer & ~0xC0000000) >> 5);
		}
	}

	return tilesetsLoaded;
}

void dKPMapData_c::unloadTilesets() {
	if (tilesetLoaders != 0) {
		for (int i = 0; i < data->tilesetCount; i++) {
			tilesetLoaders[i].unload();
		}

		delete[] tilesetLoaders;
	}

	bgLoader.unload();
}

void dKPMapData_c::fixup() {
	OSReport("Setting up Nodes");

	fixedUp = true;

	fixRef(data->layers);
	fixRef(data->tilesets);
	fixRef(data->unlockData);
	fixRef(data->sectors);
	fixRef(data->backgroundName);
	if (data->version >= 2) {
		fixRef(data->worlds);
		for (int i = 0; i < data->worldCount; i++) {
			fixRef(data->worlds[i].name);
		}
	}

	for (int iLayer = 0; iLayer < data->layerCount; iLayer++) {
		dKPLayer_s *layer = fixRef(data->layers[iLayer]);

		switch (layer->type) {
			case dKPLayer_s::OBJECTS:
				fixRef(layer->tileset);
				break;

			case dKPLayer_s::DOODADS:
				for (int iDood = 0; iDood < layer->doodadCount; iDood++) {
					dKPDoodad_s *doodad = fixRef(layer->doodads[iDood]);

					fixRef(doodad->texObj);
					fixTexObjSafe(doodad->texObj);
				}
				break;
				
			case dKPLayer_s::PATHS:
				pathLayer = layer;

				fixRef(layer->paths);
				fixRef(layer->nodes);

				for (int iPath = 0; iPath < layer->pathCount; iPath++) {
					dKPPath_s *path = fixRef(layer->paths[iPath]);

					fixRef(path->start);
					fixRef(path->end);

					fixRef(path->tileLayer);
					fixRef(path->doodadLayer);
				}

				for (int iNode = 0; iNode < layer->nodeCount; iNode++) {
					dKPNode_s *node = fixRef(layer->nodes[iNode]);

					for (int i = 0; i < 4; i++)
						fixRef(node->exits[i]);

					fixRef(node->tileLayer);
					fixRef(node->doodadLayer);

					if (node->type == dKPNode_s::CHANGE) {
						fixRef(node->destMap);
						OSReport("Node: %x, %s", node->destMap, node->destMap); }

				}
				break;
		}
	}

	
	// before we finish here, create the Node Extra classes

	// first off, count how many we need...
	int count = 0;
	for (int nodeIdx = 0; nodeIdx < pathLayer->nodeCount; nodeIdx++) {
		if (pathLayer->nodes[nodeIdx]->type == dKPNode_s::LEVEL)
			count++;
	}

	levelNodeExtraArray = new dKPNodeExtra_c[count];

	int extraIdx = 0;

	for (int nodeIdx = 0; nodeIdx < pathLayer->nodeCount; nodeIdx++) {
		if (pathLayer->nodes[nodeIdx]->type == dKPNode_s::LEVEL) {
			pathLayer->nodes[nodeIdx]->extra = &levelNodeExtraArray[extraIdx];
			extraIdx++;
		}
	}
}


const dKPWorldDef_s *dKPMapData_c::findWorldDef(int id) const {
	for (int i = 0; i < data->worldCount; i++) {
		if (data->worlds[i].key == id)
			return &data->worlds[i];
	}

	return 0;
}


/******************************************************************************
 * Generic Layer
 ******************************************************************************/


/******************************************************************************
 * Tile Layer
 ******************************************************************************/


/******************************************************************************
 * Doodad Layer
 ******************************************************************************/


/******************************************************************************
 * Path Layer
 ******************************************************************************/



