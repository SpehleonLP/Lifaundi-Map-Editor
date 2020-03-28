#include "fmodmanager.h"
#include "fmoderror.h"
#include <memory>
#include <cstring>
#include <iostream>

#ifndef _WIN32
#include <fmod/fmod.hpp>
#include <fmod/fmod_studio.hpp>
#else
#include <fmod.hpp>
#include <fmod_studio.hpp>
#endif

#define UNUSED(x) (void)x;

FMODManager * FMODManager::g_manager = nullptr;

static FMOD_CREATESOUNDEXINFO g_flacInfo;

static FMOD::System         * g_FMODSystem = 0L;
static FMOD::Studio::System * g_FMODStudio = 0L;

static bool g_FMODpaused = false;

FMOD::System * getFMODSystem()
{
	return g_FMODSystem;
}

FMOD::Studio::System * getFMODStudio()
{
	return g_FMODStudio;
}

FMODManager::FMODManager()
{
	if(g_FMODStudio)
	{
		std::cerr << "FMOD Error" << "tried to instantiate FMOD, which already exists";
		exit(-1);
	}

	g_manager = this;

	memset(&g_flacInfo, 0, sizeof(g_flacInfo));
	g_flacInfo.cbsize = sizeof(g_flacInfo);
	g_flacInfo.suggestedsoundtype = FMOD_SOUND_TYPE_FLAC;

	unsigned int      version;
	int result;

	result = FMOD::Studio::System::create(&g_FMODStudio);
	FMODError(result);
	result = g_FMODStudio->getCoreSystem(&g_FMODSystem);
	FMODError(result);
	result = g_FMODSystem->getVersion(&version);
	FMODError(result);

	if(version != FMOD_VERSION)
	{
		std::cerr << "FMOD Error:" << "FMOD lib version " << version << " doesn't match header version " << FMOD_VERSION;
		exit(-1);
	}

	result = g_FMODStudio->initialize(32, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL, 0L);
	FMODError(result);

	pause(true);

	FMOD_GUID null;
	memset(&null, 0, sizeof(FMOD_GUID));

	track_titles << QString("<nothing>");
	track_guids.push_back(null);
}

FMODManager::~FMODManager()
{
	g_manager = nullptr;

	track->stop(FMOD_STUDIO_STOP_IMMEDIATE);
	track->release();

	g_FMODStudio->release();
	g_FMODStudio = 0L;
	g_FMODSystem = 0L;
}

void FMODManager::clear()
{
	track->stop(FMOD_STUDIO_STOP_IMMEDIATE);
	track->release();
	track = nullptr;
	memset(&guid, 0, sizeof(FMOD_GUID));

	auto r = g_FMODStudio->unloadAll();
	FMODError(r);

	track_titles.clear();
	track_guids.clear();
}

void FMODManager::pause(bool pause)
{
	if(pause == g_FMODpaused)
		return;
	g_FMODpaused = pause;

	FMOD::ChannelGroup * group;
	g_FMODSystem->getMasterChannelGroup(&group);
	group->setPaused(pause);
}

void        FMODManager::LoadBank(const char * path)
{
	FMOD::Studio::Bank * bank = nullptr;
	auto r = g_FMODStudio->loadBankFile(path, FMOD_STUDIO_LOAD_BANK_NORMAL, &bank);
	FMODError(r);
	Get()->InsertTracks(bank);
}

static FMOD_RESULT g_FMOD_STUDIO_EVENT_CALLBACK(FMOD_STUDIO_EVENT_CALLBACK_TYPE type, FMOD_STUDIO_EVENTINSTANCE *event, void *parameters)
{
    UNUSED(parameters);

    if(type == FMOD_STUDIO_EVENT_CALLBACK_STOPPED)
        ((FMOD::Studio::EventInstance*)event)->release();

    return FMOD_OK;
}

bool FMODManager::PlaySong(FMOD_GUID const& song)
{
//already playing
	if(memcmp(&song, &guid, sizeof(FMOD_GUID)) == 0)
		return true;

	memcpy(&guid, &song, sizeof(FMOD_GUID));

//stop current track
	if(track)
	{
		track->stop(FMOD_STUDIO_STOP_ALLOWFADEOUT);
		track = nullptr;
	}

	FMOD::Studio::EventDescription * description;
	auto r = g_FMODStudio->getEventByID(&song, &description);

	if(r == FMOD_ERR_EVENT_NOTFOUND)
		return false;
	FMODError(r);

	r = description->createInstance(&track);
	FMODError(r);

    track->setCallback(&g_FMOD_STUDIO_EVENT_CALLBACK, FMOD_STUDIO_EVENT_CALLBACK_STOPPED);

	if(FMOD_OK != (r = g_manager->track->start()))
	{
		track->release();
		track = nullptr;

		FMODError(r);
		return false;
	}

	return true;
}

void FMODManager::ReloadTrackList()
{
	FMOD_GUID null;
	memset(&null, 0, sizeof(FMOD_GUID));

	track_titles.clear();
	track_guids.clear();

	track_titles << QString("<nothing>");
	track_guids.push_back(null);

	int bank_count = 0;
	auto r = g_FMODStudio->getBankCount(&bank_count); FMODError(r);

	if(bank_count == 0)
		return;

	std::unique_ptr<FMOD::Studio::Bank*[]> bank_list(new FMOD::Studio::Bank*[bank_count]);
	r = g_FMODStudio->getBankList(&bank_list[0], bank_count, &bank_count); FMODError(r);

	for(int i = 0; i < bank_count; ++i)
		InsertTracks(bank_list[i]);
}

void FMODManager::InsertTracks(FMOD::Studio::Bank* bank)
{
	if(bank == nullptr)
		return;

	int event_count{};
	auto r = bank->getEventCount(&event_count); FMODError(r);

	if(event_count == 0)
		return;

	std::unique_ptr<FMOD::Studio::EventDescription*[]> event_list(new FMOD::Studio::EventDescription*[event_count]);
	r = bank->getEventList(&event_list[0], event_count, &event_count); FMODError(r);

	track_guids.reserve(track_guids.size() + event_count);

	for(int j = 0; j < event_count; ++j)
	{
		FMOD_GUID eax;
		char buffer[4096];
		int length{};

		r = event_list[j]->getID(&eax); FMODError(r);
		r = event_list[j]->getPath(buffer, sizeof(buffer)-1, &length);
		buffer[length] = 0;

		track_guids.push_back(eax);
		track_titles << QString(buffer+7);
	}
}


