// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
HMODULE g_hModuleInstanceDLL = NULL;
HINSTANCE hInst = NULL;

BOOL APIENTRY DllMain(HMODULE hModule,
  DWORD  ul_reason_for_call,
  LPVOID lpReserved
)
{
  switch (ul_reason_for_call)
  {
  case DLL_PROCESS_ATTACH:
    g_hModuleInstanceDLL = hModule;
    break;
  case DLL_THREAD_ATTACH:
  case DLL_THREAD_DETACH:
    break;
  case DLL_PROCESS_DETACH:
    g_hModuleInstanceDLL = NULL;
    break;
  }
  return TRUE;
}


