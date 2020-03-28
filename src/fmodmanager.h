#ifndef FMODMANAGER_H
#define FMODMANAGER_H
#include <QStringList>
#ifndef _WIN32
#include <fmod/fmod.hpp>
#else
#include <fmod.hpp>
#endif
#include <string>
#include <vector>

typedef enum
{
	g_VoiceChannel,
	g_EffectChannel,
	g_MusicChannel,
	g_TotalChannels
} ChannelGroupId;

namespace FMOD
{
class System;
namespace Studio
{
class System;
class EventInstance;
class Bank;
}
}

class QStringList;

class FMODManager
{
public:
	FMODManager();
	~FMODManager();

	static FMODManager * Get() { return g_manager; };
	static void pause(bool);

	void clear();

	static void LoadBank(const char *);
	bool        PlaySong(FMOD_GUID const& song);

	FMOD_GUID   Get_GUID(std::string const&);
	std::string GetName(FMOD_GUID const&);

	QStringList            const& GetTrackTitles() const { return track_titles; }
	std::vector<FMOD_GUID> const& GetTrackGUIDs()  const { return track_guids; }

private:
	void ReloadTrackList();
	void InsertTracks(FMOD::Studio::Bank*);

    static FMODManager * g_manager;

    FMOD::Studio::EventInstance * track{};
    FMOD_GUID                     guid{};

	QStringList             track_titles;
	std::vector<FMOD_GUID>  track_guids;
};

FMOD::System * getFMODSystem();
FMOD::Studio::System * getFMODStudio();

#endif // FMODMANAGER_H
