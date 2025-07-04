//
// Created by jglanz on 4/19/2024.
//

#pragma once

#include <IRacingSDK/DiskClient.h>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <csignal>
#include <filesystem>
#include <memory>
#include <mutex>
#include <thread>


namespace IRacingSDK::Examples {
  namespace fs = std::filesystem;

  class FakeLiveClientSharedMemoryServer {
    DiskClient diskClient_;
    fs::path ibtPath_;
    std::unique_ptr<std::thread> thread_;
    std::atomic_bool running_{true};
    std::mutex mutex_{};
    std::condition_variable sleepCondition_{};

    void run();

  public:

    FakeLiveClientSharedMemoryServer() = delete;

    explicit FakeLiveClientSharedMemoryServer(const fs::path &ibtPath);

    FakeLiveClientSharedMemoryServer(const FakeLiveClientSharedMemoryServer &) = delete;
    FakeLiveClientSharedMemoryServer(FakeLiveClientSharedMemoryServer &&) = delete;

    /**
     * @brief Wait for thread to complete
     */
    void waitFor();

    /**
     * @brief Destroy the tool
     */
    void destroy();
  };

} // namespace IRacingSDK::Examples
