// A3DllProxy.cpp : Defines the exported functions for the DLL application.
//

#include "A3DllProxy.h"

extern HMODULE dllLib;
extern f_RVExtension RVExtensionDevFn;
extern bool isAttached;

extern wchar_t szwLibraryName[1024];

extern std::wofstream logFile;

extern char sprintf_buffer[1024];

extern "C" {
	__declspec (dllexport) void __stdcall RVExtension(char *output, int outputSize, const char *function);
}

bool DetachProxiedLibrary() {
	if (!FreeLibrary(dllLib)) {
		logFile << "Failed to free module " << szwLibraryName << '\n';
		return false;
	}

	isAttached = false;

	return true;
}

bool AttachProxiedLibrary() {
	dllLib = LoadLibraryW(szwLibraryName);

	if (!dllLib) {
		logFile << "Failed to load module " << szwLibraryName << '\n';
		return false;
	}

	RVExtensionDevFn = (f_RVExtension)GetProcAddress(dllLib, szProcName);

	if (!RVExtensionDevFn) {
		logFile << "Failed to load proc " << szProcName << '\n';
		return false;
	}

	isAttached = true;

	return true;
}

bool ReloadProxiedLibrary() {
	if (!isAttached) {
		if (!DetachProxiedLibrary()) {
			return false;
		}
	}

	return AttachProxiedLibrary();
}

void __stdcall RVExtension(char *output, int outputSize, const char *function)
{
	if (!strncmp(function, "-a3dllproxy ", 12)) {
		const char* szCommand = function + 12;

		if (!strcmp(szCommand, "d") || !strcmp(szCommand, "detach")) {
			if (!isAttached) {
				sprintf_s(sprintf_buffer, "[%d, \"%s\"]", 2, "Library is already detached, doing nothing.");
			}
			else {
				bool detachSuccess = DetachProxiedLibrary();

				sprintf_s(sprintf_buffer, "[%d, \"%s\"]", detachSuccess, "");
			}

			strncpy_s(output, outputSize, sprintf_buffer, _TRUNCATE);
			return;
		}

		if (!strcmp(szCommand, "a") || !strcmp(szCommand, "attach")) {
			if (isAttached) {
				sprintf_s(sprintf_buffer, "[%d, \"%s\"]", 2, "Library is already attached, doing nothing.");
			}
			else {
				bool attachSuccess = AttachProxiedLibrary();

				sprintf_s(sprintf_buffer, "[%d, \"%s\"]", attachSuccess, "");
			}

			strncpy_s(output, outputSize, sprintf_buffer, _TRUNCATE);
			return;
		}

		if (!strcmp(szCommand, "r") || !strcmp(szCommand, "reload")) {
			bool reloadSuccess = ReloadProxiedLibrary();

			sprintf_s(sprintf_buffer, "[%d, \"%s\"]", reloadSuccess, "");

			strncpy_s(output, outputSize, sprintf_buffer, _TRUNCATE);

			return;
		}

		if (!strcmp(szCommand, "v") || !strcmp(szCommand, "version")) {
			strncpy_s(output, outputSize, "0.1a", _TRUNCATE);

			return;
		}
	}	

	if (isAttached) {
		RVExtensionDevFn(output, outputSize, function);
	}
	else {
		strncpy_s(output, outputSize, "Error : library is not attached, cannot execute", _TRUNCATE);
	}
}

