
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <iostream>
#include <cstdlib>

#include <iostream>
#include <fstream>

typedef void(__stdcall *f_RVExtension)(char*, int, const char*);

static const char* szProcName = "_RVExtension_Dev@12";
static const wchar_t* szwModName = L"@a3dllproxytest";

