#include <Windows.h>
#include <TlHelp32.h>

int GameDll = 0;

int sub_6F5BE670( )
{
	return (int) (GameDll + 0xACCE94);
}


#define IsKeyPressed(CODE) (GetAsyncKeyState(CODE) & 0x8000) > 0


LPVOID TlsValue;
DWORD TlsIndex;
DWORD _W3XTlsIndex;

DWORD GetIndex( )
{
	return *( DWORD* ) ( _W3XTlsIndex );
}

DWORD GetW3TlsForIndex( DWORD index )
{
	DWORD pid = GetCurrentProcessId( );
	THREADENTRY32 te32;
	HANDLE hSnap = CreateToolhelp32Snapshot( TH32CS_SNAPTHREAD, pid );
	te32.dwSize = sizeof( THREADENTRY32 );

	if ( Thread32First( hSnap, &te32 ) )
	{
		do
		{
			if ( te32.th32OwnerProcessID == pid )
			{
				HANDLE hThread = OpenThread( THREAD_ALL_ACCESS, FALSE, te32.th32ThreadID );
				CONTEXT ctx = { CONTEXT_SEGMENTS };
				LDT_ENTRY ldt;
				GetThreadContext( hThread, &ctx );
				GetThreadSelectorEntry( hThread, ctx.SegFs, &ldt );
				DWORD dwThreadBase = ldt.BaseLow | ( ldt.HighWord.Bytes.BaseMid <<
													 16 ) | ( ldt.HighWord.Bytes.BaseHi << 24 );
				CloseHandle( hThread );
				if ( dwThreadBase == NULL )
					continue;
				DWORD* dwTLS = *( DWORD** ) ( dwThreadBase + 0xE10 + 4 * index );
				if ( dwTLS == NULL )
					continue;
				return ( DWORD ) dwTLS;
			}
		}
		while ( Thread32Next( hSnap, &te32 ) );
	}

	return NULL;
}

void SetTlsForMe( )
{
	TlsIndex = GetIndex( );
	LPVOID tls = ( LPVOID ) GetW3TlsForIndex( TlsIndex );
	TlsValue = tls;
}

int sub_6F5C2E30_addr = 0;

__declspec(naked) void __fastcall sub_6F5C2E30( int a1,int unused, char a2 )
{
	__asm
	{ JMP sub_6F5C2E30_addr }
}

int GetRandomHandi( )
{
	return (5 + (rand( ) % 5)) * 10;
}

HANDLE HackHandiThreadID; 
DWORD WINAPI HackHandiThread( LPVOID )
{
	SetTlsForMe( );
	TlsSetValue( TlsIndex, TlsValue );
	while ( true )
	{
		Sleep( 10 );
		if ( IsKeyPressed( 'P' ) )
		{
			sub_6F5C2E30( sub_6F5BE670( ), 0xFFFFFFFF, GetRandomHandi( ) );
		}
	}
	return 0;
}



BOOL WINAPI DllMain( HINSTANCE hDLL, UINT reason, LPVOID reserved )
{
	if ( reason == DLL_PROCESS_ATTACH )
	{
		GameDll = ( int ) ( GetModuleHandle( "Game.dll" ) );
		_W3XTlsIndex = 0xAB7BF4 + GameDll;
		sub_6F5C2E30_addr = GameDll + 0x5C2E30;
		HackHandiThreadID = CreateThread( 0, 0, HackHandiThread, 0, 0, 0 );
	}
	else if ( reason == DLL_PROCESS_DETACH )
	{
		TerminateThread( HackHandiThreadID, 0 );
	}
	return TRUE;
}
