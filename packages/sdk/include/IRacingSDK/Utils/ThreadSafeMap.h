#pragma once
#include <map>
#include <mutex>
#include <shared_mutex>

namespace IRacingSDK::Utils {
  template <typename Key, typename Value>
  class ThreadSafeMap {

    std::map<Key, Value> map_;
    mutable std::shared_mutex mutex_; // Allows multiple readers but single writer

    public:

      // Insert or update
      void set(const Key& key, const Value& value) {
        std::unique_lock lock(mutex_);
        map_[key] = value;
      }

      // Emplace
      template <typename... Args>
      std::pair<typename std::map<Key, Value>::iterator, bool>
      emplace(Args&&... args) {
        std::unique_lock lock(mutex_);
        return map_.emplace(std::forward<Args>(args)...);
      }

      // Read with optional default value
      std::optional<Value> get(const Key& key) const {
        std::shared_lock lock(mutex_);
        if (map_.contains(key))
          return map_[key];

        return std::nullopt;
      }

      template<typename F>
      Value getOrInsert(const Key& key, F&& insertFn) {
        std::unique_lock lock(mutex_);
        if (map_.contains(key))
          return map_[key];

        map_[key] = insertFn();
        return map_[key];
      }
      // Remove
      bool erase(const Key& key) {
        std::unique_lock lock(mutex_);
        return map_.erase(key) > 0;
      }

      // Check if key exists
      bool contains(const Key& key) const {
        std::shared_lock lock(mutex_);
        return map_.contains(key);
      }

      // Clear
      void clear() {
        std::unique_lock lock(mutex_);
        map_.clear();
      }

      // Size
      size_t size() const {
        std::shared_lock lock(mutex_);
        return map_.size();
      }

      // Empty check
      bool empty() const {
        std::shared_lock lock(mutex_);
        return map_.empty();
      }

      // Atomic update with callback
      template <typename F>
      std::optional<Value> mutateEntry(const Key& key, F&& updateFn) {
        std::unique_lock lock(mutex_);

        if (map_.contains(key)) {
          map_[key] = updateFn(map_[key]);
          return map_[key];
        }
        return std::nullopt;
      }

      // Thread-safe iteration
      template <typename F>
      void forEach(F&& fn) const {
        std::shared_lock lock(mutex_);
        for (const auto& [key, value] : map_) {
          fn(key, value);
        }
      }
  };
}
