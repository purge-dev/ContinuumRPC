#include "DiscordBridge.h"

double start_time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
int mode = -1;

//#define MAXPARTYSIZE 8

/* Joining/Party is too troublesome to implement and is useless now
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
void MyActivity::generatePresence()
{
	double now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();

	/*
	if (strcmp(state.core_activity.GetParty().GetId(), "") == 0) // is this user in a party?
		createParty(); // create party for others to join with a unique pw
	else
		state.core_activity.GetParty().GetSize().SetCurrentSize(state.core_activity.GetParty().GetSize().GetCurrentSize()); // update party's size 
		*/
	state.core_activity.GetAssets().SetLargeImage("large-ss");
	state.core_activity.GetAssets().SetLargeText("Subspace Continuum");	
	state.core_activity.SetType(discord::ActivityType::Playing);

	if (state.cont.inGame()) // active game window
	{
		if (mode != 0)
		{
			start_time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
			mode = 0;
		}

		state.core_activity.SetDetails("");

		if (state.cont.ship == 8)
		{
			state.core_activity.SetState("Spectating");
			state.core_activity.GetAssets().SetSmallImage("idle");
			state.core_activity.GetAssets().SetSmallText("Spectator Mode");
		}
		else
		{
			if (state.cont.ship == 0)
			{
				state.core_activity.GetAssets().SetSmallImage("playing");
				state.core_activity.GetAssets().SetSmallText("Playing as Warbird");
			}
			else if (state.cont.ship == 1)
			{
				state.core_activity.GetAssets().SetSmallImage("jav");
				state.core_activity.GetAssets().SetSmallText("Playing as Javelin");
			}
			else if (state.cont.ship == 2)
			{
				state.core_activity.GetAssets().SetSmallImage("spid");
				state.core_activity.GetAssets().SetSmallText("Playing as Spider");
			}
			else if (state.cont.ship == 3)
			{
				state.core_activity.GetAssets().SetSmallImage("lev");
				state.core_activity.GetAssets().SetSmallText("Playing as Leviathan");
			}
			else if (state.cont.ship == 4)
			{
				state.core_activity.GetAssets().SetSmallImage("terr");
				state.core_activity.GetAssets().SetSmallText("Playing as Terrier");
			}
			else if (state.cont.ship == 5)
			{
				state.core_activity.GetAssets().SetSmallImage("weas");
				state.core_activity.GetAssets().SetSmallText("Playing as Weasel");
			}
			else if (state.cont.ship == 6)
			{
				state.core_activity.GetAssets().SetSmallImage("lanc");
				state.core_activity.GetAssets().SetSmallText("Playing as Lancaster");
			}
			else if (state.cont.ship == 7)
			{
				state.core_activity.GetAssets().SetSmallImage("sha");
				state.core_activity.GetAssets().SetSmallText("Playing as Shark");
			}

			state.core_activity.SetState("Playing");
		}

		state.cont.inMenu = false;
		state.core_activity.GetTimestamps().SetStart(now - (now - start_time));
	}
	else
	{
		char text[32];
		HWND alt = GetForegroundWindow();
		GetWindowTextA(alt, text, 32);

		if ((CMPSTART("Subspace Continuum 0.40 -", text) || (CMPSTART("Continuum 0.40 -", text)))) // chat menu
		{
			if (mode != 1)
			{
				start_time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
				mode = 1;
			}

			state.core_activity.SetDetails("Chatting");
			state.core_activity.SetState("");
			state.core_activity.GetAssets().SetSmallImage("chat");
			state.core_activity.GetAssets().SetSmallText("Chatting in Game");
			state.core_activity.GetTimestamps().SetStart(now - (now - start_time));
		}
		else if ((CMPSTART("Subspace Continuum 0.40", text) || (CMPSTART("Continuum 0.40", text)))) // launcher
		{
			if (mode != 2)
			{
				start_time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
				mode = 2;
			}

			state.core_activity.SetDetails("Selecting Zone");
			state.core_activity.GetAssets().SetSmallImage("launcher");
			state.core_activity.GetAssets().SetSmallText("Game Launcher");
			state.core_activity.GetTimestamps().SetStart(0);
		}
		else
		{
			if (mode != 1) // let's consider idle + chatting to be the same mode to avoid unnecessary timers
			{
				start_time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
				mode = 1;
			}

			state.core_activity.SetDetails("Idle");
			state.core_activity.SetState("");
			state.core_activity.GetAssets().SetSmallImage("idle2");
			state.core_activity.GetAssets().SetSmallText("Waiting for Game");
			state.core_activity.GetTimestamps().SetStart(now - (now - start_time));
		}
	}

	state.core->ActivityManager().UpdateActivity(state.core_activity, [](discord::Result result) {
	});
	state.core->RunCallbacks();

	std::thread([=]() {
		std::this_thread::sleep_for(std::chrono::seconds(6)); // updateActivity ratelimited at 4 updates/20 sec
		if (GAMEPROCESSOFFLINE == 0)
			generatePresence();
		else
		{
			state.core->~Core();
			exit(0);
		}
	}).detach();
}