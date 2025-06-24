!!SPA1.0
.CONST_MODE  PAGE
.THREAD_TYPE VERTEX
.MAX_REG      19
.MAX_IBUF      9
.MAX_OBUF      9
# 
#---------------------------------------------------------------------------------
#  Single local light with local viewer, no attenuation
#---------------------------------------------------------------------------------
#  DP3 R2.x,v[2],c[72];
#  DP3 R2.y,v[2],c[73];
#  DP3 R2.z,v[2],c[74];
#
#  DP4 R3.w,v[0],c[71];
#  DP4 R3.x,v[0],c[68];
#  DP4 R3.y,v[0],c[69];
#  DP4 R3.z,v[0],c[70];
#  RCP R1.w,R3.w;
#
#  DP3 R2.w,R2,R2;
#  RSQ R2.w,R2.w;
#  MUL R2.xyz,R2.w,R2;
#
#  MAD R0,-R3,R1.w,c[116]; 
#  MAD R4,-R3,R1.w,c[40];
#  DP3 R0.w,R0,R0;
#  DP3 R4.w,R4,R4;
#  RSQ R1.x,R0.w; 
#  RSQ R1.y,R4.w;
#  MUL R0,R0,R1.x;
#  MUL R4,R4,R1.y;
#
#  ADD R3,R4,R0;
#  DP3 R3.w,R3,R3;
#  RSQ R3.w,R3.w;
#  MUL R3,R3.w,R3;
#
#  DP3 R3.x,R2,R4;    *saturate(0,1)*
#  DP3 R4.y,R2,R3;    *saturate(0,1)*
#
#  LG2 R4.y,R4.y;
#  MUL R4.y,R4.y,c[208];
#  EX2 R3.y,R4.y;
#
#  MOV R1,c[204];
#  MAD R1.xyz,R3.x,c[157],R1; 
#  MAD R1.xyz,R3.y,c[158],R1; 
#
#  DP4 R0.x,v[0],c[61];
#  DP4 R0.y,v[0],c[62];
#  DP4 R0.z,v[0],c[60];
#  DP4 R0.w,v[0],c[63];
#
BB0:
   FMUL   R8 , v[3],c[0];
   FMUL   R9 , v[3],c[1];
   FMUL   R10, v[3],c[2];
   FMAD   R8 , v[4],c[3], R8;
   FMAD   R9 , v[4],c[4], R9;
   FMAD   R10, v[4],c[5], R10;
   FMAD   R8 , v[5],c[6], R8;
   FMAD   R9 , v[5],c[7], R9;
   FMAD   R10, v[5],c[8], R10;

   FMUL   R12, v[0],c[16];
   FMUL   R13, v[0],c[17];
   FMUL   R14, v[0],c[18];
   FMUL   R15, v[0],c[19];
   FMAD   R12, v[1],c[20],R12;
   FMAD   R13, v[1],c[21],R13;
   FMAD   R14, v[1],c[22],R14;
   FMAD   R15, v[1],c[23],R15;
   FMAD   R12, v[2],c[24],R12;
   FMAD   R13, v[2],c[25],R13;
   FMAD   R14, v[2],c[26],R14;
   FMAD   R15, v[2],c[27],R15;
   FMAD   R12, v[2],c[28],R12;
   FMAD   R13, v[2],c[29],R13;
   FMAD   R14, v[2],c[30],R14;
   FMAD   R15, v[2],c[31],R15;
   RCP    R7 , R15;

   FMUL   R11, R8, R8;
   FMAD   R11, R9, R9 ,R11;
   FMAD   R11, R10,R10,R11;
   RSQ    R11, R11;
   FMUL   R8 , R8, R11;
   FMUL   R9 , R9, R11;
   FMUL   R10, R10,R11;

   FMAD   R0 , R12, R7 ,c[32];
   FMAD   R1 , R13, R7 ,c[33];
   FMAD   R2 , R14, R7 ,c[34];
   FMAD   R16, R12, R7 ,c[36];
   FMAD   R17, R13, R7 ,c[37];
   FMAD   R18, R14, R7 ,c[38];

   FMUL   R3 , R0 , R0;
   FMUL   R19, R16, R16;
   FMAD   R3 , R1 , R1,  R3;
   FMAD   R19, R17, R17, R19;
   FMAD   R3 , R2 , R2,  R3;
   FMAD   R19, R18, R18, R19;
   RSQ    R3 , R3;
   RSQ    R19, R19;
   FMUL   R0 , R0 , R3;
   FMUL   R1 , R1 , R3;
   FMUL   R2 , R2 , R3;
   FMUL   R16, R16, R19;
   FMUL   R17, R17, R19;
   FMUL   R18, R18, R19;

   FADD   R12, R16, R0;
   FADD   R13, R17, R1;
   FADD   R14, R18, R2;
   FMUL   R15, R12, R12;
   FMAD   R15, R13, R13, R15;
   FMAD   R15, R14, R14, R15;
   RSQ    R15, R15;
   FMUL   R12, R12, R15;
   FMUL   R13, R13, R15;
   FMUL   R14, R14, R15;

   FMUL      R0 , R8 , R16;
   FMUL      R1 , R8 , R12;
   FMAD      R0 , R9 , R17, R0;
   FMAD      R1 , R9 , R13, R1;
   FMAD.SAT  R0 , R10, R18, R0;
   FMAD.SAT  R1 , R10, R14, R1;

   LG2    R1 , R1;
   FMUL   R1 , R1,c[40];
   EX2    R1 , R1;

   MOV    R4 ,c[44];
   MOV    R5 ,c[45];
   MOV    R6 ,c[46];
   MOV    R7 ,c[47];
   FMAD   R4 , R0 , c[48], R4;
   FMAD   R5 , R0 , c[49], R5;
   FMAD   R6 , R0 , c[50], R6;
   FMAD   o[4] , R1 , c[52], R4;
   FMAD   o[5] , R1 , c[53], R5;
   FMAD   o[6] , R1 , c[54], R6;
   FMAD   o[7] , R1 , R1, R1;
   
   FMUL   R0, v[0],c[64];
   FMUL   R1, v[0],c[65];
   FMUL   R2, v[0],c[66];
   FMUL   R3, v[0],c[67];
   FMAD   R0, v[1],c[68],R0;
   FMAD   R1, v[1],c[69],R1;
   FMAD   R2, v[1],c[70],R2;
   FMAD   R3, v[1],c[71],R3;
   FMAD   R0, v[2],c[72],R0;
   FMAD   R1, v[2],c[73],R1;
   FMAD   R2, v[2],c[74],R2;
   FMAD   R3, v[2],c[75],R3;
   FMAD   o[0], v[2],c[76],R0;
   FMAD   o[1], v[2],c[77],R1;
   FMAD   o[2], v[2],c[78],R2;
   FMAD   o[3], v[2],c[79],R3;
   MOV    o[8], R0;
   MOV    o[9], R1;
END
#
