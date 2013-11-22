#ifndef __KOOPATLAS_MAP_H
#define __KOOPATLAS_MAP_H

#include "koopatlas/core.h"

class dWMMap_c : public dBase_c {
	public:
		dWMMap_c();

		int onCreate();
		int onDelete();
		int onExecute();
		int onDraw();

		class renderer_c : public m3d::proc_c {
			public:
				mAllocator_c allocator;

				void drawOpa();
				void drawXlu();

			private:
				void drawLayers();

				void beginRendering();
				void endRendering();

				void renderTileLayer(dKPLayer_s *layer, dKPLayer_s::sector_s *sector);
				void renderDoodadLayer(dKPLayer_s *layer);

				void loadTexture(GXTexObj *obj);

				void loadCamera();
				void loadCamera(Mtx m);

				GXTexObj *currentTexture;
				Mtx renderMtx;
				float baseZ;

				int minX, minY, maxX, maxY;
		};
		
		renderer_c renderer;
		enum EffRenderCount { EFFECT_RENDERER_COUNT = 12 };
		BGGMEffectRenderer effectRenderers[EFFECT_RENDERER_COUNT];

		mEf::es2 effects[7];
		void doEffects();

		mHeapAllocator_c allocator;
		m3d::mdl_c bgModel;
		m3d::anmTexSrt_c bgSrtAnm;
		mMtx bgMatrix;

		bool showLaunchStar;
		float launchStarX, launchStarY;
		m3d::mdl_c launchStarModel;
		m3d::anmChr_c launchStarAnm;
		mMtx launchStarMatrix;

		void spinLaunchStar();

		void renderPathLayer(dKPLayer_s *layer);

		static dWMMap_c *build();
		static dWMMap_c *instance;
};

#endif

