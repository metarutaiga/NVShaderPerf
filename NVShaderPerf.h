#pragma once

enum NVSPResult
{
    PathNotFound = -4,
    NoImplementation = -3,
    LibraryNotFound = -2,
    Error = -1,
    OK = 0,
    ErrorInvalidParameter = 1,
    InternalError = 2,
    CompileError = 3,
    BadArgumentsError = 4,
    TypeUnknownError = 5,
    FileNotFound = 6,
    UnsupportedShaderType = 7,
    GPUPerformanceUnsupported = 8,
    InvalidOptionsError = 9,
    FailureOpeningOutputFile = 10,
    GPUDriverNotSupported = 11,
    ShaderTypeMismatch = 12,
    CompileProfileInvalid = 13,
    LoopsNotSupported = 14,
    WarningEnumerationEndedEarly = 18,
    NoPixelShader = 8192,
    NoVertexShader = 8193,
};

enum ShaderPerformanceFlags
{
    VerboseOutput   = 0x0001,
    DepthReplace    = 0x0002,
    Color16         = 0x0004,
    AddsRGB         = 0x0008,
    ExpFog          = 0x0010,
    Exp2Fog         = 0x0020,
    LinearFog       = 0x0040,
    MinPrecision    = 0x0080,
    MaxPrecision    = 0x0100,
    VideoMode       = 0x0200,
    MinBranch       = 0x0400,
    SystemMemory    = 0x0800,
    ShowAssembler   = 0x1000,
    None            = 0x2000,
};

struct FragmentProgramResults
{
    char dummy[264];
    const char* GPU;
    const char* Driver;
    ShaderPerformanceFlags Flags;
    const char* AssemblyCode;
    const char* OutputText;
    unsigned int Cycles;
    unsigned int Registers;
    unsigned int Unknown;
    unsigned long long PixelThroughput;
};

struct VertexProgramResults
{
    char dummy[264];
    const char* GPU;
    const char* Driver;
    ShaderPerformanceFlags Flags;
    const char* AssemblyCode;
    const char* OutputText;
    unsigned int Cycles;
    unsigned int Registers;
    unsigned int Unknown;
    unsigned long long VertexThroughput;
};

enum ShaderType
{
//  Unknown = 0,
    HLSLVertexProgram = 1,
    HLSLFragmentProgram = 2,
    Direct3DPixelShader = 3,
    Direct3DVertexShader = 4,
    Direct3DByteCode = 5,
    GLSLVertexProgram = 6,
    GLSLFragmentProgram = 7,
    OpenGLFragmentProgram = 8,
    OpenGLVertexProgram = 9,
    CgVertexProgram = 10,
    CgFragmentProgram = 11,
    CgBytecode = 12,
};

enum ValueType
{
    BoolConstantType = 0,
    BoolConstantValue = 1,
    MRTCount = 2,
    BytecodeText = 3,
    BytecodeBinary = 4,
    OutputFile = 5,
    ErrorFile = 6,
    ShaderType = 7,
    FunctionName = 8,
    TechniqueName = 9,
    PassName = 10,
    ShaderTarget = 11,
    CgOptions = 12,
    GPUName = 13,
    DriverName = 14,
    IncludePath = 15,
    TextureDimention = 16,
    Flags = 17,
    CGCPath = 18,
    DIFF_SHADERTYPE = 19,
    DIFF_FUNCTIONNAME_PTR = 20,
    DIFF_TECHNIQUENAME_PTR = 21,
    DIFF_PASSNAME_PTR = 22,
    DIFF_SHADERTARGET_PTR = 23,
    DIFF_CG_OPTIONS_PTR = 24,
    DIFF_INCLUDEPATH_PTR = 25,
    MINRAND = 26,
    MAXRAND = 27,
    ERRTHRESH = 28,
    RANDSEED = 29,
    CONST_RANGE = 30,
    ATTRIB_RANGE = 31,
    TEXTURE_RANGE = 32,
    CHVALID = 33,
    PRINTDIFFPASS = 34,
    VP_CONDITIONAL_PRESETS = 35,
    TEXTURE_WIDTH = 36,
    TEXTURE_SPLIT = 37,
    HLSL_OPTIONS_PTR = 38,
    DIFF_HLSL_OPTIONS_PTR = 39,
    D3DXVersion = 40,
    DefinesPointer = 41,
    DIFF_DEFINES_PTR = 42,
};
