#include "ShaderMan.h"
#include <windows.h>

ShaderMan::ShaderMan()
{
}

void ShaderMan::CompileShaders()
{
	STARTUPINFOA si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	//create new process and launch a shell/python? script to compile the shaders
	char path[64] = "py Shaders\\compile_shaders.py\0";
	if (!CreateProcessA(NULL, path, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
	{
		//should keep running but cancel shader compilation
		printf("CreateProcess failed (%d).\n", GetLastError());
		return;
	}
	WaitForSingleObject(pi.hProcess, INFINITE);

	// Close process and thread handles. 
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
}

void ShaderMan::CompileShadersAsync()
{
	STARTUPINFOA si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	//create new process and launch a shell/python? script to compile the shaders
	char path[64] = "py Shaders\\compile_shaders.py\0";
	if (!CreateProcessA(NULL, path, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
	{
		//should keep running but cancel shader compilation
		printf("CreateProcess failed (%d).\n", GetLastError());
		return;
	}
	pi_HProc = pi.hProcess;
	pi_HThread = pi.hThread;
}

void ShaderMan::WaitForCompile()
{

	WaitForSingleObject(pi_HProc, INFINITE);//TODO: make not infinite

	// Close process and thread handles. 
	CloseHandle(pi_HProc);
	CloseHandle(pi_HThread);
}
