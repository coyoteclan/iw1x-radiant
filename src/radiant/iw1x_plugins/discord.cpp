#include "hook.h"
#include "pch.h"
//#include "types.h"
#if 1
#include "discord.h"
#define DEBUG 1

namespace discord
{
	bool isReady = false;
	bool updatesStarted = false;
	bool exiting = false;
	bool idle = true;
	std::time_t timestamp_init = -1;
	HANDLE hPipe;
	int discord;
	DiscordRichPresence presence{};

	AsyncScheduler rpcScheduler;
	AsyncScheduler rpcInfoScheduler;
	AsyncScheduler checkIdle;
	AsyncScheduler checkMouse;
	bool mouseDown = false;

	utils::hook::detour hook_onKeyUP;
	//utils::hook::detour hook_onKeyDown;
	//utils::hook::detour hook_mouse;
	//utils::hook::detour hook_MarkMapModified;
	//utils::hook::detour hook_ShowCursor;
	utils::hook::detour hook_Sys_UpdateWindows;

	static void ready(const DiscordUser*)
	{
#ifdef DEBUG
		std::stringstream ss;
		ss << "####### discord ready" << std::endl;
		OutputDebugStringA(ss.str().c_str());
#endif
		isReady = true;
		updateInfo();
	}

#ifdef DEBUG
	static void errored(const int error_code, const char* message)
	{
		std::stringstream ss;
		ss << "####### discord errored, error_code: " << error_code << ", message: " << message << std::endl;
		OutputDebugStringA(ss.str().c_str());
	}
#endif
	
	static void disconnected(const int, const char*)
	{
#ifdef DEBUG
		std::stringstream ss;
		ss << "####### discord disconnected" << std::endl;
		OutputDebugStringA(ss.str().c_str());
#endif
		isReady = false;
	}
	
	void updateInfo()
	{
		if (!discord)
		{
			if (updatesStarted)
			{
				// Was enabled
				Discord_ClearPresence();
				updatesStarted = false;
			}
			return;
		}
		else
		{
			if (!isReady)
			{
				printf("not ready\n");
				return;
			}
			
			if (!updatesStarted)
			{
				// First update
				Discord_UpdatePresence(&presence);
				updatesStarted = true;
			}
		}
		
		//if (true)
		//{
			const char *map = (const char*)0x01077b00;
			std::filesystem::path mapPath(map);
			std::string mapname = mapPath.filename().string();
			//printf("Updating presence info...\n");
			//printf("%s\n", mapname.c_str());
			
			presence.largeImageKey = "codradiant";
			presence.largeImageText = "CoDRadiant";
			presence.details = mapname.c_str();
			
			if (idle)
			{
				presence.state = "Idling";
				presence.smallImageKey = "idle";
				presence.smallImageText = "Idle";
			}
			else
			{
				presence.state = "";
				presence.smallImageText = "";
				presence.smallImageKey = "";
			}
			
			Discord_UpdatePresence(&presence);
		//}
		/*else
		{
			if (presence.details != nullptr || presence.state != nullptr)
			{
				// Was in a server, reset info
				presence = {};
				presence.startTimestamp = timestamp_init;
				Discord_UpdatePresence(&presence);
			}
		}//*/
	}

	static void stub_onKeyUP(UINT a1, UINT nChar, UINT nRepCnt, UINT nFlags)
	{
		// a1 could be a window
		// codradiant's function is different from q3radiant's
		printf("keyUP: %d, %d, %d, %d\n", a1, nChar, nRepCnt, nFlags);
		if (idle)
			idle = false;
		hook_onKeyUP.invoke(a1, nChar, nRepCnt, nFlags);
	}

	void idleMonitor()
	{
		if (mouseDown)
		{
			idle = false;
			return;
		}
		idle = true;
		return;
	}

	void mouseMonitor()
	{
		if (GetAsyncKeyState(VK_LBUTTON) & 0x8000
			|| GetAsyncKeyState(VK_RBUTTON) & 0x8000
			|| GetAsyncKeyState(VK_MBUTTON) & 0x8000
		)
		{
			mouseDown = true;
			if (idle)
				idle = false;
			//printf("Mouse Down\n");
			return;
		}
		mouseDown = false;
	}

	/*static void stub_onKeyDown(int a1, UINT nChar, UINT nRepCnt, UINT nFlags)
	{
		printf("keyDown: %d, %d, %d, %d\n", a1, nChar, nRepCnt, nFlags);
		hook_onKeyDown.invoke(a1, nChar, nRepCnt, nFlags);
	}//*/

	/*static void stub_mouse(int a1, int a2, int a3)
	{
		printf("mouse: %d, %d, %d\n", a1, a2, a3);
		hook_mouse.invoke(a1, a2, a3);
	}
	static void stub_mouse(int a1)
	{
		printf("test: %d\n", a1);
		hook_mouse.invoke(a1);
	}
	static void stub_MarkMapModified()
	{
		printf("Marking map as modified\n");
		hook_MarkMapModified.invoke();
	}
	static void stub_ShowCursor(BOOL bShow)
	{
		printf("ShowCursor: %d\n", bShow);
		if (idle)
			idle = false;
		hook_ShowCursor.invoke(bShow);
	}

	static void stub_Sys_UpdateWindows(UINT wnd)
	{
		printf("Window: %d\n", wnd);
		hook_Sys_UpdateWindows.invoke(wnd);
	}//*/
	
	class component final : public component_interface
	{
	public:
		void post_start() override
		{
			discord = 1; // manually setting to enabled for testing
		}

		void post_load() override
		{
			DiscordEventHandlers handlers{};
			handlers.ready = ready;
#ifdef DEBUG
			handlers.errored = errored;
#endif
			handlers.disconnected = disconnected;
			
			Discord_Initialize("1376160912104226857", &handlers, 1, nullptr);
			this->initialized = true;
			
			timestamp_init = std::time(nullptr);
			presence.startTimestamp = timestamp_init;
			
			//scheduler::loop(Discord_RunCallbacks, scheduler::client);
			//scheduler::loop(updateInfo, scheduler::client);
			if (rpcScheduler.startRepeatingTask(Discord_RunCallbacks, std::chrono::seconds(1)))
				printf("Scheduler started successfully.\n");
			else {
				printf("Failed to start scheduler.\n");
				return;
			}
			if (rpcInfoScheduler.startRepeatingTask(updateInfo, std::chrono::seconds(2)))
				printf("Scheduler started successfully.\n");
			else {
				printf("Failed to start scheduler.\n");
				return;
			}
			
			hPipe = CreateNamedPipe(
				"\\\\.\\pipe\\iw1x-radiant",
				PIPE_ACCESS_INBOUND,
				PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
				1, // nMaxInstances
				512, 512, 0, NULL
			);
			if (hPipe == INVALID_HANDLE_VALUE)
				MSG_ERR("Failed to create pipe for Discord");

			if (checkIdle.startRepeatingTask(idleMonitor, std::chrono::seconds(60)))
				printf("Scheduler started successfully.\n");
			else {
				printf("Failed to start scheduler.\n");
				return;
			}

			// TODO: Use the builtin function like onKeyUP
			if (checkMouse.startRepeatingTask(mouseMonitor, std::chrono::milliseconds(250)))
				printf("Scheduler started successfully.\n");
			else {
				printf("Failed to start scheduler.\n");
				return;
			}

			// hooks
			/* (Not used here) Calls to CMainFrame::OnKeyUp:
			 *		0x0040a8d5 0x00444135 0x00451545 0x00451dc5
			*/
			hook_onKeyUP.create(0x00411770, stub_onKeyUP);
			//hook_onKeyDown.create(0x004118a0, stub_onKeyDown);
			// hook_onKeyDown doesn't work and not worth the effort since onKeyUP is enough for idle check
			
			//hook_mouse.create(0x00410FF0, stub_mouse); // this is don't know what address
			//hook_mouse.create(0x00417330, stub_mouse); // this is actually Sys_UpdateWindows
			
			//hook_MarkMapModified.create(0x00486dd0, stub_MarkMapModified);
			// Sys_UpdateWindows and Sys_MarkMapModified are called too often
			
			//hook_ShowCursor.create(&ShowCursor, stub_ShowCursor);
			//ShowCursor is called even more often xD
		}

		void pre_destroy() override
		{
			if (this->initialized)
				Discord_Shutdown();
			
			exiting = true;
			CancelIoEx(hPipe, NULL);
			CloseHandle(hPipe);
			rpcScheduler.stop();
			rpcInfoScheduler.stop();
			checkMouse.stop();
			//checkIdle.stop();
		}

	private:
		bool initialized = false;
	};
}

extern "C" __declspec(dllexport) component_interface* create_component()
{
	printf("Creating discord component\n");
	return new discord::component();
}
#endif
