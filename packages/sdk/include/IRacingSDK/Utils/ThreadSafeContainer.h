#pragma once

#include <shared_mutex>
#include <memory>

namespace IRacingSDK::Utils {
  template <typename T>
  class ThreadSafeContainer {
    private:

      // Forward declarations
      class MutationGuard;
      class SharedGuard;

    public:

      ThreadSafeContainer() = default;

      explicit ThreadSafeContainer(const T& value) : data_(value) {
      }

      explicit ThreadSafeContainer(T&& value) : data_(std::move(value)) {
      }

      // Getter - returns a SharedGuard with shared lock
      [[nodiscard]] std::unique_ptr<SharedGuard> get() const {
        return std::make_unique<SharedGuard>(*this);
      }

      // Setter - returns a MutationGuard with exclusive lock
      [[nodiscard]] std::unique_ptr<MutationGuard> set() {
        return std::make_unique<MutationGuard>(*this);
      }

    private:

      // Internal shared guard class that manages shared lock
      class SharedGuard {
        public:

          explicit SharedGuard(const ThreadSafeContainer& container)
            : container_(container),
              lock_(container.mutex_) {
          }

          // Prevent copying
          SharedGuard(const SharedGuard&) = delete;

          SharedGuard& operator=(const SharedGuard&) = delete;

          // Allow moving
          SharedGuard(SharedGuard&&) = default;

          SharedGuard& operator=(SharedGuard&&) = default;

          // Access to the protected data
          const T* operator->() const {
            return &container_.data_;
          }

          const T& operator*() const {
            return container_.data_;
          }

        private:

          const ThreadSafeContainer& container_;
          std::shared_lock<std::shared_mutex> lock_;
      };

      // Internal mutation guard class that manages exclusive lock
      class MutationGuard {
        public:

          explicit MutationGuard(ThreadSafeContainer& container)
            : container_(container),
              lock_(container.mutex_) {
          }

          // Prevent copying
          MutationGuard(const MutationGuard&) = delete;

          MutationGuard& operator=(const MutationGuard&) = delete;

          // Allow moving
          MutationGuard(MutationGuard&&) = default;

          MutationGuard& operator=(MutationGuard&&) = default;

          // Access to the protected data
          T* operator->() {
            return &container_.data_;
          }

          T& operator*() {
            return container_.data_;
          }

        private:

          ThreadSafeContainer& container_;
          std::unique_lock<std::shared_mutex> lock_;
      };

      mutable std::shared_mutex mutex_;
      T data_;
  };
}
