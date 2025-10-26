#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <combaseapi.h>
#include <malloc.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>

#include "NVShaderPerfPatch.h"

#define INTERFACE ID3DXBuffer
DECLARE_INTERFACE_(ID3DXBuffer,IUnknown)
{
    /*** IUnknown methods ***/
    STDMETHOD_(HRESULT,QueryInterface)(THIS_ REFIID riid, void** ppvObject) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;
    /*** ID3DXBuffer methods ***/
    STDMETHOD_(LPVOID,GetBufferPointer)(THIS) PURE;
    STDMETHOD_(DWORD,GetBufferSize)(THIS) PURE;
};
#undef INTERFACE

static HRESULT(__stdcall *D3DXCreateBuffer)(DWORD NumBytes, ID3DXBuffer** ppBuffer);

const void* inputMemoryData;
size_t inputMemorySize;

char* outputBinaryData;
size_t outputBinarySize;

bool outputMemoryEnable;
char* outputMemoryData;
size_t outputMemorySize;

void* outputBinaryBlob;
void* outputMemoryBlob;

jmp_buf terminateJump;

const char* fileOutputFilename;

static FILE* PatchPrintfFile = nullptr;
static int PatchPrintf(const char* format, ...)
{
    va_list args;
    va_start(args, format);

    if (PatchPrintfFile) {
        if (strncmp(format, "end inst", 8) == 0) {
            fclose(PatchPrintfFile);
            PatchPrintfFile = nullptr;
        }
    }
    if (PatchPrintfFile) {
        if (strcmp(format, "%03d:Start\n") == 0) {
            fseek(PatchPrintfFile, 0, SEEK_SET);
        }
//      else if (strncmp(format, "%03d:  ", 7) == 0) {
//          vfprintf(PatchPrintfFile, format + 7, args);
//      }
        else {
            vfprintf(PatchPrintfFile, format, args);
        }
    }

    int length = vsnprintf(NULL, 0, format, args);
    char* buffer = (char*)malloc(length + 2);
    length = vsnprintf(buffer, length + 2, format, args);
    if (buffer && strncmp(buffer, "Called deriveFP", sizeof("Called deriveFP") - 1) == 0) {
        buffer[length + 0] = '\n';
        buffer[length + 1] = 0;
    }
    if (buffer && strncmp(buffer, "Uniqueness Dword", sizeof("Uniqueness Dword") - 1) == 0) {
        buffer[length + 0] = '\n';
        buffer[length + 1] = 0;
    }

    if (outputMemoryEnable && buffer) {
        if (strncmp(format, "end inst", 8) == 0) {
            outputMemoryEnable = false;
        }
        else if (strncmp(format, "sipu", 4) == 0) {
            outputMemoryEnable = false;
        }
        else if (strncmp(buffer, "*** Done", 8) == 0) {
            outputMemoryEnable = false;
        }
        else if (strncmp(buffer, "***** End", 9) == 0) {
            outputMemoryEnable = false;
        }
        else if (strncmp(buffer, "COPP", 4) == 0) {
            outputMemoryEnable = false;
        }
        else if (strcmp(format, "%03d:Start\n") == 0) {
            outputMemorySize = 0;
        }
        else {
            outputMemoryData = (char*)realloc(outputMemoryData, outputMemorySize + length);
            memcpy(outputMemoryData + outputMemorySize, buffer, length);
            outputMemorySize += length;
        }
    }

    WriteString(buffer);
    free(buffer);

    va_end(args);

    return length;
}

static int PatchFprintf(FILE* file, const char* format, ...)
{
    va_list args;
    va_start(args, format);

    int length = vsnprintf(NULL, 0, format, args);
    char* buffer = (char*)malloc(length + 1);
    length = vsnprintf(buffer, length + 1, format, args);
    WriteString(buffer);

    if (outputMemoryEnable == false) {
        if (strncmp(buffer, "!!SPA1.0", 8) == 0) {
            outputMemoryEnable = true;
        }
    }
    if (outputMemoryEnable && buffer) {
        if (strncmp(buffer, "*** Done", 8) == 0) {
            outputMemoryEnable = false;
        }
        else if (strncmp(buffer, "COPP", 4) == 0) {
            outputMemoryEnable = false;
        }
        else {
            outputMemoryData = (char*)realloc(outputMemoryData, outputMemorySize + length);
            memcpy(outputMemoryData + outputMemorySize, buffer, length);
            outputMemorySize += length;
        }
    }

    free(buffer);

    va_end(args);

    return length;
}

static void PatchLog(int level, const char* message)
{
    PatchFprintf(stdout, "%s", message);
}

static int verbose = 0;
int* GlobalVerbose = &verbose;

int VerboseGenCode = -1;
int ForceGenCode = -1;
static int (*SelectGenCode)(int*, int);
static int PatchSelectGenCode(int* data, bool ucode)
{
    if (VerboseGenCode >= 0) {
        data[VerboseGenCode] = 1;
    }
    if (ForceGenCode >= 0) {
        return SelectGenCode(data, ForceGenCode);
    }
    return SelectGenCode(data, ucode);
}

struct PatchNV30 {
    int PatchDumpNV30PS(DWORD* data);
};
static FILE* outputFileNV30 = nullptr;
static int (*DumpNV30VS)(DWORD* output, DWORD* instruction);
static int PatchDumpNV30VS(DWORD* output, DWORD* instruction)
{
    int result = DumpNV30VS(output, instruction);
    if (inputMemoryData) {
        outputBinaryData = (char*)realloc(outputBinaryData, outputBinarySize + 4 * 4);
        memcpy(outputBinaryData + outputBinarySize, output, 4 * 4);
        outputBinarySize += 4 * 4;
    }
    if (fileOutputFilename && strstr(fileOutputFilename, ".asm") == nullptr) {
        if (outputFileNV30 == nullptr) {
            fopen_s(&outputFileNV30, fileOutputFilename, "wb");
        }
        if (outputFileNV30) {
            fwrite(output, 4, 4, outputFileNV30);
        }
    }
    return result;
}
static int (PatchNV30::*DumpNV30PS)(DWORD* data);
int PatchNV30::PatchDumpNV30PS(DWORD* data)
{
    if (inputMemoryData) {
        outputBinaryData = (char*)realloc(outputBinaryData, outputBinarySize + 4 * 4);
        memcpy(outputBinaryData + outputBinarySize, data, 4 * 4);
        outputBinarySize += 4 * 4;
    }
    if (fileOutputFilename && strstr(fileOutputFilename, ".asm") == nullptr) {
        if (outputFileNV30 == nullptr) {
            fopen_s(&outputFileNV30, fileOutputFilename, "wb");
        }
        if (outputFileNV30) {
            fwrite(data, 4, 4, outputFileNV30);
        }
    }
    return (this->*DumpNV30PS)(data);
}

static int (*ConvertNV30VS)(DWORD convert[45], DWORD* nv30);
static int (*DecodeNV30VS)(DWORD convert[45]);
int DumpBinNV30VS(DWORD* nv30, int size)
{
    DWORD convert[45];
    for (int i = 0; i < size / 4; i += 4) {
        printf("%02x: %08lx %08lx %08lx %08lx\n", i, nv30[i + 0], nv30[i + 1], nv30[i + 2], nv30[i + 3]);
        ConvertNV30VS(convert, &nv30[i]);
        DecodeNV30VS(convert);
    }
    return 0;
}

static FILE* outputFileNV40 = nullptr;
static int (*DumpNV40VS)(DWORD* output, DWORD* instruction);
static int PatchDumpNV40VS(DWORD* output, DWORD* instruction)
{
    int result = DumpNV40VS(output, instruction);
    if (inputMemoryData) {
        outputBinaryData = (char*)realloc(outputBinaryData, outputBinarySize + 4 * 4);
        memcpy(outputBinaryData + outputBinarySize, output, 4 * 4);
        outputBinarySize += 4 * 4;
    }
    if (fileOutputFilename) {
        if (outputFileNV40 == nullptr && strstr(fileOutputFilename, ".asm") == nullptr) {
            fopen_s(&outputFileNV40, fileOutputFilename, "wb");
        }
        if (outputFileNV40) {
            fwrite(output, 4, 4, outputFileNV40);
        }
    }
    return result;
}
static int (*DumpNV40PS)(DWORD* nv40, int size);
static int PatchDumpNV40PS(DWORD* nv40, int size)
{
    if (inputMemoryData) {
        outputBinaryData = (char*)realloc(outputBinaryData, outputBinarySize + size);
        memcpy(outputBinaryData + outputBinarySize, nv40, size);
        outputBinarySize += size;

        if (outputMemorySize == 0) {
           outputMemoryEnable = true;
           outputMemorySize = 0;
        }
        else {
//         longjmp(terminateJump, 1);
        }
    }
    if (fileOutputFilename) {
        if (strstr(fileOutputFilename, ".asm") == nullptr) {
            FILE* file = nullptr;
            fopen_s(&file, fileOutputFilename, "wb");
            if (file) {
                fwrite(nv40, 1, size, file);
                fclose(file);
            }
        }
        exit(0);
    }
    return DumpNV40PS(nv40, size);
}

static int (*ConvertNV40VS)(DWORD convert[50], DWORD* nv40);
static int (*DecodeNV40VS)(DWORD convert[50]);
int DumpBinNV40VS(DWORD* nv40, int size)
{
    DWORD convert[50];
    for (int i = 0; i < size / 4; i += 4) {
        printf("%02x: %08lx %08lx %08lx %08lx\n", i, nv40[i + 0], nv40[i + 1], nv40[i + 2], nv40[i + 3]);
        ConvertNV40VS(convert, &nv40[i]);
        DecodeNV40VS(convert);
    }
    return 0;
}
int DumpBinNV40PS(DWORD* nv40, int size)
{
    return DumpNV40PS(nv40, size);
}

static int (*DumpG70PS)(DWORD* g70, int size);
static int PatchDumpG70PS(DWORD* g70, int size)
{
    if (inputMemoryData) {
        outputBinaryData = (char*)realloc(outputBinaryData, outputBinarySize + size);
        memcpy(outputBinaryData + outputBinarySize, g70, size);
        outputBinarySize += size;

        if (outputMemorySize == 0) {
           outputMemoryEnable = true;
           outputMemorySize = 0;
        }
        else {
//         longjmp(terminateJump, 1);
        }
    }
    if (fileOutputFilename) {
        if (strstr(fileOutputFilename, ".asm") == nullptr) {
            FILE* file = nullptr;
            fopen_s(&file, fileOutputFilename, "wb");
            if (file) {
                fwrite(g70, 1, size, file);
                fclose(file);
            }
        }
        exit(0);
    }
    return DumpG70PS(g70, size);
}

int DumpBinG70PS(DWORD* g70, int size)
{
    return DumpG70PS(g70, size);
}

static int (*DumpG80)(DWORD* g80);
static int PatchDumpG80(DWORD* g80)
{
    DWORD* data = (DWORD*)malloc(g80[6]);
    if (data) {
        memcpy(data, g80, g80[6]);
        for (DWORD i = 0; i < WORD(data[2]); ++i) {
            DWORD offset = data[10 + i * 8];
            if (offset) {
                offset -= (DWORD)g80;
                data[10 + i * 8] = offset;
            }
        }
    }

    if (inputMemoryData) {
        outputBinaryData = (char*)realloc(outputBinaryData, outputBinarySize + g80[6]);
        memcpy(outputBinaryData + outputBinarySize, data, g80[6]);
        outputBinarySize += g80[6];
    }

    if (fileOutputFilename && 0) {
        if (strstr(fileOutputFilename, ".asm") == nullptr) {
            FILE* file = nullptr;
            fopen_s(&file, fileOutputFilename, "wb");
            if (file) {
                fwrite(data, 1, g80[6], file);
                fclose(file);
            }
        }
    }

    free(data);

    return DumpG80(g80);
}

int DumpBinG80(DWORD* g80)
{
    if (DumpG80) {
        for (DWORD i = 0; i < WORD(g80[2]); ++i) {
            DWORD offset = g80[10 + i * 8];
            if (offset) {
                offset += (DWORD)g80;
                g80[10 + i * 8] = offset;
            }
        }
        DumpG80(g80);
    }
    return 0;
}

static int (*RankineVS)(void* shader);
static int PatchRankineVS(void* shader)
{
    if (inputMemoryData) {
        outputMemoryEnable = true;
        outputMemorySize = 0;
    }
    if (fileOutputFilename && strstr(fileOutputFilename, ".asm")) {
        fopen_s(&PatchPrintfFile, fileOutputFilename, "wb");
    }
    int result = RankineVS(shader);
    if (PatchPrintfFile) {
        fclose(PatchPrintfFile);
        PatchPrintfFile = nullptr;
    }
    outputMemoryEnable = false;
    return result;
}

static int (*RankinePS)(void* shader, int count, int type);
static int PatchRankinePS(void* shader, int count, int type)
{
    if (inputMemoryData && type == 1) {
        outputMemoryEnable = true;
        outputMemorySize = 0;
    }
    if (fileOutputFilename && type == 1 && strstr(fileOutputFilename, ".asm")) {
        fopen_s(&PatchPrintfFile, fileOutputFilename, "wb");
    }
    int result = RankinePS(shader, count, type);
    if (PatchPrintfFile) {
        fclose(PatchPrintfFile);
        PatchPrintfFile = nullptr;
    }
    outputMemoryEnable = false;
    return result;
}

static int (*CurieVS)(void* shader);
static int PatchCurieVS(void* shader)
{
    if (inputMemoryData) {
        outputMemoryEnable = true;
        outputMemorySize = 0;
    }
    outputMemoryEnable = inputMemoryData != nullptr;
    if (fileOutputFilename && strstr(fileOutputFilename, ".asm")) {
        fopen_s(&PatchPrintfFile, fileOutputFilename, "wb");
    }
    int result = CurieVS(shader);
    if (PatchPrintfFile) {
        fclose(PatchPrintfFile);
        PatchPrintfFile = nullptr;
    }
    outputMemoryEnable = false;
    return result;
}

static int (*CuriePS)(void* shader, int count, int type);
static int PatchCuriePS(void* shader, int count, int type)
{
    if (inputMemoryData && type == 1) {
        outputMemoryEnable = true;
        outputMemorySize = 0;
    }
    if (fileOutputFilename && type == 1 && strstr(fileOutputFilename, ".asm")) {
        fopen_s(&PatchPrintfFile, fileOutputFilename, "wb");
    }
    int result = CuriePS(shader, count, type);
    if (PatchPrintfFile) {
        fclose(PatchPrintfFile);
        PatchPrintfFile = nullptr;
    }
    outputMemoryEnable = false;
    return result;
}

static int (*SPA)(int type, const char* shader);
static int PatchSPA(int type, const char* shader)
{
    if (inputMemoryData) {
//      outputMemorySize = strlen(shader);
//      outputMemoryData = (char*)malloc(outputMemorySize);
//      memcpy(outputMemoryData, shader, outputMemorySize);
        printf("%s", shader);

        longjmp(terminateJump, 1);
    }
    if (fileOutputFilename && strstr(fileOutputFilename, ".asm")) {
        FILE* file = nullptr;
        fopen_s(&file, fileOutputFilename, "wb");
        if (file) {
            fprintf(file, "%s", shader);
            fclose(file);
        }
        exit(0);
    }
    printf("%s", shader);
    return SPA(type, shader);
}

void CreateBinaryBlob()
{
    ID3DXBuffer* blob = nullptr;
    D3DXCreateBuffer(DWORD(outputBinarySize), &blob);
    if (blob) {
        memcpy(blob->GetBufferPointer(), outputBinaryData, outputBinarySize); 
    }        
    outputBinaryBlob = blob;
}

void CreateMemoryBlob()
{
    ID3DXBuffer* blob = nullptr;
    D3DXCreateBuffer(DWORD(outputMemorySize), &blob);
    if (blob) {
        memcpy(blob->GetBufferPointer(), outputMemoryData, outputMemorySize); 
    }        
    outputMemoryBlob = blob;
}

static HRESULT(__stdcall *D3DXAssembleShaderFromFileA)(const char* pSrcFile, void* pDefines, void* pInclude, DWORD Flags, ID3DXBuffer** ppShader, void** ppErrorMsgs);
static HRESULT __stdcall patchD3DXAssembleShaderFromFileA(const char* pSrcFile, void* pDefines, void* pInclude, DWORD Flags, ID3DXBuffer** ppShader, void** ppErrorMsgs)
{
    if (inputMemoryData) {
        ID3DXBuffer* blob = nullptr;
        D3DXCreateBuffer(DWORD(inputMemorySize), &blob);
        if (blob) {
            memcpy(blob->GetBufferPointer(), inputMemoryData, inputMemorySize); 
        }

        if (blob) {
            if (ppShader)
                (*ppShader) = blob;
            if (ppErrorMsgs)
                (*ppErrorMsgs) = 0;
            return S_OK;
        }
    }

    FILE* file = nullptr;
    fopen_s(&file, pSrcFile, "rb");
    if (file) {
        int version = 0;
        fread(&version, sizeof(int), 1, file);

        size_t size = 0;
        ID3DXBuffer* blob = nullptr;
        switch (version & 0xFFFF0000) {
        case 0xFFFF0000:
        case 0xFFFE0000:
            fseek(file, 0, SEEK_END);
            size = ftell(file);
            fseek(file, 0, SEEK_SET);
            D3DXCreateBuffer(DWORD(size), &blob);
            if (blob) {
                fread(blob->GetBufferPointer(), 1, size, file);
            }
            break;
        }
        fclose(file);

        if (blob) {
            if (ppShader)
                (*ppShader) = blob;
            if (ppErrorMsgs)
                (*ppErrorMsgs) = 0;
            return S_OK;
        }
    }

    return D3DXAssembleShaderFromFileA(pSrcFile, pDefines, pInclude, Flags, ppShader, ppErrorMsgs);
}

static DWORD __stdcall patchGetFileSize(HANDLE hFile, LPDWORD lpFileSizeHigh)
{
    if (inputMemorySize) {
        return inputMemorySize;
    }

    return GetFileSize(hFile, lpFileSizeHigh);
}

static LPCVOID __stdcall patchMapViewOfFile(HANDLE hFileMappingObject, DWORD dwDesiredAccess, DWORD dwFileOffsetHigh, DWORD dwFileOffsetLow, SIZE_T dwNumberOfBytesToMap)
{
    if (inputMemoryData) {
        return inputMemoryData;
    }

    return MapViewOfFile(hFileMappingObject, dwDesiredAccess, dwFileOffsetHigh, dwFileOffsetLow, dwNumberOfBytesToMap);
}

static void WriteCode(void* pvTarget, const void* pvSource, size_t nSize)
{
    DWORD nOldP, nNewP;
    VirtualProtect(pvTarget, nSize, PAGE_READWRITE, &nOldP);
    WriteProcessMemory(GetCurrentProcess(), pvTarget, pvSource, nSize, NULL);
    VirtualProtect(pvTarget, nSize, nOldP, &nNewP);
}

static struct {
    const char* name;
    const char* fullname;
    int data[9];
} Table10131[] = {
  { "NV30", "GeForceFX 5800 Ultra",    { 500,  500, 3,  4, 2, 128, 0, 0x3DA0, 0x3AC0 } },
  { "NV31", "GeForceFX 5600 Ultra",    { 400,  400, 2,  2, 2, 128, 0, 0x3DA0, 0x3AC0 } },
  { "NV34", "GeForceFX 5200 Ultra",    { 400,  325, 2,  2, 2, 128, 0, 0x3DA0, 0x3AC0 } },
  { "NV35", "GeForceFX 5900 Ultra",    { 450,  425, 3,  4, 2, 256, 0, 0x3DA0, 0x3AC0 } },
  { "NV36", "GeForceFX 5700 Ultra",    { 475,  450, 3,  2, 2, 128, 0, 0x3DA0, 0x3AC0 } },
  { "NV38", "GeForceFX 5950 Ultra",    { 475,  475, 3,  4, 2, 256, 0, 0x3DA0, 0x3AC0 } },
  { "NV40", "GeForce 6800 Ultra",      { 400,  550, 6, 16, 1, 256, 0, 0x4030, 0x4170 } },
  { "NV40-GT", "GeForce 6800 GT",      { 350,  500, 6, 16, 1, 256, 0, 0x4030, 0x4170 } },
  { "NV40-12", "GeForce 6800",         { 325,  350, 5, 12, 1, 256, 0, 0x4030, 0x4170 } },
  { "NV43-GT", "GeForce 6600 GT",      { 500,  500, 3,  8, 1, 128, 0, 0x4030, 0x4170 } },
  { "NV44", "GeForce 6200",            { 350,  275, 3,  4, 1, 128, 0, 0x4030, 0x4170 } },
  { "G70-GT", "GeForce 7800 GTX",      { 430,  600, 8, 24, 1, 256, 0, 0x4030, 0x4170 } },
  { "G70", "GeForce 7800 GT",          { 400,  500, 8, 24, 1, 256, 0, 0x4030, 0x4170 } },
  { "NV50", "GeForce 8800 GTX",        { 675,  900, 8,  2, 6, 384, 0, 0x4030, 0x4170 } },
//{ "G80", "GeForce 8800 GTX",         { 675,  900, 8,  2, 6, 384, 0, 0x4030, 0x4170 } },
//{ "G80-Ultra", "GeForce 8800 Ultra", { 750, 1080, 8,  2, 6, 384, 0, 0x4030, 0x4170 } },
//{ "G80-GTS", "GeForce 8800 GTS",     { 600,  800, 8,  2, 6, 320, 0, 0x4030, 0x4170 } },
  {},
};

void Patch10131(const char* path)
{
    if (strstr(path, "2.01.10000.0305") == nullptr)
        return;

    HMODULE dll = GetModuleHandleA("NVShaderPerf_10131.dll");
    if (dll == nullptr)
        dll = LoadLibraryA("2.01.10000.0305\\NVShaderPerf_10131.dll");
    if (dll) {
        WriteCode((char*)dll + 0x4054, "\x0D\x00\x70\x00\x00", 5);

        // Enable G80
        for (int i = 0; i < sizeof(Table10131) / sizeof(Table10131[0]); ++i) {
            Table10131[i].data[7] += (int)dll;
            Table10131[i].data[8] += (int)dll;
        }
        int jumpTable = (int)Table10131;
        WriteCode((char*)dll + 0x4561, &jumpTable, 4);
        WriteCode((char*)dll + 0x456C, &jumpTable, 4);
        WriteCode((char*)dll + 0x4E93, &jumpTable, 4);
        WriteCode((char*)dll + 0x4EA9, &jumpTable, 4);
        WriteCode((char*)dll + 0x5343, &jumpTable, 4);
        WriteCode((char*)dll + 0x5359, &jumpTable, 4);

        // G70
        int jumpG70;
        (void*&)DumpG70PS = (char*)dll + 0x20130;
        jumpG70 = (int)PatchDumpG70PS - ((int)dll + 0x885D + 0x4);
        WriteCode((char*)dll + 0x885D, &jumpG70, 4);
        jumpG70 = (int)PatchDumpG70PS - ((int)dll + 0x88E1 + 0x4);
        WriteCode((char*)dll + 0x88E1, &jumpG70, 4);

        // NV40
        int jumpNV40;
        (void*&)DumpNV40PS = (char*)dll + 0x3B430;
        (void*&)ConvertNV40VS = (char*)dll + 0xAB140;
        (void*&)DecodeNV40VS = (char*)dll + 0xAB390;
        (void*&)DumpNV40VS = (char*)dll + 0xAAF10;
        jumpNV40 = (int)PatchDumpNV40PS - ((int)dll + 0x2369D + 0x4);
        WriteCode((char*)dll + 0x2369D, &jumpNV40, 4);
        jumpNV40 = (int)PatchDumpNV40PS - ((int)dll + 0x23721 + 0x4);
        WriteCode((char*)dll + 0x23721, &jumpNV40, 4);
        jumpNV40 = (int)PatchDumpNV40VS - ((int)dll + 0xAE115 + 0x4);
        WriteCode((char*)dll + 0xAE115, &jumpNV40, 4);

        // NV30
        union Alias { int (PatchNV30::*classFunction)(DWORD* data); void* function; };
        int jumpNV30;
        (void*&)DumpNV30PS = (char*)dll + 0x658C0;
        (void*&)ConvertNV30VS = (char*)dll + 0xAB140;
        (void*&)DecodeNV30VS = (char*)dll + 0xAB390;
        (void*&)DumpNV30VS = (char*)dll + 0xAAF10;
        jumpNV30 = (int)Alias { &PatchNV30::PatchDumpNV30PS }.function - ((int)dll + 0x5DBD6 + 0x4);
        WriteCode((char*)dll + 0x5DBD6, &jumpNV30, 4);
        jumpNV30 = (int)Alias { &PatchNV30::PatchDumpNV30PS }.function - ((int)dll + 0x5DEF4 + 0x4);
        WriteCode((char*)dll + 0x5DEF4, &jumpNV30, 4);
        jumpNV30 = (int)PatchDumpNV30VS - ((int)dll + 0xAB5C5 + 0x4);
        WriteCode((char*)dll + 0xAB5C5, &jumpNV30, 4);

        // G80
        int jumpG80;
        (void*&)DumpG80 = (char*)dll + 0xB4790;
        jumpG80 = (int)PatchDumpG80 - ((int)dll + 0x11C8E0 + 0x4);
        WriteCode((char*)dll + 0x11C8E0, &jumpG80, 4);

        // Select
        VerboseGenCode = 48;
        (void*&)SelectGenCode = (char*)dll + 0x168A40;
        jumpG80 = (int)PatchSelectGenCode - ((int)dll + 0x11C866 + 0x4);
        WriteCode((char*)dll + 0x11C866, &jumpG80, 4);

#if 0
        // vp30
        WriteCode((char*)dll + 0x179316, "\x6A\x01\x58\x89\x86\xC0\x00\x00\x00\x57", 10);
        WriteCode((char*)dll + 0x179806, "\x6A\x01\x58\x89\x86\xC0\x00\x00\x00\x57", 10);

        // vp40
        WriteCode((char*)dll + 0x17A056, "\x6A\x01\x58\x89\x86\xC0\x00\x00\x00\x57", 10);
        WriteCode((char*)dll + 0x17A3E5, "\x6A\x01\x58\x89\x86\xC0\x00\x00\x00\x57", 10);

        // vp50
        WriteCode((char*)dll + 0x17E6EC, "\x6A\x01\x58\x89\x86\xC0\x00\x00\x00\x57", 10);
        WriteCode((char*)dll + 0x17E75C, "\x6A\x01\x58\x89\x86\xC0\x00\x00\x00\x57", 10);

        // fp30
        WriteCode((char*)dll + 0x1782B5, "\x6A\x01\x58\x89\x86\xC0\x00\x00\x00\x57", 10);
        WriteCode((char*)dll + 0x178595, "\x6A\x01\x58\x89\x86\xC0\x00\x00\x00\x57", 10);

        // fp40
        WriteCode((char*)dll + 0x10E4A8, "\x6A\x01\x58\x89\x86\xC0\x00\x00\x00\x57", 10);
        WriteCode((char*)dll + 0x17A617, "\x6A\x01\x58\x89\x86\xC0\x00\x00\x00\x57", 10);

        // fp50
        WriteCode((char*)dll + 0x11C2DF, "\x90\x90\x90\x90\x90\x90\x90", 7);
        WriteCode((char*)dll + 0x18006C, "\x6A\x01\x58\x89\x86\xC0\x00\x00\x00\x57", 10);
        WriteCode((char*)dll + 0x1800DC, "\x6A\x01\x58\x89\x86\xC0\x00\x00\x00\x57", 10);
#else
        // fp50
        WriteCode((char*)dll + 0x11C2DF, "\x90\x90\x90\x90\x90\x90\x90", 7);
#endif

        // Rankine - VS
        (void*&)RankineVS = (char*)dll + 0xAB650;
        jumpNV30 = (int)PatchRankineVS - ((int)dll + 0x12663C + 0x4);
        WriteCode((char*)dll + 0x12663C, &jumpNV30, 4);
        jumpNV30 = (int)PatchRankineVS;
        WriteCode((char*)dll + 0x1265C5, &jumpNV30, 4);

        // Rankine - PS
        (void*&)RankinePS = (char*)dll + 0xB2620;
        jumpNV30 = (int)PatchRankinePS - ((int)dll + 0x5D958 + 0x4);
        WriteCode((char*)dll + 0x5D958, &jumpNV30, 4);

        // Curie - VS
        (void*&)CurieVS = (char*)dll + 0xAE190;
        jumpNV40 = (int)PatchCurieVS - ((int)dll + 0x1253EC + 0x4);
        WriteCode((char*)dll + 0x1253EC, &jumpNV40, 4);
        jumpNV40 = (int)PatchCurieVS;
        WriteCode((char*)dll + 0x125376, &jumpNV40, 4);

        // Curie - PS
        (void*&)CuriePS = (char*)dll + 0xB2620;
        jumpNV40 = (int)PatchCuriePS - ((int)dll + 0x5ED4A + 0x4);
        WriteCode((char*)dll + 0x5ED4A, &jumpNV40, 4);

        // SPA
//      (void*&)SPA = (char*)dll + 0x2380;
//      jumpG80 = (int)PatchSPA - ((int)dll + 0x1ECB + 0x4);
//      WriteCode((char*)dll + 0x1ECB, &jumpG80, 4);

        // printf
        static void* printf_impl = &PatchPrintf;
        WriteCode((char*)dll + 0x1CD0E0, &printf_impl, sizeof(void*));

        // fprintf
        static void* fprintf_impl = &PatchFprintf;
        WriteCode((char*)dll + 0x1CD150, &fprintf_impl, sizeof(void*));

        // GetFileSize
        static void* GetFileSize_pointer = &patchGetFileSize;
        static void* GetFileSize_impl = &GetFileSize_pointer;
        WriteCode((char*)dll + 0x11BB, &GetFileSize_impl, sizeof(void*));

        // MapViewOfFile
        static void* MapViewOfFile_pointer = &patchMapViewOfFile;
        static void* MapViewOfFile_impl = &MapViewOfFile_pointer;
        WriteCode((char*)dll + 0x11E8, &MapViewOfFile_impl, sizeof(void*));

        // Level
        GlobalVerbose = (int*)((char*)dll + 0x28E6B0);
    }

    dll = GetModuleHandleA("NVShaderPerf.dll");
    if (dll == nullptr)
        dll = LoadLibraryA("2.01.10000.0305\\NVShaderPerf.dll");
    if (dll) {
        HMODULE d3dx9 = LoadLibraryA("d3dx9_30.dll");
        if (d3dx9) {
            (void*&)D3DXCreateBuffer = GetProcAddress(d3dx9, "D3DXCreateBuffer");
            (void*&)D3DXAssembleShaderFromFileA = GetProcAddress(d3dx9, "D3DXAssembleShaderFromFileA");

            int jump = (int)patchD3DXAssembleShaderFromFileA - ((int)dll + 0x4B7F + 0x4);
            WriteCode((char*)dll + 0x4B7F, &jump, 4);
        }
    }
}

void Patch17474(const char* path)
{
    if (strstr(path, "2.07.0804.1530") == nullptr)
        return;

    HMODULE dll = GetModuleHandleA("NVShaderPerf_17474.dll");
    if (dll == nullptr)
        dll = LoadLibraryA("2.07.0804.1530\\NVShaderPerf_17474.dll");
    if (dll) {
        unsigned int flags = 0x00007000;
        WriteCode((char*)dll + 0x3E706, &flags, 4);

        // G70
        int jumpG70;
        (void*&)DumpG70PS = (char*)dll + 0x5FD90;
        jumpG70 = (int)PatchDumpG70PS - ((int)dll + 0x484FD + 0x4);
        WriteCode((char*)dll + 0x484FD, &jumpG70, 4);
        jumpG70 = (int)PatchDumpG70PS - ((int)dll + 0x48581 + 0x4);
        WriteCode((char*)dll + 0x48581, &jumpG70, 4);

        // NV40
        int jumpNV40;
        (void*&)DumpNV40PS = (char*)dll + 0x7AB50;
        (void*&)ConvertNV40VS = (char*)dll + 0xD1A50;
        (void*&)DecodeNV40VS = (char*)dll + 0xD1CE0;
        (void*&)DumpNV40VS = (char*)dll + 0xD1F30;
        jumpNV40 = (int)PatchDumpNV40PS - ((int)dll + 0x62D7D + 0x4);
        WriteCode((char*)dll + 0x62D7D, &jumpNV40, 4);
        jumpNV40 = (int)PatchDumpNV40PS - ((int)dll + 0x62E01 + 0x4);
        WriteCode((char*)dll + 0x62E01, &jumpNV40, 4);
        jumpNV40 = (int)PatchDumpNV40VS - ((int)dll + 0xD20F5 + 0x4);
        WriteCode((char*)dll + 0xD20F5, &jumpNV40, 4);

        // COPP
        WriteCode((char*)dll + 0xF44EA, "\x6A\x01\x5D\x8B\xC5\x89\x86\xC0\x01\x00\x00", 11);

        // G80
        int jumpG80;
        (void*&)DumpG80 = (char*)dll + 0x108BD0;
        jumpG80 = (int)PatchDumpG80 - ((int)dll + 0x1178C7 + 0x4);
        WriteCode((char*)dll + 0x1178C7, &jumpG80, 4);

        // parseasm
        int jumpLog = (int)&PatchLog;
        WriteCode((char*)dll + 0x116E28, &jumpLog, 4);

        // Select
        VerboseGenCode = 50;
        (void*&)SelectGenCode = (char*)dll + 0x186B30;
        jumpG80 = (int)PatchSelectGenCode - ((int)dll + 0x117846 + 0x4);
        WriteCode((char*)dll + 0x117846, &jumpG80, 4);

#if 0
        // vp40
        WriteCode((char*)dll + 0x1C0416, "\x6A\x01\x58\x89\x86\xC8\x00\x00\x00\x57", 10);
        WriteCode((char*)dll + 0x1C0705, "\x6A\x01\x58\x89\x85\xC8\x00\x00\x00\x57", 10);

        // vp50
        WriteCode((char*)dll + 0x1C48D0, "\x90\x90", 2);
        WriteCode((char*)dll + 0x1C4AF4, "\x6A\x01\x58\x89\x87\xC8\x00\x00\x00", 9);
        WriteCode((char*)dll + 0x1C4B5D, "\x6A\x01\x58\x89\x87\xC8\x00\x00\x00", 9);

        // fp40
        WriteCode((char*)dll + 0x1838E6, "\x6A\x01\x58\x89\x86\xC8\x00\x00\x00\x57", 10);
        WriteCode((char*)dll + 0x1C0937, "\x6A\x01\x58\x89\x86\xC8\x00\x00\x00\x57", 10);

        // fp50
        WriteCode((char*)dll + 0x1C609C, "\x6A\x01\x58\x89\x86\xC8\x00\x00\x00\x57", 10);
        WriteCode((char*)dll + 0x1C697C, "\x6A\x01\x58\x89\x86\xC8\x00\x00\x00\x57", 10);
#endif

        // Curie - VS
        (void*&)CurieVS = (char*)dll + 0xD2170;
        jumpNV40 = (int)PatchCurieVS - ((int)dll + 0x12051E + 0x4);
        WriteCode((char*)dll + 0x12051E, &jumpNV40, 4);
        jumpNV40 = (int)PatchCurieVS;
        WriteCode((char*)dll + 0x1204A7, &jumpNV40, 4);

        // Curie - PS
        (void*&)CuriePS = (char*)dll + 0x1062A0;
        jumpNV40 = (int)PatchCuriePS - ((int)dll + 0x9ED7F + 0x4);
        WriteCode((char*)dll + 0x9ED7F, &jumpNV40, 4);

        // SPA
        (void*&)SPA = (char*)dll + 0x2380;
        jumpG80 = (int)PatchSPA - ((int)dll + 0x1ECB + 0x4);
        WriteCode((char*)dll + 0x1ECB, &jumpG80, 4);

        // printf
        static void* printf_impl = &PatchPrintf;
        WriteCode((char*)dll + 0x22F11C, &printf_impl, sizeof(void*));

        // fprintf
        static void* fprintf_impl = &PatchFprintf;
        WriteCode((char*)dll + 0x22F278, &fprintf_impl, sizeof(void*));

        // GetFileSize
        static void* GetFileSize_pointer = &patchGetFileSize;
        static void* GetFileSize_impl = &GetFileSize_pointer;
        WriteCode((char*)dll + 0x364CB, &GetFileSize_impl, sizeof(void*));

        // MapViewOfFile
        static void* MapViewOfFile_pointer = &patchMapViewOfFile;
        static void* MapViewOfFile_impl = &MapViewOfFile_pointer;
        WriteCode((char*)dll + 0x364F8, &MapViewOfFile_impl, sizeof(void*));

        // Level
        GlobalVerbose = (int*)((char*)dll + 0x2EA418);
    }

    dll = GetModuleHandleA("NVShaderPerf.dll");
    if (dll == nullptr)
        dll = LoadLibraryA("2.07.0804.1530\\NVShaderPerf.dll");
    if (dll) {
        HMODULE d3dx9 = nullptr;
        for (int i = 100; i >= 30; --i) {
            char name[64];
            snprintf(name, 64, "d3dx9_%d.dll", i);
            d3dx9 = LoadLibraryA(name);
            if (d3dx9)
                break;
        }
        if (d3dx9) {
            (void*&)D3DXCreateBuffer = GetProcAddress(d3dx9, "D3DXCreateBuffer");
            (void*&)D3DXAssembleShaderFromFileA = GetProcAddress(d3dx9, "D3DXAssembleShaderFromFileA");

            int jump = (int)patchD3DXAssembleShaderFromFileA - ((int)dll + 0x6277 + 0x4);
            WriteCode((char*)dll + 0x6277, &jump, 4);
        }
    }
}

void PatchRSX(const char* path)
{
    if (strstr(path, "2.09.1109.0300") == nullptr)
        return;

    HMODULE dll = GetModuleHandleA("NVShaderPerf_RSX.dll");
    if (dll == nullptr)
        dll = LoadLibraryA("2.09.1109.0300\\NVShaderPerf_RSX.dll");
    if (dll) {
        // Level
        GlobalVerbose = (int*)((char*)dll + 0x200A18);
    }
}
