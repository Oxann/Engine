#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "Log.h"
#include "EngineException.h"
#include <sstream>
#include <ctime>
#include <iostream>

namespace Engine
{
	const std::filesystem::path Log::logDir("EngineLogs");
	const std::string Log::fileName = "log.txt";
	std::ofstream Log::out;
	std::queue<Log::Message> Log::Messages;
	Log Log::log;
	bool Log::Log_On = true;

	Log::Log()
	{
		#ifdef LOG_ACTIVE
		try
		{
			//Log dir creation
			if (std::filesystem::exists(logDir))
			{
				std::filesystem::remove_all(logDir);
			}
			if (!std::filesystem::create_directory(logDir))
			{
				THROW_ENGINE_EXCEPTION("Log directory could not be created.",false);
			}

			//Log file
			out.open(logDir.string() + "/" + fileName, std::ios_base::out | std::ios_base::trunc);
			if (!out.is_open())
			{
				THROW_ENGINE_EXCEPTION(fileName + " could not be opened.",false);
			}

			#ifndef NDEBUG
			//Console
			AllocConsole();
			EnableMenuItem(GetSystemMenu(GetConsoleWindow(), FALSE), SC_CLOSE, MF_DISABLED);
			SetConsoleTitle(L"LOG");
			freopen("CONIN$", "r", stdin);
			freopen("CONOUT$", "w", stdout);
			freopen("CONOUT$", "w", stderr);
			#endif

			logThread = std::make_unique<std::thread>(_Write);
			if(logThread->joinable())
				logThread->detach();
		}
		catch (const EngineException& e)
		{
			e.PopUp();
		}
		catch (const std::exception& e)
		{
			MessageBoxA(nullptr, e.what(), "Exception Caught!", MB_OK | MB_ICONSTOP);
		}
		#endif
	}

	void Log::_Write()
	{
		while (Log_On)
		{
			while(Messages.size() == 0)
				std::this_thread::yield();

			std::stringstream formattedMsg;
			std::time_t time = std::time(0);
			static std::tm now;
			localtime_s(&now,&time);
			formattedMsg << now.tm_hour << ":" << now.tm_min << ":" << now.tm_sec << " --- ";
			
			const Message& msg = Messages.front();
			switch (msg.type)
			{
			case Engine::Log::Message::MessageType::Info:
				formattedMsg << "ENGINE INFO: ";
				break;
			case Engine::Log::Message::MessageType::Warning:
				formattedMsg << "ENGINE WARNING: ";
				break;
			case Engine::Log::Message::MessageType::Error:
				formattedMsg <<  "ENGINE ERROR: ";
				break;
			default:
				break;
			}
			formattedMsg << msg.msg << "\n";

			//Console output
			#ifndef NDEBUG
			std::cout << formattedMsg.str();
			#endif
			//File output
			out << formattedMsg.rdbuf();

			log.logLock.lock();
			Messages.pop();
			log.logLock.unlock();		
		}
	}

	Log::~Log()
	{
		out.close();
	}

	void Log::Write(Message::MessageType type, std::string msg)
	{
		log.logLock.lock();
		Messages.push({ type,std::move(msg) });
		log.logLock.unlock();
	}

	void Log::Stop()
	{
		Log_On = false;
		log.logThread.reset(nullptr);
	}

	void Log::Resume()
	{
		Log_On = true;
		log.logThread = std::make_unique<std::thread>(_Write);
	}

	size_t Log::MessageCount()
	{
		return Messages.size();
	}
}
