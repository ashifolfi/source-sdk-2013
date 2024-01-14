#include "testingtool.h"
#include "tier0/icommandline.h"
#include "tier1/tier1.h"
#include "imgui_system.h"

const char *TestingTool::GetToolName() {
    return "Testing Tool";
}

bool TestingTool::Init() {
    Msg("Initing tool");
    return true;
}

bool TestingTool::ClientInit( CreateInterfaceFn clientFactory )
{


    return true;
}

void TestingTool::Shutdown() {
    Msg("Shutting down tool");
}

void TestingTool::OnToolActivate() {
    Msg("Activating tool");
}

void TestingTool::OnToolDeactivate() {
    Msg("Deactivating tool");
}
