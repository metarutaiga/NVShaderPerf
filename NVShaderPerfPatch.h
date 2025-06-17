#pragma once

#define USE_CONSOLE 1

#if USE_CONSOLE
#define WriteString(v) printf("%s", v)
#else
#define WriteString OutputDebugStringA
#endif

extern const char* fileOutputFilename;
extern void Patch10131(int verbose);
extern void Patch17474(int verbose);

extern int DumpBinG80(DWORD* g80);