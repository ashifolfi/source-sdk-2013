// Why do we do this? idk. valve does so we should as well
#ifndef TESTTOOL_H
#define TESTTOOL_H
#ifdef _WIN32
#pragma once
#endif

#include "toolframework/itoolsystem.h"
#include "imgui_window.h"

class TestingTool : public IToolSystem
{
public:
	virtual char const *GetToolName(); // Name describing the tool

	virtual bool Init(); // Called at end of engine startup
	virtual void Shutdown(); // Called during RemoveTool or when engine is shutting down.

	// Called after client.dll is loaded
	virtual bool	ClientInit( CreateInterfaceFn clientFactory );

	virtual void OnToolActivate();
	virtual void OnToolDeactivate();

	#pragma region Stubbed Functions
	// Called after server.dll is loaded
	virtual bool	ServerInit( CreateInterfaceFn serverFactory ) { return true; }

	virtual void	ServerShutdown() {}
	virtual void	ClientShutdown() {}

	// Allow tool to override quitting, called before Shutdown(), return no to abort quitting
	virtual bool	CanQuit() { return true; }

	// Called when another system wiches to post a message to the tool and/or a specific entity
	// FIXME:  Are KeyValues too inefficient here?
	virtual void	PostMessage( HTOOLHANDLE hEntity, KeyValues *message ) {}

	// Called oncer per frame even when no level is loaded... (call ProcessMessages())
	virtual void	Think( bool finalTick ) {}

	// Server calls:

		// Level init, shutdown
	virtual void	ServerLevelInitPreEntity() {}
	// entities are created / spawned / precached here
	virtual void	ServerLevelInitPostEntity() {}

	virtual void	ServerLevelShutdownPreEntity() {}
	// Entities are deleted / released here...
	virtual void	ServerLevelShutdownPostEntity() {}
	// end of level shutdown

	// Called each frame before entities think
	virtual void	ServerFrameUpdatePreEntityThink() {}
	// called after entities think
	virtual void	ServerFrameUpdatePostEntityThink() {}
	virtual void	ServerPreClientUpdate() {}
	virtual void	ServerPreSetupVisibility() {}

	// Used to allow the tool to spawn different entities when it's active
	virtual const char *GetEntityData( const char *pActualEntityData ) { return pActualEntityData; }

	// Client calls:
			// Level init, shutdown
	virtual void	ClientLevelInitPreEntity() {}
	// entities are created / spawned / precached here
	virtual void	ClientLevelInitPostEntity() {}

	virtual void	ClientLevelShutdownPreEntity() {}
	// Entities are deleted / released here...
	virtual void	ClientLevelShutdownPostEntity() {}
	// end of level shutdown
	// Called before rendering
	virtual void	ClientPreRender() {}
	virtual void	ClientPostRender() {}

	// Let tool override viewport for engine
	virtual void	AdjustEngineViewport( int &x, int &y, int &width, int &height ) { }

	// let tool override view/camera
	virtual bool	SetupEngineView( Vector &origin, QAngle &angles, float &fov ) { return false; }

	// let tool override microphone
	virtual bool	SetupAudioState( AudioState_t &audioState ) { return false; }

	// Should the client be allowed to render the view normally?
	virtual bool	ShouldGameRenderView() { return true; }
	virtual bool	IsThirdPersonCamera() { return false; }

	// is the current tool recording?
	virtual bool	IsToolRecording() { return false; }

	virtual IMaterialProxy *LookupProxy( const char *proxyName ) { return NULL; }

	virtual bool	TrapKey( ButtonCode_t key, bool down ) { return true; }

	virtual bool	GetSoundSpatialization( int iUserData, int guid, SpatializationInfo_t &info ) { return false; }

	// Unlike the client .dll pre/post render stuff, these get called no matter whether a map is loaded and they only get called once per frame!!!
	virtual void		RenderFrameBegin() {}
	virtual void		RenderFrameEnd() {}

	// wraps the entire frame - surrounding all other begin/end and pre/post calls
	virtual void		HostRunFrameBegin() {}
	virtual void		HostRunFrameEnd() {}

	// See enginevgui.h for paintmode_t enum definitions
	virtual void		VGui_PreRender( int paintMode ) {}
	virtual void		VGui_PostRender( int paintMode ) {}

	virtual void		VGui_PreSimulate() {}
	virtual void		VGui_PostSimulate() {}
	#pragma endregion

private:

};

#endif // TESTTOOL_H
