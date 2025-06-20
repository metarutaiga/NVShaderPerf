#include <Windows.h>
#include <stdio.h>

#include "NVShaderPerfPatch.h"

static int PatchPrintf(const char* format, ...)
{
    va_list args;
    va_start(args, format);

    int length = vsnprintf(NULL, 0, format, args);
    char* buffer = (char*)malloc(length + 2);
    vsnprintf(buffer, length + 2, format, args);
    if (buffer && strncmp(buffer, "Called deriveFP", sizeof("Called deriveFP") - 1) == 0) {
        buffer[length + 0] = '\n';
        buffer[length + 1] = 0;
    }
    if (buffer && strncmp(buffer, "Uniqueness Dword", sizeof("Uniqueness Dword") - 1) == 0) {
        buffer[length + 0] = '\n';
        buffer[length + 1] = 0;
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
    vsnprintf(buffer, length + 1, format, args);
    WriteString(buffer);
    free(buffer);

    va_end(args);

    return length;
}

const char* fileOutputFilename;

struct PatchNV30 {
    int PatchDumpNV30PS(DWORD* data);
};
static FILE* outputFileNV30 = nullptr;
static int (*DumpNV30VS)(DWORD* output, DWORD* instruction);
static int PatchDumpNV30VS(DWORD* output, DWORD* instruction)
{
    int result = DumpNV30VS(output, instruction);
    if (fileOutputFilename) {
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
    if (fileOutputFilename) {
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
        printf("%02x: %08x %08x %08x %08x\n", i, nv30[i + 0], nv30[i + 1], nv30[i + 2], nv30[i + 3]);
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
    if (fileOutputFilename) {
        if (outputFileNV40 == nullptr) {
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
    if (fileOutputFilename) {
        FILE* file = nullptr;
        fopen_s(&file, fileOutputFilename, "wb");
        if (file) {
            fwrite(nv40, 1, size, file);
            fclose(file);
        }
    }
    return DumpNV40PS(nv40, size);
}

static int (*ConvertNV40VS)(DWORD convert[50], DWORD* nv40);
static int (*DecodeNV40VS)(DWORD convert[50]);
int DumpBinNV40VS(DWORD* nv40, int size)
{
    DWORD convert[50];
    for (int i = 0; i < size / 4; i += 4) {
        printf("%02x: %08x %08x %08x %08x\n", i, nv40[i + 0], nv40[i + 1], nv40[i + 2], nv40[i + 3]);
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
    if (fileOutputFilename) {
        FILE* file = nullptr;
        fopen_s(&file, fileOutputFilename, "wb");
        if (file) {
            fwrite(g70, 1, size, file);
            fclose(file);
        }
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
    if (fileOutputFilename) {
        FILE* file = nullptr;
        fopen_s(&file, fileOutputFilename, "wb");
        if (file) {
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
                fwrite(data, 1, g80[6], file);
                free(data);
            }
            fclose(file);
        }
    }
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

static void WriteCode(void* pvTarget, const void* pvSource, size_t nSize)
{
    DWORD nOldP, nNewP;
    VirtualProtect(pvTarget, nSize, PAGE_READWRITE, &nOldP);
    WriteProcessMemory(GetCurrentProcess(), pvTarget, pvSource, nSize, NULL);
    VirtualProtect(pvTarget, nSize, nOldP, &nNewP);
}

void Patch10131(int verbose)
{
    HMODULE dll = GetModuleHandleA("NVShaderPerf_10131.dll");
    if (dll) {
        WriteCode((char*)dll + 0x4054, "\x0D\x00\x70\x00\x00", 5);

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

        // printf
        static void* printf_impl = &PatchPrintf;
        WriteCode((char*)dll + 0x1CD0E0, &printf_impl, sizeof(void*));

        // fprintf
        static void* fprintf_impl = &PatchFprintf;
        WriteCode((char*)dll + 0x1CD150, &fprintf_impl, sizeof(void*));

        // Level
        memcpy((char*)dll + 0x28E6B0, &verbose, 1);
    }
}

void Patch17474(int verbose)
{
    HMODULE dll = GetModuleHandleA("NVShaderPerf_17474.dll");
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

        // vp50_ucode
        WriteCode((char*)dll + 0x1C4B5D, "\x6A\x01\x58\x89\x87\xC8\x00\x00\x00", 9);

        // fp50_ucode
        WriteCode((char*)dll + 0x1C697C, "\x6A\x01\x58\x89\x86\xC8\x00\x00\x00\x57", 10);

        // printf
        static void* printf_impl = &PatchPrintf;
        WriteCode((char*)dll + 0x22F11C, &printf_impl, sizeof(void*));

        // fprintf
        static void* fprintf_impl = &PatchFprintf;
        WriteCode((char*)dll + 0x22F278, &fprintf_impl, sizeof(void*));

        // Level
        memcpy((char*)dll + 0x2EA418, &verbose, 1);
    }
}
