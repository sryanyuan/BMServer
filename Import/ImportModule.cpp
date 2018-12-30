// Import all lib

// SServerEngine
#if _MSC_VER == 1800
#else
#ifdef _DEBUG
#pragma comment(lib, "SServerEngine_d.lib")
#else
#pragma comment(lib, "SServerEngine.lib")
#endif
#endif

// glog
#ifdef _DEBUG
#pragma comment(lib, "libglog_static_d.lib")
#else
#pragma comment(lib, "libglog_static.lib")
#endif

// sqlite3
#pragma comment(lib, "sqlite3.lib")

// shlwapi
#pragma comment(lib, "shlwapi.lib")

// dbghelp
#pragma comment(lib, "DbgHelp.lib")

// zlib
#ifdef _DEBUG
#pragma comment(lib, "zlibwapi_d.lib")
#else
#pragma comment(lib, "zlibwapi.lib")
#endif

// protobuf
#ifdef _DEBUG
#pragma comment(lib, "libprotobuf_D.lib")
#else
#pragma comment(lib, "libprotobuf.lib")
#endif

// ziparchive
#ifdef _DEBUG
#pragma comment(lib, "ZipArchive_D.lib")
#else
#pragma comment(lib, "ZipArchive.lib")
#endif

// libcurl
#ifdef _DEBUG
#pragma comment(lib, "libcurld.lib")
#else
#pragma comment(lib, "libcurl.lib")
#endif

// server site using luajit + tolua++
// luajit
#ifdef _DEBUG
#pragma comment(lib, "lua51d.lib")
#else
#pragma comment(lib, "lua51.lib")
#endif

// libtolua (only tolua++ , in luajit package)
#ifdef _DEBUG
#pragma comment(lib, "libtolua_d.lib")
#else
#pragma comment(lib, "libtolua.lib")
#endif