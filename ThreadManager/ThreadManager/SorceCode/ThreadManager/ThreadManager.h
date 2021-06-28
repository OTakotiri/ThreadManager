#include <functional>
#include <thread>
#include <unordered_map>
#include <string>
#include <fstream>

/**
 * @file CThreadManager.h
 * @brief マルチスレッドをここで全て管轄するクラス.
 * @author OTakotiri.
 */

class CThreadManager
{
public:
	CThreadManager()
		: m_ThreadCount(0)
	{};
	~CThreadManager()
	{};
	// インスタンスの取得.
	static CThreadManager* GetInstance();
	// スレッド作成.
	static const bool CreateThread(const std::string& Name, std::function<void()> Proc);
	// スレッド開放、第2引数は解放するまで待つかどうかの設定.
	static const bool ReleaseThread(const std::string& Name, const bool& WaitRelease = true);
	// 全てのスレッドを解放する、解放ミス防止にウィンドウ破壊手前で呼ぶ事推奨.
	// joinを呼んでから、指定時間が経過した場合、スレッドを強制的に解放する.
	// その場合、Visual Studioの出力ウィンドウに警告ログが残る.
	static void ReleaseAllThread();
	// 使用可能最大スレッド数を返す.
	// 最終解放時にスレッドを一つ使うため、1つ引いた数を返す.
	inline static const unsigned int GetHardWereMaxThread() { return std::thread::hardware_concurrency() - 1; }
	// 稼働中のスレッド数を返す.
	inline static const bool IsMoveThread(const std::string& Name) { return GetInstance()->m_mThread[Name].joinable(); }
private:
	// joinでの無限ループ検知.
	// join処理が規定時間を超えた場合、プログラムを落とす.
	void CheckInfLoopThread(const std::string& ItrKey, bool& End);
	// ログ用時間出力宣言でも可.
	std::string GetNowTime();
	// ログファイル出力.
	void PutLog();
private:
	std::unordered_map<std::string, std::thread> m_mThread;
	unsigned int m_ThreadCount;	// 稼働中のスレッド数.
	std::vector<std::string> m_vReleaseLog;	// スレッド解放のログ吐き出し用.
};

