!!SPA1.0
.CONST_MODE  PAGE
.THREAD_TYPE PIXEL
.MAX_REG      9
.MAX_ATTR     7
# 
BB0:
IPA      R8, f[0];
RCP      R8, R8;
IPA      R4, f[1], R8;
IPA      R5, f[2], R8;
TEX      R4, 0, 0, 2D, RGBA;
MVC      R9, c[8];
IPA      R0, f[3], R8;
IPA      R1, f[4], R8;
IPA      R2, f[5], R8;
IPA      R3, f[6], R8;
IPA      R8, f[7], R8;
FMAD     R0, R0, R4, c[4];
FMAD     R1, R1, R5, c[5];
FMAD     R2, R2, R6, c[6];
FMAD.SAT R8, R8, c[9], R9;
FMUL     R3, R3, R7;
FMAD     R0, R8, R0, c[4];
FMAD     R1, R8, R1, c[5];
FMAD     R2, R8, R2, c[6];
END
#
