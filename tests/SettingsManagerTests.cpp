#include "Core/SettingsManager.h"
#include "gtest/gtest.h"
#include <fstream>
#include <cstdio>
#include <glm/glm.hpp> // Needed for glm::vec3 comparisons

class SettingsManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Reset settings to a known state for each test
        AppSettings& settings = SettingsManager::Get();
        settings.leftPaneWidth = 200.0f;
        settings.rightPaneWidth = 300.0f;
        settings.cloneOffset = {0.5f, 0.5f, 0.0f};
        settings.objImportScale = 1.0f;
        settings.gridSize = 80;
        settings.gridDivisions = 80;
        settings.cameraSpeed = 5.0f;
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
    EXPECT_EQ(settings.objImportScale, 1.0f);
    EXPECT_EQ(settings.gridSize, 80);
    EXPECT_EQ(settings.gridDivisions, 80);
    EXPECT_EQ(settings.cameraSpeed, 5.0f);
}

TEST_F(SettingsManagerTest, SaveAndLoad) {
    SettingsManager::Get().leftPaneWidth = 250.5f;
    SettingsManager::Get().cloneOffset = glm::vec3(1.0f, 0.0f, -1.0f);
    SettingsManager::Get().gridSize = 100;
    SettingsManager::Get().cameraSpeed = 10.0f;
    
    ASSERT_TRUE(SettingsManager::Save("test_settings.json"));
    
    SetUp(); // Reset to defaults
    
    ASSERT_TRUE(SettingsManager::Load("test_settings.json"));
    EXPECT_FLOAT_EQ(SettingsManager::Get().leftPaneWidth, 250.5f);
    EXPECT_EQ(SettingsManager::Get().cloneOffset, glm::vec3(1.0f, 0.0f, -1.0f));
    EXPECT_EQ(SettingsManager::Get().gridSize, 100);
    EXPECT_FLOAT_EQ(SettingsManager::Get().cameraSpeed, 10.0f);
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

TEST_F(SettingsManagerTest, GetDescriptors_AreCorrect) {
    const auto& descriptors = SettingsManager::GetDescriptors();
    AppSettings& settings = SettingsManager::Get();

    // Verify count (adjust if more settings are added/removed)
    EXPECT_EQ(descriptors.size(), 7); 

    // Test specific descriptors
    bool foundCloneOffset = false;
    bool foundLeftPaneWidth = false;
    bool foundGridSize = false;
    
    for (const auto& desc : descriptors) {
        if (desc.key == "cloneOffset") {
            foundCloneOffset = true;
            EXPECT_EQ(desc.label, "Clone Offset");
            EXPECT_EQ(desc.type, SettingType::Float3);
            EXPECT_EQ(static_cast<glm::vec3*>(desc.ptr), &settings.cloneOffset);
        } else if (desc.key == "leftPaneWidth") {
            foundLeftPaneWidth = true;
            EXPECT_EQ(desc.label, "Left Pane Width");
            EXPECT_EQ(desc.type, SettingType::Float);
            EXPECT_EQ(static_cast<float*>(desc.ptr), &settings.leftPaneWidth);
        } else if (desc.key == "gridSize") {
            foundGridSize = true;
            EXPECT_EQ(desc.label, "Grid Size");
            EXPECT_EQ(desc.type, SettingType::Int);
            EXPECT_EQ(static_cast<int*>(desc.ptr), &settings.gridSize);
        }
        // Add checks for other settings as needed
    }

    EXPECT_TRUE(foundCloneOffset) << "Descriptor for 'cloneOffset' not found or incorrect.";
    EXPECT_TRUE(foundLeftPaneWidth) << "Descriptor for 'leftPaneWidth' not found or incorrect.";
    EXPECT_TRUE(foundGridSize) << "Descriptor for 'gridSize' not found or incorrect.";
}