#include "ThreadManager/ThreadManager.h"
#include <mutex>
#include <iostream>
#include <Windows.h>
std::mutex mtx_;
void Ret(int i,bool& EndFlg) {
	std::unique_lock<std::mutex> lock(mtx_);
	int End = i + 10;
	while (1)
	{
		if (i >= End || EndFlg ) break;
		std::cout << i++ << std::endl;
		std::this_thread::sleep_for(std::chrono::seconds(3));
	}
	std::cout << "EndThread" << std::endl;

}

int main()
{
	bool EndFlg = false;
	CThreadManager::CreateThread("Th1", [&]() { Ret(1, EndFlg); });

	while (1)
	{
		if (GetAsyncKeyState(VK_RETURN) & 0x8000) {
			EndFlg = true;
			CThreadManager::ReleaseThread("Th1");
		}
		if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
			break;
		}
	}

	CThreadManager::ReleaseAllThread();

	return 0;
}