#include "OSUtilities.h"
#include <filesystem>

namespace OS
{
	std::vector<std::string>* GetAllFileNamesInDirectory(std::string dir)
	{
		//TODO: catch throw and dealloc
		auto strings = new std::vector<std::string>();

		for (const auto& entry : std::filesystem::directory_iterator(dir))
			strings->push_back(entry.path().string());

		return strings;
	}
}
