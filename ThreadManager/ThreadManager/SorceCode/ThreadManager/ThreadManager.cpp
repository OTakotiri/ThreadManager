// localtime��unsafe�x���G���[�𖳌���.
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
		std::make_unique<CThreadManager>();	// �C���X�^���X�̍쐬.
	return pInstance.get();
}
// �X���b�h�쐬�֐�.
const bool CThreadManager::CreateThread(const std::string & Name, std::function<void()> Proc)
{
	auto isCreate = GetInstance()->m_mThread.try_emplace(Name, std::thread(Proc));
	// �����L�[�ŃX���b�h�����łɉғ����Ă���.
	if (isCreate.second == false) return false;
	DWORD ThreadExitCode = -1;
	GetExitCodeThread(GetInstance()->m_mThread[Name].native_handle(), &ThreadExitCode);
	// �X���b�h�����グ���s.
	if (ThreadExitCode == 0xFFFFFFFF) return false;
	GetInstance()->m_ThreadCount++;
	// �X���b�h�����ő�X���b�h���𒴂������~.
	if (GetHardWereMaxThread() < GetInstance()->m_ThreadCount) _ASSERT_EXPR(false, "ThreadCount Is Over");
	return true;
}
// �w��X���b�h�J���֐�(����͋����I�����Ȃ�).
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
// �E�B���h�E�j���O���炢�ŌĂԃX���b�h������Y��h�~�֐�.
void CThreadManager::ReleaseAllThread()
{
	if (GetInstance()->m_ThreadCount == 0) return;
	for (auto itr = GetInstance()->m_mThread.begin(); itr != GetInstance()->m_mThread.end(); ++itr)
	{
		if (GetInstance()->m_mThread[itr->first].joinable() == true) {
			bool EndJoin = false;
			// �^�C���A�E�g�v���p�X���b�h�N��.
			std::thread TimeOut = std::thread([&]() {GetInstance()->CheckInfLoopThread(itr->first, EndJoin); });
			GetInstance()->m_mThread[itr->first].join();
			EndJoin = true;
			TimeOut.join();
			GetInstance()->m_ThreadCount--;
		}
	}

	GetInstance()->PutLog();
}

// join�������K�莞�Ԃ𒴂����ꍇ�A�v���O�����𗎂Ƃ��֐�.
void CThreadManager::CheckInfLoopThread(const std::string & ItrKey, bool& End)
{
	bool ExRelease = false;
	// ���ݎ��Ԃ���w�肵���^�C���A�E�g���ԑ������l��p��.
	auto t1 = std::chrono::steady_clock::now() + std::chrono::seconds(JOIN_TIME_OUT);
	while (!End)
	{
		// ��Ōv�Z�����������A���ݎ������߂����璆.
		if (std::chrono::steady_clock::now() > t1) {
			printf("�w�莞�Ԍo�߁A�X���b�h������s");
			std::string WarningMsg = " ## " + GetNowTime() + ": �n�b�V���L�[�� :�u " + ItrKey + " �v: �̃X���b�h���I���ł��܂���ł����B ## \n";
			// �o�̓��Otxt�p�ɑޔ�.
			m_vReleaseLog.emplace_back(WarningMsg);
			OutputDebugStringA(WarningMsg.c_str());
			GetInstance()->PutLog();
			_ASSERT_EXPR(false, " Failed Thread Release ");
			break;
		}
	}
	if (ExRelease == false) {
		std::string WarningMsg = GetNowTime() + ": �n�b�V���L�[�� :�u " + ItrKey + " �v: �̃X���b�h���ʏ�I�����܂����B\n";
		// �o�̓��Otxt�p�ɑޔ�.
		m_vReleaseLog.emplace_back(WarningMsg);
	}
}

// ���ݎ���(���A���A�b)��string�^�ŕԂ��֐�.
std::string CThreadManager::GetNowTime()
{
	// ����.
	auto now_c = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	std::stringstream ss;
	ss << std::put_time(localtime(&now_c), "%T");
	std::string Time = "[" + ss.str() + "] ";
	return Time;
}
// ���O�t�@�C���o��.
void CThreadManager::PutLog()
{
	std::ofstream outFile;
	auto now = std::chrono::system_clock::now();
	auto now_c = std::chrono::system_clock::to_time_t(now);
	std::stringstream ss;
	ss << std::put_time(localtime(&now_c), "%m��%d��%H��%M��ThreadReleaseLog");
	outFile.open("Data\\LogFile\\" + ss.str() + ".txt", std::ios::trunc);
	for (auto Log : GetInstance()->m_vReleaseLog) {
		outFile << Log;
	}
	outFile.close();
}
