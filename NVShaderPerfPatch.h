#pragma once

#define USE_CONSOLE 1

#if USE_CONSOLE
#define WriteString(v) printf("%s", v)
#else
#define WriteString OutputDebugStringA
#endif

extern const void* inputMemoryData;
extern size_t inputMemorySize;

extern void* outputBinaryBlob;
extern void* outputMemoryBlob;

extern jmp_buf terminateJump;

extern const char* fileOutputFilename;

extern void Patch10131(int verbose);
extern void Patch17474(int verbose);
extern void PatchRSX(int verbose);

extern int DumpBinNV30VS(DWORD* nv30, int size);

extern int DumpBinNV40VS(DWORD* nv40, int size);
extern int DumpBinNV40PS(DWORD* nv40, int size);
extern int DumpBinG70PS(DWORD* g70, int size);
extern int DumpBinG80(DWORD* g80);

extern void CreateBinaryBlob();
extern void CreateMemoryBlob();