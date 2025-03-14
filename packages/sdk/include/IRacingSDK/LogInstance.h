#pragma once

#include <memory>
#include <spdlog/spdlog.h>
#include <IRacingSDK/Utils/Singleton.h>

namespace IRacingSDK {

  /**
   * @brief LogInstance class to manage global logging behavior
   */
  class LogInstance : public Utils::Singleton<LogInstance> {
    /**
     * @brief Create LogInstance
     */
    explicit LogInstance(token){};
    friend Singleton;

  public:
    LogInstance() = delete;

    /**
     * @brief
     * @param newLogger new logger to assign globally
     * @return previous logger or nullptr if not assigned
     */
    std::shared_ptr<spdlog::logger> setDefaultLogger(const std::shared_ptr<spdlog::logger>& newLogger);


    /**
     * @brief Retrieves the default logger instance.
     *
     * This method ensures that a shared instance of the default logger is
     * created and returned. If the logger does not already exist, it is
     * initialized in a thread-safe manner.
     *
     * @return A shared pointer to the default logger instance.
     */
    std::shared_ptr<spdlog::logger> getDefaultLogger();


  };

  static std::shared_ptr<spdlog::logger> GetDefaultLogger() {
    return LogInstance::GetPtr()->getDefaultLogger();
  }


} // namespace IRacingSDK
