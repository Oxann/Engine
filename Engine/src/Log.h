#pragma once
#include <fstream>
#include <string>
#include <queue>
#include <filesystem>
#include <thread>
#include <mutex>


#ifdef LOG_ACTIVE
#define ENGINE_LOG(type,msg) Log::Write(type,msg);
#else
#define ENGINE_LOG(type,msg)
#endif


#define ENGINE_INFO Log::Message::MessageType::Info
#define ENGINE_WARNING Log::Message::MessageType::Warning
#define ENGINE_ERROR Log::Message::MessageType::Error


class Log
{
public:
	struct Message
	{
		enum class MessageType : unsigned char
		{
			Info,
			Warning,
			Error
		};
		MessageType type;
		std::string msg;
	};
public:
	~Log();
	Log(const Log&) = delete;
	Log(Log&&) = delete;
	Log& operator= (const Log&) = delete;
	Log& operator= (Log&&) = delete;
	static void Write(Message::MessageType type, std::string msg);
	static void Stop();
	static void Resume();
	static size_t MessageCount();
private:
	Log();
	static void _Write();
private:
	static std::ofstream out;
	static const std::string fileName;
	static const std::filesystem::path logDir;
	static Log log;
	static std::queue<Message> Messages;
	std::unique_ptr<std::thread> logThread;
	std::mutex logLock;
	static bool Log_On;
};