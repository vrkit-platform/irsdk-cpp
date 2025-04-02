//------
// ReSharper disable CppDeprecatedEntity
#define MIN_WIN_VER 0x0501

#ifndef WINVER
#define WINVER MIN_WIN_VER
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT MIN_WIN_VER
#endif

#pragma warning(disable : 4996) //_CRT_SECURE_NO_WARNINGS

#include <conio.h>
#include <csignal>
#include <cstdio>
#include <iostream>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>
#include <windows.h>

#include <IRacingSDK/LiveClient.h>
#include <IRacingSDK/LiveConnection.h>
#include <IRacingSDK/Types.h>
#include <IRacingSDK/Utils/CollectionHelpers.h>
#include <IRacingSDK/VarHolder.h>

#include "LapTimingData.h"
#include "console.h"
#include <cmath>

// Live weather info, may change as session progresses
namespace {
  using namespace IRacingSDK;
  using namespace IRacingSDK::Utils;

  std::shared_ptr<spdlog::logger> L{spdlog::basic_logger_mt("LapTimingExample", "tmp/lap-timing.log")};

  // ReSharper disable CppDeclaratorNeverUsed
  VarHolder gAirDensity("AirDensity"); // (float) kg/m^3, Density of air at start/finish line
  VarHolder gAirPressure("AirPressure"); // (float) Hg, Pressure of air at start/finish line
  VarHolder gAirTemp("AirTemp"); // (float) C, Temperature of air at start/finish line
  VarHolder gFogLevel("FogLevel"); // (float) %, Fog level
  VarHolder gRelativeHumidity("RelativeHumidity"); // (float) %, Relative Humidity
  VarHolder gSkies("Skies"); // (int) Skies (0=clear/1=p cloudy/2=m cloudy/3=overcast)
  VarHolder gTrackTempCrew("TrackTempCrew"); // (float) C, Temperature of track measured by crew around track
  VarHolder gWeatherType("WeatherType"); // (int) Weather type (0=constant 1=dynamic)
  VarHolder gWindDir("WindDir"); // (float) rad, Wind direction at start/finish line
  VarHolder gWindVel("WindVel"); // (float) m/s, Wind velocity at start/finish line

  // session status
  VarHolder gPitsOpen("PitsOpen"); // (bool) True if pit stop is allowed, basically true if caution lights not out
  VarHolder gRaceLaps("RaceLaps"); // (int) Laps completed in race
  VarHolder gSessionFlags("SessionFlags"); // (int) FlagType, bitfield
  VarHolder gSessionLapsRemain("SessionLapsRemain"); // (int) Laps left till session ends
  VarHolder gSessionLapsRemainEx("SessionLapsRemainEx"); // (int) New improved laps left till session ends
  VarHolder gSessionNum("SessionNum"); // (int) Session number
  VarHolder gSessionState("AppSessionState"); // (int) AppSessionState, Session state
  VarHolder gSessionTick("SessionTick"); // (int) Current update number
  VarHolder gSessionTime("SessionTime"); // (double), s, Seconds since session start
  VarHolder gSessionTimeOfDay("SessionTimeOfDay"); // (float) s, Time of day in seconds
  VarHolder gSessionTimeRemain("SessionTimeRemain"); // (double) s, Seconds left till session ends
  VarHolder gSessionUniqueID("SessionUniqueID"); // (int) Session ID

  // competitor information, array of up to 64 cars
  VarHolder gCarIdxEstTime("CarIdxEstTime"); // (float) s, Estimated time to reach current location on track
  VarHolder gCarIdxClassPosition("CarIdxClassPosition"); // (int) Cars class position in race by car index
  VarHolder gCarIdxF2Time("CarIdxF2Time"); // (float) s, Race time behind leader or fastest lap time otherwise
  VarHolder gCarIdxGear("CarIdxGear"); // (int) -1=reverse 0=neutral 1..n=current gear by car index
  VarHolder gCarIdxLap("CarIdxLap"); // (int) Lap count by car index
  VarHolder gCarIdxLapCompleted("CarIdxLapCompleted"); // (int) Laps completed by car index
  VarHolder gCarIdxLapDistPct("CarIdxLapDistPct"); // (float) %, Percentage distance around lap by car index
  VarHolder gCarIdxOnPitRoad("CarIdxOnPitRoad"); // (bool) On pit road between the cones by car index
  VarHolder gCarIdxPosition("CarIdxPosition"); // (int) Cars position in race by car index
  VarHolder gCarIdxRPM("CarIdxRPM"); // (float) revs/min, Engine rpm by car index
  VarHolder gCarIdxSteer("CarIdxSteer"); // (float) rad, Steering wheel angle by car index
  VarHolder gCarIdxTrackLocation("CarIdxTrackLocation"); // (int) TrackLocation, Track surface type by car index
  VarHolder gCarIdxTrackSurface("CarIdxTrackSurface"); // (int) TrackLocation, Track surface type by car index
  VarHolder gCarIdxTrackSurfaceMaterial("CarIdxTrackSurfaceMaterial");
  // (int) TrackSurface, Track surface material type by car index

  // new variables
  VarHolder gCarIdxLastLapTime("CarIdxLastLapTime"); // (float) s, Cars last lap time
  VarHolder gCarIdxBestLapTime("CarIdxBestLapTime"); // (float) s, Cars best lap time
  VarHolder gCarIdxBestLapNum("CarIdxBestLapNum"); // (int) Cars best lap number

  VarHolder gCarIdxP2P_Status("CarIdxP2P_Status"); // (bool) Push2Pass active or not
  VarHolder gCarIdxP2P_Count("CarIdxP2P_Count"); // (int) Push2Pass count of usage (or remaining in Race)

  VarHolder gPaceMode("PaceMode"); // (int) PaceMode, Are we pacing or not
  VarHolder gLatitude("Lat");
  VarHolder gLongitude("Lon");
  VarHolder gCarIdxPaceLine("CarIdxPaceLine"); // (int) What line cars are pacing in, or -1 if not pacing
  VarHolder gCarIdxPaceRow("CarIdxPaceRow"); // (int) What row cars are pacing in, or -1 if not pacing
  VarHolder gCarIdxPaceFlags("CarIdxPaceFlags"); // (int) PaceFlagType, Pacing status flags for each car
  VarHolder gLapLastLapTime("LapLastLapTime");

  // ReSharper restore CppDeclaratorNeverUsed

  LapTimingData gLapTimingData{};


  // notify everyone when we update the file
  HANDLE hDataValidEvent{nullptr};

  void ResetState(bool isNewConnection) {
    if (isNewConnection)
      memset(&gLapTimingData, 0, sizeof(gLapTimingData));

    for (int i = 0; i < kMaxCars; i++) {
      gLapTimingData.drivers[i].reset();
    }
  }

  // helper function to handle interpolation across a checkpoint
  // p1,t1 are position and time before checkpoint
  // p2,t2 are position and time after checkpoint
  // pCheck is position of checkpoint
  double interpolateTimeAcrossPoint(double t1, double t2, float p1, float p2, float pCheck) {
    // unwrap if crossing start/finish line
    //****Note, assumes p1 is a percent from 0 to 1
    // if that is not true then unwrap the numbers before calling this function
    if (p1 > p2)
      p1 -= 1;

    // calculate where line is between points
    const float pct = (pCheck - p1) / (p2 - p1);

    return t1 + (t2 - t1) * pct;
  }


  void processLapInfo() {
    // work out lap times for all cars
    const double curTime = gSessionTime.getDouble();

    // if time moves backwards were in a new session!
    if (gLapTimingData.lastSessionTime > curTime)
      ResetState(false);

    for (int i = 0; i < kMaxCars; i++) {
      auto &driver = gLapTimingData.drivers[i];
      if (driver.idx < 1)
        continue;
      auto idx = driver.idx;
      auto curDistPct = driver.lapDistPct = gCarIdxLapDistPct.getFloat(idx);
      auto curLap = driver.lap = gCarIdxLap.getInt(idx);
      driver.lapTimeCurrent = gCarIdxEstTime.getFloat(idx);
      driver.lapTimeBest = gCarIdxBestLapTime.getFloat(idx);
      driver.totalDistPct = curLap + curDistPct;
      driver.classPosition = gCarIdxClassPosition.getInt(idx);
      // driver.position = gCarIdxPosition.getInt(idx);
      // driver.gapToLeader = driver.totalDistPct
      driver.gapToLeaderF2 = gCarIdxF2Time.getFloat(idx);
      // reject if the car blinked out of the world
      if (curDistPct != -1) {
        // did we cross the lap?
        if (gLapTimingData.drivers[i].lapDistPctLast > 0.9f && curDistPct < 0.1f) {
          // calculate exact time of lap crossing
          const double curLapStartTime =
            interpolateTimeAcrossPoint(gLapTimingData.lastSessionTime, curTime, gLapTimingData.drivers[i].lapDistPctLast, curDistPct, 0);

          // calculate lap time, if already crossed start/finish
          if (gLapTimingData.drivers[i].lapStartTime != -1)
            gLapTimingData.drivers[i].lapTimeLast = static_cast<float>(curLapStartTime - gLapTimingData.drivers[i].lapStartTime);

          // and store start/finish crossing time for next lap
          gLapTimingData.drivers[i].lapStartTime = curLapStartTime;
        }

        gLapTimingData.drivers[i].lapDistPctLast = curDistPct;
      }
    }

    gLapTimingData.lastSessionTime = curTime;
  }

  void printFlags(int flags) {
    // global flags
    if (IsFlagSet(flags, FlagType::Checkered))
      printf("checkered ");
    if (IsFlagSet(flags, FlagType::White))
      printf("white ");
    if (IsFlagSet(flags, FlagType::Green))
      printf("green ");
    if (IsFlagSet(flags, FlagType::Yellow))
      printf("yellow ");
    if (IsFlagSet(flags, FlagType::Red))
      printf("red ");
    if (IsFlagSet(flags, FlagType::Blue))
      printf("blue ");
    if (IsFlagSet(flags, FlagType::Debris))
      printf("debris ");
    if (IsFlagSet(flags, FlagType::Crossed))
      printf("crossed ");
    if (IsFlagSet(flags, FlagType::YellowWaving))
      printf("yellowWaving ");
    if (IsFlagSet(flags, FlagType::OneLapToGreen))
      printf("oneLapToGreen ");
    if (IsFlagSet(flags, FlagType::GreenHeld))
      printf("greenHeld ");
    if (IsFlagSet(flags, FlagType::TenToGo))
      printf("tenToGo ");
    if (IsFlagSet(flags, FlagType::FiveToGo))
      printf("fiveToGo ");
    if (IsFlagSet(flags, FlagType::RandomWaving))
      printf("randomWaving ");
    if (IsFlagSet(flags, FlagType::Caution))
      printf("caution ");
    if (IsFlagSet(flags, FlagType::CautionWaving))
      printf("cautionWaving ");

    // drivers black flags
    if (IsFlagSet(flags, FlagType::Black))
      printf("black ");
    if (IsFlagSet(flags, FlagType::Disqualify))
      printf("disqualify ");
    if (IsFlagSet(flags, FlagType::Serviceable))
      printf("serviceable ");
    if (IsFlagSet(flags, FlagType::Furled))
      printf("furled ");
    if (IsFlagSet(flags, FlagType::Repair))
      printf("repair ");

    // start lights
    if (IsFlagSet(flags, FlagType::StartHidden))
      printf("startHidden ");
    if (IsFlagSet(flags, FlagType::StartReady))
      printf("startReady ");
    if (IsFlagSet(flags, FlagType::StartSet))
      printf("startSet ");
    if (IsFlagSet(flags, FlagType::StartGo))
      printf("startGo ");
  }

  void printTime(double time_s) {
    const int minutes = static_cast<int>(time_s / 60);
    const float seconds = static_cast<float>(time_s - (60 * minutes));
    printf("%03d:%05.2f", minutes, seconds);
  }

  void printSessionState(SessionState state) {
    switch (state) {
    case SessionState::Invalid:
      printf("Invalid");
      break;
    case SessionState::GetInCar:
      printf("GetInCar");
      break;
    case SessionState::Warmup:
      printf("Warmup");
      break;
    case SessionState::ParadeLaps:
      printf("ParadeLap");
      break;
    case SessionState::Racing:
      printf("Racing");
      break;
    case SessionState::Checkered:
      printf("Checkered");
      break;
    case SessionState::CoolDown:
      printf("CoolDown");
      break;
    }
  }

  void printPaceMode(PaceMode mode) {
    switch (mode) {
    case PaceMode::SingleFileStart:
      printf("SingleFileStart");
      break;
    case PaceMode::DoubleFileStart:
      printf("DoubleFileStart");
      break;
    case PaceMode::SingleFileRestart:
      printf("SingleFileRestart");
      break;
    case PaceMode::DoubleFileRestart:
      printf("DoubleFileRestart:");
      break;
    case PaceMode::NotPacing:
      printf("NotPacing");
      break;
    }
  }

  void printPaceFlags(uint32_t flags) {
    if (IsPaceFlagSet(flags, PaceFlagType::EndOfLine))
      printf("EndOfLine|");
    if (IsPaceFlagSet(flags, PaceFlagType::FreePass))
      printf("FreePass|");
    if (IsPaceFlagSet(flags, PaceFlagType::WavedAround))
      printf("WavedAround|");
  }

  /**
   * Update the `DriverEntry` records.
   *
   * This should only be invoked when `SessionInfo` changes
   */
  void updateDriverInfo() {
    static auto &client = LiveClient::GetInstance();
    auto sessionInfoDoc = client.getSessionInfo().lock();
    if (!sessionInfoDoc) {
      return;
    }

    auto& trackLengthStr =  sessionInfoDoc->weekendInfo.trackLength;
    gLapTimingData.trackLength = std::stod(trackLengthStr);
    if (trackLengthStr.ends_with("km"))
      gLapTimingData.trackLength *= 1000.0;

    // COPY TO BE SAFE (LOW COST)
    gLapTimingData.eventSessions = sessionInfoDoc->sessionInfo.sessions;

    auto drivers = sessionInfoDoc->driverInfo.drivers;
    for (int i = 0; i < kMaxCars && i < drivers.size(); i++) {
      // skip the rest if idx not found
      auto &driverInfo = drivers[i];
      auto idx = driverInfo.carIdx;
      if (!idx)
        continue;

      auto &driverEntry = gLapTimingData.drivers[i];
      driverEntry.idx = idx;
      driverEntry.classId = driverInfo.carClassID;
      strcpy(driverEntry.driverName, driverInfo.userName.c_str());
      strcpy(driverEntry.teamName, driverInfo.teamName.c_str());
      strcpy(driverEntry.carNumStr, driverInfo.carNumber.c_str());
    }
  }

  /**
   * Get a list of `DriverEntry` indices in `position` order
   *
   * @return vector of indices, which maps to driver entry index, keyed by race position
   */
  std::vector<std::int32_t> GetDriverPositionIndexes() {
    std::vector<std::int32_t> driverIndexes{};
    for (int i = 0; i < kMaxCars; i++) {
      auto &driver = gLapTimingData.drivers[i];
      auto idx = driver.idx;

      // CHECK FOR VALID ENTRY & STATE
      if (idx < 0 || gCarIdxTrackLocation.getInt(idx) == -1 || gCarIdxTrackSurface.getInt(idx) == -1 || gCarIdxLap.getInt(idx) == -1 || gCarIdxPosition.getInt(idx) == 0) {
        continue;
      }

      driverIndexes.push_back(i);
    }

    // ALWAYS SORT BY OVERALL POSITION FIRST
    std::ranges::sort(driverIndexes, [](int a, int b) {
      auto &aDriver = gLapTimingData.drivers[a];
      auto &bDriver = gLapTimingData.drivers[b];
      return aDriver.lap > bDriver.lap || (aDriver.lap == bDriver.lap && aDriver.lapDistPct > bDriver.lapDistPct);
    });

    std::uint32_t pos{0};
    for (auto &driverIndex : driverIndexes) {
      pos++;
      auto &driver = gLapTimingData.drivers[driverIndex];
      driver.position = pos;
    }
    return driverIndexes;
  }

  /**
   * Prepare relative lap timing view
   *
   * @param driverIndexes vector of indexes, in position order
   * @return [bool, DriverEntry*] pair, if `first` is false, `second` is `nullptr`, and vice versa
   */
  std::pair<bool, DriverEntry*> PrepareRelativeView(std::vector<std::int32_t>& driverIndexes) {
    bool relativeView = false;
    auto& data = gLapTimingData;
    DriverEntry* relativeToDriver{nullptr};
    if (data.viewMode == ViewMode::Relative && data.relativeToDriverIndex >= 0) {
      relativeView = true;
      relativeToDriver = &data.drivers[data.relativeToDriverIndex];

      // CREATE `TOP` RANGES FOR DETERMINING PRESENTATION ABOVE/BELOW
      std::vector<std::pair<double, double>> topRanges{};
      double topBound = relativeToDriver->lapDistPct + 0.5;
      if (topBound > 1.0) {
        topRanges.emplace_back(relativeToDriver->lapDistPct, 1.0);
        topRanges.emplace_back(0.0, topBound - 1.0);
      } else {
        topRanges.emplace_back(relativeToDriver->lapDistPct, topBound);
      }

      for (std::int32_t i = 0; i < driverIndexes.size();i++) {
        auto driverIndex = driverIndexes[i];
        auto &driver = data.drivers[driverIndex];

        if (driver.idx == relativeToDriver->idx) {
          driver.gapToRelative = 0.0;
          continue;
        }

        bool isAbove = std::ranges::any_of(topRanges, [&] (std::pair<double,double>& range) -> bool {
          return driver.lapDistPct >= range.first && driver.lapDistPct < range.second;
        });

        double diffPct = std::min<double>(
          std::fabs(relativeToDriver->lapDistPct - driver.lapDistPct),
          std::fabs(1.0 + driver.lapDistPct - relativeToDriver->lapDistPct)
        );

        std::vector<double> availableLapTimes{driver.lapTimeBest, driver.lapTimeLast, relativeToDriver->lapTimeBest, relativeToDriver->lapTimeLast};
        double lapTime = -1;
        for (auto lt : availableLapTimes) {
          if (lt <= 0.0) {
            continue;
          }
          lapTime = lt;
          break;
        }

        if (lapTime > 0.0) {
          driver.gapToRelative = diffPct * lapTime;
        } else {
          driver.gapToRelative = std::max<double>(relativeToDriver->lapTimeCurrent, driver.lapTimeCurrent)
            - std::min<double>(relativeToDriver->lapTimeCurrent, driver.lapTimeCurrent);

        }

        if (!isAbove) {
          driver.gapToRelative *= -1;
        }
      }

      std::ranges::sort(driverIndexes, [](int a, int b) {
        auto &aDriver = gLapTimingData.drivers[a];
        auto &bDriver = gLapTimingData.drivers[b];
        return aDriver.gapToRelative > bDriver.gapToRelative;
      });

    }

    return {relativeView, relativeToDriver};
  }

  /**
   * Update the console display
   */
  void updateDisplay() {
    auto& data = gLapTimingData;

    // CONSOLE DIMENSIONS
    int width, height;
    Console::getDimensions(width, height);

    // GET SESSION INFO
    auto sessionType{SessionType::Unknown};
    std::string sessionName{magic_enum::enum_name(SessionType::Unknown)};

    auto sessionNum = gSessionNum.getInt();
    for (auto& eventSession: data.eventSessions) {
      if (eventSession.sessionNum == sessionNum) {
        sessionName = eventSession.sessionName;
        sessionType = magic_enum::enum_cast<SessionType>(sessionName, magic_enum::case_insensitive).value_or(SessionType::Unknown);
        break;
      }
    }

    // force console to scroll to top line
    Console::setCursorPosition(0, 0);
    Console::setStyle();

    constexpr int statusOffset = 3;
    constexpr int carsOffset = 6;
    const int maxCarLines = height - carsOffset;

    // print race status line
    Console::setCursorPosition(0, statusOffset);
    Console::setStyle(Console::Color::BLACK,Console::Color::RED);
    printf("Time: ");
    printTime(gSessionTime.getDouble());
    Console::setStyle();

    std::print(std::cout, " Session ({}): {}/{}", sessionNum, sessionName, magic_enum::enum_name(sessionType));

    printf(" Coordinate: %f,%f", gLongitude.getDouble(), gLatitude.getDouble());
    printf(" LapsComplete: %03d", gRaceLaps.getInt());

    if (gSessionLapsRemainEx.getInt() < 32767)
      printf(" LapsRemain: %03d", gSessionLapsRemainEx.getInt());
    else
      printf(" LapsRemain: Unlimited");

    printf(" TimeRemain: ");
    if (gSessionTimeRemain.getDouble() < 604800.0f)
      printTime(gSessionTimeRemain.getDouble());
    else
      printf("Unlimited");

    // print flag status
    Console::setCursorPosition(0, statusOffset + 1);
    printf("Last lap time: %f ", gLapLastLapTime.getFloat());

    printf(" flags: ");
    printFlags(gSessionFlags.getInt());

    printf(" PitsOpen: %d", gPitsOpen.getBool());

    printf(" State: ");
    printSessionState(magic_enum::enum_cast<SessionState>(gSessionState.getInt()).value());

    // new variables check if on members
    if (gPaceMode.isValid()) {
      printf(" PaceMode: ");
      printPaceMode(magic_enum::enum_cast<PaceMode>(gPaceMode.getInt()).value());
    }

    // print car info
    Console::setCursorPosition(0, carsOffset);

    // SORT THE DRIVERS BASED ON VIEW MODE
    std::vector<std::int32_t> driverIndexes = GetDriverPositionIndexes();

    if (driverIndexes.size()) {
      auto& leader = gLapTimingData.drivers[driverIndexes[0]];
      for (auto driverIndex : driverIndexes) {
        auto& driver = gLapTimingData.drivers[driverIndex];
        if (driver.idx == leader.idx) {
          driver.gapToLeader = 0;
          driver.gapToNext = 0;
          continue;
        }

        if (leader.lapTimeLast < 1.0f) {
          driver.gapToLeader = leader.lapTimeCurrent - driver.lapTimeCurrent;
        } else {
          driver.gapToLeader = (leader.totalDistPct - driver.totalDistPct) * leader.lapTimeLast;
        }
      }
    }

    auto [relativeView, relativeToDriver] = PrepareRelativeView(driverIndexes);


    // don't scroll off the end of the buffer
    int linesUsed = 0;
    if (relativeView) {
      linesUsed++;
      Console::setStyle(Console::Color::BLACK, Console::Color::RED);
      std::println(std::cout, "Relative to driver: {}", relativeToDriver->driverName);
      Console::setStyle();
    }
    const int maxLines = std::min<int>(kMaxCars, maxCarLines);

    for (std::int32_t i = 0; i < driverIndexes.size();i++) {
      if (linesUsed < maxLines) {
        // is the car in the world, or did we at least collect data on it when it was?
        auto driverIndex = driverIndexes[i];
        auto &driver = gLapTimingData.drivers[driverIndex];
        auto idx = driver.idx;
        if (idx < 0 || gCarIdxTrackLocation.getInt(idx) == -1 || gCarIdxTrackSurface.getInt(idx) == -1 || gCarIdxLap.getInt(idx) == -1 || gCarIdxPosition.getInt(idx) == 0) {
          continue;
        }
        if (relativeView && !!relativeToDriver) {
          if (driver.idx == relativeToDriver->idx) {
            Console::setStyle(Console::Color::BLACK, Console::Color::GREEN, true);
          } else if (driver.gapToRelative > relativeToDriver->gapToRelative) {
            Console::setStyle(Console::Color::BLACK, Console::Color::BLUE);
          } else {
            Console::setStyle(Console::Color::BLACK, Console::Color::RED);
          }

          printf(" %2d %2d %25s %03.3f secs \n",
            driver.position,
            driver.lap,
            // driver.carNumStr,
            driver.driverName,

            driver.gapToRelative
            );

          Console::setStyle();
        } else {
          printf(" %2d %3s %25s %3.3f %3.3f %7.3f %7.3f\n",
            driver.position,
            driver.carNumStr,
            driver.driverName,
            driver.gapToLeader,
            driver.gapToLeaderF2, driver.lapTimeLast, driver.lapTimeCurrent);
        }
        // printf(" %2d %2d %10s %3s %7.3f %2d %2d %2d %6.3f %2d %8.2f %5.2f %2d %2d %2d %2d %7.3f %7.3f %7.3f %7.3f %2d %d %2d "
        //        "%2d %2d 0x%02x\n",
        //        driver.position,
        //        idx,
        //        gLapTimingData.drivers[idx].driverName,
        //        gLapTimingData.drivers[idx].carNumStr,
        //        gCarIdxEstTime.getFloat(idx),
        //        gCarIdxGear.getInt(idx),
        //        gCarIdxLap.getInt(idx),
        //        gCarIdxLapCompleted.getInt(idx),
        //        gCarIdxLapDistPct.getFloat(idx),
        //        gCarIdxOnPitRoad.getBool(idx),
        //        gCarIdxRPM.getFloat(idx),
        //        gCarIdxSteer.getFloat(idx),
        //        gCarIdxTrackSurface.getInt(idx),
        //        gCarIdxTrackSurfaceMaterial.getInt(idx),
        //        gCarIdxPosition.getInt(idx),
        //        gCarIdxClassPosition.getInt(idx),
        //        gCarIdxF2Time.getFloat(idx),
        //        //****Note, don't use this one any more, it is replaced by CarIdxLastLapTime
        //        gLapTimingData.drivers[idx].lapTimeLast,
        //        // new variables, check if they exist on members
        //        (gCarIdxLastLapTime.isValid()) ? gCarIdxLastLapTime.getFloat(idx) : -1,
        //        (gCarIdxBestLapTime.isValid()) ? gCarIdxBestLapTime.getFloat(idx) : -1,
        //        (gCarIdxBestLapNum.isValid()) ? gCarIdxBestLapNum.getInt(idx) : -1,
        //        (gCarIdxP2P_Status.isValid()) ? gCarIdxP2P_Status.getBool(idx) : -1,
        //        (gCarIdxP2P_Count.isValid()) ? gCarIdxP2P_Count.getInt(idx) : -1,
        //        (gCarIdxPaceLine.isValid()) ? gCarIdxPaceLine.getInt(idx) : -1,
        //        (gCarIdxPaceRow.isValid()) ? gCarIdxPaceRow.getInt(idx) : -1,
        //        (gCarIdxPaceFlags.isValid()) ? gCarIdxPaceFlags.getInt(idx) : -1);
        linesUsed++;
      }
    }
    // clear remaining lines
    for (int i = linesUsed; i < maxLines; i++)
      printf("                                                                     \n");
  }

  void monitorConnectionStatus() {
    // keep track of connection status
    static bool wasConnected = false;

    const auto isConnected = LiveClient::GetInstance().isConnected();
    if (wasConnected != isConnected) {
      Console::setCursorPosition(0, 1);
      if (isConnected) {
        printf("Connected to iRacing              \n");
        ResetState(true);
      } else
        printf("Lost connection to iRacing        \n");

      //****Note, put your connection handling here

      wasConnected = isConnected;
    }
  }

  void setRelativeViewMode() {
    gLapTimingData.viewMode = ViewMode::Relative;

    auto driverIndexes = GetDriverPositionIndexes();
    if (driverIndexes.empty())
      return;

    if (ContainsValue(driverIndexes, gLapTimingData.relativeToDriverIndex)) {
      return;
    }

    gLapTimingData.relativeToDriverIndex = driverIndexes[0];
  }

  void nextRelativeDriver() {
    gLapTimingData.viewMode = ViewMode::Relative;

    auto driverIndexes = GetDriverPositionIndexes();
    if (driverIndexes.empty())
      return;

    auto driverIndex = gLapTimingData.relativeToDriverIndex;
    if (driverIndex == -1 || !ContainsValue(driverIndexes, gLapTimingData.relativeToDriverIndex)) {
      gLapTimingData.relativeToDriverIndex = driverIndexes[0];
      return;
    }

    auto it = std::ranges::find(driverIndexes, driverIndex);

    if (it != driverIndexes.end()) {
      // Calculate the index
      size_t index = std::distance(driverIndexes.begin(), it);
      if (index + 1 >= driverIndexes.size())
        index = 0;
      else
        index += 1;
      gLapTimingData.relativeToDriverIndex = driverIndexes[index];
    } else {
      gLapTimingData.relativeToDriverIndex = driverIndexes[0];
    }

  }

  void run() {
    static auto &client = LiveClient::GetInstance();

    // wait up to 16 ms for start of session or new data
    if (client.waitForData(16)) {

      bool wasUpdated = client.wasSessionInfoUpdated();
      if (wasUpdated) {
        L->info("Session info updated (count={})", client.getSessionInfoUpdateCount().value());
        updateDriverInfo();
      }

      // and grab the data
      processLapInfo();

      // notify clients
      if (wasUpdated && hDataValidEvent)
        PulseEvent(hDataValidEvent);

      updateDisplay();
    }
    // else we did not grab data, do nothing

    // pump our connection status
    monitorConnectionStatus();

    //****Note, add your own additional loop processing here
    // for anything not dependant on telemetry data (keeping a UI running, etc)
  }

  void SignalHandler(int sig);


  //-----------------------

  class ExampleResources {

    std::atomic_bool shutdown_{false};

  public:

    static ExampleResources &GetInstance() {
      static ExampleResources sInstance{};
      return sInstance;
    };


    void signalHandler(int sig) {
      (void)sig;
      std::printf("Received ctrl-c, exiting\n\n");
      if (shutdown_.exchange(true)) {
        std::println(std::cout, "Shutdown already triggered");
      }
    }

    bool isShutdown() {
      return shutdown_;
    }

    bool setShutdown() {
      return shutdown_.exchange(true);
    }

    ExampleResources() {
      // trap ctrl-c
      std::signal(SIGINT, SignalHandler);

      // bump priority up so we get time from the sim
      SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);

      // ask for 1ms timer so sleeps are more precise
      timeBeginPeriod(1);

      // startup event broadcaster
      hDataValidEvent = CreateEvent(nullptr, true, false, Resources::DataValidEventName);
    }

    ~ExampleResources() {
      printf("Shutting down.\n\n");

      // shutdown
      if (hDataValidEvent) {
        // make sure event not left triggered (probably redundant)
        ResetEvent(hDataValidEvent);
        CloseHandle(hDataValidEvent);
        hDataValidEvent = nullptr;
      }

      timeEndPeriod(1);
    }
  };

  void SignalHandler(int sig) {

    ExampleResources::GetInstance().signalHandler(sig);
  }
} // namespace

int main(int argc, char *argv[]) {
  L->set_level(spdlog::level::level_enum::trace);
  L->flush_on(spdlog::level::level_enum::trace);
  std::println(std::cout, "lapTiming 1.1, press Ctrl-C or Q to exit\n");

  {
    auto &resources = ExampleResources::GetInstance();
    while (!resources.isShutdown()) {
      if (_kbhit()) {
        auto c = _getch();

        switch (c) {
        case 'q':
          resources.setShutdown();
          continue;
        case 'r':
          std::println(std::cout, "Mode change to 'relative'");
          setRelativeViewMode();

          break;
        case 'n':
          std::println(std::cout, "Next relative driver");
          nextRelativeDriver();
          break;
        case 's':
          std::println(std::cout, "Mode change to 'standings'");
          gLapTimingData.viewMode = ViewMode::Standings;
          break;
        default:
          std::println(std::cout, "Key press ignored: {}", c);
        }
      }
      run();
    }
  }

  signal(SIGINT, SIG_DFL);
  exit(0);


  return 0;
}
