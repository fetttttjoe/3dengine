#include "Core/SettingsManager.h"
#include "gtest/gtest.h"
#include <fstream>
#include <cstdio>

class SettingsManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        AppSettings& settings = SettingsManager::Get();
        settings.leftPaneWidth = 200.0f;
        settings.rightPaneWidth = 300.0f;
        settings.cloneOffset = {0.5f, 0.5f, 0.0f};
    }
    void TearDown() override {
        std::remove("test_settings.json");
    }
};

// --- Positive Tests ---

TEST_F(SettingsManagerTest, DefaultsAreCorrect) {
    AppSettings& settings = SettingsManager::Get();
    EXPECT_EQ(settings.leftPaneWidth, 200.0f);
    EXPECT_EQ(settings.rightPaneWidth, 300.0f);
    EXPECT_EQ(settings.cloneOffset, glm::vec3(0.5f, 0.5f, 0.0f));
}

TEST_F(SettingsManagerTest, SaveAndLoad) {
    SettingsManager::Get().leftPaneWidth = 250.5f;
    SettingsManager::Get().cloneOffset = glm::vec3(1.0f, 0.0f, -1.0f);
    ASSERT_TRUE(SettingsManager::Save("test_settings.json"));
    SetUp(); // Reset to defaults
    ASSERT_TRUE(SettingsManager::Load("test_settings.json"));
    EXPECT_FLOAT_EQ(SettingsManager::Get().leftPaneWidth, 250.5f);
    EXPECT_EQ(SettingsManager::Get().cloneOffset, glm::vec3(1.0f, 0.0f, -1.0f));
}

// --- Negative and Edge Case Tests ---

TEST_F(SettingsManagerTest, LoadNonExistentFile) {
    // NEGATIVE: Attempting to load a file that doesn't exist should fail but not crash.
    ASSERT_FALSE(SettingsManager::Load("non_existent_file.json"));
    EXPECT_EQ(SettingsManager::Get().leftPaneWidth, 200.0f); // Should remain default
}

TEST_F(SettingsManagerTest, HandlesCorruptedJson) {
    // NEGATIVE: A file with invalid JSON syntax should be handled gracefully.
    std::ofstream ofs("test_settings.json");
    ofs << "{ \"leftPaneWidth\": 250.0, "; // Missing closing brace
    ofs.close();
    ASSERT_FALSE(SettingsManager::Load("test_settings.json")); // Load should fail
    EXPECT_EQ(SettingsManager::Get().leftPaneWidth, 200.0f); // Should remain default
}

TEST_F(SettingsManagerTest, HandlesMissingKeys) {
    // EDGE CASE: A valid JSON file that's missing a key should not overwrite that setting.
    std::ofstream ofs("test_settings.json");
    ofs << "{ \"rightPaneWidth\": 500.0 }"; // Only contains one key
    ofs.close();
    ASSERT_TRUE(SettingsManager::Load("test_settings.json"));
    EXPECT_EQ(SettingsManager::Get().leftPaneWidth, 200.0f); // Should remain default
    EXPECT_EQ(SettingsManager::Get().rightPaneWidth, 500.0f); // Should be updated
}

TEST_F(SettingsManagerTest, HandlesWrongDataType) {
    // NEGATIVE: A JSON file with a key of the wrong type should be handled gracefully.
    std::ofstream ofs("test_settings.json");
    ofs << "{ \"leftPaneWidth\": \"this is not a float\" }";
    ofs.close();
    // The JSON library will throw an exception, which our Load function should catch.
    ASSERT_FALSE(SettingsManager::Load("test_settings.json"));
    EXPECT_EQ(SettingsManager::Get().leftPaneWidth, 200.0f); // Should remain default
}