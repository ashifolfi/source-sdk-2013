//
// Created by ENDERZOMBI102 on 22/02/2024.
//
#include "platform.h"

#if IsWindows()
	#include <direct.h>
#endif
#include "../tier0/commandline.hpp"
#include "filesystem.hpp"
#include "interface.h"
#include "system/packsystemclient.hpp"
#include "system/plainsystemclient.hpp"

static CFileSystemStdio g_FullFileSystem{};

// ---------------
// AppSystem
// ---------------
bool CFileSystemStdio::Connect( CreateInterfaceFn factory ) {
	return true;
}
void CFileSystemStdio::Disconnect() { }
void* CFileSystemStdio::QueryInterface( const char* pInterfaceName ) {
	if ( strcmp( pInterfaceName, FILESYSTEM_INTERFACE_VERSION ) == 0 )
		return &g_FullFileSystem;

	return nullptr;
}
InitReturnVal_t CFileSystemStdio::Init() {
	if ( this->m_bInitialized )
		return InitReturnVal_t::INIT_OK;

	return InitReturnVal_t::INIT_OK;
}
void CFileSystemStdio::Shutdown() { }

// ---------------
// Helpers
// ---------------
auto CFileSystemStdio::findClientHelper( FileHandle_t file ) -> std::optional<std::weak_ptr<ISystemClient>> {
	auto maybeIndex{ this->m_HandleClientsMap.find( file ) };
	if ( maybeIndex == this->m_HandleClientsMap.end() )
		return std::nullopt;

	auto index{ maybeIndex->second };
	if ( not this->m_Clients.IsValidIndex( index ) )
		return std::nullopt;

	auto client{ this->m_Clients[ index ] };
	if ( client.expired() )
		return std::nullopt;

	return client;
}

// ---------------
// IBaseFilesystem
// ---------------
int CFileSystemStdio::Read( void* pOutput, int size, FileHandle_t file ) {
	auto client{ this->findClientHelper( file ) };

	if (! client )
		return -1;

	return static_cast<int>( client->lock()->Read( file, pOutput, size ) );
}
int CFileSystemStdio::Write( const void* pInput, int size, FileHandle_t file ) {
	auto client{ this->findClientHelper( file ) };

	if (! client )
		return -1;

	return static_cast<int>( client->lock()->Write( file, pInput, size ) );
}

FileHandle_t CFileSystemStdio::Open( const char* pFileName, const char* pOptions, const char* pathID ) {
	// if we got a pathID, only look into that SearchPath
	if ( pathID != nullptr ) {
		AssertMsg( this->m_SearchPaths.contains( pathID ), "Was given pathID not loaded" );

		for ( const auto& system : this->m_SearchPaths[pathID].m_Clients ) {
			auto handle{ system->Open( pFileName, pOptions ) };
			if ( handle != nullptr ) {
				// this seems too complex for what it does...
				auto found{ false };
				for ( auto index{0}; index < this->m_Clients.Count(); index += 1 ) {
					if ( system == this->m_Clients[index].lock() ) {
						this->m_HandleClientsMap[handle] = index;
						found = true;
						break;
					}
				}
				AssertMsg( found, "`ISystemClient` impl for open handle was not found!!" );
				return handle;
			}
		}
	} else {
		// else, look into all clients
		for ( auto index{0}; index < this->m_Clients.Count(); index += 1 ) {
			auto handle{ this->m_Clients[index].lock()->Open( pFileName, pOptions ) };
			if ( handle != nullptr ) {
				this->m_HandleClientsMap[handle] = index;
				return handle;
			}
		}
	}

	AssertMsg( false, "Open: %s, %s, %s", pFileName, pOptions, pathID ); return {};
}
void CFileSystemStdio::Close( FileHandle_t file ) {
	auto client{ this->findClientHelper( file ) };

	if ( client )
		client->lock()->Close( file );
}

void CFileSystemStdio::Seek( FileHandle_t file, int pos, FileSystemSeek_t seekType ) {
	auto client{ this->findClientHelper( file ) };

	/*
	 * Seek can be implemented virtually, as both `Read` and `Write`
	 * get offsets starting from the 0th position in the file, we
	 * truly need a struct to handle opened files and their metadata.
	 */
	if ( client )
		client->lock()->( file, pos, seekType );
}
unsigned int CFileSystemStdio::Tell( FileHandle_t file ) { AssertUnreachable(); return {}; }
unsigned int CFileSystemStdio::Size( FileHandle_t file ) { AssertUnreachable(); return {}; }
unsigned int CFileSystemStdio::Size( const char* pFileName, const char* pPathID ) { AssertUnreachable(); return {}; }

void CFileSystemStdio::Flush( FileHandle_t file ) { AssertUnreachable(); }
bool CFileSystemStdio::Precache( const char* pFileName, const char* pPathID ) { AssertUnreachable(); return {}; }

bool CFileSystemStdio::FileExists( const char* pFileName, const char* pPathID ) { AssertUnreachable(); return {}; }
bool CFileSystemStdio::IsFileWritable( char const* pFileName, const char* pPathID ) { AssertUnreachable(); return {}; }
bool CFileSystemStdio::SetFileWritable( char const* pFileName, bool writable, const char* pPathID ) { AssertUnreachable(); return {}; }

long CFileSystemStdio::GetFileTime( const char* pFileName, const char* pPathID ) { AssertUnreachable(); return {}; }

bool CFileSystemStdio::ReadFile( const char* pFileName, const char* pPath, CUtlBuffer& buf, int nMaxBytes, int nStartingByte, FSAllocFunc_t pfnAlloc ) { AssertUnreachable(); return {}; }
bool CFileSystemStdio::WriteFile( const char* pFileName, const char* pPath, CUtlBuffer& buf ) { AssertUnreachable(); return {}; }
bool CFileSystemStdio::UnzipFile( const char* pFileName, const char* pPath, const char* pDestination ) { AssertUnreachable(); return {}; }


// ---------------
// IFileSystem
// ---------------
// ---- Steam operations ----
bool CFileSystemStdio::IsSteam() const {
	return false;
}

FilesystemMountRetval_t CFileSystemStdio::MountSteamContent( int nExtraAppId ) {
	return FilesystemMountRetval_t::FILESYSTEM_MOUNT_FAILED;
}


// ---- Search path manipulation ----
void CFileSystemStdio::AddSearchPath( const char* pPath, const char* pathID, SearchPathAdd_t addType ) {
	AssertFatalMsg( pPath, "Was given an empty path!!" );

	this->m_iLastId += 1;

	// calculate base dir (current `-game` dir)
	char absolute[1024];
	if ( !_getcwd( absolute, sizeof(absolute) ) )
		Error( "V_MakeAbsolutePath: _getcwd failed." );
	V_ComposeFileName( absolute, CommandLine()->ParmValue( "-game", "" ), absolute, sizeof(absolute) );

	// make absolute path of the thing
	V_MakeAbsolutePath( absolute, 1024, pPath, absolute );

	

	// try all possibilities
	auto system{ CPlainSystemClient::Open( this->m_iLastId, absolute, pPath ) };
	system = system ? system : CPackSystemClient::Open( this->m_iLastId, absolute, pPath );
	AssertFatalMsg( system, "Unsupported path entry: %s", absolute );

	if (! this->m_SearchPaths.contains( pathID ) ) {
		this->m_SearchPaths[ pathID ] = SearchPath{};
	}

	if ( addType == SearchPathAdd_t::PATH_ADD_TO_HEAD ) {
		this->m_SearchPaths[ pathID ].m_Clients.AddToHead( system );
	} else {
		this->m_SearchPaths[ pathID ].m_Clients.AddToTail( system );
	}
}
bool CFileSystemStdio::RemoveSearchPath( const char* pPath, const char* pathID ) {
	if (! this->m_SearchPaths.contains( pathID ) )
		return false;

	auto& systems{ this->m_SearchPaths[pathID].m_Clients };
	for ( int i{0}; i < systems.Count(); i += 1 ) {
		if ( V_strcmp( systems[i]->GetNativePath(), pPath ) == 0 ) {
			systems[i]->Shutdown();
			systems.Remove( i );
			return true;
		}
	}
	return false;
}

void CFileSystemStdio::RemoveAllSearchPaths() {
	for ( auto& [pathId, searchPath] : this->m_SearchPaths ) {
		for ( auto& system : searchPath.m_Clients ) {
			system->Shutdown();
		}
		searchPath.m_Clients.Purge();
	}
	this->m_HandleClientsMap.clear();
	this->m_SearchPaths.clear();
}

void CFileSystemStdio::RemoveSearchPaths( const char* szPathID ) {
	for ( auto& [pathId, searchPath] : this->m_SearchPaths ) {
		if ( strcmp( pathId, szPathID ) == 0 ) {
			// first shutdown and remove all systems
			this->m_SearchPaths[ szPathID ];
			// then remove the path itself
			this->m_SearchPaths.erase( szPathID );
			break;
		}
	}
}

void CFileSystemStdio::MarkPathIDByRequestOnly( const char* pPathID, bool bRequestOnly ) {
	if ( this->m_SearchPaths.contains( pPathID ) )
		this->m_SearchPaths[pPathID].m_bRequestOnly = bRequestOnly;
}

const char* CFileSystemStdio::RelativePathToFullPath( const char* pFileName, const char* pPathID, char* pDest, int maxLenInChars, PathTypeFilter_t pathFilter, PathTypeQuery_t* pPathType ) { AssertUnreachable(); return {}; }

int CFileSystemStdio::GetSearchPath( const char* pathID, bool bGetPackFiles, char* pDest, int maxLenInChars ) { AssertUnreachable(); return {}; }

bool CFileSystemStdio::AddPackFile( const char* fullpath, const char* pathID ) { AssertUnreachable(); return {}; }

// ---- File manipulation operations ----
void CFileSystemStdio::RemoveFile( char const* pRelativePath, const char* pathID ) { AssertUnreachable(); }

bool CFileSystemStdio::RenameFile( char const* pOldPath, char const* pNewPath, const char* pathID ) { AssertUnreachable(); return {}; }

void CFileSystemStdio::CreateDirHierarchy( const char* path, const char* pathID ) { AssertUnreachable(); }

bool CFileSystemStdio::IsDirectory( const char* pFileName, const char* pathID ) { AssertUnreachable(); return {}; }

void CFileSystemStdio::FileTimeToString( char* pStrip, int maxCharsIncludingTerminator, long fileTime ) { AssertUnreachable(); }

// ---- Open file operations ----
void CFileSystemStdio::SetBufferSize( FileHandle_t file, unsigned nBytes ) { AssertUnreachable(); }

bool CFileSystemStdio::IsOk( FileHandle_t file ) { AssertUnreachable(); return {}; }

bool CFileSystemStdio::EndOfFile( FileHandle_t file ) { AssertUnreachable(); return {}; }

char* CFileSystemStdio::ReadLine( char* pOutput, int maxChars, FileHandle_t file ) { AssertUnreachable(); return {}; }
int CFileSystemStdio::FPrintf( FileHandle_t file, PRINTF_FORMAT_STRING const char* pFormat, ... ) { AssertUnreachable(); return {}; }

// ---- Dynamic library operations ----
CSysModule* CFileSystemStdio::LoadModule( const char* pFileName, const char* pPathID, bool bValidatedDllOnly ) { AssertUnreachable(); return {}; }
void CFileSystemStdio::UnloadModule( CSysModule * pModule ) { AssertUnreachable(); }

// ---- File searching operations -----
const char* CFileSystemStdio::FindFirst( const char* pWildCard, FileFindHandle_t* pHandle ) { AssertMsg( false, "FindFirst: %s, %p", pWildCard, pHandle ); return {}; }
const char* CFileSystemStdio::FindNext( FileFindHandle_t handle ) { AssertUnreachable(); return {}; }
bool CFileSystemStdio::FindIsDirectory( FileFindHandle_t handle ) { AssertUnreachable(); return {}; }
void CFileSystemStdio::FindClose( FileFindHandle_t handle ) { AssertUnreachable(); }

const char* CFileSystemStdio::FindFirstEx( const char* pWildCard, const char* pPathID, FileFindHandle_t* pHandle ) { AssertUnreachable(); return {}; }

// ---- File name and directory operations ----
const char* CFileSystemStdio::GetLocalPath( const char* pFileName, char* pDest, int maxLenInChars ) { AssertUnreachable(); return {}; }

bool CFileSystemStdio::FullPathToRelativePath( const char* pFullpath, char* pDest, int maxLenInChars ) { AssertUnreachable(); return {}; }

bool CFileSystemStdio::GetCurrentDirectory( char* pDirectory, int maxlen ) {
    #if IsWindows()
        _getcwd(pDirectory, maxlen);
    #else
    	getcwd( pDirectory, maxlen );
    #endif
	return true;
}

// ---- Filename dictionary operations ----
FileNameHandle_t CFileSystemStdio::FindOrAddFileName( char const* pFileName ) { AssertUnreachable(); return {}; }
bool CFileSystemStdio::String( const FileNameHandle_t& handle, char* buf, int buflen ) { AssertUnreachable(); return {}; }

// ---- Global Asynchronous file operations ----
FSAsyncStatus_t CFileSystemStdio::AsyncReadMultiple( const FileAsyncRequest_t* pRequests, int nRequests, FSAsyncControl_t* phControls ) { AssertUnreachable(); return {}; }
FSAsyncStatus_t CFileSystemStdio::AsyncAppend( const char* pFileName, const void* pSrc, int nSrcBytes, bool bFreeMemory, FSAsyncControl_t* pControl ) { AssertUnreachable(); return {}; }
FSAsyncStatus_t CFileSystemStdio::AsyncAppendFile( const char* pAppendToFileName, const char* pAppendFromFileName, FSAsyncControl_t* pControl ) { AssertUnreachable(); return {}; }
void CFileSystemStdio::AsyncFinishAll( int iToPriority ) { AssertUnreachable(); }
void CFileSystemStdio::AsyncFinishAllWrites() { AssertUnreachable(); }
FSAsyncStatus_t CFileSystemStdio::AsyncFlush() { AssertUnreachable(); return {}; }
bool CFileSystemStdio::AsyncSuspend() { AssertUnreachable(); return {}; }
bool CFileSystemStdio::AsyncResume() { AssertUnreachable(); return {}; }

void CFileSystemStdio::AsyncAddFetcher( IAsyncFileFetch * pFetcher ) { AssertUnreachable(); }
void CFileSystemStdio::AsyncRemoveFetcher( IAsyncFileFetch * pFetcher ) { AssertUnreachable(); }

FSAsyncStatus_t CFileSystemStdio::AsyncBeginRead( const char* pszFile, FSAsyncFile_t* phFile ) { AssertUnreachable(); return {}; }
FSAsyncStatus_t CFileSystemStdio::AsyncEndRead( FSAsyncFile_t hFile ) { AssertUnreachable(); return {}; }

// ---- Asynchronous Request management ----
FSAsyncStatus_t CFileSystemStdio::AsyncFinish( FSAsyncControl_t hControl, bool wait ) { AssertUnreachable(); return {}; }
FSAsyncStatus_t CFileSystemStdio::AsyncGetResult( FSAsyncControl_t hControl, void** ppData, int* pSize ) { AssertUnreachable(); return {}; }
FSAsyncStatus_t CFileSystemStdio::AsyncAbort( FSAsyncControl_t hControl ) { AssertUnreachable(); return {}; }
FSAsyncStatus_t CFileSystemStdio::AsyncStatus( FSAsyncControl_t hControl ) { AssertUnreachable(); return {}; }
FSAsyncStatus_t CFileSystemStdio::AsyncSetPriority( FSAsyncControl_t hControl, int newPriority ) { AssertUnreachable(); return {}; }
void CFileSystemStdio::AsyncAddRef( FSAsyncControl_t hControl ) { AssertUnreachable(); }
void CFileSystemStdio::AsyncRelease( FSAsyncControl_t hControl ) { AssertUnreachable(); }

// ---- Remote resource management ----
WaitForResourcesHandle_t CFileSystemStdio::WaitForResources( const char* resourcelist ) { AssertUnreachable(); return {}; }
bool CFileSystemStdio::GetWaitForResourcesProgress( WaitForResourcesHandle_t handle, float* progress, bool* complete ) { AssertUnreachable(); return {}; }
void CFileSystemStdio::CancelWaitForResources( WaitForResourcesHandle_t handle ) { AssertUnreachable(); }

int CFileSystemStdio::HintResourceNeed( const char* hintlist, int forgetEverything ) { AssertUnreachable(); return {}; }
bool CFileSystemStdio::IsFileImmediatelyAvailable( const char* pFileName ) { AssertUnreachable(); return {}; }

void CFileSystemStdio::GetLocalCopy( const char* pFileName ) { AssertUnreachable(); }

// ---- Debugging operations ----
void CFileSystemStdio::PrintOpenedFiles() { AssertUnreachable(); }
void CFileSystemStdio::PrintSearchPaths() {
	Log( "---- Search Path table ----\n" );
	for ( const auto& [searchPathId, searchPath] : this->m_SearchPaths ) {
		Log( "%s:\n", searchPathId );
		for ( const auto& path : searchPath.m_Clients ) {
			Log( "  - %s\n", path->GetNativePath() );
		}
	}
}

void CFileSystemStdio::SetWarningFunc( void ( *pfnWarning )( PRINTF_FORMAT_STRING const char* fmt, ... ) ) { AssertUnreachable(); }
void CFileSystemStdio::SetWarningLevel( FileWarningLevel_t level ) { AssertUnreachable(); }
void CFileSystemStdio::AddLoggingFunc( void ( *pfnLogFunc )( const char* fileName, const char* accessType ) ) { AssertUnreachable(); }
void CFileSystemStdio::RemoveLoggingFunc( FileSystemLoggingFunc_t logFunc ) { AssertUnreachable(); }

const FileSystemStatistics* CFileSystemStdio::GetFilesystemStatistics() { AssertUnreachable(); return {}; }

// ---- Start of new functions after Lost Coast release (7/05) ----
FileHandle_t CFileSystemStdio::OpenEx( const char* pFileName, const char* pOptions, unsigned flags, const char* pathID, char** ppszResolvedFilename ) { AssertUnreachable(); return {}; }

int CFileSystemStdio::ReadEx( void* pOutput, int sizeDest, int size, FileHandle_t file ) { AssertUnreachable(); return {}; }
int CFileSystemStdio::ReadFileEx( const char* pFileName, const char* pPath, void** ppBuf, bool bNullTerminate, bool bOptimalAlloc, int nMaxBytes, int nStartingByte, FSAllocFunc_t pfnAlloc ) { AssertUnreachable(); return {}; }

FileNameHandle_t CFileSystemStdio::FindFileName( char const* pFileName ) { AssertUnreachable(); return {}; }

#if defined( TRACK_BLOCKING_IO )
	void CFileSystemStdio::EnableBlockingFileAccessTracking( bool state ) { AssertUnreachable(); }
	bool CFileSystemStdio::IsBlockingFileAccessEnabled() const { AssertUnreachable(); return {}; }

	IBlockingFileItemList* CFileSystemStdio::RetrieveBlockingFileAccessInfo() { AssertUnreachable(); return {}; }
#endif

void CFileSystemStdio::SetupPreloadData() { AssertUnreachable(); }
void CFileSystemStdio::DiscardPreloadData() { AssertUnreachable(); }

void CFileSystemStdio::LoadCompiledKeyValues( KeyValuesPreloadType_t type, char const* archiveFile ) { AssertUnreachable(); }

KeyValues* CFileSystemStdio::LoadKeyValues( KeyValuesPreloadType_t type, char const* filename, char const* pPathID ) { AssertUnreachable(); return {}; }
bool CFileSystemStdio::LoadKeyValues( KeyValues & head, KeyValuesPreloadType_t type, char const* filename, char const* pPathID ) { AssertUnreachable(); return {}; }
bool CFileSystemStdio::ExtractRootKeyName( KeyValuesPreloadType_t type, char* outbuf, size_t bufsize, char const* filename, char const* pPathID ) { AssertUnreachable(); return {}; }

FSAsyncStatus_t CFileSystemStdio::AsyncWrite( const char* pFileName, const void* pSrc, int nSrcBytes, bool bFreeMemory, bool bAppend, FSAsyncControl_t* pControl ) { AssertUnreachable(); return {}; }
FSAsyncStatus_t CFileSystemStdio::AsyncWriteFile( const char* pFileName, const CUtlBuffer* pSrc, int nSrcBytes, bool bFreeMemory, bool bAppend, FSAsyncControl_t* pControl ) { AssertUnreachable(); return {}; }
FSAsyncStatus_t CFileSystemStdio::AsyncReadMultipleCreditAlloc( const FileAsyncRequest_t* pRequests, int nRequests, const char* pszFile, int line, FSAsyncControl_t* phControls ) { AssertUnreachable(); return {}; }

bool CFileSystemStdio::GetFileTypeForFullPath( char const* pFullPath, wchar_t* buf, size_t bufSizeInBytes ) { AssertUnreachable(); return {}; }

bool CFileSystemStdio::ReadToBuffer( FileHandle_t hFile, CUtlBuffer& buf, int nMaxBytes, FSAllocFunc_t pfnAlloc ) { AssertUnreachable(); return {}; }

// ---- Optimal IO operations ----
bool CFileSystemStdio::GetOptimalIOConstraints( FileHandle_t hFile, unsigned* pOffsetAlign, unsigned* pSizeAlign, unsigned* pBufferAlign ) { AssertUnreachable(); return {}; }
inline unsigned CFileSystemStdio::GetOptimalReadSize( FileHandle_t hFile, unsigned nLogicalSize ) { AssertUnreachable(); return {}; }
void* CFileSystemStdio::AllocOptimalReadBuffer( FileHandle_t hFile, unsigned nSize, unsigned nOffset ) { AssertUnreachable(); return {}; }
void CFileSystemStdio::FreeOptimalReadBuffer( void* pBuffer ) { AssertUnreachable(); }

void CFileSystemStdio::BeginMapAccess() { AssertUnreachable(); }
void CFileSystemStdio::EndMapAccess() { AssertUnreachable(); }

bool CFileSystemStdio::FullPathToRelativePathEx( const char* pFullpath, const char* pPathId, char* pDest, int maxLenInChars ) { AssertUnreachable(); return {}; }

int CFileSystemStdio::GetPathIndex( const FileNameHandle_t& handle ) { AssertUnreachable(); return {}; }
long CFileSystemStdio::GetPathTime( const char* pPath, const char* pPathID ) { AssertUnreachable(); return {}; }

DVDMode_t CFileSystemStdio::GetDVDMode() {
	return DVDMode_t::DVDMODE_OFF;
}

// ---- Whitelisting for pure servers. ----
void CFileSystemStdio::EnableWhitelistFileTracking( bool bEnable, bool bCacheAllVPKHashes, bool bRecalculateAndCheckHashes ) { AssertUnreachable(); }

void CFileSystemStdio::RegisterFileWhitelist( IPureServerWhitelist * pWhiteList, IFileList * *pFilesToReload ) { AssertUnreachable(); }

void CFileSystemStdio::MarkAllCRCsUnverified() { AssertUnreachable(); }

void CFileSystemStdio::CacheFileCRCs( const char* pPathname, ECacheCRCType eType, IFileList* pFilter ) { AssertUnreachable(); }
EFileCRCStatus CFileSystemStdio::CheckCachedFileHash( const char* pPathID, const char* pRelativeFilename, int nFileFraction, FileHash_t* pFileHash ) { AssertUnreachable(); return {}; }

int CFileSystemStdio::GetUnverifiedFileHashes( CUnverifiedFileHash * pFiles, int nMaxFiles ) { AssertUnreachable(); return {}; }

int CFileSystemStdio::GetWhitelistSpewFlags() { AssertUnreachable(); return {}; }
void CFileSystemStdio::SetWhitelistSpewFlags( int flags ) { AssertUnreachable(); }

void CFileSystemStdio::InstallDirtyDiskReportFunc( FSDirtyDiskReportFunc_t func ) { AssertUnreachable(); }

// ---- Low-level file caching ----
FileCacheHandle_t CFileSystemStdio::CreateFileCache() { AssertUnreachable(); return {}; }
void CFileSystemStdio::AddFilesToFileCache( FileCacheHandle_t cacheId, const char** ppFileNames, int nFileNames, const char* pPathID ) { AssertUnreachable(); }
bool CFileSystemStdio::IsFileCacheFileLoaded( FileCacheHandle_t cacheId, const char* pFileName ) { AssertUnreachable(); return {}; }
bool CFileSystemStdio::IsFileCacheLoaded( FileCacheHandle_t cacheId ) { AssertUnreachable(); return {}; }
void CFileSystemStdio::DestroyFileCache( FileCacheHandle_t cacheId ) { AssertUnreachable(); }

bool CFileSystemStdio::RegisterMemoryFile( CMemoryFileBacking * pFile, CMemoryFileBacking * *ppExistingFileWithRef ) { AssertUnreachable(); return {}; }

void CFileSystemStdio::UnregisterMemoryFile( CMemoryFileBacking * pFile ) { AssertUnreachable(); }

void CFileSystemStdio::CacheAllVPKFileHashes( bool bCacheAllVPKHashes, bool bRecalculateAndCheckHashes ) { AssertUnreachable(); }
bool CFileSystemStdio::CheckVPKFileHash( int PackFileID, int nPackFileNumber, int nFileFraction, MD5Value_t& md5Value ) { AssertUnreachable(); return {}; }

void CFileSystemStdio::NotifyFileUnloaded( const char* pszFilename, const char* pPathId ) { AssertUnreachable(); }

bool CFileSystemStdio::GetCaseCorrectFullPath_Ptr( const char* pFullPath, char* pDest, int maxLenInChars ) { AssertUnreachable(); return {}; }


EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CFileSystemStdio, IFileSystem, FILESYSTEM_INTERFACE_VERSION, g_FullFileSystem );
