#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>

#include "NVShaderPerf.h"
#include "NVShaderPerfPatch.h"

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
static NVShaderPerfQueryInterface NVShaderPerfLoader(const _TCHAR* folder)
{
    _TCHAR current[MAX_PATH];
    _TCHAR directory[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, current);
    GetCurrentDirectory(MAX_PATH, directory);
    _tcscat_s(directory, MAX_PATH, _T("\\"));
    _tcscat_s(directory, MAX_PATH, folder);
    SetCurrentDirectory(directory);

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

    SetCurrentDirectory(current);
    return NVSPQueryInterface;
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
    const char* dumpbin = nullptr;

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
            else if (strcmp(argv[i], "d3d_ps") == 0)
                shaderType = Direct3DPixelShader;
            else if (strcmp(argv[i], "d3d_vs") == 0)
                shaderType = Direct3DVertexShader;
            else if (strcmp(argv[i], "d3d_bin") == 0)
                shaderType = Direct3DByteCode;
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
        if (strcmp(arg, "-fo") == 0 || strcmp(arg, "-fileoutput") == 0) {
            if (++i == argc) {
                WriteString("Invalid Parameter\n");
                return 0;
            }
            fileOutputFilename = argv[i];
            continue;
        }
        if (strcmp(arg, "-dumpbin") == 0) {
            if (++i == argc) {
                WriteString("Invalid Parameter\n");
                return 0;
            }
            verbose = 100;
            dumpbin = argv[i];
            continue;
        }
        filename = arg;
    }

    if (filename == nullptr && dumpbin == nullptr) {
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
        WriteString("\t-fo/-fileoutput fileoutputfilename\n");
        WriteString("\t-dumpbin dumpbinfilename\n");
        return 0;
    }

    switch (NULL) case NULL: {
        const _TCHAR* version = _T("174.74");
        const _TCHAR* folder = _T("2.07.0804.1530");
        if (gpu) {
            if (gpu[2] == '3') {
                version = _T("101.31");
                folder = _T("2.01.10000.0305");
            }
            else if (_stricmp(gpu, "RSX") == 0) {
                version = _T("RSX Compiler");
                folder = _T("2.09.1109.0300");
            }
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
        PatchRSX(verbose);

        if (shaderType == Direct3DByteCode) {
            FILE* file = nullptr;
            fopen_s(&file, filename, "rb");
            if (file) {
                int version = 0;
                fread(&version, sizeof(int), 1, file);
                fclose(file);

                switch (version & 0xFFFF0000) {
                case 0xFFFF0000: shaderType = Direct3DPixelShader;  break;
                case 0xFFFE0000: shaderType = Direct3DVertexShader; break;
                }
            }
        }

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

        if (dumpbin) {
            FILE* file = nullptr;
            fopen_s(&file, dumpbin, "rb");
            if (file) {
                fseek(file, 0, SEEK_END);
                size_t count = ftell(file);
                fseek(file, 0, SEEK_SET);
                DWORD* data = (DWORD*)malloc(count);
                fread(data, 1, count, file);
                fclose(file);
                if (gpu) {
                    if (_stricmp(gpu, "NV30") == 0) {
                        if (shaderType == HLSLVertexProgram) {
                            DumpBinNV30VS((DWORD*)data, count);
                        }
                        else if (shaderType == HLSLFragmentProgram) {
//                          DumpBinNV40PS((DWORD*)data, count);
                            DumpBinG70PS((DWORD*)data, count);
                        }
                    }
                    if (_stricmp(gpu, "NV40") == 0) {
                        if (shaderType == HLSLVertexProgram) {
                            DumpBinNV40VS((DWORD*)data, count);
                        }
                        else if (shaderType == HLSLFragmentProgram) {
//                          DumpBinNV40PS((DWORD*)data, count);
                            DumpBinG70PS((DWORD*)data, count);
                        }
                    }
                    if (_stricmp(gpu, "G70") == 0) {
                        if (shaderType == HLSLVertexProgram) {
                            DumpBinNV40VS((DWORD*)data, count);
                        }
                        else if (shaderType == HLSLFragmentProgram) {
                            DumpBinG70PS((DWORD*)data, count);
                        }
                    }
                    if (_stricmp(gpu, "G80") == 0) {
                        DumpBinG80((DWORD*)data);
                    }
                }
                free(data);
            }
            break;
        }

        switch (shaderType) {
        case HLSLVertexProgram: 
        case Direct3DVertexShader: 
        case GLSLVertexProgram: 
        case OpenGLVertexProgram: 
        case CgVertexProgram: {
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
            break;
        }
        case HLSLFragmentProgram: 
        case Direct3DPixelShader: 
        case GLSLFragmentProgram: 
        case OpenGLFragmentProgram: 
        case CgFragmentProgram: {
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
        default:
            break;
        }
    }

    return 0;
}
