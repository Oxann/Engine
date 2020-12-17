#pragma once
#include "Win.h"
#include <sstream>
#include "Log.h"

#ifndef NDEBUG
#define ENGINEASSERT(condition,msg) \
	if(condition)\
	{}\
	else \
	{ \
		std::stringstream formattedMsg;\
		formattedMsg << "File: " << __FILE__ << "\n"\
					 << "Funtion: " << __FUNCTION__ << "\n"\
					 << "Line: " << std::to_string(__LINE__) << "\n"\
					 << "Message: " << msg << "\n";\
		ENGINE_LOG(ENGINE_ERROR,formattedMsg.str());\
		MessageBoxA(nullptr,formattedMsg.str().c_str(),"Engine Assert!!!",MB_OK | MB_ICONERROR);\
		while(Engine::Log::MessageCount() > 0);\
		exit(-1);\
	}
#else
#define ENGINEASSERT(condition,msg)
#endif

#ifndef NDEBUG
//Must be immediately used after calling a WIN32 function 
#define CHECK_WIN32_ERROR() \
	if(false) \
	{}\
	else \
	{ \
		DWORD errorCode = GetLastError();\
		if(errorCode != 0)\
		{\
			char* errorMsgBuffer = nullptr; \
			DWORD msgLen = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, \
			nullptr, \
			errorCode, \
			MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), \
			reinterpret_cast<char*>(&errorMsgBuffer), \
			0, \
			nullptr); \
			ENGINEASSERT(false, errorMsgBuffer); \
		}\
	}
#else
#define CHECK_WIN32_ERROR()
#endif

#ifndef NDEBUG
#define CHECK_DX_ERROR(hr)\
	if(false)\
	{}\
	else\
	{\
		if(hr != 0ul)\
		{\
			char* errorMsgBuffer = nullptr; \
			DWORD msgLen = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, \
			nullptr, \
			hr, \
			MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), \
			reinterpret_cast<char*>(&errorMsgBuffer), \
			0, \
			nullptr); \
			ENGINEASSERT(false, errorMsgBuffer); \
		}\
	}
#else
#define CHECK_DX_ERROR(hr) hr
#endif