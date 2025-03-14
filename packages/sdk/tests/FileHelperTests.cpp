#include <gtest/gtest.h>
#include <IRacingSDK/Utils/FileHelpers.h>
#include <spdlog/spdlog.h>

using namespace IRacingSDK;
using namespace IRacingSDK::Utils;
using namespace spdlog;

namespace fs = std::filesystem;

namespace {
  auto L = default_logger();
}

class FileHelperTests : public testing::Test {
protected:

  FileHelperTests() = default;

  virtual void TearDown() override {
    L->flush();
  }
};


TEST_F(FileHelperTests, FileHasExtension) {
    std::vector<fs::path> goodFilenames = {"file.Json", ".Json", ".JSON","test.dashboard.irt.json"};
    for (auto& good : goodFilenames ) {
        EXPECT_TRUE(HasFileExtension(good, ".json"));
    }
    std::vector<fs::path> badFilenames = {"file.txt", "file.Json5", ".irtJSON"};

    for (auto& bad : badFilenames ) {
        EXPECT_FALSE(HasFileExtension(bad, ".json"));
    }
}

TEST_F(FileHelperTests, FindExeInPath) {
  std::vector<fs::path> requiredExes = {"7z.exe"};
  for (auto& exeName : requiredExes ) {
    auto res = FindExeInPath(exeName.string());

    ASSERT_TRUE(res.has_value()) << "Executable " << exeName.string() << " MUST be available in the systems PATH";
    if (res.has_value()) {
      L->info("Found '{}' in PATH @ '{}'", exeName.string(), res.value());
    }
  }
}

