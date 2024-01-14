#include "tooldictionary.h"

bool CToolDictionary::Connect( CreateInterfaceFn factory )
{
	return true;
}

void CToolDictionary::Disconnect() {}

void *CToolDictionary::QueryInterface( const char *pInterfaceName )
{
	CreateInterfaceFn factory = Sys_GetFactoryThis();
	return factory( pInterfaceName, NULL );
}

InitReturnVal_t CToolDictionary::Init()
{
	return INIT_OK;
}

void CToolDictionary::Shutdown() {}

void CToolDictionary::CreateTools() {} // do nothing

int	CToolDictionary::GetToolCount() const
{
	return 1;
}

IToolSystem *CToolDictionary::GetTool( int index )
{
	if ( index == 0 )
	{
		return &m_testTool;
	}
	else
	{
		return NULL;
	}
}
