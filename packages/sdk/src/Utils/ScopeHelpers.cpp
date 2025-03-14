#include <IRacingSDK/Utils/ScopeHelpers.h>

namespace IRacingSDK::Utils {
  ScopedGuard::ScopedGuard(std::function<void()> f) : callback_(f) {
  }

  // Destructors can't/shouldn't throw; if the callback throws, terminate.
  ScopedGuard::~ScopedGuard() noexcept {
    if (!callback_) {
      return;
    }
    (*callback_)();
  }

  void ScopedGuard::abandon() {
    callback_ = {};
  }
}