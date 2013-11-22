#ifndef __KAMEK_G3DHAX_H
#define __KAMEK_G3DHAX_H

#include <common.h>

// nw4r::g3d wrapper

namespace nw4r {
	namespace g3d {

		class ResName {
			public:
				const char *name;

				ResName(const char *p = NULL) {
					name = p;
				}
		};

		class ResNode {
			public:
				void* data;
				ResNode(void *p = NULL) { data = p; }

				u32 GetID() { return data ? (*((u32*)( ((u32)data) + 0xC ))) : 0; }
		};

		class ResMdl {
			public:
				void * /*ResMdlData*/ data;

				ResMdl(void *p = NULL) {
					data = p;
				}

				void * /*ResMat*/ GetResMat(const char *str) const;

				void * GetResNode(const char *str) const;
		};

		class ResAnmChr { public: void* data; ResAnmChr(void *p = NULL) { data = p; } };
		class ResAnmVis { public: void* data; ResAnmVis(void *p = NULL) { data = p; } };
		class ResAnmClr { public: void* data; ResAnmClr(void *p = NULL) { data = p; } };
		class ResAnmTexPat { public: void* data; ResAnmTexPat(void *p = NULL) { data = p; } };
		class ResAnmTexSrt { public: void* data; ResAnmTexSrt(void *p = NULL) { data = p; } };

		class ResFile {
			public:
				void * /*ResFileData*/ data;

				ResFile(void *p = NULL) {
					data = p;
				}

				void Init();
				bool CheckRevision() const;

				void *GetResMdl(const char *str) const;
				void *GetResAnmChr(const char *str) const;
				void *GetResAnmVis(const char *str) const;
				void *GetResAnmClr(const char *str) const;
				void *GetResAnmTexPat(const char *str) const;
				void *GetResAnmTexSrt(const char *str) const;

				bool Bind(ResFile file);
		};

		void *__ScnMdlSimple__Construct(
				void * /*MEMAllocator*/ heap,
				u32 *sizeOutPtr,
				void * /*ResMdl*/ resMdl,
				int bufs = 1);


		void *__ScnMdl__Construct(
				void * /*MEMAllocator*/ heap,
				u32 *sizeOutPtr,
				void * /*ResMdl*/ resMdl,
				u32 flags,
				int bufs = 1);


		void *__AnmObjChrRes__Construct(
				void * /*MEMAllocator*/ heap,
				u32 *sizeOutPtr,
				void * /*ResAnmChr*/ resAnm,
				void * /*ResMdl*/ resMdl,
				bool flag);


		void *__AnmObjVisRes__Construct(
				void * /*MEMAllocator*/ heap,
				u32 *sizeOutPtr,
				void * /*ResAnmVis*/ resAnm,
				void * /*ResMdl*/ resMdl);


		void *__AnmObjClrRes__Construct(
				void * /*MEMAllocator*/ heap,
				u32 *sizeOutPtr,
				void * /*ResAnmVis*/ resAnm,
				void * /*ResMdl*/ resMdl);


		void *__AnmObjTexPatRes__Construct(
				void * /*MEMAllocator*/ heap,
				u32 *sizeOutPtr,
				void * /*ResAnmTexPat*/ resAnm,
				void * /*ResMdl*/ resMdl,
				bool flag);


		void *__AnmObjTexSrtRes__Construct(
				void * /*MEMAllocator*/ heap,
				u32 *sizeOutPtr,
				void * /*ResAnmTexSrt*/ resAnm,
				void * /*ResMdl*/ resMdl,
				bool flag);


		void *__AnmObjMatClrRes__Construct(
				void * /*MEMAllocator*/ heap,
				u32 *sizeOutPtr,
				void * /*ResAnmMatClr*/ resAnm,
				void * /*ResMdl*/ resMdl,
				bool flag);


		void *__AnmObjShpRes__Construct(
				void * /*MEMAllocator*/ heap,
				u32 *sizeOutPtr,
				void * /*ResAnmShp*/ resAnm,
				void * /*ResMdl*/ resMdl,
				bool flag);



		bool InsertIntoScene(void *scnObj); // 80164F90

		extern void *g3dMemAllocator; // 8042A6A8

		inline void *ConstructScnMdlSimple(void *mdl, int bufs = 1) {
			return __ScnMdlSimple__Construct(g3dMemAllocator, 0, mdl, bufs);
		}

		inline void *ConstructScnMdl(void *mdl, u32 flags, int bufs = 1) {
			return __ScnMdl__Construct(g3dMemAllocator, 0, mdl, flags, bufs);
		}



	}
}


namespace EGG {
	class Allocator {
		public:
			char data1[0x10];

			Allocator(void *heap, s32 align);
			virtual ~Allocator();

			virtual void *alloc(u32 size);
			virtual void free(void *block);

			void *heap;
			s32 alignment;
	};
}

class mAllocator_c : public EGG::Allocator {
	public:
		mAllocator_c();
		~mAllocator_c();

		bool setup(/*EGG::Heap*/void *heap, s32 align);

		void *alloc(u32 size);
		void free(void *block);
};

class mHeapAllocator_c : public mAllocator_c {
	public:
		mHeapAllocator_c();
		~mHeapAllocator_c();

		bool link(s32 size, void *heap, const char *name, int align);
		u32 unlink();

		bool linkAndBecomeCurrentHeap(s32 size, void *heap, const char *name, int align);
};

namespace m3d {

	// random, just dumped it here because I dunno where else
	void DisableIndirectTexturing();
	void InitTexObjWithResTex(GXTexObj *obj, /* ResTex */ void *resTex, u32 wrapS, u32 wrapT, u32 minFilt, u32 magFilt); 


	class scnLeaf_c {
		public:
			scnLeaf_c();
			virtual ~scnLeaf_c();

			virtual int returnUnknown();
			virtual void free();
			virtual bool scheduleForDrawing();

			void setScnObjOption(ulong, ulong);
			bool getScnObjOption(ulong, ulong*);

			void setScale(float x, float y, float z);
			void setScale(Vec vec);

			void setDrawMatrix(Mtx *mtx);
			void getLocalMatrix(Mtx *mtx);
			void getViewMatrix(Mtx *mtx);

			void calcWorld(bool);
			void calcView(bool);

			void setDrawPriorities(int opa, int xlu);

			void *scnObj;
	};

	class proc_c : public scnLeaf_c {
		public:
			bool setup(void *allocator = 0, u32 *size = 0);

			int returnUnknown() { return 2; }
			virtual void drawOpa() = 0;
			virtual void drawXlu() = 0;
	};




	// TODO: bmdl_c/smdl_c
	class banm_c;
	class mdl_c {
		public:
			mdl_c();
			virtual ~mdl_c();

			virtual void _vf0C();
			virtual void _vf10();
			virtual bool scheduleForDrawing();
			virtual void bindAnim(banm_c *anim);
			virtual void _vf1C();

			bool setup(nw4r::g3d::ResMdl model, void *allocator, u32 flags, int bufs, u32 *size);
			void oneSetupType();
			void sub_80064BF0();

			void setDrawMatrix(const Mtx *matrix);
			void setScale(float x, float y, float z);
			void setScale(Vec *scale);
			void calcWorld(bool unk);

			bool getMatrixForNode(u32 nodeID, Mtx *matrix); // bmdl_c 801682C0

			void bindAnim(banm_c *animation, float unk);

			void *scnObj;
			char data[0x38];
	};




	class banm_c {
		public:
			virtual ~banm_c();

			virtual int _vf0C() = 0;
			virtual void detach();
			virtual void process();

			bool testAnmFlag(u32 flag); /* nw4r::g3d::AnmObj::AnmFlag */

			void *anmObj; // should be NW4R AnmObj class
			void *frameHeap; // should be a frame heap
			mAllocator_c allocator;

			float getCurrentFrame();
			float setCurrentFrame(float value);
			float getUpdateRate();
			float setUpdateRate(float value);
		private:
			void setupHeap(mAllocator_c *allocator, u32 *sizeOutput);
	};

	class fanm_c : public banm_c {
		public:
			fanm_c();
			~fanm_c();

			void process();

			void _80169050(float _unk, float updateRate, float _unk2, u8 _unk3);
			void setCurrentFrame(float value);
			bool isAnimationDone(); // I think
			bool querySomething(float value);

			// 28 might be Max Frame?
			float _28, _2C, currentFrame;
			u8 playState;		// animation group??
	};

	class anmChr_c : public fanm_c {
		public:
			~anmChr_c();
			int _vf0C();

			bool setup(nw4r::g3d::ResMdl modelRes, nw4r::g3d::ResAnmChr anmRes,
					mAllocator_c *allocator, u32 *sizeOutPtr);

			void bind(/*b*/mdl_c *model, nw4r::g3d::ResAnmChr anmRes, bool playsOnce);
	};

	class anmVis_c : public fanm_c {
		public:
			~anmVis_c();
			int _vf0C();

			bool setup(nw4r::g3d::ResMdl modelRes, nw4r::g3d::ResAnmVis anmRes,
					mAllocator_c *allocator, u32 *sizeOutPtr);

			void bind(/*b*/mdl_c *model, nw4r::g3d::ResAnmVis anmRes, bool playsOnce);
	};

	class anmClr_c : public banm_c {
		public:
			class child_c : public fanm_c {
				~child_c();
				int _vf0C();
				// some methods missing
			};

			~anmClr_c();
			int _vf0C();
			void detach();
			void process();

			bool setup(nw4r::g3d::ResMdl modelRes, nw4r::g3d::ResAnmClr anmRes,
					mAllocator_c *allocator, u32 *sizeOutPtr, int count); // count usually 1 or 2, sizeOutPtr usually 0

			void bind(/*b*/mdl_c *model, nw4r::g3d::ResAnmClr anmRes, int entryNumber, int _param4); // usually entryNumber is 0, param4 is 0 or 1


			float getFrameForEntry(int number);
			void setFrameForEntry(float value, int number);
			float getUpdateRateForEntry(int number);
			void setUpdateRateForEntry(float value, int number);
			bool isEntryAnimationDone(int number);
			bool queryEntrySomething(float value, int number);
			void setEntryByte34(u8 value, int number);
			float getEntryField28(int number);

		private:
			child_c *children;

	};

	class anmTexPat_c : public banm_c {
		public:
			class child_c : public fanm_c {
				~child_c();
				int _vf0C();

				// some methods missing
			};

			// some methods missing

			~anmTexPat_c();
			int _vf0C();
			void detach();
			void process();

			bool setup(nw4r::g3d::ResMdl modelRes, nw4r::g3d::ResAnmTexPat anmRes,
					mAllocator_c *allocator, u32 *sizeOutPtr, int count);

			void bindEntry(/*b*/mdl_c *model, nw4r::g3d::ResAnmTexPat *resource,
					int entryNumber, int _param4);

			float getFrameForEntry(int number);
			void setFrameForEntry(float value, int number);
			float getUpdateRateForEntry(int number);
			void setUpdateRateForEntry(float value, int number);
			bool isEntryAnimationDone(int number);
			bool queryEntrySomething(float value, int number);
			void setEntryByte34(u8 value, int number);
			float getEntryField28(int number);

		//private:
			child_c *children;

			void processOneEntry(int number);
	};

	class anmTexSrt_c : public banm_c {
		public:
			class child_c : public fanm_c {
				~child_c();
				int _vf0C();

				// some methods missing
			};

			// some methods missing

			~anmTexSrt_c();
			int _vf0C();
			void detach();
			void process();

			bool setup(nw4r::g3d::ResMdl modelRes, nw4r::g3d::ResAnmTexSrt anmRes,
					mAllocator_c *allocator, u32 *sizeOutPtr, int count);

			void bindEntry(/*b*/mdl_c *model, nw4r::g3d::ResAnmTexSrt resource,
					int entryNumber, int _param4);

			float getFrameForEntry(int number);
			void setFrameForEntry(float value, int number);
			float getUpdateRateForEntry(int number);
			void setUpdateRateForEntry(float value, int number);
			bool isEntryAnimationDone(int number);
			void setEntryByte34(u8 value, int number);
			float getEntryField28(int number);
			float getEntryField2C(int number);

		private:
			child_c *children;

			void processOneEntry(int number);
	};
};



void SetupTextures_Player(m3d::mdl_c *model, int sceneID); // 800B3E50
void SetupTextures_Map(m3d::mdl_c *model, int sceneID); // 800B3F50
void SetupTextures_Boss(m3d::mdl_c *model, int sceneID); // 800B4050
void SetupTextures_Enemy(m3d::mdl_c *model, int sceneID); // 800B4170
void SetupTextures_MapObj(m3d::mdl_c *model, int sceneID); // 800B42B0
void SetupTextures_Item(m3d::mdl_c *model, int sceneID); // 800B43D0

#endif

