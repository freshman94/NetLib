#include <string>  

#include <log/Logger.h>

int main()
{

	bool b = false;
	short si = 5;
	double d = 2.2;
	std::string str = "hello";

	LOG_INFO << b;
	LOG_INFO << si;
	LOG_INFO << d;
	LOG_INFO << str;


	return 0;
}