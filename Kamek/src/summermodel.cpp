#include <common.h>
#include <game.h>
#include <g3dhax.h>


//////////////////////////////////////////////////////////
//
//	How it works:
//
//		1) Skip down to line 70 - read the comments along the way if you like
//		2) Change the stuff inside " " to be what you want.
//		3) Copy paste an entire 'case' section of code, and change the number to change the setting it uses
//		4) give it back to Tempus to compile in
//



// This is the class allocator, you don't need to touch this
class dMakeYourOwn : public dStageActor_c {
	// Let's give ourselves a few functions
	int onCreate();
	int onDelete();
	int onExecute();
	int onDraw();

	static dMakeYourOwn *build();

	// And a model and an anmChr
	mHeapAllocator_c allocator;
	m3d::mdl_c bodyModel;
	nw4r::g3d::ResFile resFile;
	m3d::anmChr_c chrAnimation;

	nw4r::g3d::ResMdl mdl;

	// Some variables to use
	int model;
	bool isAnimating;
	float size;

	void setupAnim(const char* name, float rate);
	void setupModel(const char* arcName, const char* brresName, const char* mdlName);
};

// This sets up how much space we have in memory
dMakeYourOwn *dMakeYourOwn::build() {
	void *buffer = AllocFromGameHeap1(sizeof(dMakeYourOwn));
	return new(buffer) dMakeYourOwn;
}


// Saves space when we do it like this
void dMakeYourOwn::setupAnim(const char* name, float rate) {
	if (isAnimating) {
		nw4r::g3d::ResAnmChr anmChr;

		anmChr = this->resFile.GetResAnmChr(name);
		this->chrAnimation.setup(this->mdl, anmChr, &this->allocator, 0);
		this->chrAnimation.bind(&this->bodyModel, anmChr, 1);
		this->bodyModel.bindAnim(&this->chrAnimation, 0.0);
		this->chrAnimation.setUpdateRate(rate);
	}
}

void dMakeYourOwn::setupModel(const char* arcName, const char* brresName, const char* mdlName) {
	this->resFile.data = getResource(arcName, brresName);
	this->mdl = this->resFile.GetResMdl(mdlName);

	bodyModel.setup(mdl, &allocator, 0x224, 1, 0);
}


// This gets run when the sprite spawns!
int dMakeYourOwn::onCreate() {

	// Settings for your sprite!

	this->model = this->settings & 0xFF; 						// Sets nubble 12 to choose the model you want
	this->isAnimating = this->settings & 0x100;					// Sets nybble 11 to a checkbox for whether or not the model has an anmChr to use
	this->size = (float)((this->settings >> 24) & 0xFF) / 4.0; 	// Sets nybbles 5-6 to size. Size equals value / 4.

	// Setup the models inside an allocator
	allocator.link(-1, GameHeaps[0], 0, 0x20);


	// Makes the code shorter and clearer to put these up here

	// A switch case, add extra models in here
	switch (this->model) {

		// COPY FROM BELOW HERE

		case 0:		// If nyb 12 is 0, it'll load this model

			setupModel("morton", "g3d/model0.brres", "model0"); // arc name (no .arc), brres name, model name
			SetupTextures_Item(&bodyModel, 0);
			this->pos.z = -3300.0;

			setupAnim("anim00", 1.0); // AnmChr name, animation speed

			break;	// ends the case

		// TO HERE
		// That is a 'block' of case code, which will run depending on your sprite setting

		case 1:		// If nyb 12 is 1, it'll load this model. Add more cases for each model you'd like!

			setupModel("morton", "g3d/model0.brres", "model0"); // arc name (no .arc), brres name, model name
			SetupTextures_Item(&bodyModel, 0);
			this->pos.z = 3300.0;

			setupAnim("anim01", 1.0); // AnmChr name, animation speed
			break;	// ends the case
			
		case 2:		// If nyb 12 is 1, it'll load this model. Add more cases for each model you'd like!

			setupModel("morton", "g3d/model0.brres", "model0"); // arc name (no .arc), brres name, model name
			SetupTextures_Item(&bodyModel, 0);
			this->pos.z = 3300.0;

			setupAnim("anim02", 1.0); // AnmChr name, animation speed
			break;	// ends the case
			
		case 3:		// If nyb 12 is 1, it'll load this model. Add more cases for each model you'd like!

			setupModel("morton", "g3d/model0.brres", "model0"); // arc name (no .arc), brres name, model name
			SetupTextures_Item(&bodyModel, 0);
			this->pos.z = 3300.0;

			setupAnim("anim03", 1.0); // AnmChr name, animation speed
			break;	// ends the case
			
		case 4:		// If nyb 12 is 1, it'll load this model. Add more cases for each model you'd like!

			setupModel("morton", "g3d/model0.brres", "model0"); // arc name (no .arc), brres name, model name
			SetupTextures_Item(&bodyModel, 0);
			this->pos.z = 3300.0;

			setupAnim("anim04", 1.0); // AnmChr name, animation speed
			break;	// ends the case
			
		case 5:		// If nyb 12 is 1, it'll load this model. Add more cases for each model you'd like!

			setupModel("morton", "g3d/model0.brres", "model0"); // arc name (no .arc), brres name, model name
			SetupTextures_Item(&bodyModel, 0);
			this->pos.z = 3300.0;

			setupAnim("anim05", 1.0); // AnmChr name, animation speed
			break;	// ends the case
			
		case 6:		// If nyb 12 is 1, it'll load this model. Add more cases for each model you'd like!

			setupModel("morton", "g3d/model0.brres", "model0"); // arc name (no .arc), brres name, model name
			SetupTextures_Item(&bodyModel, 0);
			this->pos.z = 3300.0;

			setupAnim("anim06", 1.0); // AnmChr name, animation speed
			break;	// ends the case
			
		case 7:		// If nyb 12 is 1, it'll load this model. Add more cases for each model you'd like!

			setupModel("morton", "g3d/model0.brres", "model0"); // arc name (no .arc), brres name, model name
			SetupTextures_Item(&bodyModel, 0);
			this->pos.z = 3300.0;

			setupAnim("anim07", 1.0); // AnmChr name, animation speed
			break;	// ends the case
			
		case 8:		// If nyb 12 is 1, it'll load this model. Add more cases for each model you'd like!

			setupModel("morton", "g3d/model0.brres", "model0"); // arc name (no .arc), brres name, model name
			SetupTextures_Item(&bodyModel, 0);
			this->pos.z = 3300.0;

			setupAnim("anim08", 1.0); // AnmChr name, animation speed
			break;	// ends the case
			
		case 9:		// If nyb 12 is 1, it'll load this model. Add more cases for each model you'd like!

			setupModel("morton", "g3d/model0.brres", "model0"); // arc name (no .arc), brres name, model name
			SetupTextures_Item(&bodyModel, 0);
			this->pos.z = 3300.0;

			setupAnim("anim09", 1.0); // AnmChr name, animation speed
			break;	// ends the case
			
		case 10:		// If nyb 12 is 1, it'll load this model. Add more cases for each model you'd like!

			setupModel("morton", "g3d/model1.brres", "model1"); // arc name (no .arc), brres name, model name
			SetupTextures_Item(&bodyModel, 0);
			this->pos.z = 3300.0;

			setupAnim("anim00", 1.0); // AnmChr name, animation speed
			break;	// ends the case
			
		case 11:		// If nyb 12 is 1, it'll load this model. Add more cases for each model you'd like!

			setupModel("morton", "g3d/model1.brres", "model1"); // arc name (no .arc), brres name, model name
			SetupTextures_Item(&bodyModel, 0);
			this->pos.z = 3300.0;

			setupAnim("anim01", 1.0); // AnmChr name, animation speed
			break;	// ends the case	

		case 12:		// If nyb 12 is 1, it'll load this model. Add more cases for each model you'd like!

			setupModel("morton", "g3d/model1.brres", "model1"); // arc name (no .arc), brres name, model name
			SetupTextures_Item(&bodyModel, 0);
			this->pos.z = 3300.0;

			setupAnim("anim02", 1.0); // AnmChr name, animation speed
			break;	// ends the case
			
		case 13:		// If nyb 12 is 1, it'll load this model. Add more cases for each model you'd like!

			setupModel("morton", "g3d/model1.brres", "model1"); // arc name (no .arc), brres name, model name
			SetupTextures_Item(&bodyModel, 0);
			this->pos.z = 3300.0;

			setupAnim("anim03", 1.0); // AnmChr name, animation speed
			break;	// ends the case

		case 14:		// If nyb 12 is 1, it'll load this model. Add more cases for each model you'd like!

			setupModel("morton", "g3d/model1.brres", "model1"); // arc name (no .arc), brres name, model name
			SetupTextures_Item(&bodyModel, 0);
			this->pos.z = 3300.0;

			setupAnim("anim04", 1.0); // AnmChr name, animation speed
			break;	// ends the case
			
		case 15:		// If nyb 12 is 1, it'll load this model. Add more cases for each model you'd like!

			setupModel("morton", "g3d/model1.brres", "model1"); // arc name (no .arc), brres name, model name
			SetupTextures_Item(&bodyModel, 0);
			this->pos.z = 3300.0;

			setupAnim("anim05", 1.0); // AnmChr name, animation speed
			break;	// ends the case
			
		case 16:		// If nyb 12 is 1, it'll load this model. Add more cases for each model you'd like!

			setupModel("morton", "g3d/model3.brres", "model3"); // arc name (no .arc), brres name, model name
			SetupTextures_Item(&bodyModel, 0);
			this->pos.z = 3300.0;

			setupAnim("anim01", 1.0); // AnmChr name, animation speed
			break;	// ends the case	

		case 17:		// If nyb 12 is 1, it'll load this model. Add more cases for each model you'd like!

			setupModel("morton", "g3d/model3.brres", "model3"); // arc name (no .arc), brres name, model name
			SetupTextures_Item(&bodyModel, 0);
			this->pos.z = 3300.0;

			setupAnim("anim02", 1.0); // AnmChr name, animation speed
			break;	// ends the case	

		case 18:		// If nyb 12 is 1, it'll load this model. Add more cases for each model you'd like!

			setupModel("morton", "g3d/model3.brres", "model3"); // arc name (no .arc), brres name, model name
			SetupTextures_Item(&bodyModel, 0);
			this->pos.z = 3300.0;

			setupAnim("anim03", 1.0); // AnmChr name, animation speed
			break;	// ends the case

		case 19:		// If nyb 12 is 1, it'll load this model. Add more cases for each model you'd like!

			setupModel("morton", "g3d/model3.brres", "model3"); // arc name (no .arc), brres name, model name
			SetupTextures_Item(&bodyModel, 0);
			this->pos.z = 3300.0;

			setupAnim("anim04", 1.0); // AnmChr name, animation speed
			break;	// ends the case

		case 20:		// If nyb 12 is 1, it'll load this model. Add more cases for each model you'd like!

			setupModel("morton", "g3d/model3.brres", "model3"); // arc name (no .arc), brres name, model name
			SetupTextures_Item(&bodyModel, 0);
			this->pos.z = 3300.0;

			setupAnim("anim05", 1.0); // AnmChr name, animation speed
			break;	// ends the case			
			
	}

	allocator.unlink();

	if (size == 0.0) {	// If the person has the size nybble at zero, make it normal sized
		this->scale = (Vec){1.0,1.0,1.0};	
	}
	else {				// Else, use our size
		this->scale = (Vec){size,size,size};	
	}
		
	this->onExecute();
	return true;
}


// YOU'RE DONE, no need to do anything below here.


int dMakeYourOwn::onDelete() {
	return true;
}

int dMakeYourOwn::onExecute() {
	if (isAnimating) {
		bodyModel._vf1C();	// Advances the animation one update

		if(this->chrAnimation.isAnimationDone()) {
			this->chrAnimation.setCurrentFrame(0.0);	// Resets the animation when it's done
		}
	}

	return true;
}

int dMakeYourOwn::onDraw() {
	matrix.translation(pos.x, pos.y, pos.z - 6500.0);	// Set where to draw the model : -5500.0 is the official behind layer 2, while 5500.0 is in front of layer 0.
	matrix.applyRotationYXZ(&rot.x, &rot.y, &rot.z);	// Set how to rotate the drawn model 

	bodyModel.setDrawMatrix(matrix);	// Apply matrix
	bodyModel.setScale(&scale);			// Apply scale
	bodyModel.calcWorld(false);			// Do some shit

	bodyModel.scheduleForDrawing();		// Add it to the draw list for the game
	return true;
}
