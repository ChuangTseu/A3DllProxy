
#include "A3DllProxy.h"

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

HMODULE dllLib;
f_RVExtension RVExtensionDevFn;

char sprintf_buffer[1024];
wchar_t szwFullModPath[1024];

wchar_t szwLibraryName[1024];

bool isAttached = false;

std::wofstream logFile;

bool isIn(const char* sz, char c) {
	while (*sz) {
		if (*sz == c) return true;
		++sz;
	}

	return false;
}

#define NPOS (static_cast<size_t>(-1))

size_t szRFind(const char* sz, char c) {
	size_t i = 0;
	size_t findIndex = NPOS;

	while (*sz) {
		if (*sz == c) {
			findIndex = i;
		}

		++i;
		++sz;
	}

	return findIndex;
}

size_t szRFindW(const wchar_t* sz, wchar_t c) {
	size_t i = 0;
	size_t findIndex = NPOS;

	while (*sz) {
		if (*sz == c) {
			findIndex = i;
		}

		++i;
		++sz;
	}

	return findIndex;
}

bool isDllExt(const char* sz) {
	if (*sz) ++sz; // avoid ".dll" filename

	while (*sz) {
		if (*sz == '.') {
			return !strcmp(sz, ".dll");
		}

		++sz;
	}

	return false;
}

bool isDllExtW(const wchar_t* sz) {
	if (*sz) ++sz; // avoid ".dll" filename

	while (*sz) {
		if (*sz == L'.') {
			return !lstrcmpW(sz, L".dll");
		}

		++sz;
	}

	return false;
}

bool DllHasSymbolW(const wchar_t* szwDll, const char* szSymbol) {
	HMODULE dllModule = LoadLibraryW(szwDll);

	if (!dllModule) {
		logFile << "Failed to load module " << szwDll << '\n';
		return false;
	};

	bool hasSymbol = (GetProcAddress(dllModule, szSymbol) ? true : false);

	if (!FreeLibrary(dllModule)) {
		logFile << "Failed to free module " << szwDll << '\n';
	}

	return hasSymbol;
}

bool SetProxiedLibraryName() {
	WIN32_FIND_DATAW ffd;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	DWORD dwError = 0;

	wchar_t szwModuleDirectory[1024];

	GetModuleFileNameW((HINSTANCE)&__ImageBase, szwModuleDirectory, 1024);
	size_t lastBackslashIndex = szRFindW(szwModuleDirectory, L'\\');
	szwModuleDirectory[lastBackslashIndex + 1] = L'*';
	szwModuleDirectory[lastBackslashIndex + 2] = L'\0';

	logFile << "szwModuleDirectory : " << szwModuleDirectory << '\n';

	hFind = FindFirstFileW(szwModuleDirectory, &ffd);

	szwModuleDirectory[lastBackslashIndex] = L'\0'; // the \* is not needed anymore after call to FindFirstFileW

	if (INVALID_HANDLE_VALUE == hFind) {
		logFile << "INVALID_HANDLE_VALUE == hFind" << '\n';
		return false;
	}

	do
	{
		logFile << L"Checking : " << ffd.cFileName << '\n';

		if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			wsprintfW(szwFullModPath, L"%s\\%s", szwModuleDirectory, ffd.cFileName);

			logFile << L"Checking file : " << szwFullModPath << '\n';

			if (lstrcmpW(L"A3DllProxy.dll", ffd.cFileName) && isDllExtW(ffd.cFileName) && DllHasSymbolW(szwFullModPath, "_RVExtension_Dev@12")) {
				logFile << L"DLL to hook : " << ffd.cFileName << '\n';

				lstrcpyW(szwLibraryName, szwFullModPath);

				FindClose(hFind);
				return true;
			}
		}
	} while (FindNextFileW(hFind, &ffd) != 0);

	dwError = GetLastError();
	if (dwError != ERROR_NO_MORE_FILES)
	{
		logFile << "dwError != ERROR_NO_MORE_FILES" << '\n';
		return false;
	}

	FindClose(hFind);
	return false;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		{
			logFile.open("logfile.log");

			logFile << "DllMain case DLL_PROCESS_ATTACH" << '\n';

			if (!SetProxiedLibraryName()) return false;

			dllLib = LoadLibraryW(szwLibraryName);

			if (!dllLib) {
				logFile << "Failed to load module " << szwLibraryName << '\n';
				return FALSE;
			}

			RVExtensionDevFn = (f_RVExtension)GetProcAddress(dllLib, szProcName);

			if (!RVExtensionDevFn) {
				logFile << "Failed to load proc " << szProcName << '\n';
				return FALSE;
			}

			logFile << "INITIAL ATTACH SUCCESS" << '\n';

			isAttached = true;
		}
		break;
		case DLL_THREAD_ATTACH:
		{
			// ?
		}
		break;
		case DLL_THREAD_DETACH:
		{
			// ?
		}
		break;
		case DLL_PROCESS_DETACH:
		{
			logFile << "DllMain case DLL_PROCESS_DETACH" << '\n';

			if (isAttached) {
				if (!FreeLibrary(dllLib)) {
					logFile << "Failed to free module " << szwLibraryName << '\n';
					return FALSE;
				}

				isAttached = false;
			}

			logFile.close();
		}
		break;
	}
	return TRUE;
}

