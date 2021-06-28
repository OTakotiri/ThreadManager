// localtimeのunsafe警告エラーを無効化.
#pragma warning(disable:4996)

#include <thread>
#include <chrono>
#include <iomanip>
#include "ThreadManager.h"
#include <sstream>
#include <Windows.h>

namespace {
	constexpr size_t JOIN_TIME_OUT = 3;
}

CThreadManager* CThreadManager::GetInstance()
{
	static std::unique_ptr<CThreadManager> pInstance =
		std::make_unique<CThreadManager>();	// インスタンスの作成.
	return pInstance.get();
}
// スレッド作成関数.
const bool CThreadManager::CreateThread(const std::string & Name, std::function<void()> Proc)
{
	auto isCreate = GetInstance()->m_mThread.try_emplace(Name, std::thread(Proc));
	// 同じキーでスレッドがすでに稼働している.
	if (isCreate.second == false) return false;
	DWORD ThreadExitCode = -1;
	GetExitCodeThread(GetInstance()->m_mThread[Name].native_handle(), &ThreadExitCode);
	// スレッド立ち上げ失敗.
	if (ThreadExitCode == 0xFFFFFFFF) return false;
	GetInstance()->m_ThreadCount++;
	// スレッド数が最大スレッド数を超えたら停止.
	if (GetHardWereMaxThread() < GetInstance()->m_ThreadCount) _ASSERT_EXPR(false, "ThreadCount Is Over");
	return true;
}
// 指定スレッド開放関数(これは強制終了しない).
const bool CThreadManager::ReleaseThread(const std::string & Name, const bool& WaitRelease)
{
	if (GetInstance()->m_mThread[Name].joinable() == false) return true;
	GetInstance()->m_mThread[Name].join();
	if (WaitRelease == false) {
		GetInstance()->m_ThreadCount--;
		return true;
	}
	DWORD ThreadExitCode = -1;
	while (GetExitCodeThread(GetInstance()->m_mThread[Name].native_handle(), &ThreadExitCode) != 0) {}
	GetInstance()->m_ThreadCount--;
	return true;
}
// ウィンドウ破壊手前くらいで呼ぶスレッド解放し忘れ防止関数.
void CThreadManager::ReleaseAllThread()
{
	if (GetInstance()->m_ThreadCount == 0) return;
	for (auto itr = GetInstance()->m_mThread.begin(); itr != GetInstance()->m_mThread.end(); ++itr)
	{
		if (GetInstance()->m_mThread[itr->first].joinable() == true) {
			bool EndJoin = false;
			// タイムアウト計測用スレッド起動.
			std::thread TimeOut = std::thread([&]() {GetInstance()->CheckInfLoopThread(itr->first, EndJoin); });
			GetInstance()->m_mThread[itr->first].join();
			EndJoin = true;
			TimeOut.join();
			GetInstance()->m_ThreadCount--;
		}
	}

	GetInstance()->PutLog();
}

// join処理が規定時間を超えた場合、プログラムを落とす関数.
void CThreadManager::CheckInfLoopThread(const std::string & ItrKey, bool& End)
{
	bool ExRelease = false;
	// 現在時間から指定したタイムアウト時間足した値を用意.
	auto t1 = std::chrono::steady_clock::now() + std::chrono::seconds(JOIN_TIME_OUT);
	while (!End)
	{
		// 上で計算した時刻を、現在時刻が過ぎたら中.
		if (std::chrono::steady_clock::now() > t1) {
			printf("指定時間経過、スレッド解放失敗");
			std::string WarningMsg = " ## " + GetNowTime() + ": ハッシュキーが :「 " + ItrKey + " 」: のスレッドが終了できませんでした。 ## \n";
			// 出力ログtxt用に退避.
			m_vReleaseLog.emplace_back(WarningMsg);
			OutputDebugStringA(WarningMsg.c_str());
			GetInstance()->PutLog();
			_ASSERT_EXPR(false, " Failed Thread Release ");
			break;
		}
	}
	if (ExRelease == false) {
		std::string WarningMsg = GetNowTime() + ": ハッシュキーが :「 " + ItrKey + " 」: のスレッドが通常終了しました。\n";
		// 出力ログtxt用に退避.
		m_vReleaseLog.emplace_back(WarningMsg);
	}
}

// 現在時間(時、分、秒)をstring型で返す関数.
std::string CThreadManager::GetNowTime()
{
	// 時間.
	auto now_c = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	std::stringstream ss;
	ss << std::put_time(localtime(&now_c), "%T");
	std::string Time = "[" + ss.str() + "] ";
	return Time;
}
// ログファイル出力.
void CThreadManager::PutLog()
{
	std::ofstream outFile;
	auto now = std::chrono::system_clock::now();
	auto now_c = std::chrono::system_clock::to_time_t(now);
	std::stringstream ss;
	ss << std::put_time(localtime(&now_c), "%m月%d日%H時%M分ThreadReleaseLog");
	outFile.open("Data\\LogFile\\" + ss.str() + ".txt", std::ios::trunc);
	for (auto Log : GetInstance()->m_vReleaseLog) {
		outFile << Log;
	}
	outFile.close();
}
