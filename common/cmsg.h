#ifndef _INC_CMSG_
#define _INC_CMSG_

#ifndef _WIN32
// Linux
#define WM_USER 0x0400

typedef struct tagMSG {
	unsigned int        hwnd;
	unsigned int        message;
	unsigned int      wParam;
	unsigned int      lParam;
	unsigned int       time;      lPrivate;
} MSG, *PMSG;

#else
#include <Windows.h>
#endif

#define WM_INSERTMAPKEY					(WM_USER + 1)
#define WM_CLOSECONNECTION				(WM_INSERTMAPKEY + 1)
#define WM_REMOVEPLAYER					(WM_CLOSECONNECTION + 1)
#define WM_WORLDCHECKCRC				(WM_REMOVEPLAYER + 1)
#define WM_USERCONNECTED				(WM_WORLDCHECKCRC + 1)
#define WM_PLAYERCOUNT					(WM_USERCONNECTED + 1)
#define WM_PLAYERLOGIN					(WM_PLAYERCOUNT + 1)
#define WM_PLAYERRANKLIST				(WM_PLAYERLOGIN + 1)
#define WM_CHECKBUYOLSHOPITEM			(WM_PLAYERRANKLIST + 1)
#define WM_CONSUMEDONATE				(WM_CHECKBUYOLSHOPITEM + 1)
#define WM_LSCONNECTED					(WM_CONSUMEDONATE + 1)
#define WM_SCHEDULEACTIVE				(WM_LSCONNECTED + 1)
#define WM_SETDIFFICULTY				(WM_SCHEDULEACTIVE + 1)
#define WM_STOPNETENGINE				(WM_SETDIFFICULTY + 1)
#define WM_DISTINCTIP					(WM_STOPNETENGINE + 1)

#endif
