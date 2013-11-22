#include "koopatlas/camera.h"
#include "koopatlas/player.h"

dWorldCamera_c *dWorldCamera_c::instance = 0;

dWorldCamera_c *dWorldCamera_c::build() {

	void *buffer = AllocFromGameHeap1(sizeof(dWorldCamera_c));
	dWorldCamera_c *c = new(buffer) dWorldCamera_c;


	instance = c;
	return c;
}


dWorldCamera_c::dWorldCamera_c() {
	camera3d.camPos = (Vec){0.0, 10.0, 0.0};
	camera3d.target = (Vec){0.0, 0.0, 0.0};
	camera3d.camUp = (Vec){0.0, 1.0, 0.0};

	camera2d.camPos = (Vec){0.0, 10.0, 0.0};
	camera2d.target = (Vec){0.0, 0.0, 0.0};
	camera2d.camUp = (Vec){0.0, 1.0, 0.0};

	camPos = (Vec){0.0, 0.0, 1.0};
	camUp = (Vec){0.0, 1.0, 0.0};

	projection2d.near = -20000.0f;
	projection2d.far = 20000.0f;

	currentX = 416;
	currentY = -224;
	zoomLevel = STD_ZOOM;

	followPlayer = true;
}



int dWorldCamera_c::onCreate() {
	// Bad Code
	screen.projType = 0;
	screen.some_flag_bit |= 1;

	SSM *ssm = &ScreenSizesAndMultipliers[currentScreenSizeID];
	screen.height = ssm->height;
	screen.width = ssm->width;
	screen.near = 1.0;
	screen.far = 20000.0;

	if (screen.isCentered != 1) {
		screen.some_flag_bit |= 1;
		screen.isCentered = 1;
	}

	doStuff(10000.0);
	generateCameraMatrices();
	updateCameras();

	return true;
}


int dWorldCamera_c::onDelete() {
	return true;
}


int dWorldCamera_c::onExecute() {
	if (dScKoopatlas_c::instance->warpZoneHacks) {
		currentX = 2040.0f;
		currentY = -1460.0f;
		zoomLevel = 3.4f;

	} else if (panning) {
		// Calculate where we are
#define SMOOTHSTEP(x) ((x) * (x) * (3 - 2 * (x)))
		float stepRatio = panCurrentStep / panTotalSteps;
		stepRatio = 1.0f - SMOOTHSTEP(stepRatio);
		//OSReport("PAN: Step %f / %f ---- Ratio: %f", panCurrentStep, panTotalSteps, stepRatio);
		//OSReport("From %f, %f to %f, %f --- Zoom: %f to %f\n", panFromX, panFromY, panToX, panToY, panFromZoom, panToZoom);

		currentX = (panFromX * stepRatio) + (panToX * (1.0f - stepRatio));
		currentY = (panFromY * stepRatio) + (panToY * (1.0f - stepRatio));
		zoomLevel = (panFromZoom * stepRatio) + (panToZoom * (1.0f - stepRatio));
		//OSReport("Calculated: %f, %f with zoom %f\n", currentX, currentY, zoomLevel);

		panCurrentStep += 1.0f;

		if (panCurrentStep > panTotalSteps) {
			// YAY, we reached the end
			panning = false;
			currentX = panToX;
			currentY = panToY;
			zoomLevel = panToZoom;
		}

	} else if (followPlayer) {
		daWMPlayer_c *player = daWMPlayer_c::instance;
		currentX = player->pos.x;
		currentY = player->pos.y;
	}

	calculateScreenGeometry();
	doStuff(10000.0);
	generateCameraMatrices();
	updateCameras(); 
	return true;
}

void dWorldCamera_c::panToBounds(float left, float top, float right, float bottom) {
	// Pad it a bit
	left -= 64.0f;
	right += 64.0f;
	top -= 48.0f;
	bottom += 48.0f;

	//OSReport("Panning camera to bounds %f,%f to %f,%f\n", left, top, right, bottom);

	// Figure out the centre x/y we want
	float width = right - left;
	float height = bottom - top;

	float desiredCentreX = left + (width * 0.5f);
	float desiredCentreY = -(top + (height * 0.5f));

	//OSReport("Size: %f x %f ; Desired Centre: %f, %f\n", width, height, desiredCentreX, desiredCentreY);

	// Our default zoom is 2.8
	float minScreenWidth = GlobalScreenWidth * 1.2f;
	float minScreenHeight = GlobalScreenHeight * 1.2f;
	float maxScreenWidth = GlobalScreenWidth * 4.0f;
	float maxScreenHeight = GlobalScreenHeight * 4.0f;

	//OSReport("Screen Sizes: Minimum possible %f x %f ; Maximum possible %f x %f\n", minScreenWidth, minScreenHeight, maxScreenWidth, maxScreenHeight);

	// First off, gotta cap it to the ratio
	float screenRatio = GlobalScreenWidth / GlobalScreenHeight;
	float boundsRatio = width / height;
	float correctedWidth = width, correctedHeight = height;
	float desiredZoomLevel;
	//OSReport("Actual screen size is %f x %f --- Screen Ratio: %f, Bounds Ratio: %f\n", GlobalScreenWidth, GlobalScreenHeight, screenRatio, boundsRatio);

	float widthScale = width / GlobalScreenWidth;
	float heightScale = height / GlobalScreenHeight;

	if (heightScale > widthScale) {
		// Thing is constrained on the top/bottom
		desiredZoomLevel = heightScale;
	} else {
		// Thing is constrained on the left/right
		desiredZoomLevel = widthScale;
	}

	//OSReport("Desired zoom level is %f\n", desiredZoomLevel);

	// Cap the zoom
	if (desiredZoomLevel < 2.0f)
		desiredZoomLevel = 2.0f;
	if (desiredZoomLevel > 4.5f)
		desiredZoomLevel = 4.5f;
	//OSReport("After capping: %f\n", desiredZoomLevel);

	// And we're almost there YAY
	panToPosition(desiredCentreX, desiredCentreY, desiredZoomLevel);
}


void dWorldCamera_c::panToPosition(float x, float y, float zoom) {
	panFromX = currentX;
	panFromY = currentY;
	panFromZoom = zoomLevel;

	panToX = x;
	panToY = y;
	panToZoom = zoom;

	float xDiff = abs(panToX - panFromX);
	float yDiff = abs(panToY - panFromY);

	float panLength = sqrtf((xDiff*xDiff) + (yDiff*yDiff));
	float panSteps = panLength / 2.3f;
	float scaleSteps = abs(panToZoom - panFromZoom) / 0.1f;
	float stepCount = max(panSteps, scaleSteps);

	//OSReport("Pan length: %f over %f steps\n", panLength, panSteps);
	//OSReport("Scale steps: %f\n", scaleSteps);
	//OSReport("Step Count: %f\n", stepCount);

	panCurrentStep = 0.0f;
	panTotalSteps = stepCount;

	panning = true;
	followPlayer = false;
}


int dWorldCamera_c::onDraw() {
	GXRModeObj *rmode = nw4r::g3d::G3DState::GetRenderModeObj();

	nw4r::g3d::Camera cam(GetCameraByID(0));
	nw4r::g3d::Camera cam2d(GetCameraByID(1));

	if (rmode->field_rendering != 0) {
		cam.SetViewportJitter(VIGetNextField());
		cam2d.SetViewportJitter(VIGetNextField());
	}
	//cam2d.SetOrtho(rmode->efbHeight, 0.0f, 0.0f, rmode->fbWidth * (IsWideScreen() ? 1.3333334f : 1.0f), -100000.0f, 100000.0f);

	return true;
}


void dWorldCamera_c::calculateScreenGeometry() {
	zoomDivisor = 1.0 / zoomLevel;

	screenWidth = GlobalScreenWidth / zoomDivisor;
	screenHeight = GlobalScreenHeight / zoomDivisor;

	screenLeft = currentX - (screenWidth * 0.5);
	screenTop = currentY + (screenHeight * 0.5);
}


void dWorldCamera_c::doStuff(float something) {
	camTarget = (Vec){
		screenLeft + (screenWidth * 0.5f),
	   (screenTop - screenHeight) + (screenHeight * 0.5f),
	   0.0f
	};

	camPos = (Vec){camTarget.x, camTarget.y, something};
}


void dWorldCamera_c::generateCameraMatrices() {
	float orthoTop = screenHeight * 0.5;
	float orthoLeft = -screenWidth * 0.5;
	float orthoBottom = -orthoTop;
	float orthoRight = -orthoLeft;

	camera3d.camPos = camPos;
	camera3d.target = camTarget;
	camera3d.camUp = camUp;

	camera3d.calculateMatrix();

	screen.setOrtho(orthoTop, orthoBottom, orthoLeft, orthoRight, -100000.0f, 100000.0f);

	//VEC2 crap = GetSomeSizeRelatedBULLSHIT();
	// This is a horrible, horrible hack
	VEC2 crap;
	*((u64*)(&crap)) = GetSomeSizeRelatedBULLSHIT();

	Vec calcedAboutRatio = CalculateSomethingAboutRatio(orthoTop, orthoBottom, orthoLeft, orthoRight);
	screen._44 = CalculateSomethingElseAboutRatio();

	screen.width = crap.x;
	screen.some_flag_bit |= 1;
	screen.height = crap.y;
	screen.horizontalMultiplier = calcedAboutRatio.x;
	screen.verticalMultiplier = calcedAboutRatio.y;
	screen.unk3 = calcedAboutRatio.z;

	camera2d.camPos = (Vec){0.0, 0.0, 0.0};
	camera2d.target = (Vec){0.0, 0.0, -100.0};
	camera2d.camUp = camUp;

	camera2d.calculateMatrix();

	GXRModeObj *rmode = nw4r::g3d::G3DState::GetRenderModeObj();
	projection2d.setVolume(rmode->efbHeight, 0.0, 0.0, rmode->fbWidth * (IsWideScreen() ? 1.3333334f : 1.0f));
}


void dWorldCamera_c::updateCameras() {
	nw4r::g3d::Camera cam0(GetCameraByID(0));
	camera3d.assignToNW4RCamera(cam0);
	screen.loadIntoCamera(cam0);

	nw4r::g3d::Camera cam1(GetCameraByID(1));
	projection2d.setOrthoOntoCamera(cam1);
	camera2d.assignToNW4RCamera(cam1);
}

