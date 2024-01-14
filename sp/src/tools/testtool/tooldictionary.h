#ifndef TOOLDICTIONARY_H
#define TOOLDICTIONARY_H

#ifdef _WIN32
#pragma once
#endif

#include "toolframework/itooldictionary.h"
#include "tier0/icommandline.h"
#include "tier1/tier1.h"
#include "interface.h"

#include "testingtool.h"

// implementation of a tool dictionary
class CToolDictionary : public IToolDictionary
{
public:
	// Methods of IAppSystem
	virtual bool	Connect( CreateInterfaceFn factory );
	virtual void	Disconnect();

	// Returns null if it doesn't implement the requested interface
	virtual void *QueryInterface( const char *pInterfaceName );

	virtual InitReturnVal_t Init();
	virtual void	Shutdown();

public:
	virtual void CreateTools();
	virtual int GetToolCount() const;
	virtual IToolSystem *GetTool(int index);

private:
	TestingTool m_testTool;
};

static CToolDictionary g_ToolDictionary;
// Expose to engine
EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CToolDictionary, IToolDictionary, VTOOLDICTIONARY_INTERFACE_VERSION, g_ToolDictionary);

#endif // TOOLDICTIONARY_H
