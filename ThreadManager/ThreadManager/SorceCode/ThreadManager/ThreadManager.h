#include <functional>
#include <thread>
#include <unordered_map>
#include <string>
#include <fstream>

/**
 * @file CThreadManager.h
 * @brief �}���`�X���b�h�������őS�ĊǊ�����N���X.
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
	// �C���X�^���X�̎擾.
	static CThreadManager* GetInstance();
	// �X���b�h�쐬.
	static const bool CreateThread(const std::string& Name, std::function<void()> Proc);
	// �X���b�h�J���A��2�����͉������܂ő҂��ǂ����̐ݒ�.
	static const bool ReleaseThread(const std::string& Name, const bool& WaitRelease = true);
	// �S�ẴX���b�h���������A����~�X�h�~�ɃE�B���h�E�j���O�ŌĂԎ�����.
	// join���Ă�ł���A�w�莞�Ԃ��o�߂����ꍇ�A�X���b�h�������I�ɉ������.
	// ���̏ꍇ�AVisual Studio�̏o�̓E�B���h�E�Ɍx�����O���c��.
	static void ReleaseAllThread();
	// �g�p�\�ő�X���b�h����Ԃ�.
	// �ŏI������ɃX���b�h����g�����߁A1����������Ԃ�.
	inline static const unsigned int GetHardWereMaxThread() { return std::thread::hardware_concurrency() - 1; }
	// �ғ����̃X���b�h����Ԃ�.
	inline static const bool IsMoveThread(const std::string& Name) { return GetInstance()->m_mThread[Name].joinable(); }
private:
	// join�ł̖������[�v���m.
	// join�������K�莞�Ԃ𒴂����ꍇ�A�v���O�����𗎂Ƃ�.
	void CheckInfLoopThread(const std::string& ItrKey, bool& End);
	// ���O�p���ԏo�͐錾�ł���.
	std::string GetNowTime();
	// ���O�t�@�C���o��.
	void PutLog();
private:
	std::unordered_map<std::string, std::thread> m_mThread;
	unsigned int m_ThreadCount;	// �ғ����̃X���b�h��.
	std::vector<std::string> m_vReleaseLog;	// �X���b�h����̃��O�f���o���p.
};

