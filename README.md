# Gold Tracker Module (mod-gold-tracker)

## 1. 모듈 소개

`mod-gold-tracker`는 아제로스코어(AzerothCore) 월드 서버에서 플레이어의 골드(화폐) 변경 내역을 상세하게 추적하고 기록하는 모듈입니다. 이 모듈은 비정상적인 골드 축적 행위를 감지하고 제재하는 데 필요한 데이터를 수집하여 서버 운영자가 보다 효율적으로 게임 경제를 관리할 수 있도록 돕습니다.

기존의 서버 로그 시스템은 방대하여 특정 목적의 데이터 분석이 어렵다는 점을 고려하여, 이 모듈은 골드 변경에 특화된 로그를 별도의 파일에 기록함으로써 운영 편의성을 극대화합니다.

## 2. 주요 기능

*   **골드 변경 추적**: 플레이어의 골드 획득, 사용, 거래 등 모든 골드 변경 이벤트를 감지합니다.
*   **상세 로그 기록**: 변경된 골드 양, 변경 후 총 골드, 플레이어 GUID, 플레이어 이름, 타임스탬프 등 핵심 정보를 CSV 형식으로 기록합니다.
*   **전용 로그 디렉토리**: `logs/gold_tracker/` 경로에 모듈 전용 로그 파일을 생성하여 다른 서버 로그와 분리하여 관리합니다.
*   **독립적인 설정**: `worldserver.conf` 파일을 직접 수정하지 않고, 모듈 자체의 설정 파일(`mod-gold-tracker.conf`)을 통해 모듈의 활성화 여부, 시작 메시지 등을 유연하게 제어할 수 있습니다.

## 3. 설치 방법

1.  **모듈 파일 배치**: 이 모듈의 모든 파일을 아제로스코어 소스 코드의 `modules/mod-gold-tracker/` 경로에 배치합니다.
    (예시: `C:/azerothcore/modules/mod-gold-tracker/`)

2.  **아제로스코어 빌드**: 아제로스코어 프로젝트를 다시 빌드합니다. 이 과정에서 `mod-gold-tracker` 모듈이 함께 컴파일되고, 필요한 설정 파일이 설치 경로로 복사됩니다.
    (빌드 과정은 사용자의 환경에 따라 다를 수 있습니다. 일반적으로 `cmake --build .` 명령을 사용합니다.)

3.  **설정 파일 복사 및 편집**: 빌드 완료 후, 아제로스코어 설치 디렉토리의 `conf/` 폴더에 `mod-gold-tracker.conf.dist` 파일이 생성됩니다. 이 파일을 `mod-gold-tracker.conf`로 복사한 후, 필요에 따라 내용을 편집합니다.

    *   `conf/mod-gold-tracker.conf.dist` → `conf/mod-gold-tracker.conf`

## 4. 설정 (Configuration)

`mod-gold-tracker.conf` 파일에서 다음 옵션들을 설정할 수 있습니다.

```ini
#----------------------------------------------------------
# Gold Tracker Module Settings
#----------------------------------------------------------
#
# Gold Tracker 모듈 활성화 여부 (0 = 비활성화, 1 = 활성화)
GoldTracker.Enable = 1

# Gold Tracker 모듈 시작 시 표시할 메시지
GoldTracker.StartupMessage = "|cff0070DD[Gold Tracker]|r Gold Tracker Module is active. Monitoring for suspicious gold activities."

# 시작 메시지 표시 여부 (0 = 표시 안 함, 1 = 표시)
GoldTracker.ShowStartupMessage = 1
```

*   `GoldTracker.Enable`: 모듈의 기능을 전체적으로 켜거나 끕니다. 기본값은 `1` (활성화)입니다.
*   `GoldTracker.StartupMessage`: 월드 서버 시작 시 콘솔에 표시될 메시지를 설정합니다. 이 메시지는 모듈이 정상적으로 작동 중임을 알려줍니다.
*   `GoldTracker.ShowStartupMessage`: `GoldTracker.StartupMessage`의 표시 여부를 설정합니다. 기본값은 `1` (표시)입니다.

## 5. 사용 방법

1.  **월드 서버 실행**: 설정이 완료된 후 월드 서버를 실행합니다.

2.  **로그 확인**: 플레이어의 골드 변경이 발생하면, `logs/gold_tracker/` 디렉토리 내에 `gold_log_YYYY-MM-DD_HHMMSS.csv` 형식의 CSV 파일이 생성되고 골드 변경 내역이 기록됩니다.

    *   **로그 파일 경로**: `[아제로스코어 설치 경로]/logs/gold_tracker/`
    *   **로그 파일명 예시**: `gold_log_2025-07-05_103000.csv`

    로그 파일은 다음과 같은 CSV 형식으로 기록됩니다:
    `Timestamp,PlayerGUID,PlayerName,AmountChanged,NewTotalGold,Reason`

    *   `Timestamp`: 골드 변경이 발생한 시간 (`YYYY-MM-DD HH:MM:SS` 형식)
    *   `PlayerGUID`: 플레이어의 고유 ID
    *   `PlayerName`: 플레이어의 이름
    *   `AmountChanged`: 변경된 골드 양 (양수: 획득, 음수: 소모)
    *   `NewTotalGold`: 변경 후 플레이어가 가진 총 골드
    *   `Reason`: 골드 변경 이유 (현재는 `UNKNOWN`으로 기록되며, 향후 확장될 예정입니다.)

## 6. 향후 개선 사항

*   골드 변경 이유(`Reason` 필드)를 더욱 상세하게 추적하고 기록하는 기능 추가.
*   특정 임계치 이상의 비정상적인 골드 변경 감지 시 실시간 알림 기능.
*   인게임 명령어를 통한 로그 조회 및 분석 기능.

---
