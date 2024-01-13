// Why do we do this? idk. valve does so we should as well
#ifndef TESTTOOL_H
#define TESTTOOL_H
#ifdef _WIN32
#pragma once
#endif

#include "toolframework/itoolsystem.h"

class TestingTool : public IToolSystem {
public:
    override char const *GetToolName(); // Name describing the tool
    override bool Init(); // Called at end of engine startup
    override void Shutdown(); // Called during RemoveTool or when engine is shutting down.
}

#endif // TESTTOOL_H