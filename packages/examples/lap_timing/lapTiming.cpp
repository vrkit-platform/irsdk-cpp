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
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>
#include <windows.h>

#include <IRacingSDK/LiveClient.h>
#include <IRacingSDK/LiveConnection.h>
#include <IRacingSDK/Types.h>
#include <IRacingSDK/VarHolder.h>

#include "console.h"

// Live weather info, may change as session progresses
namespace {
  using namespace IRacingSDK;
  using namespace IRacingSDK::Utils;

  std::shared_ptr<spdlog::logger> L{spdlog::basic_logger_mt("LapTimingExample", "tmp/lap-timing.log")};

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

  constexpr int kMaxCars{64};
  constexpr int kMaxNameLen{64};

  double gLastTime{-1};
  float gLastDistPct[kMaxCars]{-1};
  double gLapStartTime[kMaxCars]{-1};

  // lap time for last lap, or -1 if not yet completed a lap
  float gLapTime[kMaxCars]{-1};

  struct DriverEntry {
    int carIdx;
    int carClassId;
    char driverName[kMaxNameLen];
    char teamName[kMaxNameLen];
    char carNumStr[10]; // the player car number as a character string so we can handle 001 and other oddities
  };

  // updated for each driver as they cross the start/finish line
  DriverEntry gDriverTableTable[kMaxCars];

  // notify everyone when we update the file
  HANDLE hDataValidEvent{nullptr};


  void resetState(bool isNewConnection) {
    if (isNewConnection)
      memset(gDriverTableTable, 0, sizeof(gDriverTableTable));

    for (int i = 0; i < kMaxCars; i++) {
      gLastTime = -1;
      gLastDistPct[i] = -1;
      gLapStartTime[i] = -1;
      gLapTime[i] = -1;
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
    if (gLastTime > curTime)
      resetState(false);

    for (int i = 0; i < kMaxCars; i++) {
      const float curDistPct = gCarIdxLapDistPct.getFloat(i);
      // reject if the car blinked out of the world
      if (curDistPct != -1) {
        // did we cross the lap?
        if (gLastDistPct[i] > 0.9f && curDistPct < 0.1f) {
          // calculate exact time of lap crossing
          const double curLapStartTime =
            interpolateTimeAcrossPoint(gLastTime, curTime, gLastDistPct[i], curDistPct, 0);

          // calculate lap time, if already crossed start/finish
          if (gLapStartTime[i] != -1)
            gLapTime[i] = static_cast<float>(curLapStartTime - gLapStartTime[i]);

          // and store start/finish crossing time for next lap
          gLapStartTime[i] = curLapStartTime;
        }

        gLastDistPct[i] = curDistPct;
      }
    }

    gLastTime = curTime;
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

  void updateDriverInfo() {
    static auto &client = LiveClient::GetInstance();
    auto sessionInfo = client.getSessionInfo().lock();
    if (!sessionInfo) {
      return;
    }

    auto drivers = sessionInfo->driverInfo.drivers;
    for (int i = 0; i < kMaxCars && i < drivers.size(); i++) {
      // skip the rest if carIdx not found
      auto &driverInfo = drivers[i];
      auto idx = driverInfo.carIdx;
      if (!idx)
        continue;

      auto &driverEntry = gDriverTableTable[i];
      driverEntry.carIdx = idx;
      driverEntry.carClassId = driverInfo.carClassID;
      strcpy(driverEntry.driverName, driverInfo.userName.c_str());
      strcpy(driverEntry.teamName, driverInfo.teamName.c_str());
      strcpy(driverEntry.carNumStr, driverInfo.carNumber.c_str());
    }
  }

  void updateDisplay() {
    // force console to scroll to top line
    setCursorPosition(0, 0);

    int width, height;
    getConsoleDimensions(width, height);

    const int statusOffset = 3;
    const int carsOffset = 6;
    const int maxCarLines = height - carsOffset;

    // print race status line
    setCursorPosition(0, statusOffset);
    printf("Time: ");
    printTime(gSessionTime.getDouble());

    printf(" Session: %d", gSessionNum.getInt());
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
    setCursorPosition(0, statusOffset + 1);
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
    setCursorPosition(0, carsOffset);
    // don't scroll off the end of the buffer
    int linesUsed = 0;
    const int maxLines = min(kMaxCars, maxCarLines);
    for (int i = 0; i < kMaxCars; i++) {
      if (linesUsed < maxLines) {
        // is the car in the world, or did we at least collect data on it when it was?
        if (gCarIdxTrackLocation.getInt(i) != -1 && gCarIdxTrackSurface.getInt(i) != -1 && gCarIdxLap.getInt(i) != -1 && gCarIdxPosition.getInt(i) != 0) {
          printf(" %2d %10s %3s %7.3f %2d %2d %2d %6.3f %2d %8.2f %5.2f %2d %2d %2d %2d %7.3f %7.3f %7.3f %7.3f %2d %d %2d "
                 "%2d %2d 0x%02x\n",
                 i,
                 gDriverTableTable[i].driverName,
                 gDriverTableTable[i].carNumStr,
                 gCarIdxEstTime.getFloat(i),
                 gCarIdxGear.getInt(i),
                 gCarIdxLap.getInt(i),
                 gCarIdxLapCompleted.getInt(i),
                 gCarIdxLapDistPct.getFloat(i),
                 gCarIdxOnPitRoad.getBool(i),
                 gCarIdxRPM.getFloat(i),
                 gCarIdxSteer.getFloat(i),
                 gCarIdxTrackSurface.getInt(i),
                 gCarIdxTrackSurfaceMaterial.getInt(i),
                 gCarIdxPosition.getInt(i),
                 gCarIdxClassPosition.getInt(i),
                 gCarIdxF2Time.getFloat(i),
                 //****Note, don't use this one any more, it is replaced by CarIdxLastLapTime
                 gLapTime[i],
                 // new variables, check if they exist on members
                 (gCarIdxLastLapTime.isValid()) ? gCarIdxLastLapTime.getFloat(i) : -1,
                 (gCarIdxBestLapTime.isValid()) ? gCarIdxBestLapTime.getFloat(i) : -1,
                 (gCarIdxBestLapNum.isValid()) ? gCarIdxBestLapNum.getInt(i) : -1,
                 (gCarIdxP2P_Status.isValid()) ? gCarIdxP2P_Status.getBool(i) : -1,
                 (gCarIdxP2P_Count.isValid()) ? gCarIdxP2P_Count.getInt(i) : -1,
                 (gCarIdxPaceLine.isValid()) ? gCarIdxPaceLine.getInt(i) : -1,
                 (gCarIdxPaceRow.isValid()) ? gCarIdxPaceRow.getInt(i) : -1,
                 (gCarIdxPaceFlags.isValid()) ? gCarIdxPaceFlags.getInt(i) : -1);
          linesUsed++;
        }
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
      setCursorPosition(0, 1);
      if (isConnected) {
        printf("Connected to iRacing              \n");
        resetState(true);
      } else
        printf("Lost connection to iRacing        \n");

      //****Note, put your connection handling here

      wasConnected = isConnected;
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

  //-----------------------

  void ex_program(int sig) {
    (void)sig;

    printf("recieved ctrl-c, exiting\n\n");

    timeEndPeriod(1);

    signal(SIGINT, SIG_DFL);
    exit(0);
  }

  bool init() {
    // trap ctrl-c
    signal(SIGINT, ex_program);

    // bump priority up so we get time from the sim
    SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);

    // ask for 1ms timer so sleeps are more precise
    timeBeginPeriod(1);

    // startup event broadcaster
    hDataValidEvent = CreateEvent(nullptr, true, false, Resources::DataValidEventName);

    //****Note, put your init logic here

    return true;
  }

  void deInit() {
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
} // namespace

int main(int argc, char *argv[]) {
  printf("lapTiming 1.1, press any key to exit\n");
  // L->enable_backtrace();
  L->set_level(spdlog::level::level_enum::trace);
  L->flush_on(spdlog::level::level_enum::trace);
  printf("lapTiming 1.1, press any key to exit\n");

  init();
  while (!_kbhit()) {
    run();
  }

  deInit();

  return 0;
}
