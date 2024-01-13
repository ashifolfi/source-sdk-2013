#include "testingtool.h"

char const TestingTool::GetToolName() {
    return "Testing Tool";
}

bool TestingTool::Init() {
    Msg("Initing testing tool!");
    return true;
}

void TestingTool::Shutdown() {
    Msg("Shutting down testing tool!");
}
