#include <game.h>
#include <sfx.h>
#include "koopatlas/mapmusic.h"
#include "music.h"

extern "C" void PlaySoundWithFunctionB4(void *spc, nw4r::snd::SoundHandle *handle, int id, int unk);

static nw4r::snd::StrmSoundHandle s_handle;
static bool s_playing = false;

static nw4r::snd::SoundHandle s_starHandle;
static bool s_starPlaying = false;

static int s_song = -1;
static int s_nextSong = -1;

static int s_countdownToSwitch = -1;
static int s_countdownToFadeIn = -1;

static dDvdLoader_c s_adpcmInfoLoader;

#define FADE_OUT_LEN 30
#define FADE_IN_LEN 30
#define BUFFER_CLEAR_DELAY 60

u8 hijackMusicWithSongName(const char *songName, int themeID, bool hasFast, int channelCount, int trackCount, int *wantRealStreamID);

void dKPMusic::play(int id) {
	if (s_playing) {
		// Switch track
		OSReport("Trying to switch to song %d (Current one is %d)...\n", id, s_song);
		if ((s_song == id && s_nextSong == -1) || s_nextSong == id) {
			OSReport("This song is already playing or is scheduled. Not gonna do that.\n");
		//} else if (s_countdownToFadeIn >= 0) {
		//	OSReport("There's already a track being faded in (CountdownToFadeIn=%d)\n", s_countdownToFadeIn);
		} else if (s_countdownToSwitch >= 0) {
			OSReport("We were already going to switch tracks, but the rstm hasn't been changed yet, so the next song is being changed to this one\n");
			s_nextSong = id;
		} else {
			OSReport("Will switch; Fading out current track 2 over %d frames\n", FADE_OUT_LEN);

			if (s_handle.Exists())
				s_handle.SetTrackVolume(1<<1, FADE_OUT_LEN, 0.0f);
			s_nextSong = id;
			s_countdownToSwitch = FADE_OUT_LEN;
		}

	} else {
		// New track
		OSReport("Playing song %d from the start.\n", id);

		int realStreamID;
		char brstmName[8];
		sprintf(brstmName, "map%d", id);
		hijackMusicWithSongName(brstmName, -1, false, 4, 2, &realStreamID);

		PlaySoundWithFunctionB4(SoundRelatedClass, &s_handle, realStreamID, 1);

		s_playing = true;
		s_song = id;
	}
}

// crap.
#include "fileload.h"

extern "C" void DVDCancel(void *crap);

extern "C" void AxVoice_SetADPCM(void *axVoice, void *adpcm);
extern "C" void Voice_SetADPCMLoop(void *voice, int channel, void *adpcmLoop);

bool dKPMusic::loadInfo() {
	return s_adpcmInfoLoader.load("/NewerRes/MapADPCMInfo.bin");
}

void dKPMusic::execute() {
	if (!s_playing)
		return;

	if (s_handle.GetSound() == 0) {
		nw4r::db::Exception_Printf_("SOUND IS NOT PLAYING!\n");
		return;
	}

	if (s_countdownToSwitch >= 0) {
		s_countdownToSwitch--;
		if (s_countdownToSwitch == 0) {
			nw4r::db::Exception_Printf_("Switching brstm files to song %d.\n", s_nextSong);

			char brstmPath[48];
			sprintf(brstmPath, "/Sound/new/map%d.er", s_nextSong);

			u8 *sound = (u8*)(s_handle.GetSound());
			u8 *player = sound+0x110;
			u8 **fileStreamPointer = (u8**)(player+0x808);
			u8 *fileStream = *fileStreamPointer;
			DVDHandle *fileInfo = (DVDHandle*)(fileStream+0x28);

			if (fileInfo->unk4 == 1) {
				OSReport("Was reading chunk, will try again next frame...\n");
				s_countdownToSwitch++;
				return;
			}

			if (s_nextSong > 0)
				s_countdownToFadeIn = BUFFER_CLEAR_DELAY;

			DVDCancel(fileInfo);
			//OSReport("CANCEL successfully called!\n");
			bool result = DVDFastOpen(DVDConvertPathToEntrynum(brstmPath), fileInfo);

			nw4r::db::Exception_Printf_("StrmSound is at %p, StrmPlayer is at %p, FileStream pointer is at %p, FileStream is at %p, FileInfo is at %p\n", sound, player, fileStreamPointer, fileStream, fileInfo);
			nw4r::db::Exception_Printf_("Changed to name %s. FastOpen returned: %d\n", brstmPath, result);

			u8 *trackArray = player+0xB58;
			u8 *track = (trackArray + (0x38 * 1));
			u8 **voicePointer = (u8**)(track+4);
			u8 *voice = *voicePointer;
			nw4r::db::Exception_Printf_("Track Array: %p; Track: %p; Voice Pointer: %p; Voice: %p\n", trackArray, track, voicePointer, voice);

			for (int i = 0; i < 2; i++) {
				int sourceBlockID = (s_nextSong*2)+i;
				u8 *sourceData = ((u8*)(s_adpcmInfoLoader.buffer)) + (0x30*sourceBlockID);
				nw4r::db::Exception_Printf_("Using ADPCM data for channel %d from block %d, data at %p\n", i, sourceBlockID, sourceData);

				if (!voice)
					continue;

				Voice_SetADPCMLoop(voice, i, sourceData+0x28);

				// loop through all axVoices
				for (int j = 0; j < 4; j++) {
					int axVoiceID = (i*4) + j;
					u8 **axVoicePointer = (u8**)(voice + 0xC + (axVoiceID*4));
					u8 *axVoice = *axVoicePointer;
					nw4r::db::Exception_Printf_("Setting AxVoice ID %d, with pointer at %p, located at %p\n", axVoiceID, axVoicePointer, axVoice);

					if (axVoice)
						AxVoice_SetADPCM(axVoice, sourceData);
				}
			}

			OSReport("All done\n");

			s_song = s_nextSong;
			s_nextSong = -1;
		}

	} else if (s_countdownToFadeIn >= 0) {
		s_countdownToFadeIn--;
		if (s_countdownToFadeIn == 0) {
			OSReport("Going to fade in the second track now!\n");
			if (s_handle.Exists())
				s_handle.SetTrackVolume(1<<1, FADE_IN_LEN, 1.0f);
		}
	}
}

void dKPMusic::stop() {
	if (!s_playing)
		return;

	OSReport("Stopping song\n");

	s_playing = false;
	s_song = -1;
	s_nextSong = -1;
	s_countdownToSwitch = -1;
	s_countdownToFadeIn = -1;

	if (s_handle.Exists())
		s_handle.Stop(30);

	if (s_starHandle.Exists())
		s_starHandle.Stop(15);
}


void dKPMusic::playStarMusic() {
	if (s_starPlaying)
		return;

	PlaySoundWithFunctionB4(SoundRelatedClass, &s_starHandle, SE_BGM_CS_STAR, 1);
	s_starPlaying = true;
}

