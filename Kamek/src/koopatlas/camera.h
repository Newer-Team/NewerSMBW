#ifndef __KOOPATLAS_CAMERA_H
#define __KOOPATLAS_CAMERA_H

#include "koopatlas/core.h"

#define STD_ZOOM 2.8f

class dWorldCamera_c : public dBase_c {
	public:
		int onCreate();
		int onDelete();
		int onExecute();
		int onDraw();

		dWorldCamera_c();

		EGG::Screen screen;
		EGG::LookAtCamera camera3d;

		EGG::ProjectOrtho projection2d;
		EGG::LookAtCamera camera2d;

		Vec camPos, camTarget, camUp;

		float currentX, currentY, zoomLevel;

		void calculateScreenGeometry();


		float zoomDivisor, screenLeft, screenTop, screenWidth, screenHeight;

		bool followPlayer;
		void panToBounds(float left, float top, float right, float bottom);
		void panToPosition(float x, float y, float zoom=STD_ZOOM);
		bool panning;
		float panFromX, panFromY, panToX, panToY;
		float panFromZoom, panToZoom;
		float panCurrentStep;
		float panTotalSteps;

		void doStuff(float);
		void generateCameraMatrices();
		void updateCameras();

		static dWorldCamera_c *build();
		static dWorldCamera_c *instance;
};

#endif

