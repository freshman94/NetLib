#include <iostream>  
#include <string>  

#include <log/LogStream.h>
using namespace std;

int main()
{

	bool b = false;
	short si = 5;
	double d = 2.2;
	string str = "hello";

	LogStream logstream;

	logstream << b;
	cout << logstream.buffer().avail() << endl;

	logstream << si;
	cout << logstream.buffer().avail() << endl;

	logstream << d;
	cout << logstream.buffer().avail() << endl;

	logstream << str;
	cout << logstream.buffer().avail() << endl;


	cout << logstream.buffer().data() << endl;

	return 0;
}