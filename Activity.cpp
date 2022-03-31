#include "DiscordBridge.h"

double start_time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
int mode = -1;

/* Joining/Party is too troublesome to implement and is useless now
#define MAXPARTYSIZE 8
void MyActivity::joinParty(const char* pw)
{
	state.core_activity.GetParty().GetSize().SetCurrentSize(state.core_activity.GetParty().GetSize().GetCurrentSize() + 1);

	state.core->ActivityManager().UpdateActivity(state.core_activity, [](discord::Result result) {
	});
	state.core->RunCallbacks();
}

void MyActivity::updateParty() // Parties are not networked, instead we may want to use Lobbies, but its use has no more benefit due to closed source
{
	state.core_activity.GetParty().GetSize().SetCurrentSize(state.core_activity.GetParty().GetSize().GetCurrentSize() + 1);
	state.core->ActivityManager().UpdateActivity(state.core_activity, [](discord::Result result) {
	});
	state.core->RunCallbacks();
}

void MyActivity::createParty()
{
	state.core_activity.GetParty().SetId("pubbing");
	state.core_activity.GetParty().GetSize().SetMaxSize(MAXPARTYSIZE);
	state.core_activity.GetParty().GetSize().SetCurrentSize(1);
	state.core_activity.GetSecrets().SetJoin(createHash(6).c_str());
	// must eventually end in updateActivity in calling function
}
*/

void MyActivity::setMainRPC(const char* largeImage, const char* largeImageText, discord::ActivityType type)
{
	state.core_activity.GetAssets().SetLargeImage(largeImage);
	state.core_activity.GetAssets().SetLargeText(largeImageText);
	state.core_activity.SetType(type);
}

void MyActivity::setSubRPC(const char* stateText, const char* smallImage, const char* smallImageText, const char* details, discord::Timestamp timer)
{
	state.core_activity.SetState(stateText);
	state.core_activity.GetAssets().SetSmallImage(smallImage);
	state.core_activity.GetAssets().SetSmallText(smallImageText);
	state.core_activity.SetDetails(details);
	state.core_activity.GetTimestamps().SetStart(timer);
}

// Sets the user's presence every 6 seconds according to window mode/ship status
void MyActivity::generatePresence()
{
	double now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	setMainRPC("large-ss", "Subspace Continuum", discord::ActivityType::Playing); // set the common rich presence assets
	/*
	if (strcmp(state.core_activity.GetParty().GetId(), "") == 0) // is this user in a party?
		createParty(); // create party for others to join with a unique pw
	else
		state.core_activity.GetParty().GetSize().SetCurrentSize(state.core_activity.GetParty().GetSize().GetCurrentSize()); // update party's size 
		*/
	if (state.cont.inGame()) // active game window
	{
		if (mode != 0)
		{
			start_time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
			mode = 0;
		}

		if (state.cont.ship == 8)
			setSubRPC("Spectating", "idle", "Spectator Mode", multiByteString(state.cont.getRegValue(L"Zone")).c_str(), now - (now - start_time));
		else
		{
			if (state.cont.ship == 0)
				setSubRPC("Playing", "playing", "Playing as Warbird", multiByteString(state.cont.getRegValue(L"Zone")).c_str(), now - (now - start_time));
			else if (state.cont.ship == 1)
				setSubRPC("Playing", "jav", "Playing as Javelin", multiByteString(state.cont.getRegValue(L"Zone")).c_str(), now - (now - start_time));
			else if (state.cont.ship == 2)
				setSubRPC("Playing", "spid", "Playing as Spider", multiByteString(state.cont.getRegValue(L"Zone")).c_str(), now - (now - start_time));
			else if (state.cont.ship == 3)
				setSubRPC("Playing", "lev", "Playing as Leviathan", multiByteString(state.cont.getRegValue(L"Zone")).c_str(), now - (now - start_time));
			else if (state.cont.ship == 4)
				setSubRPC("Playing", "terr", "Playing as Terrier", multiByteString(state.cont.getRegValue(L"Zone")).c_str(), now - (now - start_time));
			else if (state.cont.ship == 5)
				setSubRPC("Playing", "weas", "Playing as Weasel", multiByteString(state.cont.getRegValue(L"Zone")).c_str(), now - (now - start_time));
			else if (state.cont.ship == 6)
				setSubRPC("Playing", "lanc", "Playing as Lancaster", multiByteString(state.cont.getRegValue(L"Zone")).c_str(), now - (now - start_time));
			else if (state.cont.ship == 7)
				setSubRPC("Playing", "sha", "Playing as Shark", multiByteString(state.cont.getRegValue(L"Zone")).c_str(), now - (now - start_time));
		}
		state.cont.inMenu = false;
	}
	else
	{
		if (state.cont.gameWindow() == 1) // chat menu
		{
			if (mode != 1)
			{
				start_time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
				mode = 1;
			}
			setSubRPC("Chatting", "chat", "Chatting in Game", multiByteString(state.cont.getRegValue(L"Zone")).c_str(), now - (now - start_time));
		}
		else if (state.cont.gameWindow() == 2) // launcher
		{
			if (mode != 2)
			{
				start_time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
				mode = 2;
			}
			std::string skin = "Skin: " + multiByteString(state.cont.getRegValue(L"Skin"));
			if (skin.length() > 6) // make sure it returned a proper skin value from registry
				setSubRPC(skin.c_str(), "launcher", "Game Launcher", "Selecting Zone", 0);
			else
				setSubRPC("", "launcher", "Game Launcher", "Selecting Zone", 0);
		}
		else if (state.cont.gameWindow() == 3) // doing other things/idle
		{
			if (mode != 1) // let's consider idle + chatting to be the same mode to avoid unnecessary timers
			{
				start_time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
				mode = 1;
			}
			setSubRPC("Idle", "idle2", "Waiting for Game", multiByteString(state.cont.getRegValue(L"Zone")).c_str(), now - (now - start_time));
		}
		else if (state.cont.gameWindow() == 4) // idle, launcher open and out of focus
		{
			if (mode != 1) 
			{
				start_time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
				mode = 1;
			}
			setSubRPC("Idle", "idle2", "Idle", "Selecting Zone", now - (now - start_time));
		}
	}

	state.core->ActivityManager().UpdateActivity(state.core_activity, [](discord::Result result) {
	});
	state.core->RunCallbacks();

	std::thread([=]() {
		std::this_thread::sleep_for(std::chrono::seconds(5)); // updateActivity ratelimited at 1 update/5 sec; SDK docs say requests get queued and
		if (GAMEPROCESSOFFLINE == 0)                          // do not require developer-side intervention, but apparently it isn't so (?bug)
			generatePresence();
		else
		{
			state.core->~Core();
			exit(0);
		}}).detach();
}