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

#include <FakeLiveClientSharedMemoryServer.h>

using namespace IRacingSDK::Examples;

namespace {

  std::shared_ptr<spdlog::logger> L{spdlog::basic_logger_mt("LiveFromDiskPlayer", "tmp/live-from-disk-player.log")};
  std::shared_ptr<FakeLiveClientSharedMemoryServer> gFakeLiveClientSharedMemoryServer{nullptr};

  void SignalHandler(int signal) {
    std::cerr << "Interrupted by Signal" << signal << "\n";
    L->info("Interrupted ({})", signal);

    if (gFakeLiveClientSharedMemoryServer)
      gFakeLiveClientSharedMemoryServer->destroy();
  }
} // namespace

int main(int argc, char *argv[]) {
  L->set_level(spdlog::level::level_enum::trace);
  L->flush_on(spdlog::level::level_enum::trace);
  std::println(std::cout, "lapTiming 1.1, press Ctrl-C or Q to exit\n");
  if (argc < 2) {
    std::println(std::cerr, "Usage: {} <ibt-file>", argv[0]);
    return 1;
  }
  std::signal(SIGINT, SignalHandler);
  std::string ibtPath(argv[1]);
  auto tool = gFakeLiveClientSharedMemoryServer = std::make_shared<FakeLiveClientSharedMemoryServer>(ibtPath);
  tool->waitFor();

  return 0;
}
