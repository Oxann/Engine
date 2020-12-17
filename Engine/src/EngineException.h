#pragma once

#include "Win.h"
#include <exception>
#include <string>
#include "Log.h"

//This Macro throws formatted Engine Exception
#define THROW_ENGINE_EXCEPTION(what,log_on) \
{\
if(!log_on)\
{}\
else\
	ENGINE_LOG(ENGINE_WARNING, what); \
throw EngineException(what,__FILE__,__LINE__,__FUNCTION__);\
}
//////////////////////////////////////////////////////////



class EngineException : public std::exception
{
public:
	EngineException(std::string what, std::string file, int line, std::string function) noexcept;
	EngineException() = delete;
	~EngineException() = default;
	EngineException(const EngineException&) = delete;
	EngineException(EngineException&&) = delete;
	EngineException& operator= (const EngineException&) = delete;
	EngineException& operator= (EngineException&&) = delete;
	const char* what() const noexcept override;
	void PopUp() const;
private:
	std::string whatBuffer;
};

