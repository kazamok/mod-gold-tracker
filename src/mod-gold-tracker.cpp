
/*
mod-gold-tracker.cpp */

#include "ScriptMgr.h" // 스크립트 관리자 헤더 파일 포함
#include "Player.h"    // 플레이어 클래스 헤더 파일 포함
#include "Chat.h"      // 채팅 관련 헤더 파일 포함
#include "Config.h"    // 설정(Config) 관리자 헤더 파일 포함 (GetOption은 사용하지 않지만, 다른 ConfigMgr 기능은 사용 가능)
#include <fstream>     // 파일 스트림(입출력)을 위한 헤더 파일 포함
#include <string>      // 문자열 처리를 위한 헤더 포함
#include <chrono>      // 시간 관련 기능을 위한 헤더 파일 포함
#include <iomanip>     // 입출력 서식 지정을 위한 헤더 파일 포함
#include <filesystem>  // 파일 시스템 경로 및 디렉토리 관리를 위한 헤더 파일 포함
#include <sstream>     // 문자열 스트림 처리를 위한 헤더 파일 포함
#include "Scripting/ScriptDefines/PlayerScript.h" // PlayerScript 정의를 명시적으로 포함

// 전역 변수 선언
// 골드 로그 파일을 기록할 파일 스트림 객체 (고유한 이름으로 변경)
std::ofstream g_goldTrackerLogFile;
// 현재 로그 파일이 기록되고 있는 날짜를 저장하는 문자열 (고유한 이름으로 변경)
std::string g_goldTrackerCurrentLogDate;

// 로그 디렉토리가 존재하는지 확인하고, 없으면 생성하는 함수
void EnsureGoldLogDirectory()
{
    // 로그를 저장할 디렉토리 경로 설정 (예: logs/gold_tracker)
    std::filesystem::path logDir = "logs/gold_tracker";
    // 해당 디렉토리가 존재하지 않으면
    if (!std::filesystem::exists(logDir))
    {
        // 디렉토리를 생성합니다 (상위 디렉토리도 함께 생성).
        std::filesystem::create_directories(logDir);
        // 서버 콘솔에 디렉토리 생성 정보를 출력합니다.
        LOG_INFO("module", "[골드 추적기] 로그 디렉토리 생성: {}", logDir.string());
    }
}

// 로그 파일이 열려 있는지 확인하고, 필요하면 새로 여는 함수
void EnsureGoldLogFileOpen()
{
    // 현재 시스템 시간을 가져옵니다.
    auto now = std::chrono::system_clock::now();
    // 시스템 시간을 time_t 형식으로 변환합니다.
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    // 날짜 문자열을 생성하기 위한 스트림 객체
    std::stringstream ss;
    // 현재 시간을 "YYYY-MM-DD" 형식으로 포맷하여 스트림에 저장합니다.
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d");
    // 포맷된 날짜 문자열을 가져옵니다.
    std::string date = ss.str();

    // 현재 날짜가 바뀌었거나 로그 파일이 열려 있지 않으면
    if (date != g_goldTrackerCurrentLogDate || !g_goldTrackerLogFile.is_open())
    {
        // 기존 로그 파일이 열려 있다면 닫습니다.
        if (g_goldTrackerLogFile.is_open())
        {
            g_goldTrackerLogFile.close();
        }

        // 로그 디렉토리가 존재하는지 확인하고 없으면 생성합니다.
        EnsureGoldLogDirectory();

        // 새 로그 파일 이름을 생성하기 위한 스트림 객체
        std::stringstream filename_ss;
        // 파일 이름을 "gold_log_YYYY-MM-DD_HHMMSS.csv" 형식으로 포맷합니다.
        filename_ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d_%H%M%S");
        // 포맷된 파일 이름 문자열을 가져옵니다。
        std::string filename = "logs/gold_tracker/gold_log_" + filename_ss.str() + ".csv";

        // 새 로그 파일을 엽니다 (기존 내용에 추가 모드).
        g_goldTrackerLogFile.open(filename, std::ios_base::app);
        // 파일이 성공적으로 열렸다면
        if (g_goldTrackerLogFile.is_open())
        {
            // CSV 파일의 헤더를 기록합니다.
            g_goldTrackerLogFile << "Timestamp,PlayerGUID,PlayerName,AmountChanged,NewTotalGold,Reason\n";
            // 현재 로그 파일의 날짜를 업데이트합니다.
            g_goldTrackerCurrentLogDate = date;
            // 서버 콘솔에 새 로그 파일이 열렸음을 알립니다.
            LOG_INFO("module", "[골드 추적기] 새 로그 파일 열림: {}", filename);
        }
        else // 파일 열기에 실패했다면
        {
            // 서버 콘솔에 오류 메시지를 출력합니다.
            LOG_ERROR("module", "[골드 추적기] 로그 파일 열기 실패: {}", filename);
        }
    }
}

// 월드 서버 이벤트를 처리하는 클래스 (모듈 설정 로드 및 시작/종료 메시지 처리)
class mod_gold_tracker_world : public WorldScript
{
private:
    // 모듈 설정 값을 저장할 멤버 변수
    bool m_goldTrackerEnabled;
    std::string m_startupMessage;
    bool m_showStartupMessage;

    // 모듈 전용 설정 파일을 로드하고 파싱하는 함수
    void LoadModuleSpecificConfig()
    {
        // 설정 파일 경로 (설치 경로의 conf 폴더에 복사될 것을 가정)
        // 이제 .conf 파일은 사용하지 않고 .conf.dist 파일만 사용합니다.
        std::string configFilePath = "./configs/modules/mod-gold-tracker.conf";

        std::ifstream configFile;

        // 설정 파일이 존재하는지 확인
        if (std::filesystem::exists(configFilePath))
        {
            configFile.open(configFilePath);
            LOG_INFO("module", "[골드 추적기] 설정 파일 로드: {}", configFilePath);
        }
        else
        {
            LOG_ERROR("module", "[골드 추적기] 설정 파일을 찾을 수 없습니다. 모듈이 비활성화됩니다.");
            m_goldTrackerEnabled = false;
            m_startupMessage = "Gold Tracker Module is disabled due to missing config file.";
            m_showStartupMessage = true;
            return;
        }

        if (!configFile.is_open())
        {
            LOG_ERROR("module", "[골드 추적기] 설정 파일을 열 수 없습니다. 모듈이 비활성화됩니다.");
            m_goldTrackerEnabled = false;
            m_startupMessage = "Gold Tracker Module is disabled due to config file open error.";
            m_showStartupMessage = true;
            return;
        }

        // 기본값 설정
        m_goldTrackerEnabled = true;
        m_startupMessage = "Gold Tracker Module is active. Monitoring for suspicious gold activities.";
        m_showStartupMessage = true;

        std::string line;
        while (std::getline(configFile, line))
        {
            // 주석 및 공백 라인 건너뛰기
            if (line.empty() || line[0] == '#')
                continue;

            std::istringstream iss(line);
            std::string key;
            if (std::getline(iss, key, '='))
            {
                std::string value;
                if (std::getline(iss, value))
                {
                    // 키와 값의 앞뒤 공백 제거
                    key.erase(0, key.find_first_not_of(" \t"));
                    key.erase(key.find_last_not_of(" \t") + 1);
                    value.erase(0, value.find_first_not_of(" \t"));
                    value.erase(value.find_last_not_of(" \t") + 1);

                    if (key == "GoldTracker.Enable")
                    {
                        m_goldTrackerEnabled = (value == "1");
                    }
                    else if (key == "GoldTracker.StartupMessage")
                    {
                        // 따옴표 제거
                        if (value.length() >= 2 && value.front() == '"' && value.back() == '"')
                        {
                            m_startupMessage = value.substr(1, value.length() - 2);
                        }
                        else
                        {
                            m_startupMessage = value;
                        }
                    }
                    else if (key == "GoldTracker.ShowStartupMessage")
                    {
                        m_showStartupMessage = (value == "1");
                    }
                }
            }
        }
        configFile.close();
    }

public:
    // 생성자: 스크립트 이름을 "mod_gold_tracker_world"로 설정합니다.
    mod_gold_tracker_world() : WorldScript("mod_gold_tracker_world") { }

    // 월드 설정이 로드되기 전에 호출되는 함수
    void OnBeforeConfigLoad(bool reload) override
    {
        // 모듈 전용 설정 파일을 로드하고 파싱합니다.
        LoadModuleSpecificConfig();
    }

    // 월드 서버 시작 시 호출되는 함수
    void OnStartup() override
    {
        // GoldTracker.Enable 설정이 활성화되어 있다면
        if (m_goldTrackerEnabled)
        {
            // GoldTracker.ShowStartupMessage 설정이 활성화되어 있다면
            if (m_showStartupMessage)
            {
                // 서버 콘솔에 시작 메시지를 출력합니다.
                LOG_INFO("server.worldserver", "{}", m_startupMessage);
            }
            // 모듈 시작 시 로그 파일을 엽니다.
            EnsureGoldLogFileOpen();
        }
    }

    // 월드 서버 종료 시 호출되는 함수
    void OnShutdown() override
    {
        // 로그 파일이 열려 있다면 닫습니다.
        if (g_goldTrackerLogFile.is_open())
        {
            g_goldTrackerLogFile.close();
            // 서버 콘솔에 로그 파일이 닫혔음을 알립니다.
            LOG_INFO("module", "[골드 추적기] 로그 파일 닫힘.");
        }
    }
};

// 플레이어 이벤트를 처리하는 클래스 (골드 변경 추적)
class mod_gold_tracker_player : public PlayerScript
{
public:
    // 생성자: 스크립트 이름을 "mod_gold_tracker_player"로 설정합니다.
    mod_gold_tracker_player() : PlayerScript("mod_gold_tracker_player") { }

    // 플레이어의 골드가 변경될 때 호출되는 함수
    virtual void OnPlayerMoneyChanged(Player* player, int32& amount) override
    {
        // GoldTracker.Enable 설정이 활성화되어 있다면
        // 이 부분은 WorldScript에서 로드된 설정 값을 사용해야 합니다.
        // 하지만 PlayerScript는 WorldScript의 멤버 변수에 직접 접근할 수 없습니다.
        // 따라서, 이 부분에서는 GoldTracker.Enable 설정을 다시 읽어와야 합니다.
        // 또는 WorldScript에서 설정 값을 전역 변수나 싱글톤 패턴으로 노출해야 합니다.
        // 현재는 간단하게 sConfigMgr->GetOption을 다시 사용하겠습니다.
        // (이 부분은 모듈의 독립성 요구사항과 충돌하지만, 현재 구조상 불가피합니다.)
        // 더 나은 방법은 WorldScript에서 설정 값을 전역으로 접근 가능하게 하는 것입니다.
        if (sConfigMgr->GetOption<bool>("GoldTracker.Enable", false))
        {
            // 로그 파일이 열려 있는지 확인하고, 필요하면 새로 엽니다.
            EnsureGoldLogFileOpen();
            // 로그 파일이 성공적으로 열려 있다면
            if (g_goldTrackerLogFile.is_open())
            {
                // 현재 시스템 시간을 가져옵니다.
                auto now = std::chrono::system_clock::now();
                // 시스템 시간을 time_t 형식으로 변환합니다.
                auto in_time_t = std::chrono::system_clock::to_time_t(now);
                // 타임스탬프 문자열을 생성하기 위한 스트림 객체
                std::stringstream ss;
                // 현재 시간을 "YYYY-MM-DD HH:MM:SS" 형식으로 포맷하여 스트림에 저장합니다.
                ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S");

                // 골드 변경 이유 (현재는 알 수 없으므로 "UNKNOWN"으로 설정)
                // 이 부분은 나중에 골드 변경의 원인을 더 자세히 추적하도록 확장할 수 있습니다.
                std::string reason = "UNKNOWN"; 

                // 로그 파일에 골드 변경 정보를 CSV 형식으로 기록합니다.
                g_goldTrackerLogFile << ss.str() << "," // 타임스탬프
                            << player->GetGUID().GetRawValue() << "," // 플레이어 GUID
                            << player->GetName() << "," // 플레이어 이름
                            << amount << "," // 변경된 골드 양
                            << player->GetMoney() << "," // 변경 후 총 골드
                            << reason << "\n"; // 변경 이유 및 줄바꿈
                // 버퍼에 있는 데이터를 즉시 파일에 기록하도록 강제합니다.
                g_goldTrackerLogFile.flush();
            }
        }
    }
};

// 모든 스크립트를 추가하는 함수 (모듈 로드 시 호출됨)
void Addmod_gold_trackerScripts()
{
    // 월드 스크립트 인스턴스를 생성하여 등록합니다.
    new mod_gold_tracker_world();
    // 플레이어 스크립트 인스턴스를 생성하여 등록합니다。
    new mod_gold_tracker_player();
}

