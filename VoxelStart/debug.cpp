#include "debug.h"
#ifdef DEBUG
#include <iostream>
namespace debug
{
	void log(const std::string& output)
	{
		std::cout << output << std::endl;
	}

}
#else
namespace debug
{
	void log(const std::string& output)
	{
	}

}
#endif