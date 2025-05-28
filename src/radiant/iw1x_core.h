#ifndef IW1X_CORE_H
#define IW1X_CORE_H

#ifdef BUILDING_IW1X_DLL
#define IW1X_API __declspec(dllexport)
#else
#define IW1X_API __declspec(dllimport)
#endif

//extern "C" IW1X_API DWORD address_cgame_mp;
//extern "C" IW1X_API DWORD address_ui_mp;

IW1X_API void MSG_ERR(const char* msg);

IW1X_API bool RunningUnderWine();

#endif
