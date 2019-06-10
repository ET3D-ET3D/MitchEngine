#pragma once
#include <windows.h>
#include <string>
#include <algorithm>
#include "Dementia.h"
#include <fstream>
#include <filesystem>
#include "Logger.h"

class Path
{
public:
	Path() = default;

	explicit Path(const std::string& InFile)
	{
		char buf[1024];
		GetModuleFileNameA(NULL, buf, 1024);
		std::string ProgramPath(buf);

		std::replace(ProgramPath.begin(), ProgramPath.end(), '\\', '/');
		size_t pos = ProgramPath.find_last_of("/");
		ProgramPath = ProgramPath.substr(0, pos + 1);

		std::string assetPrefix;
#if ME_EDITOR
		assetPrefix = "../../";
#endif
		LocalPath = InFile;

		std::replace(LocalPath.begin(), LocalPath.end(), '\\', '/');

		size_t path = LocalPath.find(':');
		if (path != std::string::npos)
		{
			FullPath = LocalPath;
		}
		else
		{
			FullPath = ProgramPath + assetPrefix + LocalPath;
		}

#if ME_EDITOR
		if (!std::filesystem::exists(FullPath))
		{
			assetPrefix = assetPrefix.append("Engine/");
			FullPath = ProgramPath + assetPrefix + LocalPath;
		}
#endif
		if (!std::filesystem::exists(FullPath))
		{
			Logger::Log(Logger::LogType::Warning, "File doesn't exist." + LocalPath);
		}
		if (std::filesystem::is_regular_file(FullPath))
		{
			IsFile = true;
			pos = FullPath.find_last_of("/");
			Directory = FullPath.substr(0, pos + 1);
		}
		else
		{
			IsFolder = true;
			Directory = FullPath;
		}

		path = LocalPath.rfind("Assets");
		if (path != std::string::npos)
		{
			LocalPath = assetPrefix + LocalPath.substr(path, LocalPath.size());
		}
		
#if ME_PLATFORM_UWP
		std::replace(LocalPath.begin(), LocalPath.end(), '/', '\\');
#endif
	}

	~Path()
	{

	}
	bool IsFile = false;
	bool IsFolder = false;
	std::string FullPath;
	std::string LocalPath;
	std::string Directory;
};