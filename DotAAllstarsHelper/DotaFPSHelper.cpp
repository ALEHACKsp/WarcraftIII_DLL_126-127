#include "Main.h"
#include <gl/GL.h>
#include <gl/GLU.h>
#include <wglext.h>


unsigned int( __thiscall  * DestroyUnitHpBar )( int HpBarAddr );


BOOL FPS_LIMIT_ENABLED = FALSE;

ULARGE_INTEGER lastCPU, lastSysCPU, lastUserCPU;
int numProcessors;
unsigned int CPU_cores;
HANDLE Warcraft3_Process = NULL;
p_SetMaxFps _SetMaxFps = NULL;

int __stdcall SetMaxFps( int fps )
{
	_SetMaxFps( fps );
	return fps;
}


bool WGLExtensionSupported( const char *extension_name )
{
	return false;
	// this is pointer to function which returns pointer to string with list of all wgl extensions
	PFNWGLGETEXTENSIONSSTRINGEXTPROC _wglGetExtensionsStringEXT = NULL;

	// determine pointer to wglGetExtensionsStringEXT function
	_wglGetExtensionsStringEXT = ( PFNWGLGETEXTENSIONSSTRINGEXTPROC )wglGetProcAddress( "wglGetExtensionsStringEXT" );

	if ( _wglGetExtensionsStringEXT == NULL || strstr( _wglGetExtensionsStringEXT( ), extension_name ) == NULL )
	{
		// string was not found
		return false;
	}

	// extension is supported
	return true;
}


void __stdcall EnableVsync( BOOL enable )
{
	return;
	PFNWGLSWAPINTERVALEXTPROC       wglSwapIntervalEXT = NULL;
	PFNWGLGETSWAPINTERVALEXTPROC    wglGetSwapIntervalEXT = NULL;

	if ( WGLExtensionSupported( "WGL_EXT_swap_control" ) )
	{
		// Extension is supported, init pointers.
		wglSwapIntervalEXT = ( PFNWGLSWAPINTERVALEXTPROC )wglGetProcAddress( "wglSwapIntervalEXT" );

		//// this is another function from WGL_EXT_swap_control extension
		//wglGetSwapIntervalEXT = (PFNWGLGETSWAPINTERVALEXTPROC)wglGetProcAddress("wglGetSwapIntervalEXT");
		//
		if ( wglSwapIntervalEXT )
		{
			if ( SetInfoObjDebugVal )
			{
				PrintText( "CHANGE VSYNC STATE OPENGL!" );
			}
			wglSwapIntervalEXT( enable );
		}
	}



	PatchOffsetValue1( ( void* )( GameDll + 0x62D7FB ), enable ? 0xFB : 0xFF );

	InitD3DVSync( enable );

}



void InitThreadCpuUsage( ) {
	SYSTEM_INFO sysInfo;
	FILETIME ftime, fsys, fuser;


	numProcessors = std::thread::hardware_concurrency( );
	if ( numProcessors == 0 )
	{
		GetSystemInfo( &sysInfo );
		numProcessors = sysInfo.dwNumberOfProcessors;
	}

	GetSystemTimeAsFileTime( &ftime );
	std::memcpy( &lastCPU, &ftime, sizeof( FILETIME ) );


	GetProcessTimes( Warcraft3_Process, &ftime, &ftime, &fsys, &fuser );
	std::memcpy( &lastSysCPU, &fsys, sizeof( FILETIME ) );
	std::memcpy( &lastUserCPU, &fuser, sizeof( FILETIME ) );
}

double GetWar3CpuUsage( ) {
	FILETIME ftime, fsys, fuser;
	ULARGE_INTEGER now, sys, user;
	double percent;
	GetSystemTimeAsFileTime( &ftime );
	std::memcpy( &now, &ftime, sizeof( FILETIME ) );
	GetProcessTimes( Warcraft3_Process, &ftime, &ftime, &fsys, &fuser );
	std::memcpy( &sys, &fsys, sizeof( FILETIME ) );
	std::memcpy( &user, &fuser, sizeof( FILETIME ) );
	percent = ( double )( ( sys.QuadPart - lastSysCPU.QuadPart ) +
		( user.QuadPart - lastUserCPU.QuadPart ) );
	percent /= ( now.QuadPart - lastCPU.QuadPart );
	lastCPU = now;
	lastUserCPU = user;
	lastSysCPU = sys;
	return percent * 100.0;
}


#define MAX_WAR3_FPS 80
#define MIN_WAR3_FPS 24
int CurrentFps = 64;
void UpdateFPS( )
{
	double currentcpuusage = GetWar3CpuUsage( );
	if ( currentcpuusage > 88.0 )
	{
		if ( CurrentFps > MIN_WAR3_FPS )
		{
			CurrentFps -= 4;
			if ( IsGame( ) )
				_SetMaxFps( CurrentFps );
			sprintf_s( MyFpsString, 512, "%s%i.0 CPU:%.1f", "|nFPS: %.1f/", CurrentFps, currentcpuusage );
		}
	}
	else if ( currentcpuusage < 70.0 )
	{
		if ( CurrentFps < MAX_WAR3_FPS )
		{
			CurrentFps += 4;
			if ( IsGame( ) )
				_SetMaxFps( CurrentFps );
			sprintf_s( MyFpsString, 512, "%s%i.0 CPU:%.1f", "|nFPS: %.1f/", CurrentFps, currentcpuusage );
		}
	}
}



int __stdcall EnableAutoFPSlimit( BOOL enable )
{
	FPS_LIMIT_ENABLED = enable;
	if ( !FPS_LIMIT_ENABLED )
		_SetMaxFps( 200 );
	return enable;
}


//map<int, BOOL> NeedDrawBarForUnit;

BOOL __stdcall UnitNeedDrawBar( int unitaddr )
{
	if ( unitaddr && IsNotBadUnit( unitaddr ) )
	{
		if ( GetUnitOwnerSlot( unitaddr ) >= 15 && IsUnitInvulnerable( unitaddr ) )
		{
			//if ( NeedDrawBarForUnit[ unitaddr ] )
			return FALSE;
			/*else
				NeedDrawBarForUnit[ unitaddr ] = TRUE;*/
		}
		//else
		//	NeedDrawBarForUnit[ unitaddr ] = FALSE;

		return TRUE;
	}
	return FALSE;
}

pDrawBarForUnit DrawBarForUnit_org;
pDrawBarForUnit DrawBarForUnit_ptr;

BOOL FPSfix1Enabled = FALSE;

void __fastcall DrawBarForUnit_my( int unitaddr )
{
	if ( !FPSfix1Enabled )
	{
		DrawBarForUnit_ptr( unitaddr );
	}
	else /*if ( UnitNeedDrawBar( unitaddr ) )*/
	{
		BOOL needremove = FALSE;
		int hpbaraddr = *( int* )( unitaddr + 0x50 );
		if ( hpbaraddr )
		{
			if ( !IsNotBadUnit( unitaddr ) || ( IsTower( unitaddr ) && IsUnitInvulnerable( unitaddr ) ) )
			{
				needremove = TRUE;
				*( int * )( hpbaraddr + 8 ) = 0;
			}
		}

		DrawBarForUnit_ptr( unitaddr );

		if ( needremove )
		{
			*( int* )( unitaddr + 0x50 ) = 0;
		}
	}
}

int __stdcall EnableFPSfix1( BOOL enable )
{
	FPSfix1Enabled = enable;
	return 0;
}