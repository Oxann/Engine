#pragma once
#include <filesystem>

//Base class for resources, resources must be inside Resources folder.
class ResourceBase
{
public:
	ResourceBase(const std::filesystem::path& file)
		:file(file)
	{}
	virtual ~ResourceBase() = 0;
	const std::filesystem::path& GetPath() const
	{
		return file;
	}
	std::string GetFileName() const
	{
		return file.filename().string();
	}
protected:
	//For mesh initialization
	ResourceBase()
	{}
protected:
	std::filesystem::path file;
};

inline ResourceBase::~ResourceBase() 
{
}