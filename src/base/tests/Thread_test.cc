#include <iostream>  
#include <unistd.h>

#include <base/Thread.h>
using namespace std;

void thr_fn() {
	cout << "new thread: " << pthread_self() << endl;
	while (1) { sleep(5); }
}

int main()
{
	string name = "Thread_test";
	Thread thread(&thr_fn, name);
	thread.start();
	//thread.join();
	while (1) { sleep(1); }

	return 0;
}

//���г����ں�̨���У�ʹ������ps -L�����Կ����߳�Thread_test��������