#include "Core/SettingsManager.h"
#include "gtest/gtest.h"
#include <fstream>

class SettingsManagerTest : public ::testing::Test {
protected:
    void TearDown() override {
        std::remove("test_settings.json");
    }
};

TEST_F(SettingsManagerTest, DefaultsAreCorrect) {
    AppSettings& settings = SettingsManager::Get();
    EXPECT_EQ(settings.leftPaneWidth, 200.0f);
    EXPECT_EQ(settings.rightPaneWidth, 300.0f);
    EXPECT_EQ(settings.cloneOffset, glm::vec3(0.5f, 0.5f, 0.0f));
}

TEST_F(SettingsManagerTest, SaveAndLoad) {
    SettingsManager::Get().leftPaneWidth = 250.0f;
    SettingsManager::Get().cloneOffset = glm::vec3(1.0f, 0.0f, 1.0f);

    ASSERT_TRUE(SettingsManager::Save("test_settings.json"));
    
    // Reset to defaults before loading to ensure it's not just holding the old values
    SettingsManager::Get().leftPaneWidth = 200.0f; 
    
    ASSERT_TRUE(SettingsManager::Load("test_settings.json"));

    EXPECT_EQ(SettingsManager::Get().leftPaneWidth, 250.0f);
    EXPECT_EQ(SettingsManager::Get().cloneOffset, glm::vec3(1.0f, 0.0f, 1.0f));
}