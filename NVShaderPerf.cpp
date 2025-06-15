#include <Windows.h>
#include <stdio.h>

#define USE_CONSOLE 1

#if USE_CONSOLE
#define WriteString(v) printf("%s", v)
#else
#define WriteString OutputDebugStringA
#endif

#include "NVShaderPerf.h"

union NVShaderPerf {
    struct {
        NVSPResult (*NVSPInit)(NVSPResult* result);
        NVSPResult (*Unknown__1)();
        NVSPResult (*EnumerateGPUs)(const void* version, int (*callback)(const char* version, const char* gpuLong, const char* gpuShort, void* userData), void* userData);
        NVSPResult (*EnumerateDrivers)(const void* gpuShort, int (*callback)(const char* gpuShort, const char* version, void* userData), void* userData);
        NVSPResult (*Unknown__4)();
        NVSPResult (*SetValue)(ValueType type, DWORD value);
        NVSPResult (*SetValuePtr)(ValueType type, const void* value);
        NVSPResult (*Unknown__7)();
        NVSPResult (*SetOutputFunction)(void (*function)(const char* text));
        NVSPResult (*FragmentProgramPerformance)(const char* program, int flags, FragmentProgramResults** results, int* count);
        NVSPResult (*FreeFragmentResults)(FragmentProgramResults* results);
        NVSPResult (*Unknown_10)();
        NVSPResult (*VertexProgramPerformance)(const char* program, int flags, VertexProgramResults** results, int* count);
        NVSPResult (*FreeVertexResults)(VertexProgramResults* results);
        DWORD hashes[14];
    };
    void* functions[14];
} NVShaderPerf = {
    .hashes = {
        0x955d95d9, // NVSPInit
        0x7dc41d69, //
        0xfd79cb32, // EnumerateGPUs
        0x28ce67fe, // EnumerateDrivers
        0x58a4635e, //
        0xe8717fc5, // SetValue
        0xef679b71, // SetValuePtr
        0xdb31c1a0, //
        0x5d89fc44, // SetOutputFunction
        0x85e162e9, // FragmentProgramPerformance
        0x86e853ae, // FreeFragmentResults
        0x63021cc1, //
        0x31f286c9, // VertexProgramPerformance
        0x5fd8ee9c, // FreeVertexResults
    },
};

typedef void* (*NVShaderPerfQueryInterface)(DWORD hash);
static NVShaderPerfQueryInterface NVShaderPerfLoader(const wchar_t* folder)
{
    wchar_t current[MAX_PATH];
    wchar_t directory[MAX_PATH];
    GetCurrentDirectoryW(MAX_PATH, current);
    GetCurrentDirectoryW(MAX_PATH, directory);
    wcscat_s(directory, MAX_PATH, L"\\");
    wcscat_s(directory, MAX_PATH, folder);
    SetCurrentDirectoryW(directory);

    HMODULE dll = LoadLibraryA("NVShaderPerf.dll");
    if (dll == nullptr) {
        WriteString("NVShaderPerf.dll is not found!\n");
        return nullptr;
    }
    NVShaderPerfQueryInterface NVSPQueryInterface;
    (void*&)NVSPQueryInterface = GetProcAddress(dll, "NVSPQueryInterface");
    if (NVSPQueryInterface == nullptr) {
        WriteString("NVSPQueryInterface is not found!\n");
        return nullptr;
    }
    NVSPResult(*NVSPLoader)();
    (void*&)NVSPLoader = NVSPQueryInterface(0x3A3AE0A4);
    if (NVSPLoader == nullptr) {
        WriteString("0x3A3AE0A4 is not found!\n");
        return nullptr;
    }
    if (NVSPLoader() < 0) {
        WriteString("NVShaderPerf_XXYY.dll is not found!\n");
        return nullptr;
    }

    SetCurrentDirectoryW(current);
    return NVSPQueryInterface;
}

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

static void WriteCode(void* pvTarget, const void* pvSource, size_t nSize)
{
    DWORD nOldP, nNewP;
    VirtualProtect(pvTarget, nSize, PAGE_READWRITE, &nOldP);
    WriteProcessMemory(GetCurrentProcess(), pvTarget, pvSource, nSize, NULL);
    VirtualProtect(pvTarget, nSize, nOldP, &nNewP);
}

static void Patch10131(int verbose)
{
    HMODULE dll = GetModuleHandleA("NVShaderPerf_10131.dll");
    if (dll) {
        // 0D 00 70 00 00
        WriteCode((char*)dll + 0x4054, "\x0D\x00\x70\x00\x00", 5);

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

static void Patch17474(int verbose)
{
    HMODULE dll = GetModuleHandleA("NVShaderPerf_17474.dll");
    if (dll) {
        unsigned int flags = 0x00007000;
        WriteCode((char*)dll + 0x3E706, &flags, 4);

        // COPP
        // 6A 01
        // 5D
        // 8B C5
        // 89 86 C0 01 00 00
        WriteCode((char*)dll + 0xF44EA, "\x6A\x01\x5D\x8B\xC5\x89\x86\xC0\x01\x00\x00", 11);

        // vp50_ucode
        // 6A 01
        // 58
        // 89 87 C8 00 00 00
        WriteCode((char*)dll + 0x1C4B5D, "\x6A\x01\x58\x89\x87\xC8\x00\x00\x00", 9);

        // fp50_ucode
        // 6A 01
        // 58
        // 89 86 C8 00 00 00
        // 57
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

#if USE_CONSOLE
int main(int argc, char* argv[])
#else
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
#endif
{
    WriteString("NVShaderPerf : version 2.0, build date " __DATE__ ", " __TIME__ "\n");
    WriteString("Copyright (C) 2002 - 2006, NVIDIA Corporation\n");

    int mrt = 0;
    const char* output = nullptr;
    const char* error = nullptr;
    enum ShaderType shaderType = (enum ShaderType)0;
    const char* function = nullptr;
    const char* technique = nullptr;
    const char* pass = nullptr;
    const char* profile = nullptr;
    const char* gpu = nullptr;
    const char* include = nullptr;
    const char* cgcpath = nullptr;
    int verbose = 0;
    ShaderPerformanceFlags flags = ShaderPerformanceFlags();
    const char* filename = nullptr;
    bool listgpus = false;
    bool listdrivers = false;

    // help
    for (int i = 1; i < argc; ++i) {
        const char* arg = argv[i];
        if (strcmp(arg, "-h") == 0 || strcmp(arg, "-help") == 0) {
            filename = nullptr;
            break;
        }
        if (strcmp(arg, "-m") == 0 || strcmp(arg, "-mrt") == 0) {
            if (++i == argc) {
                WriteString("Invalid Parameter\n");
                return 0;
            }
            mrt = atoi(argv[i]);
            continue;
        }
        if (strcmp(arg, "-o") == 0 || strcmp(arg, "-output") == 0) {
            if (++i == argc) {
                WriteString("Invalid Parameter\n");
                return 0;
            }
            output = argv[i];
            continue;
        }
        if (strcmp(arg, "-e") == 0 || strcmp(arg, "-error") == 0) {
            if (++i == argc) {
                WriteString("Invalid Parameter\n");
                return 0;
            }
            error = argv[i];
            continue;
        }
        if (strcmp(arg, "-type") == 0) {
            if (++i == argc) {
                WriteString("Invalid Parameter\n");
                return 0;
            }
            if (strcmp(argv[i], "hlsl_vs") == 0)
                shaderType = HLSLVertexProgram;
            else if (strcmp(argv[i], "hlsl_ps") == 0)
                shaderType = HLSLFragmentProgram;
            continue;
        }
        if (strcmp(arg, "-f") == 0 || strcmp(arg, "-function") == 0) {
            if (++i == argc) {
                WriteString("Invalid Parameter\n");
                return 0;
            }
            function = argv[i];
            continue;
        }
        if (strcmp(arg, "-t") == 0 || strcmp(arg, "-technique") == 0) {
            if (++i == argc) {
                WriteString("Invalid Parameter\n");
                return 0;
            }
            technique = argv[i];
            continue;
        }
        if (strcmp(arg, "-p") == 0 || strcmp(arg, "-pass") == 0) {
            if (++i == argc) {
                WriteString("Invalid Parameter\n");
                return 0;
            }
            pass = argv[i];
            continue;
        }
        if (strcmp(arg, "-profile") == 0) {
            if (++i == argc) {
                WriteString("Invalid Parameter\n");
                return 0;
            }
            profile = argv[i];
            continue;
        }
        if (strcmp(arg, "-g") == 0 || strcmp(arg, "-gpu") == 0) {
            if (++i == argc) {
                WriteString("Invalid Parameter\n");
                return 0;
            }
            gpu = argv[i];
            continue;
        }
        if (strcmp(arg, "-include") == 0) {
            if (++i == argc) {
                WriteString("Invalid Parameter\n");
                return 0;
            }
            include = argv[i];
            continue;
        }
        if (strcmp(arg, "-cgcpath") == 0) {
            if (++i == argc) {
                WriteString("Invalid Parameter\n");
                return 0;
            }
            cgcpath = argv[i];
            continue;
        }
        if (strcmp(arg, "-verbose") == 0) {
            if (++i == argc) {
                WriteString("Invalid Parameter\n");
                return 0;
            }
            verbose = atoi(argv[i]);
            flags = ShaderPerformanceFlags(flags | (verbose ? VerboseOutput : 0));
            continue;
        }
        if (strcmp(arg, "-z") == 0 || strcmp(arg, "-zreplace") == 0) {
            flags = ShaderPerformanceFlags(flags | DepthReplace);
            continue;
        }
        if (strcmp(arg, "-color16") == 0) {
            flags = ShaderPerformanceFlags(flags | Color16);
            continue;
        }
        if (strcmp(arg, "-minbranch") == 0) {
            flags = ShaderPerformanceFlags(flags | MinBranch);
            continue;
        }
        if (strcmp(arg, "-asm") == 0) {
            flags = ShaderPerformanceFlags(flags | ShowAssembler);
            continue;
        }
        if (strcmp(arg, "-listgpus") == 0) {
            listgpus = true;
            filename = "";
            continue;
        }
        if (strcmp(arg, "-listdrivers") == 0) {
            listdrivers = true;
            filename = "";
            continue;
        }
        if (strcmp(arg, "-allprec") == 0) {
            continue;
        }
        if (strcmp(arg, "-allbranch") == 0) {
            continue;
        }
        if (strcmp(arg, "-subdirs") == 0) {
            continue;
        }
        filename = arg;
    }

    if (filename == nullptr) {
        WriteString("\t-h/-help argname\n");
        WriteString("\t-m/-mrt count\n");
        WriteString("\t-o/-output outputfilename\n");
        WriteString("\t-e/-error errorfilename\n");
        WriteString("\t-type filetype\n");
        WriteString("\t-f/-function funcname\n");
        WriteString("\t-t/-technique techid\n");
        WriteString("\t-p/-pass passid\n");
        WriteString("\t-profile targetprofile\n");
        WriteString("\t-g/-gpu gpuname\n");
        WriteString("\t-include includepath\n");
        WriteString("\t-cgcpath path\n");
        WriteString("\t-v/-verbose level\n");
        WriteString("\t-z/-zreplace\n");
        WriteString("\t-color16\n");
        WriteString("\t-minbranch\n");
        WriteString("\t-asm\n");
        WriteString("\t-listgpus\n");
        WriteString("\t-listdrivers\n");
        WriteString("\t-allprec\n");
        WriteString("\t-allbranch\n");
        WriteString("\t-subdirs\n");
        return 0;
    }

    switch (NULL) case NULL: {
        const char* version = "174.74";
        const wchar_t* folder = L"2.07.0804.1530";
        if (gpu && gpu[2] == '3') {
            version = "101.31";
            folder = L"2.01.10000.0305";
        }
        NVShaderPerfQueryInterface NVSPQueryInterface = NVShaderPerfLoader(folder);
        if (NVSPQueryInterface == nullptr)
            break;
        for (int i = 0; i < 14; ++i) {
            NVShaderPerf.functions[i] = NVSPQueryInterface(NVShaderPerf.hashes[i]);
        }
        NVSPResult unknown;
        NVSPResult result;
        result = NVShaderPerf.NVSPInit(&unknown);
        result = NVShaderPerf.SetOutputFunction([](const char* text) {
            WriteString(text);
            size_t length = strlen(text);
            if (length && text[length - 1] != '\n')
                WriteString("\n");
        });

        if (listgpus) {
            result = NVShaderPerf.EnumerateGPUs(nullptr, [](const char* version, const char* gpuLong, const char* gpuShort, void* userData) -> int {
                char temp[256];
                snprintf(temp, 256, "%s : %s\n", gpuLong, gpuShort);
                WriteString(temp);
                return 1;
            }, nullptr);
            break;
        }
        if (listdrivers) {
            result = NVShaderPerf.EnumerateDrivers(nullptr, [](const char* gpuShort, const char* version, void* userData) -> int {
                char temp[256];
                snprintf(temp, 256, "%s\n", version);
                WriteString(temp);
                return 1;
            }, nullptr);
            break;
        }

        Patch10131(verbose);
        Patch17474(verbose);

        if (mrt)        result = NVShaderPerf.SetValue(MRTCount, mrt);
        if (output)     result = NVShaderPerf.SetValuePtr(OutputFile, output);
        if (error)      result = NVShaderPerf.SetValuePtr(ErrorFile, error);
        if (shaderType) result = NVShaderPerf.SetValue(ShaderType, shaderType);
        if (function)   result = NVShaderPerf.SetValuePtr(FunctionName, function);
        if (technique)  result = NVShaderPerf.SetValuePtr(TechniqueName, technique);
        if (pass)       result = NVShaderPerf.SetValuePtr(PassName, pass);
        if (profile)    result = NVShaderPerf.SetValuePtr(ShaderTarget, profile);
        if (gpu)        result = NVShaderPerf.SetValuePtr(GPUName, gpu);
        if (include)    result = NVShaderPerf.SetValuePtr(IncludePath, include);
        if (cgcpath)    result = NVShaderPerf.SetValuePtr(CGCPath, cgcpath);
        if (flags)      result = NVShaderPerf.SetValue(Flags, flags);

        for (int i = 0; i < 2; ++i) {
            if (i == 0) {
                int count = 0;
                VertexProgramResults* results = nullptr;
                result = NVShaderPerf.VertexProgramPerformance(filename, 0, &results, &count);
                for (int i = 0; i < count; ++i) {
                    char temp[256];
                    snprintf(temp, 256, "%sVertex Performance Setup: Driver %s, GPU %s, Flags 0x%X\n", "", results[i].Driver, results[i].GPU, results[i].Flags);
                    snprintf(temp, 256, "%sResults %d cycles, %d r regs, %lld vertices/s\n", temp, results[i].Cycles, results[i].Registers, results[i].VertexThroughput);
                    WriteString(temp);
                }
                result = NVShaderPerf.FreeVertexResults(results);
            }
            else if (i == 1) {
                int count = 0;
                FragmentProgramResults* results = nullptr;
                result = NVShaderPerf.FragmentProgramPerformance(filename, 0, &results, &count);
                for (int i = 0; i < count; ++i) {
                    char temp[256];
                    snprintf(temp, 256, "%sFragment Performance Setup: Driver %s, GPU %s, Flags 0x%X\n", "", results[i].Driver, results[i].GPU, results[i].Flags);
                    snprintf(temp, 256, "%sResults %d cycles, %d r regs, %lld pixels/s\n", temp, results[i].Cycles, results[i].Registers, results[i].PixelThroughput);
                    WriteString(temp);
                }
                result = NVShaderPerf.FreeFragmentResults(results);
            }
        }
    }

    return 0;
}
