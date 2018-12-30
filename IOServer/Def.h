#ifndef _INC_DEF_
#define _INC_DEF_
//////////////////////////////////////////////////////////////////////////
#define DEF_DEFAULT_MAX_CONN		100
#define DEF_NETPROTOCOL_HEADER_LENGTH	4
#define DEF_DEFAULT_WRITEBUFFERSIZE	(1024 * 5)
#define DEF_DEFAULT_READBUFFERSIZE	(1024 * 5)
#define DEF_DEFAULT_ENGINE_WRITEBUFFERSIZE	(5 * 1024 * 1024)	// 5M
#define DEF_DEFAULT_MAX_PACKET_LENGTH 0xffff // 64k

//////////////////////////////////////////////////////////////////////////
//	function
//////////////////////////////////////////////////////////////////////////
typedef void (__stdcall *FUNC_ONACCEPT)(unsigned int);
typedef void (__stdcall *FUNC_ONDISCONNECTED)(unsigned int);
typedef void (__stdcall *FUNC_ONRECV)(unsigned int, char*, unsigned int);
typedef void (__stdcall *FUNC_ONCONNECTSUCCESS)(unsigned int, void*);
typedef void(__stdcall *FUNC_ONCONNECTFAILED)(unsigned int, void*);
typedef void (__stdcall *FUNC_ONTIMER)(unsigned int);
//////////////////////////////////////////////////////////////////////////
#endif