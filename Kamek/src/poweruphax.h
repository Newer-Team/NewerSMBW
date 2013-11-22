#ifndef POWERUPHAX_H
#define POWERUPHAX_H 

class dHammerSuitRenderer_c {
	public:
		mHeapAllocator_c allocator;

		m3d::mdl_c helmet, shell;

		m3d::mdl_c *victimModel;
		u32 headNodeID, rootNodeID;

		dPlayerModel_c *victim;

		dHammerSuitRenderer_c();
		~dHammerSuitRenderer_c();
		void setup(dPlayerModelHandler_c *handler);
		void setup(dPlayerModelHandler_c *handler, int sceneID);
		void draw();

		static dHammerSuitRenderer_c *build();
};

#endif /* POWERUPHAX_H */
