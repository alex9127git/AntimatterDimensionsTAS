#include <nlohmann/json.hpp>
#include "gameinstance.h"
#include "instructions.h"
#include <gtest/gtest.h>
#include "simulation.h"
#include "testHelper.h"
#include <algorithm>
#include <vector>
#include <cmath>
#include <omp.h>
using json = nlohmann::json;
using namespace std;


TEST(InstructionTest, FirstDimboostRoute) {
    GameInstance instance = GameInstance();
    instance.addInstructions({
        11, 130, 19, 20, 91, 32, 91, 38, 10, 91, 41, 91, 49, 91, 20, 91, 10, 91, 30, 91, 20, 10, 92, 40, 100
    });
    runAllQueuedInstructions(instance);
    EXPECT_EQ(instance.game().dimensionBoosts(), 1);
    EXPECT_EQ(instance.game().realTimePlayed(), 1080816);
}

TEST(InstructionTest, SixthDimboostRoute) {
    json save = {
        {"achievementState", {
            {"unlockedAchievements", {11,12,13,14,15,16,17,18,22,35,44,76}},
        }},
        {"canUseKonami", false},
        {"dimensionBoosts", 5},
        {"realTimePlayed", 0}
    };
    GameInstance instance = GameInstance(save);
    instance.addInstructions({
        10, 20, 91, 31, 91, 39, 10, 92, 40, 91, 20, 91, 10, 91, 50, 91, 30, 91, 20, 10, 92, 40,
        60, 91, 10, 91, 30, 20, 92, 10, 91, 70, 50, 91, 40, 20, 91, 30, 10, 92, 20, 91, 10, 91, 60,
        80, 108, log10(3.588), 91, 40, 30, 91, 50, 10, 91, 20, 92, 10, 91, 30, 91, 70, 40, 20, 91, 10,
        91, 60, 91, 50, 91, 30, 20, 10, 91, 40, 92, 10, 20, 92, 80, 108, log10(3.588 * 2.842), 30, 91, 10, 91,
        50, 91, 70, 20, 40, 91, 60, 10, 91, 30, 92, 20, 10, 92, 40, 91, 50, 10, 30, 91, 20, 92,
        10, 91, 60, 80, 91, 70, 40, 30, 20, 91, 10, 92, 50, 91, 20, 10, 91, 30, 91, 40, 91, 10,
        91, 20, 91, 60, 91, 30, 10, 50, 91, 40, 91, 20, 70, 91, 10, 85, 100
    });
    runAllQueuedInstructions(instance);
    EXPECT_EQ(instance.game().dimensionBoosts(), 6);
    EXPECT_EQ(instance.game().realTimePlayed(), 1239513);
}

TEST(RouteRetracing, CanRetracePurchases) {
    json save = readJson(TEST_FILES_DIR "/01_purchaseTest/in.json");
    GameInstance instance = GameInstance(save);
    json cmd = readJson(TEST_FILES_DIR "/01_purchaseTest/cmd.json");
    vector<double> instructions = parseCommands(cmd);
    instance.addInstructions(instructions);
    runAllQueuedInstructions(instance);
    json saveExpected = readJson(TEST_FILES_DIR "/01_purchaseTest/out.json");
    json saveActual = instance.game().to_json();
    EXPECT_EQ(saveExpected, restrictToKeysOf(saveActual, saveExpected));
}

TEST(RouteRetracing, CanRetracePurchasesAndSacrifices) {
    json save = readJson(TEST_FILES_DIR "/02_purchAndSacTest/in.json");
    GameInstance instance = GameInstance(save);
    json cmd = readJson(TEST_FILES_DIR "/02_purchAndSacTest/cmd.json");
    vector<double> instructions = parseCommands(cmd);
    instance.addInstructions(instructions);
    runAllQueuedInstructions(instance);
    json saveExpected = readJson(TEST_FILES_DIR "/02_purchAndSacTest/out.json");
    json saveActual = instance.game().to_json();
    EXPECT_EQ(saveExpected, restrictToKeysOf(saveActual, saveExpected));
}

TEST(IntegrationTesting, FirstDimboostSearchAndRetraceTest) {
    omp_set_num_threads(max(1, omp_get_num_procs() / 2));
    GameState st = GameState();
    GameInstance actual = runDimboost(st, true, false);
    json saveExpected = readJson(TEST_FILES_DIR "/03_simulationTest/out.json");
    json saveActual = actual.game().to_json();
    EXPECT_EQ(saveExpected, restrictToKeysOf(saveActual, saveExpected));
}

TEST(IntegrationTesting, ComplexSearchAndRetraceTest) {
    omp_set_num_threads(max(1, omp_get_num_procs() / 2));
    json save = readJson(TEST_FILES_DIR "/02_purchAndSacTest/in.json");
    GameState st = GameState(save);

    testing::internal::CaptureStdout();
    GameInstance actual = runDimboost(st, true, false);
    string output = testing::internal::GetCapturedStdout();

    json saveExpected = readJson(TEST_FILES_DIR "/02_purchAndSacTest/out.json");
    json saveActual = actual.game().to_json();
    EXPECT_EQ(saveExpected, restrictToKeysOf(saveActual, saveExpected));

    int count = 0;
    size_t pos = 0;
    while ((pos = output.find("Running iteration", pos)) != string::npos) {
        count++;
        pos++;
    }
    EXPECT_EQ(count, 21);
}
