#pragma once
#include <cstdint>

constexpr int kMaxCars{64};
constexpr int kMaxNameLen{64};

enum class SessionType: std::int32_t {
  Unknown = -1,
  Practice,
  Qualifying,
  Race
};

enum class ViewMode: std::int32_t {
  Unknown = -1,
  Standings,
  Relative
};

struct DriverEntry {
  int idx{-1};
  int classId{-1};
  char driverName[kMaxNameLen]{};
  char teamName[kMaxNameLen]{};
  char carNumStr[10]{}; // the player car number as a character string so we can handle 001 and other oddities

  int lap{-1};
  int position{-1};
  int classPosition{-1};
  float gapToLeader{-1};
  float gapToLeaderF2{-1};
  float gapToNext{-1};
  float lapDistPct{-1};
  float lapDistPctLast{-1};

  double totalDistPct {-1};
  double lapTimeLast{-1};
  double lapTimeBest{-1};
  double lapStartTime{-1};
  double lapTimeCurrent{-1};
  void reset() {
    idx = -1;
    position = -1;
    classId = -1;
    classPosition = -1;
    gapToLeader = -1;
    gapToNext = -1;
    gapToLeaderF2 = -1;
    totalDistPct = -1;
    lap = -1;
    lapDistPct = -1;
    lapDistPctLast = -1;
    lapTimeLast = -1;
    lapStartTime = -1;
    lapTimeCurrent = -1;
  }
};

struct LapTimingData {
  SessionType sessionType{SessionType::Unknown};
  double sessionTime{-1};
  double lastSessionTime{-1};

  ViewMode viewMode{ViewMode::Standings};

  std::int32_t relativeToDriverIndex{-1};
  std::int32_t leaderIdx{-1};
  DriverEntry drivers[kMaxCars]{};

  LapTimingData() {
    reset(true);
  }

  void reset(bool isNewConnection) {
    if (isNewConnection) {
      sessionType = SessionType::Unknown;
      sessionTime = -1;
      viewMode = ViewMode::Standings;
      relativeToDriverIndex = -1;
      leaderIdx = -1;
      lastSessionTime = -1;
    }

    std::memset(drivers, 0, sizeof(drivers));

    for (int i = 0; i < kMaxCars; i++) {
      drivers[i].reset();
    }
  }
};