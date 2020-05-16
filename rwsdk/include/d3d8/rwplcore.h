/******************************************/
/*                                        */
/*    RenderWare(TM) Graphics Library     */
/*                                        */
/******************************************/

/*
 * This file is a product of Criterion Software Ltd.
 *
 * This file is provided as is with no warranties of any kind and is
 * provided without any obligation on Criterion Software Ltd.
 * or Canon Inc. to assist in its use or modification.
 *
 * Criterion Software Ltd. and Canon Inc. will not, under any
 * circumstances, be liable for any lost revenue or other damages
 * arising from the use of this file.
 *
 * Copyright (c) 1999. Criterion Software Ltd.
 * All Rights Reserved.
 */

/*************************************************************************
 *
 * Filename: <C:/daily/rwsdk/include/d3d8/rwplcore.h>
 * Automatically Generated on: Wed Jul 10 10:45:00 2002
 *
 ************************************************************************/

#ifndef RWPLCORE_H
#define RWPLCORE_H

/*--- System Header Files ---*/
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>


/*--- Automatically derived from: C:/daily/rwsdk/os/win/ostypes.h ---*/
#ifndef WIN_OSTYPES_H
#define WIN_OSTYPES_H

#define rwLITTLEENDIAN         /* This is a little endian machine */

typedef long RwFixed;
typedef int  RwInt32;
typedef unsigned int RwUInt32;
typedef short RwInt16;
typedef unsigned short RwUInt16;
typedef unsigned char RwUInt8;
typedef signed char RwInt8;

#ifdef RWUNICODE
typedef wchar_t RwChar;
#else /* RWUNICODE */
typedef char RwChar;
#endif /* RWUNICODE */
typedef float RwReal;
typedef RwInt32 RwBool;

#ifdef _MSC_VER
typedef __int64 RwInt64;
typedef unsigned __int64 RwUInt64;
#define RWZERO64 ((RwUInt64)0)
#else /* _MSC_VER */

typedef struct _RwUInt64 RwUInt64;
typedef struct _RwInt64 RwInt64;

/* We'll do it with structures (can't do maths on these, but OK for allocation): */
#ifdef rwBIGENDIAN
struct _RwUInt64
{
    RwUInt32 top;
    RwUInt32 bottom;
};

struct _RwInt64
{
    RwInt32 top;
    RwUInt32 bottom;
};

#else /* rwBIGENDIAN */
#ifdef rwLITTLEENDIAN
struct _RwUInt64
{
    RwUInt32 bottom;
    RwUInt32 top;
};

struct _RwInt64
{
    RwUInt32 bottom;
    RwInt32 top;
};

#else /* rwLITTLEENDIAN */
#error "ENDIAN-ness undefined!"
#endif /* rwLITTLEENDIAN */
#endif /* rwBIGENDIAN */

#define RWZERO64 { (RwUInt32)0, (RwUInt32)0 }
#endif /* _MSC_VER */

typedef struct _RwUInt128 RwUInt128;
typedef struct _RwInt128 RwInt128;

/* We'll do it with structures
 * (can't do maths on these, but OK for allocation): */
#ifdef rwBIGENDIAN
struct _RwUInt128
{
    RwUInt64 top;
    RwUInt64 bottom;
};

struct _RwInt128
{
    RwInt64 top;
    RwUInt64 bottom;
};

#else /* rwBIGENDIAN */
#ifdef rwLITTLEENDIAN
struct _RwUInt128
{
    RwUInt64 bottom;
    RwUInt64 top;
};

struct _RwInt128
{
    RwUInt64 bottom;
    RwInt64 top;
};

#else /* rwLITTLEENDIAN */
#error "ENDIAN-ness undefined!"
#endif /* rwLITTLEENDIAN */
#endif /* rwBIGENDIAN */

#define RWZERO128 { RWZERO64, RWZERO64 }

/* Limits of types */
#define RwInt32MAXVAL       0x7FFFFFFF
#define RwInt32MINVAL       0x80000000
#define RwUInt32MAXVAL      0xFFFFFFFF
#define RwUInt32MINVAL      0x00000000
#define RwRealMAXVAL        (RwReal)(3.40282347e+38)
#define RwRealMINVAL        (RwReal)(1.17549435e-38)
#define RwInt16MAXVAL       0x7FFF
#define RwInt16MINVAL       0x8000
#define RwUInt16MAXVAL      0xFFFF
#define RwUInt16MINVAL      0x0000

/* Structure alignment */
#define RWALIGN(type, x)   type /* nothing */
#define rwMATRIXALIGNMENT sizeof(RwUInt32)
#define rwFRAMEALIGNMENT sizeof(RwUInt32)
#define rwV4DALIGNMENT sizeof(RwUInt32)

#if (defined(_MSC_VER))

#if (defined(RWVERBOSE))
#include <tchar.h>
#pragma comment (lib , "advapi32.LIB") /* Registry functions */

/*
 * registry code
 */

#if (defined(RpWinRegGetDWordValue))
#undef RpWinRegGetDWordValue
#endif /* (defined(RpWinRegGetDWordValue)) */

#define RpWinRegGetDWordValue(_result, _hKey, _name, _val)              \
MACRO_START                                                             \
{                                                                       \
    DWORD               _size;                                          \
    DWORD               _type;                                          \
    LONG                _status;                                        \
                                                                        \
    _status =                                                           \
        RegQueryValueEx((_hKey), (_name), 0, &_type, NULL, &_size);     \
    (_result) = ((ERROR_SUCCESS == _status) && (REG_DWORD == _type));   \
                                                                        \
    if ((_result))                                                      \
    {                                                                   \
        _status =                                                       \
            RegQueryValueEx((_hKey), (_name), 0, &_type,                \
                            (BYTE *) (_val), &_size);                   \
        (_result) = (ERROR_SUCCESS == _status);                         \
    }                                                                   \
}                                                                       \
MACRO_STOP

#if (defined(RpWinRegGetBinaryValue))
#undef RpWinRegGetBinaryValue
#endif /* (defined(RpWinRegGetBinaryValue)) */

#define RpWinRegGetBinaryValue(_result, _hKey, _name, _val)             \
MACRO_START                                                             \
{                                                                       \
    DWORD               _size;                                          \
    DWORD               _type;                                          \
    LONG                _status;                                        \
                                                                        \
    _status =                                                           \
        RegQueryValueEx((_hKey), (_name), 0, &_type, NULL, &_size);     \
    (_result) =                                                         \
        ((ERROR_SUCCESS == _status) &&                                  \
         (REG_BINARY == _type) && (0 < _size));                         \
                                                                        \
    if ((_result))                                                      \
    {                                                                   \
        *(_val) = RwMalloc(sizeof(BYTE) * _size);                       \
        (_result) = (NULL != *(_val));                                  \
                                                                        \
        if ((_result))                                                  \
        {                                                               \
                                                                        \
            _status =                                                   \
                RegQueryValueEx((_hKey),                                \
                                (_name), 0, &_type,                     \
                                (BYTE *) * (_val), &_size);             \
            (_result =) (ERROR_SUCCESS == _status);                     \
                                                                        \
            if (!(_result))                                             \
            {                                                           \
                RwFree(*(_val));                                        \
                *(_val) = NULL;                                         \
            }                                                           \
                                                                        \
        }                                                               \
                                                                        \
    }                                                                   \
}                                                                       \
MACRO_STOP

#if (defined(RpWinRegGetStringValue))
#undef RpWinRegGetStringValue
#endif /* (defined(RpWinRegGetStringValue)) */

#define RpWinRegGetStringValue(_result, _hKey, _name, _val)             \
MACRO_START                                                             \
{                                                                       \
    DWORD               _size;                                          \
    DWORD               _type;                                          \
    LONG                _status;                                        \
                                                                        \
    _status =                                                           \
        RegQueryValueEx((_hKey), (_name), 0, &_type, NULL, &_size);     \
    (_result) =                                                         \
        ((ERROR_SUCCESS == _status) &&                                  \
         (REG_SZ == _type) && (0 < _size));                             \
                                                                        \
    if ((_result))                                                      \
    {                                                                   \
                                                                        \
        *(_val) = RwMalloc(sizeof(TCHAR) * _size);                      \
        (_result) = (NULL != *(_val));                                  \
                                                                        \
        if ((_result))                                                  \
        {                                                               \
            _status =                                                   \
                RegQueryValueEx((_hKey), (_name), 0, &_type,            \
                                (BYTE *) * (_val), &_size);             \
            (_result) = (ERROR_SUCCESS == _status);                     \
                                                                        \
            if (!(_result))                                             \
            {                                                           \
                RwFree(*(_val));                                        \
                *(_val) = NULL;                                         \
            }                                                           \
        }                                                               \
    }                                                                   \
}                                                                       \
MACRO_STOP

/* ------------------------------------------------------------------- */

#define RpWinRegCloseKey(hKey)                  \
MACRO_START                                     \
{                                               \
    RegCloseKey(hKey);                          \
}                                               \
MACRO_STOP

/* ------------------------------------------------------------------- */

#define RpWinRegOpenMachineKey(result)                          \
MACRO_START                                                     \
{                                                               \
    static const TCHAR  RenderWareKey[] =                       \
        "Software\\Criterion\\RenderWare";                      \
    DWORD               disposition;                            \
    LONG                status =                                \
        RegCreateKeyEx(HKEY_LOCAL_MACHINE, RenderWareKey, 0,    \
                       REG_NONE, REG_OPTION_NON_VOLATILE,       \
                       KEY_READ | KEY_WRITE,                    \
                       NULL, &result, &disposition);            \
                                                                \
    if (status != ERROR_SUCCESS)                                \
    {                                                           \
        result = NULL;                                          \
    }                                                           \
}                                                               \
MACRO_STOP

/* ------------------------------------------------------------------- */

#if (defined(RWGETWINREGDWORD))
#undef RWGETWINREGDWORD
#endif /* (defined(RWGETWINREGDWORD)) */

#define RWGETWINREGDWORD(result, match)                 \
MACRO_START                                             \
{                                                       \
    HKEY                hKey;                           \
                                                        \
    RpWinRegOpenMachineKey(hKey);                       \
    if (hKey)                                           \
    {                                                   \
        RwBool              success;                    \
                                                        \
        RpWinRegGetDWordValue(success, hKey, match,     \
                               &result);                \
                                                        \
        RpWinRegCloseKey(hKey);                         \
    }                                                   \
}                                                       \
MACRO_STOP

#if (defined(RWGETWINREGBINARY))
#undef RWGETWINREGBINARY
#endif /* (defined(RWGETWINREGBINARY)) */

#define RWGETWINREGBINARY(result, match)                \
MACRO_START                                             \
{                                                       \
    HKEY                hKey;                           \
                                                        \
    result = NULL;                                      \
    RpWinRegOpenMachineKey(hKey);                       \
    if (hKey)                                           \
    {                                                   \
        RwBool              success;                    \
                                                        \
        RpWinRegGetBinaryValue(success, hKey, match,    \
                                &result, NULL);         \
                                                        \
        if (!success)                                   \
            result = NULL;                              \
                                                        \
        RpWinRegCloseKey(hKey);                         \
    }                                                   \
}                                                       \
MACRO_STOP

#if (defined(RWGETWINREGSTRING))
#undef RWGETWINREGSTRING
#endif /* (defined(RWGETWINREGSTRING)) */

#define RWGETWINREGSTRING(result, match)                \
MACRO_START                                             \
{                                                       \
    HKEY                hKey;                           \
                                                        \
    result = NULL;                                      \
    RpWinRegOpenMachineKey(hKey);                       \
    if (hKey)                                           \
    {                                                   \
        RwBool              success;                    \
                                                        \
        RpWinRegGetStringValue(success, hKey, match,    \
                                &result);               \
                                                        \
        if (!success)                                   \
            result = NULL;                              \
                                                        \
        RpWinRegCloseKey(hKey);                         \
    }                                                   \
}                                                       \
MACRO_STOP

#if (defined(_DEBUG))

#if (defined(RWREGSETBREAKALLOC))
#undef RWREGSETBREAKALLOC
#endif /* (defined(RWREGSETBREAKALLOC)) */

#define RWREGSETBREAKALLOC(_name)                               \
MACRO_START                                                     \
{                                                               \
    char _message[256];                                         \
    long _lBreakAlloc = -1;                                     \
                                                                \
    RWGETWINREGDWORD(_lBreakAlloc, _name);                      \
                                                                \
    RWCRTSETBREAKALLOC(_lBreakAlloc);                           \
                                                                \
    _snprintf(_message, sizeof(_message),                       \
              "%s(%d): RWCRTSETBREAKALLOC(%ld)\n",              \
              __FILE__, __LINE__,                               \
              _lBreakAlloc);                                    \
    OutputDebugString(_message);                                \
                                                                \
}                                                               \
MACRO_STOP

#if (defined(RWREGSETDEBUGTRACE))
#undef RWREGSETDEBUGTRACE
#endif /* (defined(RWREGSETDEBUGTRACE)) */

#define RWREGSETDEBUGTRACE(_name)                     \
MACRO_START                                           \
{                                                     \
    char _message[256];                               \
    long _lDebugtrace = 0;                            \
                                                      \
    RWGETWINREGDWORD(_lDebugtrace, _name);            \
                                                      \
    RwDebugSetTraceState(_lDebugtrace);               \
                                                      \
    _snprintf(_message, sizeof(_message),             \
              "%s(%d): RwDebugSetTraceState(%ld)\n",  \
              __FILE__, __LINE__,                     \
              _lDebugtrace);                          \
    OutputDebugString(_message);                      \
                                                      \
}                                                     \
MACRO_STOP

#if (defined(_CRTDBG_FLAGS))
#undef _CRTDBG_FLAGS
#endif /* (defined(_CRTDBG_FLAGS)) */

#define _CRTDBG_FLAGS                                                           \
( _CRTDBG_ALLOC_MEM_DF || /* Turn on the debug heap allocations                 \
                           * and use the memory block identifiers.              \
                           * This is the only flag that's on by default. */     \
  _CRTDBG_CHECK_ALWAYS_DF || /* Check and validate all memory                   \
                              * on each allocation and deallocation request.    \
                              * Setting this flag on is what catches the        \
                              * under and overwrites                            \
                              * so it is very important to                      \
                              * get it turned on. */                            \
  _CRTDBG_CHECK_CRT_DF || /* Include _CRT_BLOCK memory allocations              \
                           * in all leak detection and state differences. */    \
  _CRTDBG_DELAY_FREE_MEM_DF || /* Instead of truly freeing memory,              \
                                * keep the block allocated and                  \
                                * in the internal heap list.                    \
                                * The blocks are filled with the value0xDD      \
                                * so you know the memory is freed when          \
                                * looking at it in the debugger.                \
                                * By also not freeing the memory,               \
                                * this can help provide stress                  \
                                * conditions for the program. */                \
  _CRTDBG_LEAK_CHECK_DF) /* Do memory leak checking at                          \
                          * the end of the program. */

#endif /* (defined(_DEBUG)) */
#endif /* (defined(RWVERBOSE)) */

#include <math.h>
/*
 * Keep true calls to these functions since
 * some x86 runtime libraries do not support _CIpow() etc
 */
#pragma function( acos, asin, cosh, fmod, pow, sinh , tanh ) 

#if (!defined(RWINT32FROMFLOAT))

static __inline RwInt32
int32fromreal(RwReal x)
{
    RwInt16 savemode;
    RwInt16 workmode;
    RwInt32 res;
    
    _asm
    {
        fnstcw    savemode      ; get fpu mode
        fld dword ptr[x]        ; load rwreal x  
  
        mov       ax,savemode   ; put fpu mode in register
        or        ah,0ch        ; or-in truncate mode
  
        mov       workmode,ax   ; make ready to set fpu mode
        fldcw     workmode      ; set fpu to truncate mode
        fistp     dword ptr[res]; store the rwint32eger result 
        fldcw     savemode      ; restore fpu mode
    }

    return res;
}
#define RwInt32FromRealMacro(x) int32fromreal(x)

#endif /* (!defined(RWINT32FROMFLOAT)) */

#if (!defined(NOASM))
static __inline RwUInt32 
RwFastRealToUInt32(RwReal x)
{
    RwUInt32 res;

    __asm FLD DWord Ptr[x];
    __asm FISTP DWord Ptr[res];
    
    return(res);
}
#endif /* (defined(NOASM)) */

#endif /* (defined(_MSC_VER)) */

#endif /* WIN_OSTYPES_H */


/*--- Automatically derived from: C:/daily/rwsdk/src/plcore/bamath.h ---*/

/****************************************************************************
 Defines
 */

#if (!defined(RwInt32FromRealMacro))
#define RwInt32FromRealMacro(x)                 \
    ((RwInt32)(x))
#endif /* (!defined(RwInt32FromRealMacro)) */

#if (!defined(RwFastRealToUInt32))
#define RwFastRealToUInt32(_x)                  \
   ((RwUInt32)RwInt32FromRealMacro(((RwReal)(_x))))
#endif /* (!defined(RwFastRealToUInt32)) */

/*
 * Ensure inclusion of prototypes for single precison maths functions
 * e.g. from
 * /usr/local/sce/ee/gcc/ee/include/math.h
 * /Program Files/Intel/Compiler4.0/include/mathf.h
 */

#if (defined(__ICL))

#if (defined(RWVERBOSE))

/*
 * See
 * http://www.eskimo.com/~scs/C-faq/q11.17.html
 */

#define _STRINGIFY(X) #X
#define _STRINGIFY_EXP(X) _STRINGIFY(X)

#pragma message ("Intel Compiler Version " _STRINGIFY_EXP(__ICL) ":" __FILE__ "(" _STRINGIFY_EXP(__LINE__) ")\n")
#pragma comment ( user, "comment:" "Intel Compiler Version " _STRINGIFY_EXP(__ICL) ":" __FILE__ "(" _STRINGIFY_EXP(__LINE__) ")\n")

#endif /* (defined(RWVERBOSE)) */

#if (400 <= __ICL)
#if (defined(__cplusplus))
#define _INC_MATH
#endif /* (defined(__cplusplus)) */
#include <mathf.h>
#else /* (400 < __ICL) */
#undef RW_USE_SPF
#endif /* (400 < __ICL) */

#endif /* (defined(__ICL)) */

#include <math.h>

#define   _RW_C1      ( (float) 4.1666667908e-02 )
#define   _RW_C2      ( (float)-1.3888889225e-03 )
#define   _RW_C3      ( (float) 2.4801587642e-05 )
#define   _RW_C4      ( (float)-2.7557314297e-07 )
#define   _RW_C5      ( (float) 2.0875723372e-09 )
#define   _RW_C6      ( (float)-1.1359647598e-11 )
#define   _RW_S1      ( (float)-1.6666667163e-01 )
#define   _RW_S2      ( (float) 8.3333337680e-03 )
#define   _RW_S3      ( (float)-1.9841270114e-04 )
#define   _RW_S4      ( (float) 2.7557314297e-06 )
#define   _RW_S5      ( (float)-2.5050759689e-08 )
#define   _RW_S6      ( (float) 1.5896910177e-10 )
#define   _RW_one     ( (float) 1.0000000000e+00 )
#define   _RW_pS0     ( (float) 1.6666667163e-01 )
#define   _RW_pS1     ( (float)-3.2556581497e-01 )
#define   _RW_pS2     ( (float) 2.0121252537e-01 )
#define   _RW_pS3     ( (float)-4.0055535734e-02 )
#define   _RW_pS4     ( (float) 7.9153501429e-04 )
#define   _RW_pS5     ( (float) 3.4793309169e-05 )
#define   _RW_pi      ( (float) 3.1415925026e+00 )
#define   _RW_pi_tol  ( (float) 0.0312500000e+00 )
#define   _RW_pio2_hi ( (float) 1.5707962513e+00 )
#define   _RW_pio2_lo ( (float) 7.5497894159e-08 )
#define   _RW_qS1     ( (float)-2.4033949375e+00 )
#define   _RW_qS2     ( (float) 2.0209457874e+00 )
#define   _RW_qS3     ( (float)-6.8828397989e-01 )
#define   _RW_qS4     ( (float) 7.7038154006e-02 )

#define RwCosMinusPiToPiMacro(result, x)                          \
MACRO_START                                                       \
{                                                                 \
    const float z = x * x;                                        \
    const float r = ( z * (_RW_C1  +                              \
                           z * (_RW_C2  +                         \
                                z * (_RW_C3  +                    \
                                     z * (_RW_C4  +               \
                                          z * (_RW_C5  +          \
                                               z * _RW_C6))))));  \
    result = (_RW_one - ((float) 0.5 * z - (z * r )));            \
}                                                                 \
MACRO_STOP

#define RwSinMinusPiToPiMacro(result, x)                          \
do                                                                \
{                                                                 \
    const float z = x * x;                                        \
    const float v = z * x;                                        \
    const float r = ( _RW_S2 +                                    \
                      z * (_RW_S3 +                               \
                           z * (_RW_S4 +                          \
                                z * (_RW_S5 +                     \
                                     z * _RW_S6))) );             \
    result = x + v * (_RW_S1 + z * r);                            \
}                                                                 \
while(0)                                                                  

typedef union _rwIEEEFloatShapeType _rwIEEEFloatShapeType;
union _rwIEEEFloatShapeType
{
    float               value;
    unsigned int        word;
};

#define _RW_GET_FLOAT_WORD(i,d)                 \
do {                                            \
  _rwIEEEFloatShapeType gf_u;                   \
  gf_u.value = (d);                             \
  (i) = gf_u.word;                              \
} while (0)

/* Set a float from a 32 bit int.  */

#define _RW_SET_FLOAT_WORD(d,i)                 \
do {                                            \
  _rwIEEEFloatShapeType sf_u;                   \
  sf_u.word = (i);                              \
  (d) = sf_u.value;                             \
} while (0)

#define RwIEEEACosfMacro(result, x)                                      \
do                                                                       \
{                                                                        \
    float               z, p, q, r, w, s, c, df;                         \
    int                 hx, ix;                                          \
                                                                         \
    _RW_GET_FLOAT_WORD(hx, x);                                           \
    ix = hx & 0x7fffffff;                                                \
    if (ix >= 0x3f800000)                                                \
    {                          /* |x|>=1 */                              \
        if (hx > 0)                                                      \
        {                                                                \
            /* acos(1) = 0  */                                           \
            result = (0.0);                                              \
        }                                                                \
        else                                                             \
        {                                                                \
            /* acos(-1)= _RW_pi */                                       \
            result = (_RW_pi + (float) 2.0 * _RW_pio2_lo);               \
        }                                                                \
                                                                         \
    }                                                                    \
    else if (ix < 0x3f000000)                                            \
    {                          /* |x| < 0.5 */                           \
        if (ix <= 0x23000000)                                            \
        {                                                                \
            /*if |x|<2**-57 */                                           \
            result = (_RW_pio2_hi + _RW_pio2_lo);                        \
        }                                                                \
        else                                                             \
        {                                                                \
            z = x * x;                                                   \
            p = z * (_RW_pS0 +                                           \
                     z * (_RW_pS1 +                                      \
                          z * (_RW_pS2 +                                 \
                               z * (_RW_pS3 +                            \
                                    z * (_RW_pS4 + z * _RW_pS5)))));     \
            q = _RW_one + z * (_RW_qS1 +                                 \
                               z * (_RW_qS2 +                            \
                                    z * (_RW_qS3 + z * _RW_qS4)));       \
            r = p / q;                                                   \
            result = (_RW_pio2_hi - (x - (_RW_pio2_lo - x * r)));        \
        }                                                                \
                                                                         \
    }                                                                    \
    else if (hx < 0)                                                     \
    {                          /* x < -0.5 */                            \
        z = (_RW_one + x) * (float) 0.5;                                 \
        p = z * (_RW_pS0 +                                               \
                 z * (_RW_pS1 +                                          \
                      z * (_RW_pS2 +                                     \
                           z * (_RW_pS3 +                                \
                                z * (_RW_pS4 + z * _RW_pS5)))));         \
        q = _RW_one + z * (_RW_qS1 +                                     \
                           z * (_RW_qS2 + z * (_RW_qS3 + z * _RW_qS4))); \
        rwSqrtMacro(&s, z);                                              \
        r = p / q;                                                       \
        w = r * s - _RW_pio2_lo;                                         \
        result = (_RW_pi - (float) 2.0 * (s + w));                       \
    }                                                                    \
    else                                                                 \
    {                          /* x > 0.5 */                             \
        int                 idf;                                         \
                                                                         \
        z = (_RW_one - x) * (float) 0.5;                                 \
        rwSqrtMacro(&s, z);                                              \
        df = s;                                                          \
        _RW_GET_FLOAT_WORD(idf, df);                                     \
        _RW_SET_FLOAT_WORD(df, idf & 0xfffff000);                        \
        c = (z - df * df) / (s + df);                                    \
        p = z * (_RW_pS0 +                                               \
                 z * (_RW_pS1 +                                          \
                      z * (_RW_pS2 +                                     \
                           z * (_RW_pS3 +                                \
                                z * (_RW_pS4 + z * _RW_pS5)))));         \
        q = _RW_one + z * (_RW_qS1 +                                     \
                           z * (_RW_qS2 + z * (_RW_qS3 + z * _RW_qS4))); \
        r = p / q;                                                       \
        w = r * s + c;                                                   \
        result = ((float) 2.0 * (df + w));                               \
    }                                                                    \
}                                                                        \
while(0)

#if (defined(RW_USE_SPF))

#define RwACos(_x)              acosf(_x)
#define RwACosh(_x)             acoshf(_x)
#define RwASin(_x)              asinf(_x)
#define RwASinh(_x)             asinhf(_x)

#if (!defined(__ICL))
/*
 * No SPF version in
 * Program Files/Intel/compilerXXX/include/mathf.h
 * of atan2()
 */
#define RwATan2(_x, _y)         atan2f(_x, _y)
#endif /* (!defined(__ICL)) */

#define RwATan(_x)              atanf(_x)
#define RwATanh(_x)             atanhf(_x)
#define RwCabs()                cabsf()
#define RwCbrt(_x)              cbrtf(_x)
#define RwCeil(_x)              ceilf(_x)
#define RwCopysign(_x, _y)      copysignf(_x, _y)
#define RwCos(_x)               cosf(_x)
#define RwCosh(_x)              coshf(_x)
#define RwDrem(_x, _y)          dremf(_x, _y)
#define RwErfc(_x)              erfcf(_x)
#define RwErf(_x)               erff(_x)
#define RwExp(_x)               expf(_x)
#define RwExpm1(_x)             expm1f(_x)
#define RwFinite(_x)            finitef(_x)
#define RwIlogb(_x)             ilogbf(_x)
#define RwIsinf(_x)             isinff(_x)
#define RwIsnan(_x)             isnanf(_x)
#define RwFabs(_x)              fabsf(_x)
#define RwFloor(_x)             floorf(_x)
#define RwFmod(_x, _y)          fmodf(_x, _y)
#define RwFrexp(_x, _iptr)      frexpf(_x, _iptr)
#define RwGamma(_x)             gammaf(_x)
#define RwGammaf_(_x, _iptr)    gammaf_r(_x, _iptr)
#define RwHypot(_x, _y)         hypotf(_x, _y)
#define RwInfinity()            infinityf()
#define RwJ0(_x)                j0f(_x)
#define RwJ1(_x)                j1f(_x)
#define RwJn(_i, _x)            jnf(_i, _x)
#define RwLdexp(_x, _i)         ldexpf(_x, _i)
#define RwLgamma(_x)            lgammaf(_x)
#define RwLgammaf_(_x, _iptr)   lgammaf_r(_x, _iptr)
#define RwLog10(_x)             log10f(_x)
#define RwLog1p(_x)             log1pf(_x)
#define RwLog(_x)               logf(_x)
#define RwModf(_x, _y)          modff(_x, _y)
#define RwNan()                 nanf()
#define RwNextafter(_x, _y)     nextafterf(_x, _y)
#define RwPow(_x, _y)           powf(_x, _y)
#define RwRemainder(_x, _y)     remainderf(_x, _y)
#define RwRint(_x)              rintf(_x)
#define RwScalbn(_x, _i)        scalbnf(_x, _i)
#define RwSin(_x)               sinf(_x)
#define RwSinh(_x)              sinhf(_x)
/* rwSqrtMacro/rwInvSqrtMacro are overloaded in drvmodel.h
 * (if they are at all) and wrapped as func/macro below */
#define RwTan(_x)               tanf(_x)
#define RwTanh(_x)              tanhf(_x)
#define RwY0(_x)                y0f(_x)
#define RwY1(_x)                y1f(_x)
#define RwYn(_i, _x)            ynf(_i, _x)

#endif /* (defined(RW_USE_SPF)) */

#if (!defined(RwACos))
#define RwACos(_x)              acos(_x)
#endif /* (!defined(RwACos)) */
#if (!defined(RwACosh))
#define RwACosh(_x)             acosh(_x)
#endif /* (!defined(RwACosh)) */
#if (!defined(RwASin))
#define RwASin(_x)              asin(_x)
#endif /* (!defined(RwASin)) */
#if (!defined(RwASinh))
#define RwASinh(_x)             asinh(_x)
#endif /* (!defined(RwASinh)) */
#if (!defined(RwATan2))
#define RwATan2(_x, _y)         atan2(_x, _y)
#endif /* (!defined(RwATan2)) */
#if (!defined(RwATan))
#define RwATan(_x)              atan(_x)
#endif /* (!defined(RwATan)) */
#if (!defined(RwATanh))
#define RwATanh(_x)             atanh(_x)
#endif /* (!defined(RwATanh)) */
#if (!defined(RwCabs))
#define RwCabs()                cabs()
#endif /* (!defined(RwCabs)) */
#if (!defined(RwCbrt))
#define RwCbrt(_x)              cbrt(_x)
#endif /* (!defined(RwCbrt)) */
#if (!defined(RwCeil))
#define RwCeil(_x)              ceil(_x)
#endif /* (!defined(RwCeil)) */
#if (!defined(RwCopysign))
#define RwCopysign(_x, _y)      copysign(_x, _y)
#endif /* (!defined(RwCopysign)) */
#if (!defined(RwCos))
#define RwCos(_x)               cos(_x)
#endif /* (!defined(RwCos)) */
#if (!defined(RwCosh))
#define RwCosh(_x)              cosh(_x)
#endif /* (!defined(RwCosh)) */
#if (!defined(RwDrem))
#define RwDrem(_x, _y)          drem(_x, _y)
#endif /* (!defined(RwDrem)) */
#if (!defined(RwErfc))
#define RwErfc(_x)              erfc(_x)
#endif /* (!defined(RwErfc)) */
#if (!defined(RwEr))
#define RwEr(_x)               erf(_x)
#endif /* (!defined(RwEr)) */
#if (!defined(RwExp))
#define RwExp(_x)               exp(_x)
#endif /* (!defined(RwExp)) */
#if (!defined(RwExpm1))
#define RwExpm1(_x)             expm1(_x)
#endif /* (!defined(RwExpm1)) */
#if (!defined(RwFinite))
#define RwFinite(_x)            finite(_x)
#endif /* (!defined(RwFinite)) */
#if (!defined(RwIlogb))
#define RwIlogb(_x)             ilogb(_x)
#endif /* (!defined(RwIlogb)) */
#if (!defined(RwIsin))
#define RwIsin(_x)             isinf(_x)
#endif /* (!defined(RwIsin)) */
#if (!defined(RwIsnan))
#define RwIsnan(_x)             isnan(_x)
#endif /* (!defined(RwIsnan)) */
#if (!defined(RwFabs))
#define RwFabs(_x)              fabs(_x)
#endif /* (!defined(RwFabs)) */
#if (!defined(RwFloor))
#define RwFloor(_x)             floor(_x)
#endif /* (!defined(RwFloor)) */
#if (!defined(RwFmod))
#define RwFmod(_x, _y)          fmod(_x, _y)
#endif /* (!defined(RwFmod)) */
#if (!defined(RwFrexp))
#define RwFrexp(_x, _iptr)      frexp(_x, _iptr)
#endif /* (!defined(RwFrexp)) */
#if (!defined(RwGamma))
#define RwGamma(_x)             gamma(_x)
#endif /* (!defined(RwGamma)) */
#if (!defined(RwGammaf_))
#define RwGammaf_(_x, _iptr)    gammaf_r(_x, _iptr)
#endif /* (!defined(RwGammaf_)) */
#if (!defined(RwHypot))
#define RwHypot(_x, _y)         hypot(_x, _y)
#endif /* (!defined(RwHypot)) */
#if (!defined(RwInfinity))
#define RwInfinity()            infinity()
#endif /* (!defined(RwInfinity)) */
#if (!defined(RwJ0))
#define RwJ0(_x)                j0(_x)
#endif /* (!defined(RwJ0)) */
#if (!defined(RwJ1))
#define RwJ1(_x)                j1(_x)
#endif /* (!defined(RwJ1)) */
#if (!defined(RwJn))
#define RwJn(_i, _x)            jn(_i, _x)
#endif /* (!defined(RwJn)) */
#if (!defined(RwLdexp))
#define RwLdexp(_x, _i)         ldexp(_x, _i)
#endif /* (!defined(RwLdexp)) */
#if (!defined(RwLgamma))
#define RwLgamma(_x)            lgamma(_x)
#endif /* (!defined(RwLgamma)) */
#if (!defined(RwLgammaf_))
#define RwLgammaf_(_x, _iptr)   lgammaf_r(_x, _iptr)
#endif /* (!defined(RwLgammaf_)) */
#if (!defined(RwLog10))
#define RwLog10(_x)             log10(_x)
#endif /* (!defined(RwLog10)) */
#if (!defined(RwLog1p))
#define RwLog1p(_x)             log1p(_x)
#endif /* (!defined(RwLog1p)) */
#if (!defined(RwLog))
#define RwLog(_x)               log(_x)
#endif /* (!defined(RwLog)) */
#if (!defined(RwMod))
#define RwMod(_x, _y)           mod(_x, _y )
#endif /* (!defined(RwMod)) */
#if (!defined(RwNan))
#define RwNan()                 nan()
#endif /* (!defined(RwNan)) */
#if (!defined(RwNextafter))
#define RwNextafter(_x, _y)     nextafter(_x, _y)
#endif /* (!defined(RwNextafter)) */
#if (!defined(RwPow))
#define RwPow(_x, _y)           pow(_x, _y)
#endif /* (!defined(RwPow)) */
#if (!defined(RwRemainder))
#define RwRemainder(_x, _y)     remainder(_x, _y)
#endif /* (!defined(RwRemainder)) */
#if (!defined(RwRint))
#define RwRint(_x)              rint(_x)
#endif /* (!defined(RwRint)) */
#if (!defined(RwScalbn))
#define RwScalbn(_x, _i)        scalbn(_x, _i)
#endif /* (!defined(RwScalbn)) */
#if (!defined(RwSin))
#define RwSin(_x)               sin(_x)
#endif /* (!defined(RwSin)) */
#if (!defined(RwSinh))
#define RwSinh(_x)              sinh(_x)
#endif /* (!defined(RwSinh)) */
#if (!defined(rwSqrt))
/* NOTE: this is overloaded in drvmodel.h for some targets (SKY2 and XBOX atm)
 * [we do in fact do overload w/ sqrtf there, if RW_USE_SPF,
 *  for D3D7, D3D8, OpenGL and SoftRas] */
#define rwSqrt(_result, _x)     rwSqrtMacro(_result, _x)
#endif /* (!defined(rwSqrt)) */
#if (!defined(rwInvSqrt))
/* NOTE: this is overloaded in drvmodel.h for some targets (SKY2 and XBOX atm)
 * [we do in fact do overload w/ (1 / sqrtf) there, if RW_USE_SPF,
 *  for D3D7, D3D8, OpenGL and SoftRas] */
#define rwInvSqrt(_recip, _x)   rwInvSqrtMacro(_recip, _x)
#endif /* (!defined(rwInvSqrt)) */
#if (!defined(RwTan))
#define RwTan(_x)               tan(_x)
#endif /* (!defined(RwTan)) */
#if (!defined(RwTanh))
#define RwTanh(_x)              tanh(_x)
#endif /* (!defined(RwTanh)) */
#if (!defined(RwY0))
#define RwY0(_x)                y0(_x)
#endif /* (!defined(RwY0)) */
#if (!defined(RwY1))
#define RwY1(_x)                y1(_x)
#endif /* (!defined(RwY1)) */
#if (!defined(RwYn))
#define RwYn(_i, _x)            yn(_i, _x)
#endif /* (!defined(RwYn)) */


/*--- Automatically derived from: C:/daily/rwsdk/src/plcore/batypes.h ---*/
#define rwLIBRARYBASEVERSION    0x31000
#define rwLIBRARYCURRENTVERSION 0x33002

/*
 * RWBUILDNUMBER
 * This 16-bit int will be externally defined in an official build, and 
 * is used to construct chunk header library ID when streaming out. All
 * unofficial builds will be stamped with the following:-
 */
#if !defined(RWBUILDNUMBER)
#define RWBUILDNUMBER 0xffff
#endif

/* IMPORTANT:
 * The following Doxygen comment MUST be copied into RwCore.h,
 * so don't move it from here. */

/**
 * \ingroup rwcore
 * \page rwcoreoverview Core Library Overview
 *
 * LIBRARY: rwcore.lib
 * HEADER: rwcore.h
 *
 * This library provides the fundamental RenderWare features.
 *
 * When creating a RenderWare application, this library must always be
 * linked.
 *
 * Functionality includes:
 * \li Immediate Modes (2D \ref rwim2d and 3D \ref rwim3d )
 * \li Plugin Management
 * \li Base Datatypes
 * \li Cameras \ref rwcamera
 * \li Frames \ref rwframe
 * \li the RenderWare Engine \ref rwengine
 *
 * RenderWare uses an object-oriented design philosophy, so this
 * documentation is split across a number of objects.
 *
 * These objects are implemented in C, so C terminology is generally
 * used, rather than C++ -- hence 'functions' instead of 'methods' and
 * 'elements' instead of 'members'.
 *
 * If you are new to RenderWare programming, please read through the
 * supplied User Guide.  The RenderWare Engine \ref rwengine API is
 * usually the starting point for new developers.
 */


#if (!defined(RWFORCEENUMSIZEINT))
#define RWFORCEENUMSIZEINT ((RwInt32)((~((RwUInt32)0))>>1))
#endif /* (!defined(RWFORCEENUMSIZEINT)) */

/*
 * See
 * http://www.eskimo.com/~scs/C-faq/q11.17.html
 */

#define RW_STRINGIFY(X) #X
#define RW_STRINGIFY_EXPANDED(X) RW_STRINGIFY(X)

/****************************************************************************
 Attributes
 */

#if (defined(__GNUC__))

/* See http://www.gnu.org/software/gcc/onlinedocs/gcc_4.html#SEC91 */

#if (!(defined(__cplusplus) || defined(__MWERKS__) || defined(__RWUNUSED__)))
#define __RWUNUSED__ __attribute__ ((unused))
#endif /* (!(defined(__cplusplus) || defined(__MWERKS__) || defined(__RWUNUSED__))) */

#if (!(defined(__RWUNUSEDRELEASE__) || defined(RWVALIDATEPARAM)))
#if (!( defined(__cplusplus) || defined(__MWERKS__) || defined(RWDEBUG)))
#define __RWUNUSEDRELEASE__ __attribute__ ((unused))
#endif /* (!(defined(__cplusplus) || defined(__MWERKS__) || defined(RWDEBUG))) */
#endif /* (!(defined(__RWUNUSEDRELEASE__) || defined(RWVALIDATEPARAM))) */

#if (!defined(__RWFORMAT__))
#define __RWFORMAT__(_archetype, _string_index, _first_to_check)        \
    __attribute__ ((format (_archetype, _string_index, _first_to_check)))
#endif /* (!defined(__RWFORMAT__)) */

#endif /* (defined(__GNUC__)) */

#if (!defined(__RWUNUSED__))
#define __RWUNUSED__           /* No op */
#endif /* (!defined(__RWUNUSED__)) */

#if (!defined(__RWUNUSEDRELEASE__))
#define __RWUNUSEDRELEASE__           /* No op */
#endif /* (!defined(__RWUNUSEDRELEASE__)) */

#if (!defined(__RWFORMAT__))
#define __RWFORMAT__(_archetype, _string_index, _first_to_check)  /* No op */
#endif /* (!defined(__RWFORMAT__)) */

/****************************************************************************
 Calling conventions
 */

#if (defined(WIN32))
#define RWASMCALL   __cdecl
#define RWASMAPI(TYPE) TYPE RWASMCALL
#endif /* (defined(WIN32)) */

#if (!defined(RWASMCALL))
#define RWASMCALL              /* No op */
#endif /* (!defined(RWASMCALL)) */

#if (!defined(RWASMAPI))
#define RWASMAPI(TYPE) TYPE
#endif /* (!defined(RWASMAPI)) */


/* Maximum number of nested contexts */
#define rwMAXPIPECONTEXT 10


/****************************************************************************
 Macro wrappers. These are needed everywhere.
 */

#ifndef MACRO_START
#define MACRO_START do
#endif /* MACRO_START */

#ifndef MACRO_STOP
#define MACRO_STOP while(0)
#endif /* MACRO_STOP */

/****************************************************************************
 Types needed everywhere
 */

#ifdef FALSE
#undef FALSE
#endif
#define FALSE 0

#ifdef TRUE
#undef TRUE
#endif
#define TRUE !FALSE

/****************************************************************************
 MS VC/C++ Specific
 */

#if (defined(_MSC_VER))
#if (_MSC_VER>=1000)


/*
 * Check for correct compiler version
 */
#define RW_MSC_VER 1200

#if (0 && !defined(RW_NO_COMPILER_CHECK))
#if (_MSC_VER != RW_MSC_VER )
# pragma message (__FILE__ "(" RW_STRINGIFY_EXPANDED(__LINE__) "):" "\n  This compiler is a different version (" RW_STRINGIFY_EXPANDED(_MSC_VER) ")\n  to the compiler used to build the RenderWare product libraries (" RW_STRINGIFY_EXPANDED(RW_MSC_VER) ") \n  To turn off this warning please define RW_NO_COMPILER_CHECK " )
# pragma comment ( user, "comment:" __FILE__ "(" RW_STRINGIFY_EXPANDED(__LINE__) "):" "\n  This compiler is a different version (" RW_STRINGIFY_EXPANDED(_MSC_VER) ")\n  to the compiler used to build the RenderWare product libraries (" RW_STRINGIFY_EXPANDED(RW_MSC_VER) ") \n  To turn off this warning please define RW_NO_COMPILER_CHECK " )
#endif /* (_MSC_VER != RW_MSC_VER ) */
#endif /* (0 && !defined(RW_NO_COMPILER_CHECK)) */

/*
 * Output some compiler messages and object file comments
 */

#pragma comment ( compiler )

#pragma comment ( user, "comment:" __DATE__" "  __TIME__ " - " __FILE__ "(" RW_STRINGIFY_EXPANDED(__LINE__) ")")
#pragma comment ( user, "comment:" " _MSC_VER==" RW_STRINGIFY_EXPANDED(_MSC_VER) "; _M_IX86==" RW_STRINGIFY_EXPANDED(_M_IX86))
#if (defined(rwLIBRARYCURRENTVERSION))
#pragma comment ( user, "comment:" "rwLIBRARYCURRENTVERSION:" RW_STRINGIFY_EXPANDED(rwLIBRARYCURRENTVERSION) )
#endif /* (defined(rwLIBRARYCURRENTVERSION)) */

#if (defined(RWDEBUG) && defined(RWVERBOSE))

/* #include <windows.h> */
#if (defined(RWMEMDEBUG) && !defined(_CRTDBG_MAP_ALLOC))
#define _CRTDBG_MAP_ALLOC
#endif /* defined(RWMEMDEBUG) && !defined(_CRTDBG_MAP_ALLOC)) */
#include <crtdbg.h>

#pragma message (__DATE__" "  __TIME__ " - " __FILE__ "(" RW_STRINGIFY_EXPANDED(__LINE__) ")" )
#pragma message ("_MSC_VER==" RW_STRINGIFY_EXPANDED(_MSC_VER) "; _M_IX86==" RW_STRINGIFY_EXPANDED(_M_IX86))

#if (defined(rwLIBRARYCURRENTVERSION))
#pragma message ( "rwLIBRARYCURRENTVERSION:" RW_STRINGIFY_EXPANDED(rwLIBRARYCURRENTVERSION) )
#endif /* (defined(rwLIBRARYCURRENTVERSION)) */

#endif /* (defined(RWDEBUG) && defined(RWVERBOSE) ) */

#endif /* (_MSC_VER>=1000) */
#endif /* (defined(_MSC_VER)) */

/*******************/
/* Primitive types */
/*******************/

/* String construction stuff (gets us UNICODE support) */
#ifdef RWUNICODE
#define _RWSTRING(x) L ## x
#else /* RWUNICODE */
#define _RWSTRING(x) x
#endif /* RWUNICODE */
#define RWSTRING(x) _RWSTRING(x)

/* NB volatile keyword required for VC5.0 to ensure a reload - AMB */
typedef union RwSplitBits RwSplitBits;
union RwSplitBits
{
    RwReal nReal;
    volatile RwInt32 nInt;
    volatile RwUInt32 nUInt;
};

typedef struct RwSplitFixed RwSplitFixed;

#ifdef rwBIGENDIAN
struct RwSplitFixed
{
    RwInt16 integral;
    RwUInt16 fractional;
};

#else /* rwBIGENDIAN */
#ifdef rwLITTLEENDIAN
struct RwSplitFixed
{
    RwUInt16 fractional;
    RwInt16 integral;
};

#else /* rwLITTLEENDIAN */
#error "ENDIAN-ness undefined!"
#endif /* rwLITTLEENDIAN */
#endif /* rwBIGENDIAN */

typedef union RwUnionReal RwUnionReal;
union RwUnionReal /* MSB is sign bit in any circumstance */
{
    RwReal real; /* 4 bytes interpreted as RwReal */
    float floating; /* 4 bytes interpreted as float */
    RwFixed fixed; /* 4 bytes interpreted as 16:16 fixed */
    RwSplitFixed splitfixed; /* 4 bytes interpreted as 16:16 fixed */
};

/*****************/

/* Complex types */

/*****************/

/**
 * \ingroup datatypes
 * \typedef RwV2d
 * typedef for struct RwV2d
 */
typedef struct RwV2d RwV2d;
/**
 * \ingroup datatypes
 * \struct RwV2d
 * This type represents points in a 2D space, such as device
 * space, specified by the (x, y) coordinates of the point.
 */
struct RwV2d
{
    RwReal x;   /**< X value*/
    RwReal y;   /**< Y vlaue */
};

/**
 * \ingroup datatypes
 * \typedef RwV3d
 * typedef for struct RwV3d
 */
typedef struct RwV3d RwV3d;
/**
 * \ingroup datatypes
 * \struct RwV3d
 *  This type represents 3D points and vectors specified by
 * the (x, y, z) coordinates of a 3D point or the (x, y, z) components of a
 * 3D vector.
 */
struct RwV3d
{
    RwReal x;   /**< X value */
    RwReal y;   /**< Y value */
    RwReal z;   /**< Z value */
};

#define RWV4DALIGNMENT(_v4d) \
   (! (((rwV4DALIGNMENT)-1) & ((RwUInt32)(_v4d))))

/**
 * \ingroup datatypes
 * \struct RwV4d
 *  This type represents 4D points and vectors specified by
 * the (x, y, z, w) coordinates of a 4D point or the (x, y, z, w) components of a
 * 4D vector.
 */
struct RwV4d
{
    RwReal x;   /**< X value */
    RwReal y;   /**< Y value */
    RwReal z;   /**< Z value */
    RwReal w;   /**< W value */
};

/**
 * \ingroup datatypes
 * \typedef RwV4d
 * typedef for struct RwV4d
 */
typedef struct RwV4d RWALIGN(RwV4d, rwV4DALIGNMENT);


/**
 * \ingroup datatypes
 * \typedef RwRect
 * typedef for struct RwRect
 */
typedef struct RwRect RwRect;
/**
 * \ingroup datatypes
 * \struct RwRect
 * This type represents a 2D device space rectangle specified
 * by the position of the top-left corner (the offset x, y) and its width (w)
 * and height (h).
 */
struct RwRect
{
    RwInt32 x;  /**< X value of the top-left corner */
    RwInt32 y;  /**< Y value of the top-left corner */
    RwInt32 w;  /**< Width of the rectangle */
    RwInt32 h;  /**< Height of the rectangle */
};

/**
 * \ingroup datatypes
 * \typedef RwSphere
 * typedef for struct RwSphere
 */
typedef struct RwSphere RwSphere;
/**
 * \ingroup datatypes
 * \struct RwSphere
 * This type represents a sphere specified by the position
 * of its center and its radius
 */
struct RwSphere
{
    RwV3d center;   /**< Sphere center */
    RwReal radius;  /**< Sphere radius */
};

#if (!defined(RwSphereAssign))
#define RwSphereAssign(_target, _source)             \
    ( *(_target) = *(_source) )
#endif /* (!defined(RwSphereAssign)) */

/**
 * \ingroup datatypes
 * \typedef RwLine
 * typedef for struct RwLine
 */
typedef struct RwLine RwLine;
/**
 * \ingroup datatypes
 * \struct RwLine
 * This type represents a 3D line specified by the position
 * of its start and end points.
 */
struct RwLine
{
    RwV3d start;    /**< Line start */
    RwV3d end;      /**< Line end */
};

#if (!defined(RwLineAssign))
#define RwLineAssign(_target, _source)             \
    ( *(_target) = *(_source) )
#endif /* (!defined(RwLineAssign)) */

/* The maximum number of texture coordinates */
#define rwMAXTEXTURECOORDS 8

/**
 * \ingroup datatypes
 * RwTextureCoordinateIndex
 *  This type represents the index for texture coordinates.
 */
enum RwTextureCoordinateIndex
{
    rwNARWTEXTURECOORDINATEINDEX = 0,
    rwTEXTURECOORDINATEINDEX0,
    rwTEXTURECOORDINATEINDEX1,
    rwTEXTURECOORDINATEINDEX2,
    rwTEXTURECOORDINATEINDEX3,
    rwTEXTURECOORDINATEINDEX4,
    rwTEXTURECOORDINATEINDEX5,
    rwTEXTURECOORDINATEINDEX6,
    rwTEXTURECOORDINATEINDEX7,
    rwTEXTURECOORDINATEINDEXFORCEENUMSIZEINT = RWFORCEENUMSIZEINT
};
typedef enum RwTextureCoordinateIndex RwTextureCoordinateIndex;

/**
 * \ingroup datatypes
 * \typedef RwTexCoords
 * typedef for struct RwTexCoords
 */
typedef struct RwTexCoords RwTexCoords;
/**
 * \ingroup datatypes
 * \struct RwTexCoords
 * This type represents the the u and v texture
 * coordinates of a particular vertex.
 */
struct RwTexCoords
{
    RwReal u;   /**< U value */
    RwReal v;   /**< V value */
};


/* Singley linked list macros. End marked as NULL */

typedef struct RwSLLink RwSLLink;    /*** RwSLLink ***/
struct RwSLLink
{
    RwSLLink  *next;
};

#define rwSLLinkGetData(link,type,entry)                                \
    ((type *)(((RwUInt8 *)(link))-offsetof(type,entry)))

#define rwSLLinkGetConstData(link,type,entry)                           \
    ((const type *)(((const RwUInt8 *)(link))-offsetof(type,entry)))

#define rwSLLinkInitialize(linkvar)                                     \
    (linkvar)->next = NULL;

#define rwSLLinkGetNext(linkvar)                                        \
    ((linkvar)->next)

typedef struct RwSingleList RwSingleList;
struct RwSingleList
{
    RwSLLink link;
};

#define rwSingleListInitialize(list)                                    \
    (list)->link.next= NULL;
#define rwSingleListEmpty(list)                                         \
    (((list)->link.next)==NULL)
#define rwSingleListAddSLLink(list,linkvar)                             \
    ( (linkvar)->next = (list)->link.next,                              \
      (list)->link.next = (linkvar) )
#define rwSingleListGetFirstSLLink(list)                                \
    ((list)->link.next)
#define rwSingleListGetTerminator(list) (NULL)

/* Doubly linked list. End marked as start (its a ring) */

typedef struct RwLLLink  RwLLLink;                     /*** RwLLLink ***/
struct RwLLLink
{
    RwLLLink *next;
    RwLLLink *prev;
};

#define rwLLLinkGetData(linkvar,type,entry)                             \
    ((type *)(((RwUInt8 *)(linkvar))-offsetof(type,entry)))

#define rwLLLinkGetConstData(linkvar,type,entry)                        \
    ((const type *)(((const RwUInt8 *)(linkvar))-offsetof(type,entry)))

#define rwLLLinkGetNext(linkvar)                                        \
    ((linkvar)->next)

#define rwLLLinkGetPrevious(linkvar)                                    \
    ((linkvar)->prev)

#define rwLLLinkInitialize(linkvar)                                     \
    ( (linkvar)->prev = (RwLLLink *)NULL,                               \
      (linkvar)->next = (RwLLLink *)NULL )

#define rwLLLinkAttached(linkvar)                                       \
    ((linkvar)->next)

typedef struct RwLinkList RwLinkList;
struct RwLinkList
{
    RwLLLink link;
};

#define rwLinkListInitialize(list)                                      \
    ( (list)->link.next = ((RwLLLink *)(list)),                         \
      (list)->link.prev = ((RwLLLink *)(list)) )
#define rwLinkListEmpty(list)                                           \
    (((list)->link.next) == (&(list)->link))
#define rwLinkListAddLLLink(list, linkvar)                              \
    ( (linkvar)->next = (list)->link.next,                              \
      (linkvar)->prev = (&(list)->link),                                \
      ((list)->link.next)->prev = (linkvar),                            \
      (list)->link.next = (linkvar) )
#define rwLinkListRemoveLLLink(linkvar)                                 \
    ( ((linkvar)->prev)->next = (linkvar)->next,                        \
      ((linkvar)->next)->prev = (linkvar)->prev )
#define rwLinkListGetFirstLLLink(list)                                  \
    ((list)->link.next)
#define rwLinkListGetLastLLLink(list)                                   \
    ((list)->link.prev)
#define rwLinkListGetTerminator(list)                                   \
    (&((list)->link))

/**
 * \ingroup datatypes
 * \typedef RwSurfaceProperties
 * typedef for struct RwSurfaceProperties
 */
typedef struct RwSurfaceProperties RwSurfaceProperties;
/**
 * \ingroup datatypes
 * \struct RwSurfaceProperties
 *  This type represents the ambient, diffuse and
 * specular reflection coefficients of a particular geometry. Each coefficient
 * is specified in the range 0.0 (no reflection) to 1.0 (maximum reflection).
 */
struct RwSurfaceProperties
{
    RwReal ambient;   /**< ambient reflection coefficient */
    RwReal specular;  /**< specular reflection coefficient */
    RwReal diffuse;   /**< reflection coefficient */
};

#if (!defined(RwSurfacePropertiesAssign))
#define RwSurfacePropertiesAssign(_target, _source)             \
    ( *(_target) = *(_source) )
#endif /* (!defined(RwSurfacePropertiesAssign)) */

/**********
 * Macros *
 **********/

/* ANSI C defines the offsetof(type,member) macro; should be in <stddef.h> */

/* If not, fall back to this: */
#ifndef offsetof
#define offsetof(type, member)                                          \
    ((size_t)((RwUInt8 *)&((type *) 0)->member - (RwUInt8 *)((type *) 0)))
#endif /* offsetof */

/*
 *
 * Numeric Macros to handle Fixed/Floating point versions of RenderWare
 *
 */
#define RWFIX_MIN     (1)
#define RWFIX_MAX     (0x7fffffff)
#define RwFixedCast(A)     (RwInt32FromRealMacro((A) * 65536.0f))
#define RwFixedToInt(A)    ((A) >> 16)
#define RwFixedToFloat(A)  ((float)(((float)(A)) * (1.0f / 65536.0f)))
#define RwFixedToReal(a)   ((RwReal)(((RwReal)(a)) * (1.0f / 65536.0f)))
#define RwRealToFixed(a)   (RwInt32FromRealMacro((a) * 65536.0f))
#define RwRealAbs(a)       ((RwReal)((a) >= (RwReal)(0.0) ? (a) : (-(a))))
#define RwRealMin2(a,b)    ((RwReal)( ((a) <= (b)) ?  (a) : (b)))
#define RwRealMax2(a,b)    ((RwReal)( ((a) >= (b)) ?  (a) : (b)))
#define RwRealMin3(a,b,c)  RwRealMin2(a,RwRealMin2(b,c))
#define RwRealMax3(a,b,c)  RwRealMax2(a,RwRealMax2(b,c))

#ifndef NORWREALSHORTCUT
#define RToFixed RwRealToFixed
#define RAbs     RwRealAbs
#define FxCast    RwFixedCast
#define FxToInt   RwFixedToInt
#define FxToFloat RwFixedToFloat
#define FxToReal  RwFixedToFloat

#endif

#ifndef rwPI
#define rwPI ((RwReal)(3.1415926535f))
#define rwPIOVER2 (rwPI / (RwReal)(2.0f))
#endif
#define RWRGBALONG(r,g,b,a)                                             \
    ((uintptr) (((uintptr)(a) << 24) | ((uintptr)(r) << 16) | ((uintptr)(g) << 8) | (uintptr)(b)))

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

                             RwPlane

 !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */


/*
 * typedef for struct RwPlane
 */
typedef struct RwPlane RwPlane;
/*
 * This type represents a plane
 */
struct RwPlane
{
    RwV3d normal;    /**< Normal to the plane */
    RwReal distance; /**< Distance to plane from origin in normal direction*/
};


/****************************************************************************
 Defines
 */

enum RwPlaneType
{
    rwXPLANE = 0, /* These are deliberately multiples of sizeof(RwReal) */
    rwYPLANE = 4,
    rwZPLANE = 8,
    rwPLANETYPEFORCEENUMSIZEINT = RWFORCEENUMSIZEINT
};
typedef enum RwPlaneType RwPlaneType;

#define rwSECTORATOMIC -1
#define rwSECTORBUILD -2       /* Only used when building a world */

/* vect is a RwV3d, y is the component */
#define GETCOORD(vect,y)                                                \
    (*(RwReal *)(((RwUInt8 *)(&((vect).x)))+(RwInt32)(y)))
#define GETCONSTCOORD(vect,y)                                           \
    (*(const RwReal *)(((const RwUInt8 *)(&((vect).x)))+(RwInt32)(y)))
#define SETCOORD(vect,y,value)                                          \
    (((*(RwReal *)(((RwUInt8 *)(&((vect).x)))+(RwInt32)(y))))=(value))
#define SETCONTCOORD(vect,y,value)                                      \
    (((*(const RwReal *)                                                \
       (((const RwUInt8 *)                                              \
         (&((vect).x)))+(RwInt32)(y))))=(value))
#define GETCOORDINT(vect,y)                                             \
    (*(RwInt32 *)(((RwUInt8 *)(&((vect).x)))+(y)))
#define GETCONSTCOORDINT(vect,y)                                        \
    (*(const RwInt32 *)(((const RwUInt8 *)(&((vect).x)))+(y)))


/**
 * \ingroup rwcore
 * \page inttypes Integer Types
 *
 * RenderWare supports a number of integer types:
 *
 * RwInt8 8-bit signed integer.
 *    \li RwUInt8 8-bit unsigned integer.
 *    \li RwChar Character type.
 *    \li RwInt16 16-bit signed integer.
 *    \li RwUInt16 16-bit unsigned integer.
 *    \li RwInt32 32-bit signed integer.
 *    \li RwUInt32 32-bit unsigned integer.
 *    \li RwInt64 64-bit signed integer.
 *    \li RwUInt64 64-bit unsigned integer.
 *    \li RwInt128 128-bit signed integer.
 *    \li RwUInt128 128-bit unsigned integer.
 *    \li RwBool Boolean type (in 32 bits).
 *
 * These types should be used in applications in preference to the underlying
 * native types. 
 * 
 * The following constants indicate the maximum and minimum values possible
 * for the various RenderWare integer types:
 *
 *    \li RwInt32MAXVAL   Maximum RwInt32 value.
 *    \li RwInt32MINVAL   Minimum RwInt32 value.
 *    \li RwUInt32MAXVAL   Maximum RwUInt32 value.
 *    \li RwUInt32MINVAL   Minimum RwUInt32 value.
 *    \li RwInt16MAXVAL   Maximum RwInt16 value.
 *    \li RwInt16MINVAL   Minimum RwInt16 value.
 *    \li RwUInt16MAXVAL   Maximum RwUInt16 value.
 *    \li RwUInt16MINVAL   Minimum RwUInt16 value.
 *
 * \see RwReal
 */

/**
 * \ingroup datatypes
 * \typedef RwReal 
 *
 * RenderWare supports a single RwReal floating-point type to aid portability
 * across platforms. This type should be used in applications in preference to
 * the underlying native type.
 *
 * The constants RwRealMAXVAL and RwRealMINVAL are provided for determining
 * the maximum and minimum values possible using the RwReal type.
 *
 * In addition, the following macros are available for operations on RwReal
 * types:
 *      \li RwRealMin2(a, b)    Find the minimum of two RwReal values.
 *      \li RwRealMax2(a, b)    Find the maximum of two RwReal values.
 *      \li RwRealMin3(a, b, c)    Find the minimum of three RwReal values.
 *      \li RwRealMax3(a, b, c)    Find the maximum of three RwReal values.
 *      \li RwRealAbs(x)    Find the absolute value of a RwReal value.
 *
 * \see \ref inttypes
 */

/** 
 * \ingroup datatypes
 * \typedef RwFixed
 * 
 * RenderWare supports a single RwFixed fixed-point type.
 *
 * Although popular in the days when integer mathematics was much faster than
 * floating point mathematics, fixed-point math is now rarely used. It is
 * provided because it is still useful for some processes.
 *
 * The maximum and minimum size of an RwFixed value are defined by the constants
 * RWFIX_MAX and RWFIX_MIN respectively.
 *
 * The following macros are provided to help you work with RwFixed datatypes:
 *    \li RwFixedCast(x)        Cast the integer portion of an RwFixed to another type.
 *    \li RwFixedToInt(x)       Convert an RwFixed to an integer. (The fractional portion is lost.)
 *    \li RwFixedToFloat(x)     Convert an RwFixed to a float.
 *    \li RwFixedToReal(x)      Convert an RwFixed to an RwReal.
 *    \li RwRealToFixed(x)      Convert an RwReal to an RwFixed. (Some precision may be lost.)
 */

/** 
 * \ingroup datatypes
 * \typedef RwInt8    
 * 
 * Signed 8 bit integer type.
 * \see \ref inttypes
 */

/** 
 * \ingroup datatypes
 * \typedef RwUInt8   
 *
 * Unsigned 8bit integer type.
 * \see \ref inttypes
 */

/** 
 * \ingroup datatypes
 * \typedef RwChar    
 *
 * Character type.
 * \see \ref inttypes
 */

/** 
 * \ingroup datatypes
 * \typedef RwInt16   
 *
 * Signed 16 bit integer type.
 * \see \ref inttypes
 */

/** 
 * \ingroup datatypes
 * \typedef RwUInt16  
 *
 * Unsigned 16 bit integer type.
 * \see \ref inttypes
 */

/** 
 * \ingroup datatypes
 * \typedef RwInt32   
 *
 * Signed 32 bit integer type.
 * \see \ref inttypes
 */

/** 
 * \ingroup datatypes
 * \typedef RwUInt32  
 *
 * Unsigned 32 bit integer type.
 * \see \ref inttypes
 */

/** 
 * \ingroup datatypes
 * \typedef RwInt64   
 *
 * Signed 64 bit integer type.
 * \see \ref inttypes
 */

/** 
 * \ingroup datatypes
 * \typedef RwUInt64  
 *
 * Unsigned 64 bit integer type.
 * \see \ref inttypes
 */

/** 
 * \ingroup datatypes
 * \typedef RwInt128  
 *
 * Signed 128 bit integer type.
 * \see \ref inttypes
 */

/** 
 * \ingroup datatypes
 * \typedef RwUInt128 
 *
 * Unsigned 128 bit integer type.
 * \see \ref inttypes
 */

/** 
 * \ingroup datatypes
 * \typedef RwBool    
 *
 * Boolean type.
 * \see \ref inttypes
 */
 

/*--- Automatically derived from: C:/daily/rwsdk/src/plcore/batype.h ---*/
/****************************************************************************
 Defines
*/

/*
 * Object Types - these are used in the binary object
 * representations and in the debug library. They must
 * be unique.  They are the old system.
 */

#define rwID_DATABASE 0x64617462     /* datb */

#define MAKECHUNKID(vendorID, chunkID) (((vendorID & 0xFFFFFF) << 8) | (chunkID & 0xFF))
#define GETOBJECTID(chunkID) (chunkID & 0xFF)
#define GETVENDORID(chunkID) ((chunkID >> 8) & 0xFFFFFF)

/***
 *** These are the vendor IDs.  A customer must reserve a vendor ID in order
 *** to be able to write toolkits (this prevents clashes between toolkits).
 *** We reserve some for our own use as shown below.  These are all 24 bit.
 ***
 *** IMPORTANT NOTE: DO NOT UNDER ANY CIRCUMSTANCES CHANGE THESE VALUES. IF
 ***                 YOU ARE ADDING A NEW ONE, APPEND IT!
 ***
 *** They must all be unique.
 ***/

enum RwPluginVendor
{
    rwVENDORID_CORE             = 0x000000L,
    rwVENDORID_CRITERIONTK      = 0x000001L,
    rwVENDORID_REDLINERACER     = 0x000002L,
    rwVENDORID_CSLRD            = 0x000003L,
    rwVENDORID_CRITERIONINT     = 0x000004L,
    rwVENDORID_CRITERIONWORLD   = 0x000005L,
    rwVENDORID_BETA             = 0x000006L,
    rwVENDORID_CRITERIONRM      = 0x000007L,
    rwVENDORID_CRITERIONRWA     = 0x000008L, /* RenderWare Audio */
    rwPLUGINVENDORFORCEENUMSIZEINT = RWFORCEENUMSIZEINT
};
typedef enum RwPluginVendor RwPluginVendor;

/***
 *** These are the core objects (8 bit IDs).  They must all be unique.
 *** We can get away without using the MAKECHUNKID macro because the
 *** vendor ID in all cases will be zero (rwVENDORID_CORE).
 ***
 *** IMPORTANT NOTE: DO NOT UNDER ANY CIRCUMSTANCES CHANGE THESE VALUES. IF
 ***                 YOU ARE ADDING A NEW ONE, APPEND IT!
 ***/

/* These are the internal ones.  Because the core ID is 0, we can get away without
 * using the MAKECHUNKID macro for the CORE chunks.
 */

enum RwCorePluginID
{
    rwID_NAOBJECT               = 0x00,
    rwID_STRUCT                 = 0x01,
    rwID_STRING                 = 0x02,
    rwID_EXTENSION              = 0x03,
    rwID_CAMERA                 = 0x05,
    rwID_TEXTURE                = 0x06,
    rwID_MATERIAL               = 0x07,
    rwID_MATLIST                = 0x08,
    rwID_ATOMICSECT             = 0x09,
    rwID_PLANESECT              = 0x0A,
    rwID_WORLD                  = 0x0B,
    rwID_SPLINE                 = 0x0C,
    rwID_MATRIX                 = 0x0D,
    rwID_FRAMELIST              = 0x0E,
    rwID_GEOMETRY               = 0x0F,
    rwID_CLUMP                  = 0x10,
    rwID_LIGHT                  = 0x12,
    rwID_UNICODESTRING          = 0x13,
    rwID_ATOMIC                 = 0x14,
    rwID_TEXTURENATIVE          = 0x15,
    rwID_TEXDICTIONARY          = 0x16,
    rwID_ANIMDATABASE           = 0x17,
    rwID_IMAGE                  = 0x18,
    rwID_SKINANIMATION          = 0x19,
    rwID_GEOMETRYLIST           = 0x1A,
    rwID_HANIMANIMATION         = 0x1B,
    rwID_TEAM                   = 0x1C,
    rwID_CROWD                  = 0x1D,
    rwID_DMORPHANIMATION        = 0x1E,
    rwID_RIGHTTORENDER          = 0x1f,
    rwID_MTEFFECTNATIVE         = 0x20,
    rwID_MTEFFECTDICT           = 0x21,
    rwID_TEAMDICTIONARY         = 0x22,
    rwID_PITEXDICTIONARY        = 0x23,
    rwID_TOC                    = 0x24,
    rwID_PRTSTDGLOBALDATA       = 0x25,
    /* Insert before MAX and increment MAX */
    rwID_COREPLUGINIDMAX        = 0x26,
    rwCOREPLUGINIDFORCEENUMSIZEINT = RWFORCEENUMSIZEINT
};
typedef enum RwCorePluginID RwCorePluginID ;

/***
 *** These are the Criterion internal plugin extensions.  Use with rwVENDORID_CRITERIONINT.
 ***
 *** IMPORTANT NOTE: DO NOT UNDER ANY CIRCUMSTANCES CHANGE THESE VALUES. IF
 ***                 YOU ARE ADDING A NEW ONE, APPEND IT!
 ***/

enum RwCriterionPluginID
{
    rwID_COREPLUGIN             = 0x01,
    rwID_WORLDPLUGIN            = 0x02,
    rwID_TOOLPLUGIN             = 0x03,
    rwID_TOOL2PLUGIN            = 0x04,
    rwCRITERIONPLUGINIDFORCEENUMSIZEINT = RWFORCEENUMSIZEINT
};
typedef enum RwCriterionPluginID RwCriterionPluginID;


/***
 *** These are the Criterion internal platform identifies.
 ***
 *** IMPORTANT NOTE: DO NOT UNDER ANY CIRCUMSTANCES CHANGE THESE VALUES. IF
 ***                 YOU ARE ADDING A NEW ONE, APPEND IT!
 ***/
enum RwPlatformID
{
    rwID_PCD3D7 = 1,
    rwID_PCOGL,
    rwID_MAC,
    rwID_PS2,
    rwID_XBOX,
    rwID_GAMECUBE,
    rwID_SOFTRAS,
    rwID_PCD3D8,
    rwPLATFROMIDFORCEENUMSIZEINT = RWFORCEENUMSIZEINT
};
typedef enum RwPlatformID RwPlatformID;


/****************************************************************************
 Global Types
 */

typedef struct RwObject RwObject;
/**
 * \ingroup datatypes
 * \struct RwObject
 * This should be considered an opaque type. Use
 * the RwObject API functions to access.
 */
struct RwObject
{
        RwUInt8 type;                /**< Internal Use */
        RwUInt8 subType;             /**< Internal Use */
        RwUInt8 flags;               /**< Internal Use */
        RwUInt8 privateFlags;        /**< Internal Use */
        void   *parent;              /**< Internal Use */
                                     /* Often a Frame  */
};

/**
 * \ingroup datatypes
 * \typedef RwObjectCallBack
 * callback function supplied for object callback functions.
 *
 * \return Pointer to the current object
 *
 * \param  object   Pointer to the current object, supplied by
 * iterator.
 * \param  data  Pointer to developer-defined data structure.
 *
 * \see RwFrameForAllObjects
 *
 */
typedef RwObject *(*RwObjectCallBack)(RwObject *object, void *data);

/****************************************************************************
 Function prototypes
 */

#ifdef    __cplusplus
extern "C"
{
#endif                          /* __cplusplus */

/* TYPE METHODS */

/* Creation/cloning */

#define rwObjectCopy(d,s)                               \
MACRO_START                                             \
{                                                       \
    ((RwObject *)(d))->type =                           \
        ((const RwObject *)(s))->type;                  \
    ((RwObject *)(d))->subType =                        \
        ((const RwObject *)(s))->subType;               \
    ((RwObject *)(d))->flags =                          \
        ((const RwObject *)(s))->flags;                 \
    ((RwObject *)(d))->privateFlags =                   \
        ((const RwObject *)(s))->privateFlags;          \
    ((RwObject *)(d))->parent =                         \
        NULL;                                           \
}                                                       \
MACRO_STOP

#define rwObjectInitialize(o, t, s)                     \
MACRO_START                                             \
{                                                       \
    ((RwObject *)(o))->type = (RwUInt8)(t);             \
    ((RwObject *)(o))->subType = (RwUInt8)(s);          \
    ((RwObject *)(o))->flags = 0;                       \
    ((RwObject *)(o))->privateFlags = 0;                \
    ((RwObject *)(o))->parent = NULL;                   \
}                                                       \
MACRO_STOP

/* Debug */
#define RwObjectGetType(o)                  (((const RwObject *)(o))->type)

#define rwObjectSetType(o, t)               (((RwObject *)(o))->type) = (RwUInt8)(t)

/* Sub type */
#define rwObjectGetSubType(o)               (((const RwObject *)(o))->subType)
#define rwObjectSetSubType(o, t)            (((RwObject *)(o))->subType) = (RwUInt8)(t)

/* Flags */
#define rwObjectGetFlags(o)                 (((const RwObject *)(o))->flags)
#define rwObjectSetFlags(o, f)              (((RwObject *)(o))->flags) = (RwUInt8)(f)
#define rwObjectTestFlags(o, f)             ((((const RwObject *)(o))->flags) & (RwUInt8)(f))

/* Private flags */
#define rwObjectGetPrivateFlags(c)          (((const RwObject *)(c))->privateFlags)
#define rwObjectSetPrivateFlags(c,f)        (((RwObject *)(c))->privateFlags) = (RwUInt8)(f)
#define rwObjectTestPrivateFlags(c,flag)    ((((const RwObject *)(c))->privateFlags) & (RwUInt8)(flag))

/* Hierarchy */
#define rwObjectGetParent(object)           (((const RwObject *)(object))->parent)
#define rwObjectSetParent(c,p)              (((RwObject *)(c))->parent) = (void *)(p)

#ifdef    __cplusplus
}
#endif                          /* __cplusplus */


/*--- Automatically derived from: C:/daily/rwsdk/os/win/osintf.h ---*/

/*--- Automatically derived from: C:/daily/rwsdk/src/plcore/rwstring.h ---*/

/****************************************************************************
 Defines
 */

#define rwsprintf   RWSRCGLOBAL(stringFuncs).vecSprintf
#define rwvsprintf  RWSRCGLOBAL(stringFuncs).vecVsprintf
#define rwstrcpy    RWSRCGLOBAL(stringFuncs).vecStrcpy
#define rwstrncpy   RWSRCGLOBAL(stringFuncs).vecStrncpy
#define rwstrcat    RWSRCGLOBAL(stringFuncs).vecStrcat
#define rwstrncat   RWSRCGLOBAL(stringFuncs).vecStrncat
#define rwstrrchr   RWSRCGLOBAL(stringFuncs).vecStrrchr
#define rwstrchr    RWSRCGLOBAL(stringFuncs).vecStrchr
#define rwstrstr    RWSRCGLOBAL(stringFuncs).vecStrstr
#define rwstrcmp    RWSRCGLOBAL(stringFuncs).vecStrcmp
#define rwstricmp   RWSRCGLOBAL(stringFuncs).vecStricmp
#define rwstrlen    RWSRCGLOBAL(stringFuncs).vecStrlen
#define rwstrupr    RWSRCGLOBAL(stringFuncs).vecStrupr
#define rwstrlwr    RWSRCGLOBAL(stringFuncs).vecStrlwr
#define rwstrtok    RWSRCGLOBAL(stringFuncs).vecStrtok
#define rwsscanf    RWSRCGLOBAL(stringFuncs).vecSscanf

#define rwstrdup(_result, _string)                \
do                                                \
{                                                 \
    _result = ((RwChar*)NULL);                    \
                                                  \
    if (((RwChar*)NULL) != (_string))             \
    {                                             \
        _result = (RwChar *)                      \
            RwMalloc( (rwstrlen(_string) + 1) *   \
                      sizeof (RwChar) );          \
                                                  \
        if (((RwChar*)NULL) != (_result))         \
        {                                         \
            rwstrcpy(_result, _string);           \
        }                                         \
    }                                             \
}                                                 \
while (0)


/****************************************************************************
 Global Types
 */

typedef int (*vecSprintfFunc)(RwChar *buffer,
                              const RwChar *format,
                              ...) /* __RWFORMAT__(printf, 2, 3) */; 
typedef int (*vecVsprintfFunc)(RwChar *buffer,
                               const RwChar *format,
                               va_list argptr); 
typedef RwChar *(*vecStrcpyFunc)(RwChar *dest,
                                 const RwChar *srce);
typedef RwChar *(*vecStrncpyFunc)(RwChar *dest,
                                  const RwChar *srce,
                                  size_t size);
typedef RwChar *(*vecStrcatFunc)(RwChar *dest,
                                 const RwChar *srce);
typedef RwChar *(*vecStrncatFunc)(RwChar *dest,
                                  const RwChar *srce,
                                  size_t size);
typedef RwChar *(*vecStrrchrFunc)(const RwChar *string,
                                  int findThis);
typedef RwChar *(*vecStrchrFunc)(const RwChar *string,
                                 int findThis);
typedef RwChar *(*vecStrstrFunc)(const RwChar *string,
                                 const RwChar *findThis);
typedef int (*vecStrcmpFunc)(const RwChar *string1,
                             const RwChar *string2);
typedef int (*vecStricmpFunc)(const RwChar *string1,
                              const RwChar *string2);
typedef size_t (*vecStrlenFunc)(const RwChar *string);
typedef RwChar *(*vecStruprFunc)(RwChar *string);
typedef RwChar *(*vecStrlwrFunc)(RwChar *string);
typedef RwChar *(*vecStrtokFunc)(RwChar *string, const RwChar *delimit);
typedef int (*vecSscanfFunc)(const RwChar *buffer,
                             const RwChar *format,
                             ...) /* __RWFORMAT__(scanf, 2, 3) */;

typedef struct RwStringFunctions RwStringFunctions;
struct RwStringFunctions
{
    vecSprintfFunc vecSprintf ;
    vecVsprintfFunc vecVsprintf;
    vecStrcpyFunc vecStrcpy;
    vecStrncpyFunc vecStrncpy;
    vecStrcatFunc vecStrcat;
    vecStrncatFunc vecStrncat;
    vecStrrchrFunc vecStrrchr;
    vecStrchrFunc vecStrchr;
    vecStrstrFunc vecStrstr;
    vecStrcmpFunc vecStrcmp;
    vecStricmpFunc vecStricmp;
    vecStrlenFunc vecStrlen;
    vecStruprFunc vecStrupr;
    vecStrlwrFunc vecStrlwr;
    vecStrtokFunc vecStrtok;
    vecSscanfFunc vecSscanf;
};


/*--- Automatically derived from: C:/daily/rwsdk/src/plcore/rwdbgerr.h ---*/
#define RWECODE(a,b) a,

/* Construct an enum type with all the plugin error codes (for the app to use) */
enum RwErrorCodePlugin_errcore 
{
#include "errcore.def"
    rwLASTERROR_errcore = RWFORCEENUMSIZEINT
};
typedef enum RwErrorCodePlugin_errcore RwErrorCodePlugin_errcore;


#undef RWECODE


/*--- Automatically derived from: C:/daily/rwsdk/src/plcore/resmem.h ---*/

/*--- Automatically derived from: C:/daily/rwsdk/src/plcore/bamemory.h ---*/
#if (defined(RWMEMDEBUG))
#ifdef _XBOX
/* Need OutputDebugString macros */
#include <xtl.h>
#endif
#endif

/****************************************************************************
 Defines
 */

/*
 * Debug fill bytes for compatibility with MSVC/C++ debug heap
 * See
 * \Program Files\Microsoft Visual Studio\VC98\CRT\SRC\DBGHEAP.C:
 * static unsigned char _bNoMansLandFill = 0xFD;
 *   // fill no-man's land with this
 * static unsigned char _bDeadLandFill   = 0xDD;
 *   // fill free objects with this
 * static unsigned char _bCleanLandFill  = 0xCD;
 *   // fill new objects with this
 */

#if (!defined(rwFREELISTNOMANSLANDFILL))
#define rwFREELISTNOMANSLANDFILL    0xFD
#endif /* (!defined(rwFREELISTNOMANSLANDFILL)) */

#if (!defined(rwFREELISTDEADLANDFILL))
#define rwFREELISTDEADLANDFILL      0xDD
#endif /* (!defined(rwFREELISTDEADLANDFILL)) */

#if (!defined(rwFREELISTCLEANLANDFILL))
#define rwFREELISTCLEANLANDFILL     0xCD
#endif /* (!defined(rwFREELISTCLEANLANDFILL)) */

#define RWFREELISTALIGNED(_pData, _freelist) \
  (! (((RwUInt32)(_pData)) & ((_freelist)->alignmentMinusOne)) )

/*****************************
 * REGULAR MEMORY ALLOCATION *
 *****************************/

/**
 * \ingroup rwmem
 * \def RwMalloc
 * RwMalloc(_s) is a macro for malloc(_s).
 */

/**
 * \ingroup rwmem
 * \def RwFree
 * RwFree(_p) is a macro for free(_p).
 */

/**
 * \ingroup rwmem
 * \def RwCalloc
 * RwCalloc(_n, _s) is a macro for calloc(_n, _s).
 */

/**
 * \ingroup rwmem
 * \def RwRealloc
 * RwRealloc(_p, _s) is a macro for realloc(_p, _s).
 */

#if ( (defined(RWMEMDEBUG)) && defined(RWDEBUG) )

#if (!defined(RWNOFREELISTS))
#define RWNOFREELISTS
#endif /* (!defined(RWNOFREELISTS)) */

#if (defined(rwPLUGIN_ID))
#define _CLIENT_TAG \
    ( 0xFFFF & (rwPLUGIN_ID) )
#endif /* (defined(rwPLUGIN_ID)) */

#if (!defined(_CLIENT_TAG))
#define _CLIENT_TAG \
    ( 0xFFFF & (MAKECHUNKID(rwVENDORID_CRITERIONTK, 0x00) ) )
#endif /* (!defined(_CLIENT_TAG)) */

# if (defined(_MSC_VER))
#  if ((_MSC_VER>=1000) && defined(_DEBUG))

/* Pick up _ASSERTE() macro */
/* #include <windows.h> */
#if (defined(RWMEMDEBUG) && !defined(_CRTDBG_MAP_ALLOC))
#define _CRTDBG_MAP_ALLOC
#endif /* defined(RWMEMDEBUG) && !defined(_CRTDBG_MAP_ALLOC)) */
#include <crtdbg.h>

#define RwMalloc(_s)                                    \
    _malloc_dbg((_s),                                   \
               _CLIENT_BLOCK | ((_CLIENT_TAG)<<16),     \
               __FILE__,                                \
               __LINE__)

#define RwFree(_p)                                      \
   _free_dbg((_p),                                      \
               _CLIENT_BLOCK | ((_CLIENT_TAG)<<16))

#define RwCalloc(_n, _s)                                \
   _calloc_dbg((_n), (_s),                              \
               _CLIENT_BLOCK | ((_CLIENT_TAG)<<16),     \
               __FILE__,                                \
               __LINE__)

#define RwRealloc(_p, _s)                               \
   _realloc_dbg((_p),                                   \
                (_s),                                   \
               _CLIENT_BLOCK | ((_CLIENT_TAG)<<16),     \
               __FILE__,                                \
               __LINE__)

#define     RWCRTORDBGFLAG(_flag)                       \
    do                                                  \
    {                                                   \
        int            _DbgFlag;                        \
                                                        \
        _DbgFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG); \
        _DbgFlag |= (_flag);                            \
        _CrtSetDbgFlag(_DbgFlag);                       \
    }   while(0)

#define VALID_HEAP_STR \
  __FILE__##"("##RW_STRINGIFY_EXPANDED(__LINE__)##"): valid heap\n"

#define     RWCRTCHECKMEMORY()                          \
    do                                                  \
    {                                                   \
        int             valid_heap;                     \
                                                        \
        valid_heap = _CrtCheckMemory();                 \
        _ASSERTE(valid_heap);                           \
    }   while(0)

/*
 *      if (valid_heap)                                 \
 *          OutputDebugString(VALID_HEAP_STR);          \
 */

#define NO_LEAKS_FOUND_STR \
   __FILE__##"("##RW_STRINGIFY_EXPANDED(__LINE__)##"): no heap leaks found\n"

#define     RWCRTDUMPMEMORYLEAKS()                      \
    do                                                  \
    {                                                   \
        int             leaks_found;                    \
                                                        \
        leaks_found = _CrtDumpMemoryLeaks();            \
        _ASSERTE(!leaks_found);                         \
        if (!leaks_found)                               \
            OutputDebugString(NO_LEAKS_FOUND_STR);      \
    }   while(0)

#define HEAP_DIFFERENCES_FOUND_STR \
   __FILE__##"("##RW_STRINGIFY_EXPANDED(__LINE__)##"): heap differences found\n"

#define NO_DIFFERENCES_FOUND_STR \
   __FILE__##"("##RW_STRINGIFY_EXPANDED(__LINE__)##"): no heap differences found\n"

#define RWCRTHEAPDIFFERENCESINCE(_Then)                                   \
    do                                                                    \
    {                                                                     \
        /* only dump differences when                                     \
         * there are in fact differences */                               \
        _CrtMemState _Now;                                                \
        _CrtMemState _Delta;                                              \
        const int _DbgFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);         \
        int Differences;                                                  \
                                                                          \
        _CrtMemCheckpoint(&_Now);                                         \
        _CrtMemDifference(&_Delta, _Then, &_Now);                         \
                                                                          \
        (Differences) = ( ( 0 != _Delta.lCounts[_CLIENT_BLOCK] ) ||       \
                          ( 0 != _Delta.lCounts[_NORMAL_BLOCK] ) ||       \
                          ( (_DbgFlag & _CRTDBG_CHECK_CRT_DF) &&          \
                            ( 0 != _Delta.lCounts[_CRT_BLOCK]) ) );       \
                                                                          \
        if ( (Differences) )                                              \
        {                                                                 \
            /* difference detected: dump objects since _Then. */          \
            OutputDebugString(HEAP_DIFFERENCES_FOUND_STR);                \
            _CrtMemDumpAllObjectsSince(_Then);                            \
            _CrtMemDumpStatistics(&_Delta);                               \
        }                                                                 \
        else                                                              \
        {                                                                 \
            OutputDebugString(NO_DIFFERENCES_FOUND_STR);                  \
        }                                                                 \
    }   while (0)

#define RWCRTDBGBREAK() \
    _CrtDbgBreak()

#define RWCRTDOFORALLCLIENTOBJECTS(_f, _c) \
    _CrtDoForAllClientObjects(_f, _c)

#define RWCRTISMEMORYBLOCK(_p, _t, _r, _f, _l) \
    _CrtIsMemoryBlock(_p, _t, _r, _f, _l)

#define RWCRTISVALIDHEAPPOINTER(_p) \
    _CrtIsValidHeapPointer(_p)

#define RWCRTISVALIDPOINTER(_p, _n, _r) \
    _CrtIsValidPointer(_p, _n, _r)

#define RWCRTMEMCHECKPOINT(_s) \
    _CrtMemCheckpoint(_s)

#define RWCRTMEMDIFFERENCE(_s1, _s2, _s3) \
    _CrtMemDifference(_s1, _s2, _s3)

#define RWCRTMEMDUMPALLOBJECTSSINCE(_s) \
    _CrtMemDumpAllObjectsSince(_s)

#define RWCRTMEMDUMPSTATISTICS(_s) \
    _CrtMemDumpStatistics(_s)

#define RWCRTSETALLOCHOOK(_f) \
    _CrtSetAllocHook(_f)

#define RWCRTSETBREAKALLOC(_a) \
    _CrtSetBreakAlloc(_a)

#define RWCRTSETDBGFLAG(_f) \
    _CrtSetDbgFlag(_f)

#define RWCRTSETDUMPCLIENT(_f) \
    _CrtSetDumpClient(_f)

#define RWCRTSETREPORTFILE(_t, _f) \
    _CrtSetReportFile(_t, _f)

#define RWCRTSETREPORTHOOK(_f) \
    _CrtSetReportHook(_f)

#define RWCRTSETREPORTMODE(_t, _f) \
    _CrtSetReportMode(_t, _f)

#if (!defined(_CRTDBG_FLAGS))
#define _CRTDBG_FLAGS                                           \
    ( (_CRTDBG_ALLOC_MEM_DF | _CRTDBG_DELAY_FREE_MEM_DF |       \
       _CRTDBG_CHECK_CRT_DF | _CRTDBG_LEAK_CHECK_DF) &          \
     ~(_CRTDBG_CHECK_ALWAYS_DF |_CRTDBG_RESERVED_DF) )
#endif /* (!defined(_CRTDBG_FLAGS)) */

#  endif /* ((_MSC_VER>=1000) && defined(_DEBUG)) */
# endif /* (defined(_MSC_VER)) */



#if (!defined(rwDEADPTRFILL))
#define rwDEADPTRFILL      ((void *)0xDDDDDDDD)
#endif /* (!defined(rwDEADPTRFILL)) */

#endif /* (defined(RWDEBUG) && (defined(RWMEMDEBUG))) */

#if (!defined(rwDEADPTRFILL))
#define rwDEADPTRFILL      (NULL)
#endif /* (!defined(rwDEADPTRFILL)) */

#if (!defined(RwMalloc))
#define RwMalloc(_s)   ((RWSRCGLOBAL(memoryFuncs).rwmalloc)((_s)))
#endif /* (!defined(RwMalloc)) */

#if (!defined(RwFree))
#define RwFree(_p)     ((RWSRCGLOBAL(memoryFuncs).rwfree)((_p)))
#endif /* (!defined(RwFree)) */

#if (!defined(RwCalloc))
#define RwCalloc(_n, _s)   ((RWSRCGLOBAL(memoryFuncs).rwcalloc)((_n), (_s)))
#endif /* (!defined(RwCalloc)) */

#if (!defined(RwRealloc))
#define RwRealloc(_p, _s)  ((RWSRCGLOBAL(memoryFuncs).rwrealloc)((_p),(_s)))
#endif /* (!defined(RwRealloc)) */

#if (!defined(RWCRTORDBGFLAG))
#define     RWCRTORDBGFLAG(_flag) /* No op */
#endif /* (!defined(RWCRTORDBGFLAG)) */

#if (!defined(RWCRTCHECKMEMORY))
#define RWCRTCHECKMEMORY()     /* No Op */
#endif /* (!defined(RWCRTCHECKMEMORY)) */

#if (!defined(RWCRTDBGBREAK))
#define RWCRTDBGBREAK()        /* No Op */
#endif /* (!defined(RWCRTDBGBREAK)) */

#if (!defined(RWCRTDOFORALLCLIENTOBJECTS))
#define RWCRTDOFORALLCLIENTOBJECTS(_f, _c) /* No Op */
#endif /* (!defined(RWCRTDOFORALLCLIENTOBJECTS)) */

#if (!defined(RWCRTDUMPMEMORYLEAKS))
#define RWCRTDUMPMEMORYLEAKS() /* No Op */
#endif /* (!defined(RWCRTDUMPMEMORYLEAKS)) */

#if (!defined(RWCRTHEAPDIFFERENCESINCE))
#define RWCRTHEAPDIFFERENCESINCE(_Then)           /* No Op */
#endif /* (!defined(RWCRTHEAPDIFFERENCESINCE)) */

#if (!defined(RWCRTISMEMORYBLOCK))
#define RWCRTISMEMORYBLOCK(_p, _t, _r, _f, _l)    (NULL != (_p))
#endif /* (!defined(RWCRTISMEMORYBLOCK)) */

#if (!defined(RWCRTISVALIDHEAPPOINTER))
#define RWCRTISVALIDHEAPPOINTER(_p)               (NULL != (_p))
#endif /* (!defined(RWCRTISVALIDHEAPPOINTER)) */

#if (!defined(RWCRTISVALIDPOINTER))
#define RWCRTISVALIDPOINTER(_p, _n, _r)           (NULL != (_p))
#endif /* (!defined(RWCRTISVALIDPOINTER)) */

#if (!defined(RWCRTMEMCHECKPOINT))
#define RWCRTMEMCHECKPOINT(_s) /* No Op */
#endif /* (!defined(RWCRTMEMCHECKPOINT)) */

#if (!defined(RWCRTMEMDIFFERENCE))
#define RWCRTMEMDIFFERENCE(_s1, _s2, _s3) /* No Op */
#endif /* (!defined(RWCRTMEMDIFFERENCE)) */

#if (!defined(RWCRTMEMDUMPALLOBJECTSSINCE))
#define RWCRTMEMDUMPALLOBJECTSSINCE(_s) /* No Op */
#endif /* (!defined(RWCRTMEMDUMPALLOBJECTSSINCE)) */

#if (!defined(RWCRTMEMDUMPSTATISTICS))
#define RWCRTMEMDUMPSTATISTICS(_s)                (NULL)
#endif /* (!defined(RWCRTMEMDUMPSTATISTICS)) */

#if (!defined(RWCRTSETALLOCHOOK))
#define RWCRTSETALLOCHOOK(_f)                     (NULL)
#endif /* (!defined(RWCRTSETALLOCHOOK)) */

#if (!defined(RWCRTSETBREAKALLOC))
#define RWCRTSETBREAKALLOC(_a)                    (0)
#endif /* (!defined(RWCRTSETBREAKALLOC)) */

#if (!defined(RWCRTSETDBGFLAG))
#define RWCRTSETDBGFLAG(_f)                       (0)
#endif /* (!defined(RWCRTSETDBGFLAG)) */

#if (!defined(RWCRTSETDUMPCLIENT))
#define RWCRTSETDUMPCLIENT(_f)                    (NULL)
#endif /* (!defined(RWCRTSETDUMPCLIENT)) */

#if (!defined(RWCRTSETREPORTFILE))
#define RWCRTSETREPORTFILE(_t, _f)                (NULL)
#endif /* (!defined(RWCRTSETREPORTFILE)) */

#if (!defined(RWCRTSETREPORTHOOK))
#define RWCRTSETREPORTHOOK(_f)                    (NULL)
#endif /* (!defined(RWCRTSETREPORTHOOK)) */

#if (!defined(RWCRTSETREPORTMODE))
#define RWCRTSETREPORTMODE(_t, _f)                (0)
#endif /* (!defined(RWCRTSETREPORTMODE)) */

#if (!defined(RWREGSETBREAKALLOC))
#define RWREGSETBREAKALLOC(_name) /* No op */
#endif /* (!defined(RWREGSETBREAKALLOC)) */

#if (!defined(RWREGSETASSERTPRINT))
#define RWREGSETASSERTPRINT(_name) /* No op */
#endif /* (!defined(RWREGSETASSERTPRINT)) */

#if (!defined(RWGETWINREGDWORD))
#define  RWGETWINREGDWORD(_env_var, _match) /* No op */
#endif /* (!defined(RWGETWINREGDWORD)) */

#if (!defined(RWGETWINREGBINARY))
#define  RWGETWINREGBINARY(_env_var, _match) /* No op */
#endif /* (!defined(RWGETWINREGBINARY)) */

#if (!defined(RWGETWINREGSTRING))
#define  RWGETWINREGSTRING(_env_var, _match) /* No op */
#endif /* (!defined(RWGETWINREGSTRING)) */

#if (!defined(_CRTDBG_FLAGS))
#define  _CRTDBG_FLAGS 0x33
#endif /* (!defined(_CRTDBG_FLAGS)) */

/****************************************************************************
 Global Types
 */

typedef struct RwMemoryFunctions RwMemoryFunctions;
/**
 * \ingroup datatypes
 * \struct RwMemoryFunctions
 * This type represents the memory functions used
 * by RenderWare. By default, the standard ANSI functions are used. The
 * application may install an alternative interface providing that it is ANSI
 * compliant (see API function \ref RwEngineInit):
 */
struct RwMemoryFunctions
{
    /* c.f.
     * Program Files/Microsoft Visual Studio/VC98/Include/MALLOC.H
     */
    void *(*rwmalloc)(size_t size);                   /**< rwmalloc   malloc */
    void  (*rwfree)(void *mem);                       /**< rwfree     free */
    void *(*rwrealloc)(void *mem, size_t newSize);    /**< rwrealloc  realloc */
    void *(*rwcalloc)(size_t numObj, size_t sizeObj); /**< calloc     calloc */
};

typedef struct RwFreeBlock RwFreeBlock;
/*
 * Freelists -- from Page 131
 * Advanced Animation and Rendering Techniques
 * Alan Watt and Mark Watt
 * Addison-Wesley 1993,
 * ISBN 0-201-54412-1:
 *
 * "Lastly, on a more general note concerning speedups for renderers, the
 *  implementor should be aware that a lot of suggestions for improving
 *  efficiency fall into the category of ingenious, but complex,
 *  algorithms for very specific contexts that may save a few microseconds
 *  but which make your code unreadable.  A more general computer science
 *  perspective that takes a `global view' of the renderer can be more
 *  fruitful.  For example, the renderer devotes a lot of time to
 *  allocating and deallocating chunks of memory for storing data. A lot
 *  of these chunks are always the same size - such as those that are
 *  continually required to store the data structure for fragment lists.
 *  Using memory management techniques that recognize this fact can yield
 *  considerable dividends.  One such scheme would be to hold a series of
 *  empty lists in memory for all the commonly used data structures.  An
 *  empty list for fragments, say, would contain a list of previously
 *  allocated, but no longer needed, fragment structures.  When the
 *  renderer needs memory for a new fragment, it looks first at this empty
 *  list.  If there is nothing there it allocates space directly,
 *  otherwise it takes a fragments off the end of the list and uses that.
 *  Conversely, when the renderer no longer needs a fragment, instead of
 *  freeing it, it goes onto the end of the empty list.  In the authors'
 *  experience, replacing the naive allocate/deallocate scheme with this
 *  way of managing memory can result in 100% speedup. "
 */
struct RwFreeBlock
{
    RwFreeBlock        *nextBlock;
};

typedef struct RwFreeList RwFreeList;
struct RwFreeList
{
    void              **freeListStack; /* Stack of unused entries */
    void              **freeListStackTop; /* Pointer to the top of the stack */

    RwFreeBlock        *firstBlock; /* Data start */

    RwInt32             blockSize; /* Size of block in bytes */
    RwInt32             entrySize; /* Entry size */
    RwInt32             alignmentMinusOne; /* Entry alignment minus 1 */
    RwInt32             entriesPerBlock; /* Amount of space in a block */

    RwInt32             entriesAllocated; /* Total slots allocated
                                           * (but not necessarily being used */

    /* All freelists */
    RwLLLink            lFreeList;

#if (defined(RWDEBUG) && !defined(DOXYGEN))
    const RwChar       *fileCreate;
    RwUInt32            lineCreate;
#endif /* (defined(RWDEBUG) && !defined(DOXYGEN)) */
};

/**
 * \ingroup datatypes
 * \ref RwFreeListCallBack represents
 * the function called from \ref RwFreeListForAllUsed for all used entries in a
 * given free list.
 *
 * \param  pMem   Pointer to the start of the current entries.
 *
 * \param  pData   User-defined data pointer.
 *
 * \see RwFreeListForAllUsed
 *
 */
typedef void        (*RwFreeListCallBack) (void *pMem, void *pData);
typedef void       *(*RwMemoryAllocFn)    (RwFreeList * fl);
typedef RwFreeList *(*RwMemoryFreeFn)     (RwFreeList * fl, void *pData);

/****************************************************************************
 Function prototypes
 */

#ifdef    __cplusplus
extern              "C"
{
#endif                          /* __cplusplus */

extern RwMemoryFunctions *RwOsGetMemoryInterface(void);

/*************
 * FREELISTS *
 *************/

/* Allocation and freeing */
#if (defined(RWDEBUG) && !defined(DOXYGEN))

extern RwFreeList  *_rwFreeListCreate(RwInt32 entrySize,
                                      RwInt32 entriesPerBlock,
                                      RwInt32 alignment,
                                      const RwChar *fileCreate,
                                      RwUInt32 lineCreate );

#define RwFreeListCreate(entrySize, entriesPerBlock, alignment) \
                    _rwFreeListCreate(entrySize,                \
                                      entriesPerBlock,          \
                                      alignment,                \
                                      __FILE__,                 \
                                      __LINE__)
#else /* (defined(RWDEBUG) && !defined(DOXYGEN)) */

extern RwFreeList  *RwFreeListCreate(RwInt32 entrySize,
                                     RwInt32 entriesPerBlock,
                                     RwInt32 alignment);
#endif /* (defined(RWDEBUG) && !defined(DOXYGEN)) */

extern RwBool       RwFreeListDestroy(RwFreeList * freelist);
/* Garbage collection/enumeration */
extern RwInt32      RwFreeListPurge(RwFreeList * freelist);
extern RwFreeList  *RwFreeListForAllUsed(RwFreeList * freelist,
                                         RwFreeListCallBack
                                         fpCallBack, void *pData);
extern RwInt32      RwFreeListPurgeAllFreeLists(void);

#ifdef    __cplusplus
}
#endif                          /* __cplusplus */

#if (defined(RWDEBUG) && defined(RWNOFREELISTS) && !defined(RWKEEPFREELISTS))

#if ((defined(__MWERKS__) || defined(__GNUC__)) && defined(__R5900__))

/*
 * for more on memalign, see
 * http://www.gnu.org/manual/glibc-2.0.6/html_chapter/libc_3.html#SEC28
 */
#include <rtdbmalloc.h>

#define RwFreeListAlloc(_f)     \
    memalign((1 + (_f)->alignmentMinusOne),  (_f)->entrySize)

#else /* ((defined(__MWERKS__) || defined(__GNUC__)) && defined(__R5900__)) */

#define RwFreeListAlloc(_f)     \
    RwMalloc((_f)->entrySize)

#endif /* ((defined(__MWERKS__) || defined(__GNUC__)) && defined(__R5900__)) */

#define RwFreeListFree(_f, _p)  \
    RwFree((_p))

#endif /* (defined(RWDEBUG) && defined(RWNOFREELISTS) && !defined(RWKEEPFREELISTS)) */

#if (!defined(RwFreeListAlloc))
#define RwFreeListAlloc(_f)     \
    RWSRCGLOBAL(memoryAlloc)(_f)
#endif /* (!defined(RwFreeListAlloc)) */

#if (!defined(RwFreeListFree))
#define RwFreeListFree(_f, _p)  \
    RWSRCGLOBAL(memoryFree)(_f, _p)
#endif /* (!defined(RwFreeListFree)) */


/*--- Automatically derived from: C:/daily/rwsdk/src/plcore/bastream.h ---*/

/****************************************************************************
 Defines
 */

#define rwSTREAMSTACKSIZE    512

/****************************************************************************
 Global Types
 */

/**
 * \ingroup datatypes
 * \ref RwStreamType 
 * This type represents the different types of stream that 
 * can be used. 
 * See API section \ref rwstream 
 */
enum RwStreamType
{
    rwNASTREAM = 0,     /**<Invalid stream type */
    rwSTREAMFILE,       /**<File */
    rwSTREAMFILENAME,   /**<File name */
    rwSTREAMMEMORY,     /**<Memory*/
    rwSTREAMCUSTOM,     /**<Custom */
    rwSTREAMTYPEFORCEENUMSIZEINT = RWFORCEENUMSIZEINT
};
typedef enum RwStreamType RwStreamType;

/**
 * \ingroup datatypes
 * \ref RwStreamAccessType 
 * This type represents the options available for 
 * accessing a stream when it is opened.
 * See API section \ref rwstream */
enum RwStreamAccessType
{
    rwNASTREAMACCESS = 0,   /**<Invalid stream access */
    rwSTREAMREAD,           /**<Read */
    rwSTREAMWRITE,          /**<Write */
    rwSTREAMAPPEND,         /**<Append */
    rwSTREAMACCESSTYPEFORCEENUMSIZEINT = RWFORCEENUMSIZEINT
};
typedef enum RwStreamAccessType RwStreamAccessType;

/* Memory stream */
/**
 * \ingroup datatypes
 * \typedef RwStreamMemory
 * This should be considered an opaque type.
 * Use the RwStream API functions to access.
 */
typedef struct RwStreamMemory RwStreamMemory;
#if (!defined(DOXYGEN))
struct RwStreamMemory
{
    RwUInt32            position; /* Current 'memory' position 0 is first byte */
    RwUInt32            nSize;  /* Space allocated currently */
    RwUInt8            *memBlock; /* Current memory block pointer */
};
#endif /* (!defined(DOXYGEN)) */


typedef union RwStreamFile RwStreamFile;
/**
 * \ingroup datatypes
 * \union RwStreamFile 
 * This type is used to represent a file pointer for
 * accessing data on disk through the stream mechanism.
 * See API section \ref rwstream. */
union RwStreamFile
{
    void    *fpFile;               /**< file pointer */
    const void    *constfpFile;    /**< const file pointer */
};


/* Custom stream function pointer types */
typedef             RwBool(*rwCustomStreamFnClose) (void *data);
typedef             RwUInt32(*rwCustomStreamFnRead) (void *data, void *buffer,
                                                     RwUInt32 length);
typedef             RwBool(*rwCustomStreamFnWrite) (void *data,
                                                    const void *buffer,

                                                    RwUInt32 length);
typedef             RwBool(*rwCustomStreamFnSkip) (void *data,

                                                   RwUInt32 offset);

/* Custom stream */
/**
 * \ingroup datatypes
 * \typedef  RwStreamCustom
 * This should be considered an opaque type.
 * Use the RwStream API functions to access.
 */
typedef struct RwStreamCustom RwStreamCustom;
#if (!defined(DOXYGEN))
struct RwStreamCustom
{
    rwCustomStreamFnClose sfnclose;
    rwCustomStreamFnRead sfnread;
    rwCustomStreamFnWrite sfnwrite;
    rwCustomStreamFnSkip sfnskip;
    void               *data;
};
#endif /* (!defined(DOXYGEN)) */

/* Stream */

typedef union RwStreamUnion RwStreamUnion;
/**
 * \ingroup datatypes
 * \union RwStreamUnion
 * The union of all supported stream types
 */
union RwStreamUnion
{
    RwStreamMemory      memory; /**< memory */
    RwStreamFile        file; /**< file */
    RwStreamCustom      custom; /**< custom */
};

/**
 * \ingroup datatypes
 * \typedef RwStream 
 * Binary stream for reading or writing object data. 
 * This should be considered an opaque type.
 * Use the RwStream API functions to access.
 */
typedef struct RwStream RwStream;
#if (!defined(DOXYGEN))
struct RwStream
{
    RwStreamType        type;
    RwStreamAccessType  accessType;
    RwInt32             position;
    RwStreamUnion       Type;
    RwBool              rwOwned;
};
#endif /* (!defined(DOXYGEN)) */


typedef struct RwMemory RwMemory;
/**
 * \ingroup datatypes
 * \struct RwMemory
 * This type represents a block of allocated memory.
 * It is used to specify an area of memory connected to a stream of type
 * rwSTREAMMEMORY.
 * See \ref rwstream */
struct RwMemory
{
    RwUInt8     *start; /**< starting address */
    RwUInt32    length; /**< length in bytes*/
};


/****************************************************************************
 Function prototypes
 */

#ifdef    __cplusplus
extern              "C"
{
#endif                          /* __cplusplus */

/* Open/Close streams */

extern RwStream *
_rwStreamInitialize(RwStream *stream,
                    RwBool rwOwned,
                    RwStreamType type,
                    RwStreamAccessType accessType,
                    const void *pData);

extern RwStream *
RwStreamOpen(RwStreamType type,
             RwStreamAccessType accessType,
             const void *pData);

extern RwBool       
RwStreamClose(RwStream * stream,
              void *pData);


/* Stream read/write */
extern RwUInt32     
RwStreamRead(RwStream * stream,
             void *buffer,
             RwUInt32 length);

extern RwStream *
RwStreamWrite(RwStream * stream,
              const void *buffer,
              RwUInt32 length);


/* Stream skipping */
extern RwStream *
RwStreamSkip(RwStream * stream,
             RwUInt32 offset);


#ifdef    __cplusplus
}
#endif                          /* __cplusplus */


/*--- Automatically derived from: C:/daily/rwsdk/src/plcore/batkreg.h ---*/

/****************************************************************************
 Global Types
 */

/**
 * \ingroup datatypes
 * \ref RwPluginDataChunkWriteCallBack represents the function
 * registered by \ref RwCameraRegisterPluginStream, etc. as the function that
 * writes extension data to a binary stream.
 *
 * \param  stream   Pointer to the binary stream
 *
 * \param  binaryLength   A RwInt32 value equal to the binary
 * size (in bytes) of the extension data that will be written to the binary
 * stream.
 *
 * \param  object   Pointer to the object containing the extension
 * data.
 *
 * \param  offsetInObject   A RwInt32 value equal to the byte
 * offset of the extension data in the object.
 *
 * \param  sizeInObject   A RwInt32 value equal to the size
 * (in bytes) of the extension data.
 *
 * \return Pointer to the stream
 */
typedef RwStream *(*RwPluginDataChunkWriteCallBack)(RwStream *stream, RwInt32 binaryLength, const void *object, RwInt32 offsetInObject, RwInt32 sizeInObject);

/**
 * \ingroup datatypes
 * \ref RwPluginDataChunkReadCallBack represents the function
 * registered by \ref RwCameraRegisterPluginStream, etc. as the function that
 * reads extension data from a binary stream.
 *
 * \param  stream   Pointer to the binary stream
 *
 * \param  binaryLength   A RwInt32 value equal to the binary
 * size (in bytes) of the extension data that will be read from a binary
 * stream.
 *
 * \param  object   Pointer to the object containing the extension
 * data.
 *
 * \param  offsetInObject   A RwInt32 value equal to the byte
 * offset of the extension data in the object.
 *
 * \param  sizeInObject   A RwInt32 value equal to the size
 * (in bytes) of the extension data.
 *
 * \return Pointer to the stream
 */
typedef RwStream *(*RwPluginDataChunkReadCallBack)(RwStream *stream, RwInt32 binaryLength, void *object, RwInt32 offsetInObject, RwInt32 sizeInObject);

/**
 * \ingroup datatypes
 * \ref RwPluginDataChunkGetSizeCallBack represents the callback
 * registered by \ref RwCameraRegisterPluginStream, etc. as the function that
 * determines the binary size of the extension data.
 *
 * \param  object   Pointer to the object containing the extension data.
 *
 * \param  offsetInObject   A RwInt32 value equal to the byte
 * offset of the extension data in the object.
 *
 * \param  sizeInObject   A RwInt32 value equal to the size
 * (in bytes) of the extension data.
 *
 * \return A RwInt32 value equal to the size in bytes of the plugin extension data.
 */
typedef RwInt32(*RwPluginDataChunkGetSizeCallBack)(const void *object, RwInt32 offsetInObject, RwInt32 sizeInObject);

/**
 * \ingroup datatypes
 * \ref RwPluginDataChunkAlwaysCallBack represents the callback
 * registered by \ref RwCameraSetStreamAlwaysCallBack, etc. as the
 * function that is called after the reading of plugin stream data is
 * finished (useful to set up plugin data for plugins that found no
 * data in the stream, but that cannot set up the data during the
 * \ref RwPluginObjectConstructor callback).
 *
 * \param  object   Pointer to the object containing the extension data.
 *
 * \param  offsetInObject   A RwInt32 value equal to the byte
 * offset of the extension data in the object.
 *
 * \param  sizeInObject   A RwInt32 value equal to the size
 * (in bytes) of the extension data.
 */
typedef RwBool(*RwPluginDataChunkAlwaysCallBack)(void *object, RwInt32 offsetInObject, RwInt32 sizeInObject);

/**
 * \ingroup datatypes
 * \ref RwPluginDataChunkRightsCallBack represents the callback
 * registered by RwCameraSetStreamRightsCallBack, etc. as the
 * function that is called after the reading of plugin stream data is
 * finished, and the object finalised, if and only if the object's rights
 * id was equal to that of the plugin registering the call.
 * For convience the extension data is passed to the callback.
 *
 * \param  object   Pointer to the object containing the extension data.
 *
 * \param  offsetInObject   A RwInt32 value equal to the byte
 * offset of the extension data in the object.
 *
 * \param  sizeInObject   A RwInt32 value equal to the size
 * (in bytes) of the extension data.
 *
 * \param  extraData     An RwUInt32 writen with the plugin id.
 */
typedef RwBool(*RwPluginDataChunkRightsCallBack)(void *object, RwInt32 offsetInObject, RwInt32 sizeInObject, RwUInt32 extraData);

/**
 * \ingroup datatypes
 * \ref RwPluginObjectConstructor represents the callback
 * registered by \ref RwEngineRegisterPlugin, \ref RwCameraRegisterPlugin, etc.
 * as the function that initializes either the global extension data (in the
 * case of \ref RwEngineRegisterPlugin) or the object extension data (in all
 * other cases). Registered by \ref RwCameraSetStreamAlwaysCallBack, etc.
 *
 * \param  object   Pointer to the object (global or otherwise)
 * that contains the extension data.
 *
 * \param  offsetInObject   A RwInt32 value equal to the
 * byte offset of the extension data in the object.
 *
 * \param  sizeInObject   A RwInt32 value equal to the size
 * (in bytes) of the extension data.
 *
 * \return Pointer to the object
 */
typedef void *(*RwPluginObjectConstructor)(void *object, RwInt32 offsetInObject, RwInt32 sizeInObject);

/**
 * \ingroup datatypes
 * \ref RwPluginObjectCopy represents the callback registered by
 * \ref RwCameraRegisterPlugin, etc. as the function that copies the object
 * extension data when an object is duplicated.
 *
 * \param  dstObject   Pointer to the destination object that will
 * receive the extension data.
 *
 * \param  srcObject   Pointer to the source object containing
 * extension data.
 *
 * \param  offsetInObject   A RwInt32 value equal to the byte offset
 * of the extension data in the object.
 *
 * \param  sizeInObject   A RwInt32 value equal to the size
 * (in bytes) of the extension data.
 *
 * \return Pointer to the object
 */
typedef void *(*RwPluginObjectCopy)(void *dstObject, const void *srcObject, RwInt32 offsetInObject, RwInt32 sizeInObject);

/**
 * \ingroup datatypes
 * \ref RwPluginObjectDestructor represents the callback registered
 * by \ref RwEngineRegisterPlugin, \ref RwCameraRegisterPlugin, etc. as the
 * function that destroys either the global extension data (in the case of
 * \ref RwEngineRegisterPlugin) or the object extension data (in all other
 * cases).
 *
 * \param  object   Pointer to the object (global or otherwise)
 * containing the extension data.
 *
 * \param  offsetInObject   A RwInt32 value equal to the byte
 * offset of the extension data in the object.
 *
 * \param  sizeInObject   A RwInt32 value equal to the size
 * (in bytes) of the extension data.
 *
 * \return Pointer to the object.
 */
typedef void *(*RwPluginObjectDestructor)(void *object, RwInt32 offsetInObject, RwInt32 sizeInObject);

typedef void *(*RwPluginErrorStrCallBack)(void *);

typedef struct RwPluginRegistry RwPluginRegistry;
typedef struct RwPluginRegEntry RwPluginRegEntry;

struct RwPluginRegistry
{
    RwInt32          sizeOfStruct;
    RwInt32          origSizeOfStruct;
    RwInt32          maxSizeOfStruct;
    RwInt32          staticAlloc;
    RwPluginRegEntry *firstRegEntry;
    RwPluginRegEntry *lastRegEntry;
};

struct RwPluginRegEntry
{
    RwInt32         offset;
    RwInt32         size;
    RwUInt32        pluginID;
    RwPluginDataChunkReadCallBack readCB;
    RwPluginDataChunkWriteCallBack writeCB;
    RwPluginDataChunkGetSizeCallBack getSizeCB;
    RwPluginDataChunkAlwaysCallBack alwaysCB;
    RwPluginDataChunkRightsCallBack rightsCB;
    RwPluginObjectConstructor constructCB;
    RwPluginObjectDestructor destructCB;
    RwPluginObjectCopy copyCB;
    RwPluginErrorStrCallBack errStrCB;
    RwPluginRegEntry *nextRegEntry;
    RwPluginRegEntry *prevRegEntry;
    RwPluginRegistry *parentRegistry;
};



/****************************************************************************
 Function prototypes
 */

#ifdef    __cplusplus
extern          "C"
{
#endif         /* __cplusplus */


/* Registering toolkits and allocating memory */
extern RwBool
_rwPluginRegistrySetStaticPluginsSize(RwPluginRegistry * reg,
                                      RwInt32 size);
extern RwInt32
_rwPluginRegistryAddPlugin(RwPluginRegistry * reg,
                           RwInt32 size,
                           RwUInt32 pluginID,
                           RwPluginObjectConstructor constructCB,
                           RwPluginObjectDestructor destructCB,
                           RwPluginObjectCopy copyCB);
extern RwInt32
_rwPluginRegistryGetPluginOffset(const RwPluginRegistry *reg,
                                RwUInt32 pluginID);

/* Initializing/De-initializing instances */
extern const
RwPluginRegistry *_rwPluginRegistryInitObject(const RwPluginRegistry * reg,
                                             void *object);
extern const
RwPluginRegistry *_rwPluginRegistryDeInitObject(const RwPluginRegistry * reg,
                                               void *object);
extern const
RwPluginRegistry *_rwPluginRegistryCopyObject(const RwPluginRegistry * reg,
                                             void *dstObject,
                                             const void *srcObject);

#ifdef RWDEBUG
extern RwBool
_rwPluginRegistryValidateObject(const RwPluginRegistry * reg,
                               const void *object);
#endif         /* RWDEBUG */


#ifdef    __cplusplus
}
#endif         /* __cplusplus */

/* Compatibility macros */

#define rwPluginRegistryOpen() \
       _rwPluginRegistryOpen()
#define rwPluginRegistryClose() \
       _rwPluginRegistryClose()
#define rwPluginRegistrySetStaticPluginsSize(reg, size) \
       _rwPluginRegistrySetStaticPluginsSize(reg, size)
#define rwPluginRegistryAddPlugin(reg, size, pluginID, constructCB, destructCB, copyCB) \
       _rwPluginRegistryAddPlugin(reg, size, pluginID, constructCB, destructCB, copyCB)
#define rwPluginRegistryGetPluginOffset(reg, pluginID) \
       _rwPluginRegistryGetPluginOffset(reg, pluginID)
#define rwPluginRegistryInitObject(reg, object) \
       _rwPluginRegistryInitObject(reg, object)
#define rwPluginRegistryDeInitObject(reg, object) \
       _rwPluginRegistryDeInitObject(reg, object)
#define rwPluginRegistryCopyObject(reg, dstObject, srcObject) \
       _rwPluginRegistryCopyObject(reg, dstObject, srcObject)
#define rwPluginRegistryValidateObject(reg, object) \
       _rwPluginRegistryValidateObject(reg, object)


/*--- Automatically derived from: C:/daily/rwsdk/src/plcore/batkbin.h ---*/

/****************************************************************************
 Function prototypes
 */

#ifdef    __cplusplus
extern "C"
{
#endif                          /* __cplusplus */

/* Plugin binary stream stuff */
extern RwInt32 _rwPluginRegistryAddPluginStream(
                   RwPluginRegistry *reg,
                   RwUInt32 pluginID,
                   RwPluginDataChunkReadCallBack readCB,
                   RwPluginDataChunkWriteCallBack writeCB,
                   RwPluginDataChunkGetSizeCallBack getSizeCB);
extern RwInt32 _rwPluginRegistryAddPlgnStrmlwysCB(
                   RwPluginRegistry *reg,
                   RwUInt32 pluginID,
                   RwPluginDataChunkAlwaysCallBack alwaysCB);
extern RwInt32 _rwPluginRegistryAddPlgnStrmRightsCB(
                   RwPluginRegistry *reg,
                   RwUInt32 pluginID,
                   RwPluginDataChunkRightsCallBack rightsCB);
extern const RwPluginRegistry *_rwPluginRegistryReadDataChunks(const RwPluginRegistry *reg,
                                                              RwStream *stream,
                                                              void *object);
extern const RwPluginRegistry *_rwPluginRegistryWriteDataChunks(const RwPluginRegistry *reg,
                                                               RwStream *stream,
                                                               const void *object);
extern const RwPluginRegistry *_rwPluginRegistrySkipDataChunks(const RwPluginRegistry * reg,
                                                              RwStream * stream);
extern RwInt32 _rwPluginRegistryGetSize(const RwPluginRegistry *reg, const void *object);
extern const RwPluginRegistry *_rwPluginRegistryInvokeRights(const RwPluginRegistry *reg,
                                                             RwUInt32 id,
                                                             void *obj,
                                                             RwUInt32 extraData);

#ifdef    __cplusplus
}
#endif                          /* __cplusplus */

/* Compatibility macros */

#define rwPluginRegistryAddPluginStream(reg, pluginID, readCB, writeCB, getSizeCB) \
       _rwPluginRegistryAddPluginStream(reg, pluginID, readCB, writeCB, getSizeCB)
#define rwPluginRegistryAddPluginStreamAlwaysCB(reg, pluginID, alwaysCB) \
       _rwPluginRegistryAddPlgnStrmlwysCB(reg, pluginID, alwaysCB)
#define rwPluginRegistryReadDataChunks(reg, stream, object) \
       _rwPluginRegistryReadDataChunks(reg, stream, object)
#define rwPluginRegistryWriteDataChunks(reg, stream, object) \
       _rwPluginRegistryWriteDataChunks(reg, stream, object)
#define rwPluginRegistrySkipDataChunks(reg, stream) \
       _rwPluginRegistrySkipDataChunks(reg, stream)
#define rwPluginRegistryGetSize(reg, object) \
       _rwPluginRegistryGetSize(reg, object)



/*--- Automatically derived from: C:/daily/rwsdk/src/plcore/bamatrix.h ---*/

/****************************************************************************
 Defines
 */

#define RWMATRIXALIGNMENT(_mat) \
   (! (((rwMATRIXALIGNMENT)-1) & ((RwUInt32)(_mat))))

#if (defined(RWMATRIXMONITOR))
# if (defined(_MSC_VER))
#  if ((_MSC_VER>=1000) && defined(_DEBUG))

typedef char        MatrixString[1024];

#define RWMATRIXPRINT(_matrix)                             \
MACRO_START                                                \
{                                                          \
    MatrixString        message;                           \
    MatrixString        output;                            \
                                                           \
    if (NULL != (_matrix))                                 \
    {                                                      \
        const RwV3d * const _x = &(_matrix)->right;        \
        const RwV3d * const _y = &(_matrix)->up;           \
        const RwV3d * const _z = &(_matrix)->at;           \
        const RwV3d * const _w = &(_matrix)->pos;          \
                                                           \
        _snprintf(message, sizeof(MatrixString),           \
                  "[ [ %8.4f, %8.4f, %8.4f, %8.4f ]\n"     \
                  "  [ %8.4f, %8.4f, %8.4f, %8.4f ]\n"     \
                  "  [ %8.4f, %8.4f, %8.4f, %8.4f ]\n"     \
                  "  [ %8.4f, %8.4f, %8.4f, %8.4f ] ]\n"   \
                  "  %08x == flags\n",                     \
                  _x->x, _x->y, _x->z, (RwReal) 0,         \
                  _y->x, _y->y, _y->z, (RwReal) 0,         \
                  _z->x, _z->y, _z->z, (RwReal) 0,         \
                  _w->x, _w->y, _w->z, (RwReal) 1,         \
                  (_matrix)->flags);                       \
    }                                                      \
    else                                                   \
    {                                                      \
        _snprintf(message, sizeof(MatrixString),           \
                  "NULL");                                 \
    }                                                      \
                                                           \
    _snprintf(output, sizeof(MatrixString),                \
              "%s(%d): %s [%p] ==\n%s\n",                  \
              __FILE__, __LINE__,                          \
              #_matrix, _matrix, message);                 \
                                                           \
    OutputDebugString(RWSTRING(output));                   \
}                                                          \
MACRO_STOP

#  endif /* ((_MSC_VER>=1000) && defined(_DEBUG)) */
# endif /* (defined(_MSC_VER)) */
#endif /* (defined(RWMATRIXMONITOR)) */

#if (!(defined(RWMATRIXPRINT)))
#define RWMATRIXPRINT(_matrix) /* No op */
#endif /* (!(defined(RWMATRIXPRINT))) */

/**
 * \ingroup datatypes
 * enum RwOpCombineType 
 * This type represents a combination operator which 
 * can be applied to frames and matrices.
 * The operator determines the order 
 * in which one object is combined with another 
 */
enum RwOpCombineType
{
    rwCOMBINEREPLACE = 0,   /**<Replace - 
                                all previous transformations are lost */
    rwCOMBINEPRECONCAT,     /**<Pre-concatenation - 
                                the given transformation is applied 
                                before all others */
    rwCOMBINEPOSTCONCAT,    /**<Post-concatenation - 
                                the given transformation is applied 
                                after all others */
    rwOPCOMBINETYPEFORCEENUMSIZEINT = RWFORCEENUMSIZEINT
};

/**
 * \ingroup datatypes
 * \typedef RwOpCombineType typedef for enum RwOpCombineType
 */
typedef enum RwOpCombineType RwOpCombineType;

/* External flags (bits 0-15) */

/* Internal flags (bits 16-31) */
enum RwMatrixType
{
    rwMATRIXTYPENORMAL = 0x00000001,
    rwMATRIXTYPEORTHOGANAL = 0x00000002,
    rwMATRIXTYPEORTHONORMAL = 0x00000003,
    rwMATRIXTYPEMASK = 0x00000003,
    rwMATRIXTYPEFORCEENUMSIZEINT = RWFORCEENUMSIZEINT
};
typedef enum RwMatrixType RwMatrixType;

enum RwMatrixFlag
{
    rwMATRIXINTERNALIDENTITY = 0x00020000,
    rwMATRIXFLAGFORCEENUMSIZEINT = RWFORCEENUMSIZEINT
};
typedef enum RwMatrixFlag RwMatrixFlag;

/* Flags describing what will optimize for */
enum RwMatrixOptimizations
{
    rwMATRIXOPTIMIZE_IDENTITY = 0x00020000,
    rwMATRIXOPTIMIZATIONSFORCEENUMSIZEINT = RWFORCEENUMSIZEINT
};
typedef enum RwMatrixOptimizations RwMatrixOptimizations;

/****************************************************************************
 Global Types
 */

#if (!defined(DOXYGEN))
struct RwMatrixTag
{
    /* These are padded to be 16 byte quantities per line */
    RwV3d               right;
    RwUInt32            flags;
    RwV3d               up;
    RwUInt32            pad1;
    RwV3d               at;
    RwUInt32            pad2;
    RwV3d               pos;
    RwUInt32            pad3;
};

/*
 * RwMatrix & RwMatrixTag must be different otherwise the alignment
 * directive is ignored when compiling under C++ on Visual C
 */
typedef struct RwMatrixTag RWALIGN(RwMatrix, rwMATRIXALIGNMENT);

#else

/*
 * RwMatrix must appear as the struct name & the typedef,
 * otherwise Doxygen puts RwMatrixTag in the API ref instead of RwMatrix!
 */

/**
 * \ingroup datatypes
 * \typedef RwMatrix
 * Matrix to define transformations. 
 * This should be considered an opaque type.
 * Use the RwMatrix API functions to access.
 */
typedef struct RwMatrix RWALIGN(RwMatrix, rwMATRIXALIGNMENT);
#endif /* (!defined(DOXYGEN)) */

#if (!defined(RwMatrixCopyMacro))
#define RwMatrixCopyMacro(_target, _source)             \
    ( *(_target) = *(_source) )
#endif /* (!defined(RwMatrixCopyMacro)) */

#if (!defined(RwMatrixSetIdentityMacro))
#define RwMatrixSetIdentityMacro(m)                                     \
MACRO_START                                                             \
{                                                                       \
    (m)->right.x = (m)->up.y    = (m)->at.z    = (RwReal)((1.0));       \
    (m)->right.y = (m)->right.z = (m)->up.x    = (RwReal)((0.0));       \
    (m)->up.z    = (m)->at.x    = (m)->at.y    = (RwReal)((0.0));       \
    (m)->pos.x   = (m)->pos.y   = (m)->pos.z   = (RwReal)((0.0));       \
    rwMatrixSetFlags((m),                                               \
                     rwMatrixGetFlags(m) |                              \
                     (rwMATRIXINTERNALIDENTITY |                        \
                      rwMATRIXTYPEORTHONORMAL));                        \
}                                                                       \
MACRO_STOP
#endif /* (!defined(RwMatrixSetIdentityMacro)) */

typedef void (RWASMCALL * rwMatrixMultFn) (RwMatrix * dstMat,
                                           const RwMatrix * matA,
                                           const RwMatrix * matB);

/*
 * \ingroup datatypes
 * \typedef RwMatrixTolerance
 * Typedef for RwMatrixTolerance structure
 */
typedef struct RwMatrixTolerance RwMatrixTolerance;

/*
 * \ingroup datatypes
 * \struct RwMatrixTolerance
 * Holds tolerances for matrix optimizations with \ref RwMatrixOptimize
 */
struct RwMatrixTolerance
{
    RwReal              Normal;
        /**< Tolerance within which matrix is deemed to be normal   */
    RwReal              Orthogonal;
        /**< Tolerance within which matrix is deemed to be orthogonal */
    RwReal              Identity;
        /**< Tolerance within which matrix is deemed to be identity */
};


/****************************************************************************
 Function prototypes
 */

/* Matrix operations */

#ifdef    __cplusplus
extern "C"
{
#endif                          /* __cplusplus */

extern RwBool
RwEngineGetMatrixTolerances(RwMatrixTolerance * const tolerance);

extern RwBool
RwEngineSetMatrixTolerances(const RwMatrixTolerance * const tolerance);

/* Update */
#define rwMatrixSetFlags(m, flagsbit)     ((m)->flags = (flagsbit))
#define rwMatrixGetFlags(m)               ((m)->flags)
#define rwMatrixTestFlags(m, flagsbit)    ((m)->flags & (RwInt32)(flagsbit))

/* Creation/destruction */
extern RwBool       
RwMatrixDestroy(RwMatrix * mpMat);

extern RwMatrix *
RwMatrixCreate(void);

#ifdef RWDEBUG

/* Functions for debug */
extern void
RwMatrixCopy(RwMatrix * dstMatrix, const RwMatrix * srcMatrix);

extern void         
RwMatrixSetIdentity(RwMatrix * matrix);

#else /* RWDEBUG */

#define RwMatrixCopy(dst, src)   RwMatrixCopyMacro(dst, src)
#define RwMatrixSetIdentity(m)   RwMatrixSetIdentityMacro(m)

#endif /* RWDEBUG */

/* Matrix multiply */
extern RwMatrix *
RwMatrixMultiply(RwMatrix * matrixOut,
                 const RwMatrix * MatrixIn1,
                 const RwMatrix * matrixIn2);

extern RwMatrix *
RwMatrixTransform(RwMatrix * matrix,
                  const RwMatrix * transform,
                  RwOpCombineType combineOp);

/* Normalise */
extern RwMatrix *
RwMatrixOrthoNormalize(RwMatrix * matrixOut,
                       const RwMatrix * matrixIn);

/* Inversion */
extern RwMatrix *
RwMatrixInvert(RwMatrix * matrixOut,
               const RwMatrix * matrixIn);

/* Unary matrix operations */
extern RwMatrix *
RwMatrixScale(RwMatrix * matrix,
              const RwV3d * scale,
              RwOpCombineType combineOp);

extern RwMatrix *
RwMatrixTranslate(RwMatrix * matrix,
                  const RwV3d * translation,
                  RwOpCombineType combineOp);

extern RwMatrix *
RwMatrixRotate(RwMatrix * matrix,
               const RwV3d * axis,
               RwReal angle,
               RwOpCombineType combineOp);

extern RwMatrix *
RwMatrixRotateOneMinusCosineSine(RwMatrix * matrix,
                                 const RwV3d * unitAxis,
                                 RwReal oneMinusCosine,
                                 RwReal sine,
                                 RwOpCombineType combineOp);

/* Query what the matrix is */
extern const RwMatrix *RwMatrixQueryRotate(const RwMatrix * matrix,
                                           RwV3d * unitAxis,
                                           RwReal * angle,
                                           RwV3d * center);

/* Get components */
#ifndef RWDEBUG

#define RwMatrixGetRight(m)    (&(m)->right)
#define RwMatrixGetUp(m)       (&(m)->up)
#define RwMatrixGetAt(m)       (&(m)->at)
#define RwMatrixGetPos(m)      (&(m)->pos)

#else /* RWDEBUG */

extern RwV3d *
RwMatrixGetRight(RwMatrix * matrix);

extern RwV3d *
RwMatrixGetUp(RwMatrix * matrix);

extern RwV3d *
RwMatrixGetAt(RwMatrix * matrix);

extern RwV3d *
RwMatrixGetPos(RwMatrix * matrix);

#endif /* RWDEBUG */

/* Update the internal matrix state wrt its elements */
extern RwMatrix *
RwMatrixUpdate(RwMatrix * matrix);

/* Update the internal matrix flags wrt its elements */
extern RwMatrix *
RwMatrixOptimize(RwMatrix * matrix,
                 const RwMatrixTolerance *
                 tolerance);

extern RwReal
_rwMatrixDeterminant(const RwMatrix * matrix);

extern RwReal
_rwMatrixNormalError(const RwMatrix * matrix);

extern RwReal
_rwMatrixOrthogonalError(const RwMatrix * matrix);

extern RwReal
_rwMatrixIdentityError(const RwMatrix * matrix);

#ifdef    __cplusplus
}
#endif                          /* __cplusplus */

/* Compatibility macros */

#define rwMatrixSetOptimizations(optimizeFlags) \
       _rwMatrixSetOptimizations(optimizeFlags)

#define rwMatrixSetMultFn(multMat) \
       _rwMatrixSetMultFn(multMat)

#define rwMatrixOpen(instance, offset, size) \
       _rwMatrixOpen(instance, offset, size)

#define rwMatrixClose(instance, offset, size) \
       _rwMatrixClose(instance, offset, size)

/* Initialisation/deinitialisation */
#define rwMatrixInitialize(m, t)                \
MACRO_START                                     \
{                                               \
    rwMatrixSetFlags((m), (t));                 \
}                                               \
MACRO_STOP

#define rwMatrixIsNormal(_matrix, _epsilon)        \
  ( (_epsilon) >= _rwMatrixNormalError(_matrix) )

#define rwMatrixIsOrthogonal(_matrix, _epsilon)        \
  ( (_epsilon) >= _rwMatrixOrthogonalError(_matrix) )

#define rwMatrixIsOrthonormal(_matrix, _epsilon)        \
  ( rwMatrixIsNormal(_matrix, _epsilon) &&              \
    rwMatrixIsOrthogonal(_matrix, _epsilon)  )

#define rwMatrixIsOrthonormalPositive(_matrix, _epsilon)                \
  ( rwMatrixIsOrthonormal(_matrix, _epsilon) &&                         \
( (((RwReal)1) - (_epsilon)) <= _rwMatrixDeterminant(_matrix) ) )

#define rwMatrixIsIdentity(_matrix, _epsilon)           \
   ( (_epsilon) >= _rwMatrixIdentityError(_matrix) )

#define rwMatrixValidFlags(_matrix, _epsilon)                           \
   ( (_matrix) &&                       /* valid pointer */             \
 ( ( !( rwMatrixGetFlags(_matrix) &    /* not flagged as identity */    \
        rwMATRIXINTERNALIDENTITY) ) || /* .. or actually is */          \
   rwMatrixIsIdentity(_matrix, _epsilon)) &&                            \
 ( ( !( rwMatrixGetFlags(_matrix) &    /* not flagged as normal */      \
        rwMATRIXTYPENORMAL) ) ||  /* ... or actually is */              \
   rwMatrixIsNormal(_matrix, _epsilon)) &&                              \
 ( ( !( rwMatrixGetFlags(_matrix) &    /* not flagged as orthogonal */  \
        rwMATRIXTYPEORTHOGANAL) ) ||  /* ... or actually is */          \
   rwMatrixIsOrthogonal(_matrix, _epsilon)) )

#define      rwMat01Det(_mAA)                                           \
  ( (_mAA) )

#define      rwMat02Det(_mAA, _mAB,                                     \
                        _mBA, _mBB)                                     \
  ( (_mAA) * rwMat01Det(_mBB)                                           \
  - (_mAB) * rwMat01Det(_mBA)                                           \
  )

#define      rwMat03Det(_mAA, _mAB, _mAC,                               \
                        _mBA, _mBB, _mBC,                               \
                        _mCA, _mCB, _mCC)                               \
  ( (_mAA) * rwMat02Det(_mBB, _mBC,                                     \
                        _mCB, _mCC)                                     \
  - (_mAB) * rwMat02Det(_mBA, _mBC,                                     \
                        _mCA, _mCC)                                     \
  + (_mAC) * rwMat02Det(_mBA, _mBB,                                     \
                        _mCA, _mCB)                                     \
  )

#define      rwMat04Det(_mAA, _mAB, _mAC, _mAD,                         \
                        _mBA, _mBB, _mBC, _mBD,                         \
                        _mCA, _mCB, _mCC, _mCD,                         \
                        _mDA, _mDB, _mDC, _mDD)                         \
  ( (_mAA) * rwMat03Det(_mBB, _mBC, _mBD,                               \
                        _mCB, _mCC, _mCD,                               \
                        _mDB, _mDC, _mDD)                               \
  - (_mAB) * rwMat03Det(_mBA, _mBC, _mBD,                               \
                        _mCA, _mCC, _mCD,                               \
                        _mDA, _mDC, _mDD)                               \
  + (_mAC) * rwMat03Det(_mBA, _mBB, _mBD,                               \
                        _mCA, _mCB, _mCD,                               \
                        _mDA, _mDB, _mDD)                               \
  - (_mAD) * rwMat03Det(_mBA, _mBB, _mBC,                               \
                        _mCA, _mCB, _mCC,                               \
                        _mDA, _mDB, _mDC)                               \
  )


#define rwMat02Inv(_rAA, _rAB,                                          \
                   _rBA, _rBB)                                          \
                   _mAA, _mAB,                                          \
                   _mBA, _mBB)                                          \
MACRO_START                                                             \
{                                                                       \
    RwSplitBits determinant;                                            \
                                                                        \
    (_rAA) =  rwMat01Det(_mBB);                                         \
    (_rAB) = -rwMat01Det(_mAB);                                         \
                                                                        \
    determinant.nReal = ( (_rAA) * (_mAA) +                             \
                          (_rAB) * (_mBA) );                            \
                                                                        \
                                                                        \
    {                                                                   \
        const RwReal normalize = ( (determinant.nInt != 0)?             \
                                   (((RwReal)1)/determinant.nReal):     \
                                   ((RwReal)1) );                       \
                                                                        \
        (_rAA) *= normalize;                                            \
        (_rAB) *= normalize;                                            \
                                                                        \
        (_rBA) = -rwMat01Det(_mBA) * normalize;                         \
        (_rBB) =  rwMat01Det(_mAA) * normalize;                         \
    }                                                                   \
}                                                                       \
MACRO_STOP

#define    rwMat03Inv(_rAA, _rAB, _rAC,                                 \
                      _rBA, _rBB, _rBC,                                 \
                      _rCA, _rCB, _rCC,                                 \
                      _mAA, _mAB, _mAC,                                 \
                      _mBA, _mBB, _mBC,                                 \
                      _mCA, _mCB, _mCC)                                 \
MACRO_START                                                             \
{                                                                       \
    RwSplitBits determinant;                                            \
                                                                        \
    (_rAA)=  rwMat02Det(_mBB, _mBC,                                     \
                        _mCB, _mCC);                                    \
    (_rAB)= -rwMat02Det(_mAB, _mAC,                                     \
                        _mCB, _mCC);                                    \
    (_rAC)=  rwMat02Det(_mAB, _mAC,                                     \
                        _mBB, _mBC);                                    \
                                                                        \
    determinant.nReal = ( (_rAA) * (_mAA) +                             \
                          (_rAB) * (_mBA) +                             \
                          (_rAC) * (_mCA) );                            \
                                                                        \
    {                                                                   \
        const RwReal normalize = ( (determinant.nInt != 0)?             \
                                   (((RwReal)1)/determinant.nReal):     \
                                   ((RwReal)1) );                       \
        (_rAA) *= normalize;                                            \
        (_rAB) *= normalize;                                            \
        (_rAC) *= normalize;                                            \
                                                                        \
        (_rBA)= -rwMat02Det(_mBA, _mBC,                                 \
                            _mCA, _mCC) * normalize ;                   \
        (_rBB)=  rwMat02Det(_mAA, _mAC,                                 \
                            _mCA, _mCC) * normalize ;                   \
        (_rBC)= -rwMat02Det(_mAA, _mAC,                                 \
                            _mBA, _mBC) * normalize ;                   \
                                                                        \
        (_rCA)=  rwMat02Det(_mBA, _mBB,                                 \
                            _mCA, _mCB) * normalize ;                   \
        (_rCB)= -rwMat02Det(_mAA, _mAB,                                 \
                            _mCA, _mCB) * normalize ;                   \
        (_rCC)=  rwMat02Det(_mAA, _mAB,                                 \
                            _mBA, _mBB) * normalize ;                   \
    }                                                                   \
                                                                        \
}                                                                       \
MACRO_STOP

#define    rwMat04Inv(_rAA, _rAB, _rAC, _rAD,                           \
                      _rBA, _rBB, _rBC, _rBD,                           \
                      _rCA, _rCB, _rCC, _rCD,                           \
                      _rDA, _rDB, _rDC, _rDD,                           \
                      _mAA, _mAB, _mAC, _mAD,                           \
                      _mBA, _mBB, _mBC, _mBD,                           \
                      _mCA, _mCB, _mCC, _mCD,                           \
                      _mDA, _mDB, _mDC, _mDD)                           \
MACRO_START                                                             \
{                                                                       \
    RwSplitBits determinant;                                            \
                                                                        \
    (_rAA)=  rwMat03Det(_mBB, _mBC, _mBD,                               \
                        _mCB, _mCC, _mCD,                               \
                        _mDB, _mDC, _mDD);                              \
    (_rAB)= -rwMat03Det(_mAB, _mAC, _mAD,                               \
                        _mCB, _mCC, _mCD,                               \
                        _mDB, _mDC, _mDD);                              \
    (_rAC)=  rwMat03Det(_mAB, _mAC, _mAD,                               \
                        _mBB, _mBC, _mBD,                               \
                        _mDB, _mDC, _mDD);                              \
    (_rAD)= -rwMat03Det(_mAB, _mAC, _mAD,                               \
                        _mBB, _mBC, _mBD,                               \
                        _mCB, _mCC, _mCD);                              \
                                                                        \
    determinant.nReal = ( (_rAA) * (_mAA) +                             \
                          (_rAB) * (_mBA) +                             \
                          (_rAC) * (_mCA) +                             \
                          (_rAD) * (_mDA) );                            \
                                                                        \
    {                                                                   \
        const RwReal normalize = ( (determinant.nInt != 0)?             \
                                   (((RwReal)1)/determinant.nReal):     \
                                   ((RwReal)1) );                       \
                                                                        \
        (_rAA) *= normalize;                                            \
        (_rAB) *= normalize;                                            \
        (_rAC) *= normalize;                                            \
        (_rAD) *= normalize;                                            \
                                                                        \
        (_rBA)= -rwMat03Det(_mBA, _mBC, _mBD,                           \
                            _mCA, _mCC, _mCD,                           \
                            _mDA, _mDC, _mDD) * normalize ;             \
        (_rBB)=  rwMat03Det(_mAA, _mAC, _mAD,                           \
                            _mCA, _mCC, _mCD,                           \
                            _mDA, _mDC, _mDD) * normalize ;             \
        (_rBC)= -rwMat03Det(_mAA, _mAC, _mAD,                           \
                            _mBA, _mBC, _mBD,                           \
                            _mDA, _mDC, _mDD) * normalize ;             \
        (_rBD)=  rwMat03Det(_mAA, _mAC, _mAD,                           \
                            _mBA, _mBC, _mBD,                           \
                            _mCA, _mCC, _mCD) * normalize ;             \
                                                                        \
        (_rCA)=  rwMat03Det(_mBA, _mBB, _mBD,                           \
                            _mCA, _mCB, _mCD,                           \
                            _mDA, _mDB, _mDD) * normalize ;             \
        (_rCB)= -rwMat03Det(_mAA, _mAB, _mAD,                           \
                            _mCA, _mCB, _mCD,                           \
                            _mDA, _mDB, _mDD) * normalize ;             \
        (_rCC)=  rwMat03Det(_mAA, _mAB, _mAD,                           \
                            _mBA, _mBB, _mBD,                           \
                            _mDA, _mDB, _mDD) * normalize ;             \
        (_rCD)= -rwMat03Det(_mAA, _mAB, _mAD,                           \
                            _mBA, _mBB, _mBD,                           \
                            _mCA, _mCB, _mCD) * normalize ;             \
                                                                        \
        (_rDA)= -rwMat03Det(_mBA, _mBB, _mBC,                           \
                            _mCA, _mCB, _mCC,                           \
                            _mDA, _mDB, _mDC) * normalize ;             \
        (_rDB)=  rwMat03Det(_mAA, _mAB, _mAC,                           \
                            _mCA, _mCB, _mCC,                           \
                            _mDA, _mDB, _mDC) * normalize ;             \
        (_rDC)= -rwMat03Det(_mAA, _mAB, _mAC,                           \
                            _mBA, _mBB, _mBC,                           \
                            _mDA, _mDB, _mDC) * normalize ;             \
        (_rDD)=  rwMat03Det(_mAA, _mAB, _mAC,                           \
                            _mBA, _mBB, _mBC,                           \
                            _mCA, _mCB, _mCC) * normalize ;             \
    }                                                                   \
}                                                                       \
MACRO_STOP


/*--- Automatically derived from: C:/daily/rwsdk/driver/d3d8/drvmodel.h ---*/
#ifndef D3D8_DRVMODEL_H
#define D3D8_DRVMODEL_H

#if (defined(__ICL))
/* Avoid voluminous
 *   'warning #344: typedef name has already been declared (with same type)'
 * warnings from MS include files
 */
#pragma warning( disable : 344 )
#endif /* (defined(__ICL)) */

//nobody needed that - AAP
//#include <windows.h>

#if (defined(RWDEBUG))
#if (defined(RWMEMDEBUG) && !defined(_CRTDBG_MAP_ALLOC))
#define _CRTDBG_MAP_ALLOC
#endif /* defined(RWMEMDEBUG) && !defined(_CRTDBG_MAP_ALLOC)) */
#include <crtdbg.h>
#define ERR_WRAP(A) (_rwRePrintErrorDDD3D((A), __FILE__, __LINE__))
#endif /* (defined(RWDEBUG)) */

#if (!defined(ERR_WRAP))
#define ERR_WRAP(A) (A)
#endif /* (!defined(ERR_WRAP)) */

/****************************************************************************
 Defines
 */

/* Set true depth information (for fogging, eg) */
#define RwIm2DVertexSetCameraX(vert, camx)          /* Nothing */
#define RwIm2DVertexSetCameraY(vert, camy)          /* Nothing */
#define RwIm2DVertexSetCameraZ(vert, camz)          /* Nothing */

#define RwIm2DVertexSetRecipCameraZ(vert, recipz)   ((vert)->rhw = recipz)

#define RwIm2DVertexGetCameraX(vert)                (cause an error)
#define RwIm2DVertexGetCameraY(vert)                (cause an error)
#define RwIm2DVertexGetCameraZ(vert)                (cause an error)
#define RwIm2DVertexGetRecipCameraZ(vert)           ((vert)->rhw)

/* Set screen space coordinates in a device vertex */
#define RwIm2DVertexSetScreenX(vert, scrnx)         ((vert)->x = (scrnx))
#define RwIm2DVertexSetScreenY(vert, scrny)         ((vert)->y = (scrny))
#define RwIm2DVertexSetScreenZ(vert, scrnz)         ((vert)->z = (scrnz))
#define RwIm2DVertexGetScreenX(vert)                ((vert)->x)
#define RwIm2DVertexGetScreenY(vert)                ((vert)->y)
#define RwIm2DVertexGetScreenZ(vert)                ((vert)->z)

/* Set texture coordinates in a device vertex */
#define RwIm2DVertexSetU(vert, texU, recipz)        ((vert)->u = (texU))
#define RwIm2DVertexSetV(vert, texV, recipz)        ((vert)->v = (texV))
#define RwIm2DVertexGetU(vert)                      ((vert)->u)
#define RwIm2DVertexGetV(vert)                      ((vert)->v)

/* Modify the luminance stuff */
#define RwIm2DVertexSetRealRGBA(vert, red, green, blue, alpha)  \
    ((vert)->emissiveColor =                                    \
     (((RwFastRealToUInt32(alpha)) << 24) |                        \
      ((RwFastRealToUInt32(red)) << 16) |                          \
      ((RwFastRealToUInt32(green)) << 8) |                         \
      ((RwFastRealToUInt32(blue)))))

#define RwIm2DVertexSetIntRGBA(vert, red, green, blue, alpha)   \
    ((vert)->emissiveColor =                                    \
     ((((RwUInt32)(alpha)) << 24) |                             \
      (((RwUInt32)(red)) << 16) |                               \
      (((RwUInt32)(green)) << 8) |                              \
      (((RwUInt32)(blue)))))

#define RwIm2DVertexGetRed(vert)    \
    (((vert)->emissiveColor >> 16) & 0xFF)

#define RwIm2DVertexGetGreen(vert)  \
    (((vert)->emissiveColor >> 8) & 0xFF)

#define RwIm2DVertexGetBlue(vert)   \
    ((vert)->emissiveColor & 0xFF)

#define RwIm2DVertexGetAlpha(vert)  \
    (((vert)->emissiveColor >> 24) & 0xFF)

#define RwIm2DVertexCopyRGBA(dst, src)  \
    ((dst)->emissiveColor = (src)->emissiveColor)

/* Clipper stuff */

#define RwIm2DVertexClipRGBA(o, i, n, f)                                \
MACRO_START                                                             \
{                                                                       \
    const RwInt32        _factor =                                      \
        (RwFastRealToUInt32(i * (RwReal)(255))) & 255;                  \
                                                                        \
    (o)->emissiveColor =                                                \
        (((((RwIm2DVertexGetAlpha(f) - RwIm2DVertexGetAlpha(n)) *       \
            _factor) >> 8) + RwIm2DVertexGetAlpha(n)) << 24) |          \
        (((((RwIm2DVertexGetRed(f) - RwIm2DVertexGetRed(n)) *           \
            _factor) >> 8) + RwIm2DVertexGetRed(n)) << 16) |            \
        (((((RwIm2DVertexGetGreen(f) - RwIm2DVertexGetGreen(n)) *       \
            _factor) >> 8) + RwIm2DVertexGetGreen(n)) << 8) |           \
        (((((RwIm2DVertexGetBlue(f) - RwIm2DVertexGetBlue(n)) *         \
            _factor) >> 8) + RwIm2DVertexGetBlue(n)));                  \
}                                                                       \
MACRO_STOP

/* LEGACY-SUPPORT macros */
#define RWIM2DVERTEXSetCameraX(vert, camx)  RwIm2DVertexSetCameraX(vert, camx)
#define RWIM2DVERTEXSetCameraY(vert, camy)  RwIm2DVertexSetCameraY(vert, camy)
#define RWIM2DVERTEXSetCameraZ(vert, camz)  RwIm2DVertexSetCameraZ(vert, camz)
#define RWIM2DVERTEXSetRecipCameraZ(vert, recipz) \
    RwIm2DVertexSetRecipCameraZ(vert, recipz)
#define RWIM2DVERTEXGetCameraX(vert)        RwIm2DVertexGetCameraX(vert)
#define RWIM2DVERTEXGetCameraY(vert)        RwIm2DVertexGetCameraY(vert)
#define RWIM2DVERTEXGetCameraZ(vert)        RwIm2DVertexGetCameraZ(vert)
#define RWIM2DVERTEXGetRecipCameraZ(vert)   RwIm2DVertexGetRecipCameraZ(vert)
#define RWIM2DVERTEXSetScreenX(vert, scrnx) RwIm2DVertexSetScreenX(vert, scrnx)
#define RWIM2DVERTEXSetScreenY(vert, scrny) RwIm2DVertexSetScreenY(vert, scrny)
#define RWIM2DVERTEXSetScreenZ(vert, scrnz) RwIm2DVertexSetScreenZ(vert, scrnz)
#define RWIM2DVERTEXGetScreenX(vert)        RwIm2DVertexGetScreenX(vert)
#define RWIM2DVERTEXGetScreenY(vert)        RwIm2DVertexGetScreenY(vert)
#define RWIM2DVERTEXGetScreenZ(vert)        RwIm2DVertexGetScreenZ(vert)
#define RWIM2DVERTEXSetU(vert, u, recipz)   RwIm2DVertexSetU(vert, u, recipz)
#define RWIM2DVERTEXSetV(vert, v, recipz)   RwIm2DVertexSetV(vert, v, recipz)
#define RWIM2DVERTEXGetU(vert)              RwIm2DVertexGetU(vert)
#define RWIM2DVERTEXGetV(vert)              RwIm2DVertexGetV(vert)
#define RWIM2DVERTEXSetRealRGBA(vert, red, green, blue, alpha) \
    RwIm2DVertexSetRealRGBA(vert, red, green, blue, alpha)
#define RWIM2DVERTEXSetIntRGBA(vert, red, green, blue, alpha)  \
    RwIm2DVertexSetIntRGBA(vert, red, green, blue, alpha)
#define RWIM2DVERTEXGetRed(vert)            RwIm2DVertexGetRed(vert)
#define RWIM2DVERTEXGetGreen(vert)          RwIm2DVertexGetGreen(vert)
#define RWIM2DVERTEXGetBlue(vert)           RwIm2DVertexGetBlue(vert)
#define RWIM2DVERTEXGetAlpha(vert)          RwIm2DVertexGetAlpha(vert)
#define RWIM2DVERTEXCopyRGBA(dst, src)      RwIm2DVertexCopyRGBA(dst, src)
#define RWIM2DVERTEXClipRGBA(o, i, n, f)    RwIm2DVertexClipRGBA(o, i, n, f)

/****************************************************************************
 Global Types
 */

/* We use RwD3D8Vertex to drive the hardware in 2D mode */

/**
 * \ingroup rwcoredriverd3d8
 * \typedef RwD3D8Vertex
 * D3D8 vertex structure definition for 2D geometry
 */
typedef struct RwD3D8Vertex RwD3D8Vertex;
/**
 * \ingroup rwcoredriverd3d8
 * \struct RwD3D8Vertex
 * D3D8 vertex structure definition for 2D geometry
 */
struct RwD3D8Vertex
{
    RwReal      x;              /**< Screen X */
    RwReal      y;              /**< Screen Y */
    RwReal      z;              /**< Screen Z */
    RwReal      rhw;            /**< Reciprocal of homogeneous W */

    RwUInt32    emissiveColor;  /**< Vertex color */

    RwReal      u;              /**< Texture coordinate U */
    RwReal      v;              /**< Texture coordinate V */
};

/* Define types used */

/**
 * \ingroup rwcoredriverd3d8
 * \typedef RwIm2DVertex
 * Typedef for a RenderWare Graphics Immediate Mode 2D Vertex
 */
typedef RwD3D8Vertex    RwIm2DVertex;

/* LEGACY-SUPPORT macro */
/**
 * \ingroup rwcoredriverd3d8
 * \def RWIM2DVERTEX
 * RWIM2DVERTEX is a legacy macro for RwIm2DVertex
 */
#define RWIM2DVERTEX    RwIm2DVertex

/**
 * \ingroup rwcoredriverd3d8
 * \typedef RxVertexIndex
 *
 * Typedef for a RenderWare Graphics PowerPipe Immediate
 * Mode Vertex
 */
typedef RwUInt16        RxVertexIndex;

/**
 * \ingroup rwcoredriverd3d8
 * \typedef RwImVertexIndex
 * Typedef for a RenderWare Graphics Immediate Mode Vertex.
 */
typedef RxVertexIndex   RwImVertexIndex;

/* LEGACY-SUPPORT macro */
/**
 * \ingroup rwcoredriverd3d8
 * \def RWIMVERTEXINDEX
 * RWIMVERTEXINDEX is a legacy macro for RwImVertexIndex
 */
#define RWIMVERTEXINDEX RwImVertexIndex

/**
 * \ingroup rwcoredriverd3d8
 * \struct RwD3D8Metrics
 * Structure containing metrics counters
 */
typedef struct
{
    RwUInt32    numRenderStateChanges;          /**< Number of Render States changed */
    RwUInt32    numTextureStageStateChanges;    /**< Number of Texture Stage States changed */
    RwUInt32    numMaterialChanges;             /**< Number of Material changes */
    RwUInt32    numLightsChanged;               /**< Number of Lights changed */
}
RwD3D8Metrics;

#endif /* D3D8_DRVMODEL_H */

/*--- Automatically derived from: C:/daily/rwsdk/src/plcore/bavector.h ---*/

/*
 * Typedef for pointer to Vector multiplication by Matrix function
 */

typedef RwV3d *(*rwVectorMultFn) (RwV3d * pointsOut, 
                                  const RwV3d * pointsIn,
                                  RwInt32 numPoints,
                                  const RwMatrix * matrix);


/* If sqrt is overloaded for this platform, we will remove
 * all the sqrt table stuff from the build entirely
 * currently applies to SKY2 and XBOX - IDBS [2/11/2001]
 * [and, if using the intel compiler version 400 or above,
 *  we will use the single-precision float "sqrtf" under
 *  D3D7, D3D8, OpenGL or SoftRas] */
#if (defined(rwSqrtMacro))
#define RWNOSQRTTABLE
#endif /* (defined(rwSqrtMacro)) */
#if (defined(rwInvSqrtMacro))
#define RWNOINVSQRTTABLE
#endif /* (defined(rwSqrtMacro)) */

#if (!defined(rwSqrtMacro))
#define rwSqrtMacro(_root, _input) \
    ( *(_root) = _rwSqrt(_input) )
#endif /* (!defined(rwSqrtMacro)) */

#if (!defined(rwInvSqrtMacro))
#define rwInvSqrtMacro(_recip, _input) \
    ( *(_recip) = _rwInvSqrt(_input) )
#endif /* (!defined(rwInvSqrtMacro)) */

#if (!defined(rwSqrtInvSqrtMacro))
#define rwSqrtInvSqrtMacro(_root, _recip, _input)       \
MACRO_START                                             \
{                                                       \
    RwReal _tmp = _input;                               \
    rwSqrt((_root), _tmp);                              \
    rwInvSqrt((_recip), _tmp);                          \
}                                                       \
MACRO_STOP
#endif /* (!defined(rwSqrtInvSqrtMacro)) */

/* Vector operations Macros */

#if (!defined(RwV2dAssignMacro))
#define RwV2dAssignMacro(_target, _source)                      \
    ( *(_target) = *(_source) )
#endif /* (!defined(RwV2dAssignMacro)) */

#define RwV2dAddMacro(o, a, b)                                  \
MACRO_START                                                     \
{                                                               \
    (o)->x = (((a)->x) + ( (b)->x));                            \
    (o)->y = (((a)->y) + ( (b)->y));                            \
}                                                               \
MACRO_STOP

#define RwV2dSubMacro(o, a, b)                                  \
MACRO_START                                                     \
{                                                               \
    (o)->x = (((a)->x) - ( (b)->x));                            \
    (o)->y = (((a)->y) - ( (b)->y));                            \
}                                                               \
MACRO_STOP

#define RwV2dScaleMacro(o, i, s)                                \
MACRO_START                                                     \
{                                                               \
    (o)->x = (((i)->x) * ( (s)));                               \
    (o)->y = (((i)->y) * ( (s)));                               \
}                                                               \
MACRO_STOP

#define RwV2dDotProductMacro(a,b)                               \
    (( ((((a)->x) * ( (b)->x))) +                               \
      ( (((a)->y) * ( (b)->y)))))                               \

#define _rwV2dNormalizeMacro(_result, _out, _in)                \
MACRO_START                                                     \
{                                                               \
    RwReal length2 = RwV2dDotProductMacro((_in), (_in));        \
    rwInvSqrtMacro(&(_result), length2);                        \
    RwV2dScaleMacro((_out), (_in), (_result));                  \
}                                                               \
MACRO_STOP

#define RwV2dNormalizeMacro(_result, _out, _in)                 \
MACRO_START                                                     \
{                                                               \
    RwReal length2 = RwV2dDotProductMacro((_in), (_in));        \
    RwReal recip;                                               \
                                                                \
    rwSqrtInvSqrtMacro(&(_result), &recip, length2);            \
    RwV2dScaleMacro((_out), (_in), recip);                      \
}                                                               \
MACRO_STOP

#define RwV2dLengthMacro(_result, _in)                          \
MACRO_START                                                     \
{                                                               \
    (_result) = RwV2dDotProductMacro(_in, _in);                 \
    rwSqrtMacro(&(_result), (_result));                         \
}                                                               \
MACRO_STOP

#define RwV2dLineNormalMacro(_o, _a, _b)                        \
MACRO_START                                                     \
{                                                               \
    RwReal recip;                                               \
                                                                \
    (_o)->y = (((_b)->x) - ( (_a)->x));                         \
    (_o)->x = (((_a)->y) - ( (_b)->y));                         \
    _rwV2dNormalizeMacro(recip, _o,_o);                         \
}                                                               \
MACRO_STOP

#define RwV2dPerpMacro(o, a)                                    \
MACRO_START                                                     \
{                                                               \
    (o)->x = -(a)->y;                                           \
    (o)->y = (a)->x;                                            \
}                                                               \
MACRO_STOP

/* RwV3d */

#if (!defined(RwV3dAssignMacro))
#define RwV3dAssignMacro(_target, _source)                     \
    ( *(_target) = *(_source) )
#endif /* (!defined(RwV3dAssignMacro)) */


#define RwV3dAddMacro(o, a, b)                                  \
MACRO_START                                                     \
{                                                               \
    (o)->x = (((a)->x) + ( (b)->x));                            \
    (o)->y = (((a)->y) + ( (b)->y));                            \
    (o)->z = (((a)->z) + ( (b)->z));                            \
}                                                               \
MACRO_STOP

#define RwV3dSubMacro(o, a, b)                                  \
MACRO_START                                                     \
{                                                               \
    (o)->x = (((a)->x) - ( (b)->x));                            \
    (o)->y = (((a)->y) - ( (b)->y));                            \
    (o)->z = (((a)->z) - ( (b)->z));                            \
}                                                               \
MACRO_STOP

#define RwV3dScaleMacro(o, a, s)                                \
MACRO_START                                                     \
{                                                               \
    (o)->x = (((a)->x) * ( (s)));                               \
    (o)->y = (((a)->y) * ( (s)));                               \
    (o)->z = (((a)->z) * ( (s)));                               \
}                                                               \
MACRO_STOP

#define RwV3dIncrementScaledMacro(o, a, s)                      \
MACRO_START                                                     \
{                                                               \
    (o)->x += (((a)->x) * ( (s)));                              \
    (o)->y += (((a)->y) * ( (s)));                              \
    (o)->z += (((a)->z) * ( (s)));                              \
}                                                               \
MACRO_STOP

#define RwV3dNegateMacro(o, a)                                  \
MACRO_START                                                     \
{                                                               \
    (o)->x = -(a)->x;                                           \
    (o)->y = -(a)->y;                                           \
    (o)->z = -(a)->z;                                           \
}                                                               \
MACRO_STOP

#define RwV3dDotProductMacro(a, b)                              \
    ((((( (((a)->x) * ((b)->x))) +                              \
        ( (((a)->y) * ((b)->y))))) +                            \
        ( (((a)->z) * ((b)->z)))))                              \

#define RwV3dCrossProductMacro(o, a, b)                         \
MACRO_START                                                     \
{                                                               \
    (o)->x =                                                    \
        (( (((a)->y) * ( (b)->z))) -                            \
         ( (((a)->z) * ( (b)->y))));                            \
    (o)->y =                                                    \
        (( (((a)->z) * ( (b)->x))) -                            \
         ( (((a)->x) * ( (b)->z))));                            \
    (o)->z =                                                    \
        (( (((a)->x) * ( (b)->y))) -                            \
         ( (((a)->y) * ( (b)->x))));                            \
}                                                               \
MACRO_STOP

#define _rwV3dNormalizeMacro(_result, _out, _in)                \
MACRO_START                                                     \
{                                                               \
    RwReal length2 = RwV3dDotProductMacro(_in, _in);            \
    rwInvSqrtMacro(&(_result), length2);                        \
    RwV3dScaleMacro(_out, _in, _result);                        \
}                                                               \
MACRO_STOP

#define RwV3dNormalizeMacro(_result, _out, _in)                 \
MACRO_START                                                     \
{                                                               \
    RwReal length2 = RwV3dDotProductMacro((_in), (_in));        \
    RwReal recip;                                               \
                                                                \
    rwSqrtInvSqrtMacro(&(_result), &recip, length2);            \
    RwV3dScaleMacro((_out), (_in), recip);                      \
}                                                               \
MACRO_STOP

#define RwV3dLengthMacro(_result, _in)                          \
MACRO_START                                                     \
{                                                               \
    (_result) = RwV3dDotProductMacro(_in, _in);                 \
    rwSqrtMacro(&(_result), _result);                           \
}                                                               \
MACRO_STOP

#if (! ( defined(RWDEBUG) || defined(RWSUPPRESSINLINE) ))

#define RwV2dAssign(o, a)               RwV2dAssignMacro(o, a)
#define RwV2dAdd(o, a, b)               RwV2dAddMacro(o, a, b)
#define RwV2dSub(o, a, b)               RwV2dSubMacro(o, a, b)
#define RwV2dLineNormal(_o, _a, _b)     RwV2dLineNormalMacro(_o, _a, _b)
#define RwV2dScale(o, i, s)             RwV2dScaleMacro(o, i, s)
#define RwV2dDotProduct(a,b)            RwV2dDotProductMacro(a,b)
#define RwV2dPerp(o, a)                 RwV2dPerpMacro(o, a)
#define RwV3dAssign(o, a)               RwV3dAssignMacro(o, a)
#define RwV3dAdd(o, a, b)               RwV3dAddMacro(o, a, b)
#define RwV3dSub(o, a, b)               RwV3dSubMacro(o, a, b)
#define RwV3dScale(o, a, s)             RwV3dScaleMacro(o, a, s)
#define RwV3dIncrementScaled(o, a, s)   RwV3dIncrementScaledMacro(o, a, s)
#define RwV3dNegate(o, a)               RwV3dNegateMacro(o, a)
#define RwV3dDotProduct(a, b)           RwV3dDotProductMacro(a, b)
#define RwV3dCrossProduct(o, a, b)      RwV3dCrossProductMacro(o, a, b)

#endif /* (! ( defined(RWDEBUG) || defined(RWSUPPRESSINLINE) )) */

#define RWRAD2DEG(_x) ((_x) * (((RwReal)180)/(rwPI)))

#if (!defined(rw4OVERPISQ))
#define rw4OVERPISQ  ( ((RwReal)4) / ( rwPI * rwPI ))
#endif /* (!defined(rw4OVERPISQ)) */

#if (!defined(rwPI3))
#define rwPI3  (rwPI * (RwReal)3) 
#endif /* (!defined(rwPI3)) */

#if (!defined(rwPI3OVER2))
#define rwPI3OVER2  ( rwPI3 / (RwReal)2 )
#endif /* (!defined(rwPI3OVER2)) */

#if (!defined(rwPI3OVER8))
#define rwPI3OVER8  (rwPI3 / (RwReal)8 )
#endif /* (!defined(rwPI3OVER8)) */

#define RwQuadSin(_x)                                                       \
    ( rw4OVERPISQ *                                                         \
      ( ( (_x) < 0 ) ?                                                      \
        ( ( rwPI + (_x) ) * (_x) ) :                                        \
        ( ( rwPI - (_x) ) * (_x) ) ) )

#define RwQuadASin(_result, _s)                                             \
    ( rwPIOVER2 * ( ((_s)<0) ?                                              \
                    ( rwSqrtMacro((_result), 1.0f + (_s)) - 1 ) :           \
                    ( 1 - rwSqrtMacro((_result), 1.0f - (_s)) ) ) )

#define RwQuadCos(_x)                                                       \
    ( rw4OVERPISQ *                                                         \
      ( ( (_x) < -rwPIOVER2 ) ?                                             \
        ( ( -rwPI3OVER2 - (_x) ) * ( -rwPIOVER2 - (_x) ) ) :                \
        ( ( (_x) < rwPIOVER2) ?                                             \
          ( ( rwPIOVER2 + (_x) ) * ( rwPIOVER2 - (_x) ) ) :                 \
          ( ( rwPIOVER2 - (_x) ) * ( rwPI3OVER2 - (_x) ) ) ) ) )

#define RwQuadACos(_result, _c)                                             \
    ( rwPIOVER2 * ( ((_c)<0) ?                                              \
                    (2.0f - rwSqrtMacro((_result), 1.0f + (_c))):           \
                    rwSqrtMacro((_result), 1.0f - (_c))) )

#define RwQuadTan(_x)                                                       \
    ( rwPI3 * (_x) / ( rwPI * rwPI - (_x) * (_x) * 4.0f ) )

#define RwQuadATan(_result, _t)                                             \
    ( ( rwSqrtMacro((_result), (rwPI3OVER8 * rwPI3OVER8) +                  \
                    (_t) * (_t) * (rwPIOVER2 * rwPIOVER2) ) - rwPI3OVER8 )  \
      / ( _t) )

#define RwQuadATan2(_result, _s, _c)                                        \
    ( ( rwSqrtMacro((_result), (_c) * (_c) * (rwPI3OVER8 * rwPI3OVER8) +    \
                    (_s) * (_s) * (rwPIOVER2 * rwPIOVER2) )                 \
        - (_c) * rwPI3OVER8 ) / ( _s)  )

/****************************************************************************
 Function prototypes
 */

#ifdef    __cplusplus
extern "C"
{
#endif         /* __cplusplus */

/* Other useful stuff */

extern RwReal RwV3dNormalize(RwV3d * out, const RwV3d * in);
extern RwReal RwV3dLength(const RwV3d * in);

extern RwReal RwV2dLength(const RwV2d * in);
extern RwReal RwV2dNormalize(RwV2d * out, const RwV2d * in);

#if ( defined(RWDEBUG) || defined(RWSUPPRESSINLINE) )

extern void RwV2dAssign(RwV2d * out,
                        const RwV2d * ina);
extern void RwV2dAdd(RwV2d * out,
                     const RwV2d * ina, const RwV2d * inb);
extern void RwV2dLineNormal(RwV2d * out,
                     const RwV2d * ina, const RwV2d * inb);
extern void RwV2dSub(RwV2d * out,
                     const RwV2d * ina, const RwV2d * inb);
extern void RwV2dPerp(RwV2d * out, const RwV2d * in);
extern void RwV2dScale(RwV2d * out,
                       const RwV2d * in, RwReal scalar);
extern RwReal RwV2dDotProduct(const RwV2d * ina, const RwV2d * inb);

extern void RwV3dAssign(RwV3d * out,
                        const RwV3d * ina);
extern void RwV3dAdd(RwV3d * out,
                     const RwV3d * ina, const RwV3d * inb);
extern void RwV3dSub(RwV3d * out,
                     const RwV3d * ina, const RwV3d * inb);
extern void RwV3dScale(RwV3d * out,
                       const RwV3d * in, RwReal scalar);
extern void RwV3dIncrementScaled(RwV3d * out, 
                                 const RwV3d * in, RwReal scalar);
extern void RwV3dNegate(RwV3d * out, const RwV3d * in);
extern RwReal RwV3dDotProduct(const RwV3d * ina, const RwV3d * inb);
extern void RwV3dCrossProduct(RwV3d * out,
                              const RwV3d * ina, const RwV3d * inb);

#endif         /* ( defined(RWDEBUG) || defined(RWSUPPRESSINLINE) ) */

/* Transform points/vectors */
extern RwV3d *RwV3dTransformPoints(RwV3d * pointsOut,
                                   const RwV3d * pointsIn,
                                   RwInt32 numPoints,
                                   const RwMatrix * matrix);
extern RwV3d *RwV3dTransformVectors(RwV3d * vectorsOut,
                                    const RwV3d * vectorsIn,
                                    RwInt32 numPoints,
                                    const RwMatrix * matrix);

/* SPI */

#if (!defined(RWNOSQRTTABLE))
extern RwReal _rwSqrt(const RwReal num);
#endif /* (!defined(RWNOSQRTTABLE)) */
#if (!defined(RWNOINVSQRTTABLE))
extern RwReal _rwInvSqrt(const RwReal num);
#endif /* (!defined(RWNOINVSQRTTABLE)) */

extern RwReal _rwV3dNormalize(RwV3d * out, const RwV3d * in);

#ifdef    __cplusplus
}
#endif         /* __cplusplus */

#define rwVectorOpen(instance, offset, size) \
       _rwVectorOpen(instance, offset, size)

#define rwVectorClose(instance, offset, size) \
       _rwVectorClose(instance, offset, size)

#define rwVectorSetMultFn(multPoint,multVector) \
       _rwVectorSetMultFn(multPoint,multVector)


/*--- Automatically derived from: C:/daily/rwsdk/src/plcore/balist.h ---*/
/****************************************************************************
 Global Types
 */

typedef struct RwSList RwSList;
struct RwSList
{
    RwUInt8    *listElements;
    RwInt32     numElementsFilled;
    RwInt32     numElementsAlloced;
    RwInt32     entrySize;
};



/****************************************************************************
 Function prototypes
 */

#ifdef    __cplusplus
extern "C"
{
#endif                          /* __cplusplus */

/* SList functions */
extern RwSList    *_rwSListCreate(RwInt32 size);
extern RwBool      _rwSListDestroy(RwSList *sList);
extern RwBool      _rwSListDestroyArray(RwUInt8 *array);
extern void         _rwSListDestroyEndEntries(RwSList *sList, RwInt32 amount);
extern RwBool      _rwSListDestroyEntry(RwSList *sList, RwInt32 entry);
extern void         _rwSListEmpty(RwSList *sList);
extern void        *_rwSListGetArray(RwSList *sList);
extern void        *_rwSListGetEntry(RwSList *sList, RwInt32 entry);
extern void        *_rwSListGetNewEntry(RwSList *sList);
extern void        *_rwSListGetNewEntries(RwSList *sList, RwInt32 entry);
extern RwInt32     _rwSListGetNumEntries(const RwSList *sList);
extern RwBool      _rwSListReleaseArray(RwSList *sList);
extern void        *_rwSListToArray(RwSList *sList);


#ifdef    __cplusplus
}
#endif                          /* __cplusplus */

/* Comparibility macros */

#define rwSListCreate(size) \
       _rwSListCreate(size)
#define rwSListDestroy(sList) \
       _rwSListDestroy(sList)
#define rwSListDestroyArray(array) \
       _rwSListDestroyArray(array)
#define rwSListDestroyEndEntries(sList, amount) \
       _rwSListDestroyEndEntries(sList, amount)
#define rwSListDestroyEntry(sList, entry) \
       _rwSListDestroyEntry(sList, entry)
#define rwSListEmpty(sList) \
       _rwSListEmpty(sList)
#define rwSListGetArray(sList) \
       _rwSListGetArray(sList)
#define rwSListGetEntry(sList, entry) \
       _rwSListGetEntry(sList, entry)
#define rwSListGetNewEntry(sList) \
       _rwSListGetNewEntry(sList)
#define rwSListGetNewEntries(sList, entry) \
       _rwSListGetNewEntries(sList, entry)
#define rwSListGetNumEntries(sList) \
       _rwSListGetNumEntries(sList)
#define rwSListReleaseArray(sList) \
       _rwSListReleaseArray(sList)
#define rwSListToArray(sList) \
       _rwSListToArray(sList)


/*--- Automatically derived from: C:/daily/rwsdk/src/plcore/baimmedi.h ---*/

/****************************************************************************
 Defines
 */


/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

                        Immediate mode interface V2.0

 !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

/**
 * \ingroup datatypes
 * RwRenderState
 *     This type represents the various render states that
 * can be set using the API function \ref RwRenderStateSet. This function also
 * takes a render state value or pointer to an object depending on the type.
 * For render states that are toggles, the value should be TRUE to switch the
 * state on and FALSE to turn it off.
 *
 * Note that many of these render states may not be supported on certain
 * platforms. The \ref RwRenderStateSet functions will return FALSE in such cases.
 */
enum RwRenderState
{
    rwRENDERSTATENARENDERSTATE = 0,
    rwRENDERSTATETEXTURERASTER,             /**<Raster to texture with. \ref RwRenderStateSet
                                             *  takes a pointer to an \ref RwRaster */
    rwRENDERSTATETEXTUREADDRESS,            /**<\ref RwTextureAddressMode: wrap, clamp, mirror or border */
    rwRENDERSTATETEXTUREADDRESSU,           /**<\ref RwTextureAddressMode in u only */
    rwRENDERSTATETEXTUREADDRESSV,           /**<\ref RwTextureAddressMode in v only */
    rwRENDERSTATETEXTUREPERSPECTIVE,        /**<Perspective correction on/off */
    rwRENDERSTATEZTESTENABLE,               /**<Z-buffer test on/off */
    rwRENDERSTATESHADEMODE,                 /**<\ref RwShadeMode: flat or gouraud shading */
    rwRENDERSTATEZWRITEENABLE,              /**<Z-buffer write on/off */
    rwRENDERSTATETEXTUREFILTER,             /**<\ref RwTextureFilterMode: point sample, bilinear, trilinear, etc */
    rwRENDERSTATESRCBLEND,                  /**<Source alpha \ref RwBlendFunction: src alpha, 1-src alpha, etc */
    rwRENDERSTATEDESTBLEND,                 /**<Destination alpha \ref RwBlendFunction */
    rwRENDERSTATEVERTEXALPHAENABLE,         /**<Vertex alpha transparency on/off */
    rwRENDERSTATEBORDERCOLOR,               /**<Border color for \ref RwTextureAddressMode rwTEXTUREADDRESSBORDER.
                                             *  The value should be a packed RwUInt32 in ARGB form. The macro
                                             *  RWRGBALONG(r, g, b, a) may be used to construct this using 8-bit
                                             *  color components. */
    rwRENDERSTATEFOGENABLE,                 /**<Fogging on/off (all polygons will be fogged) */
    rwRENDERSTATEFOGCOLOR,                  /**<Color used for fogging. The value should be a packed RwUInt32
                                             *  in ARGB form. The macro RWRGBALONG(r, g, b, a) may be used to
                                             *  construct this using 8-bit color components */
    rwRENDERSTATEFOGTYPE,                   /**<Sets \ref RwFogType, the type of fogging to use */
    rwRENDERSTATEFOGDENSITY,                /**<Select the fog density for \ref RwFogType of rwFOGTYPEEXPONENTIAL
                                             *  or rwFOGTYPEEXPONENTIAL2. The value should be a pointer to
                                             *  an RwReal in the range 0 to 1. */
    rwRENDERSTATEFOGTABLE,                  /**<Install a 256 entry fog table placed between fog distance and far
                                             *  clip-plane */
    rwRENDERSTATEALPHAPRIMITIVEBUFFER,      /**<Render transparent alpha polygons last - on/off. \e Not \e supported 
                                             * \e on \e any \e platform.*/
    rwRENDERSTATECULLMODE,                  /**<Sets \ref RwCullMode, for selecting face culling. */
    rwRENDERSTATESTENCILENABLE,             /**< TRUE to enable stenciling, or FALSE to disable stenciling.
                                             * \e Supported \e on \e Xbox, \e D3D8, \e and \e OpenGL \e only. */
    rwRENDERSTATESTENCILFAIL,               /**< Stencil test operator for the fail case.
                                             * \e Supported \e on \e Xbox, \e D3D8, \e and \e OpenGL \e only */
    rwRENDERSTATESTENCILZFAIL,              /**< Stencil operation to perform if the stencil test passes
                                             *   and the depth test (z-test) fails.
                                             * \e Supported \e on \e Xbox, \e D3D8, \e and \e OpenGL \e only */
    rwRENDERSTATESTENCILPASS,               /**< Stencil operation to perform if both the stencil and the 
                                             *   depth (z) tests pass.
                                             * \e Supported \e on \e Xbox, \e D3D8, \e and \e OpenGL \e only */
    rwRENDERSTATESTENCILFUNCTION,           /**< Comparison function for the stencil test.
                                             * \e Supported \e on \e Xbox, \e D3D8, \e and \e OpenGL \e only */
    rwRENDERSTATESTENCILFUNCTIONREF,        /**< Integer reference value for the stencil test.
                                             * \e Supported \e on \e Xbox, \e D3D8, \e and \e OpenGL \e only */
    rwRENDERSTATESTENCILFUNCTIONMASK,       /**< Mask applied to the reference value and each stencil buffer 
                                             *   entry to determine the significant bits for the stencil test.
                                             * \e Supported \e on \e Xbox, \e D3D8, \e and \e OpenGL \e only */
    rwRENDERSTATESTENCILFUNCTIONWRITEMASK,  /**< Write mask applied to values written into the stencil buffer.
                                             * \e Supported \e on \e Xbox, \e D3D8, \e and \e OpenGL \e only */

    rwRENDERSTATEFORCEENUMSIZEINT = RWFORCEENUMSIZEINT
};
typedef enum RwRenderState RwRenderState;


/**
 * \ingroup datatypes
 * RwShadeMode
 *  This type represents the options available for setting the
 * rwRENDERSTATESHADEMODE render state */
enum RwShadeMode
{
    rwSHADEMODENASHADEMODE = 0,     /**<Invalid shading mode */
    rwSHADEMODEFLAT,                /**<Flat shading */
    rwSHADEMODEGOURAUD,             /**<Gouraud shading */
    rwSHADEMODEFORCEENUMSIZEINT = RWFORCEENUMSIZEINT
};
typedef enum RwShadeMode RwShadeMode;


/**
 * \ingroup datatypes
 * RwTextureFilterMode
 *  This type represents the options available for texture
 * filtering (see API functions \ref RwTextureSetFilterMode and \ref RwRenderStateSet)*/
enum RwTextureFilterMode
{
    rwFILTERNAFILTERMODE = 0,       /**<Invalid filter mode */
    rwFILTERNEAREST,                /**<Point sampled */
    rwFILTERLINEAR,                 /**<Bilinear */
    rwFILTERMIPNEAREST,             /**<Point sampled per pixel mip map */
    rwFILTERMIPLINEAR,              /**<Bilinear per pixel mipmap */
    rwFILTERLINEARMIPNEAREST,       /**<MipMap interp point sampled */
    rwFILTERLINEARMIPLINEAR,        /**<Trilinear */
    rwTEXTUREFILTERMODEFORCEENUMSIZEINT = RWFORCEENUMSIZEINT
};
typedef enum RwTextureFilterMode RwTextureFilterMode;


/**
 * \ingroup datatypes
 * RwFogType
 *  This type represents the options available to the
 * rwRENDERSTATEFOGTYPE render state (see \ref RwRenderState);*/
enum RwFogType
{
    rwFOGTYPENAFOGTYPE = 0,     /**<Invalid fog type */
    rwFOGTYPELINEAR,            /**<Linear fog */
    rwFOGTYPEEXPONENTIAL,       /**<Exponential fog */
    rwFOGTYPEEXPONENTIAL2,      /**<Exponential^2 fog */
    rwFOGTYPEFORCEENUMSIZEINT = RWFORCEENUMSIZEINT
};
typedef enum RwFogType RwFogType;


/**
 * \ingroup datatypes
 * RwBlendFunction
 *  This type represents the options available
 * to the rwRENDERSTATESRCBLEND and rwRENDERSTATEDESTBLEND render states
 * (see \ref RwRenderState).  In the following description,
 * a subscript s refers to a source value while subscript d refers to a
 * destination value.*/
enum RwBlendFunction
{
    rwBLENDNABLEND = 0,     /**<Invalid blend mode */
    rwBLENDZERO,            /**<(0,    0,    0,    0   ) */
    rwBLENDONE,             /**<(1,    1,    1,    1   ) */
    rwBLENDSRCCOLOR,        /**<(Rs,   Gs,   Bs,   As  ) */
    rwBLENDINVSRCCOLOR,     /**<(1-Rs, 1-Gs, 1-Bs, 1-As) */
    rwBLENDSRCALPHA,        /**<(As,   As,   As,   As  ) */
    rwBLENDINVSRCALPHA,     /**<(1-As, 1-As, 1-As, 1-As) */
    rwBLENDDESTALPHA,       /**<(Ad,   Ad,   Ad,   Ad  ) */
    rwBLENDINVDESTALPHA,    /**<(1-Ad, 1-Ad, 1-Ad, 1-Ad) */
    rwBLENDDESTCOLOR,       /**<(Rd,   Gd,   Bd,   Ad  ) */
    rwBLENDINVDESTCOLOR,    /**<(1-Rd, 1-Gd, 1-Bd, 1-Ad) */
    rwBLENDSRCALPHASAT,     /**<(f,    f,    f,    1   )  f = min (As, 1-Ad) */
    rwBLENDFUNCTIONFORCEENUMSIZEINT = RWFORCEENUMSIZEINT
};
typedef enum RwBlendFunction RwBlendFunction;


/**
 * \ingroup datatypes
 * RwTextureAddressMode
 *  This type represents the options available for
 * applying textures to polygons (see API functions \ref RwTextureSetAddressing
 * and \ref RwRenderStateSet, and the \ref RwRenderState type) */
enum RwTextureAddressMode
{
    rwTEXTUREADDRESSNATEXTUREADDRESS = 0,   /**<Invalid addressing mode */
    rwTEXTUREADDRESSWRAP,                   /**<UV wraps (tiles) */
    rwTEXTUREADDRESSMIRROR,                 /**<Alternate UV is flipped */
    rwTEXTUREADDRESSCLAMP,                  /**<UV is clamped to 0-1 */
    rwTEXTUREADDRESSBORDER,                 /**<Border colour takes effect outside of 0-1 */
    rwTEXTUREADDRESSMODEFORCEENUMSIZEINT = RWFORCEENUMSIZEINT
};
typedef enum RwTextureAddressMode RwTextureAddressMode;

/**
 * \ingroup datatypes
 * RwStencilOperation
 *  This type represents the stencil operations to perform 
 * based on the results of the  
 * \ref rwRENDERSTATESTENCILFAIL, \ref rwRENDERSTATESTENCILZFAIL
 * and \ref rwRENDERSTATESTENCILPASS tests.
 */ 
enum RwStencilOperation
{
    rwSTENCILOPERATIONNASTENCILOPERATION = 0,
    rwSTENCILOPERATIONKEEP,     /* Do not update the entry in the stencil buffer */
    rwSTENCILOPERATIONZERO,     /* Set the stencil-buffer entry to 0 */
    rwSTENCILOPERATIONREPLACE,  /* Replace the stencil-buffer entry with reference value */
    rwSTENCILOPERATIONINCRSAT,  /* Increment the stencil-buffer entry, clamping to the maximum value */
    rwSTENCILOPERATIONDECRSAT,  /* Decrement the stencil-buffer entry, clamping to zero */    
    rwSTENCILOPERATIONINVERT,   /* Invert the bits in the stencil-buffer entry */
    rwSTENCILOPERATIONINCR,     /* Increment the stencil-buffer entry, wrapping to zero if the new value exceeds the maximum value */
    rwSTENCILOPERATIONDECR,     /* Decrement the stencil-buffer entry, wrapping to the maximum value if the new value is less than zero */
    rwSTENCILOPERATIONFORCEENUMSIZEINT = RWFORCEENUMSIZEINT
};
typedef enum RwStencilOperation RwStencilOperation;

/**
 * \ingroup datatypes
 * RwStencilFunction
 *  This type represents the options available for
 * the comparison function for the stencil test \ref rwRENDERSTATESTENCILFUNCTION.
 */
enum RwStencilFunction
{
    rwSTENCILFUNCTIONNASTENCILFUNCTION = 0,
    rwSTENCILFUNCTIONNEVER,         /* Always fail the test */
    rwSTENCILFUNCTIONLESS,          /* Accept the new pixel if its value is less than the value of the current pixel */
    rwSTENCILFUNCTIONEQUAL,         /* Accept the new pixel if its value equals the value of the current pixel */
    rwSTENCILFUNCTIONLESSEQUAL,     /* Accept the new pixel if its value is less than or equal to the value of the current pixel */
    rwSTENCILFUNCTIONGREATER,       /* Accept the new pixel if its value is greater than the value of the current pixel */
    rwSTENCILFUNCTIONNOTEQUAL,      /* Accept the new pixel if its value does not equal the value of the current pixel */
    rwSTENCILFUNCTIONGREATEREQUAL,  /* Accept the new pixel if its value is greater than or equal to the value of the current pixel */
    rwSTENCILFUNCTIONALWAYS,        /* Always pass the test */
    rwSTENCILFUNCTIONFORCEENUMSIZEINT = RWFORCEENUMSIZEINT
};
typedef enum RwStencilFunction RwStencilFunction;

/**
 * \ingroup datatypes
 * RwCullMode
 *  This type represents the options available for culling polygons during rendering.
 * and \ref RwRenderStateSet, and the \ref RwRenderState type) */
enum RwCullMode
{
    rwCULLMODENACULLMODE = 0,
    rwCULLMODECULLNONE,                /**< Both front and back-facing triangles are drawn. */
    rwCULLMODECULLBACK,                /**< Just front-facing triangles are drawn */
    rwCULLMODECULLFRONT,               /**< Just rear-facing triangles are drawn */

    rwCULLMODEFORCEENUMSIZEINT = RWFORCEENUMSIZEINT
};
typedef enum RwCullMode RwCullMode;

/**
 * \ingroup datatypes
 * RwPrimitiveType
 *  This type represents the different types of indexed
 * line and indexed triangle primitives that are available when rendering 2D
 * and 3D immediate mode objects (see API functions \ref RwIm2DRenderIndexedPrimitive,
 * \ref RwIm2DRenderPrimitive, \ref RwIm3DRenderIndexedPrimitive and \ref RwIm3DRenderPrimitive).
 * Indices are into a vertex list and must be defined in a counter-clockwise order
 * (as seen from the camera) to be visible.*/
enum RwPrimitiveType
{
    rwPRIMTYPENAPRIMTYPE = 0,   /**<Invalid primative type */
    rwPRIMTYPELINELIST = 1,     /**<Unconnected line segments, each line is specified by
                                 * both its start and end index, independently of other lines
                                 * (for example, 3 segments specified as 0-1, 2-3, 4-5) */
    rwPRIMTYPEPOLYLINE = 2,     /**<Connected line segments, each line's start index
                                 * (except the first) is specified by the index of the end of
                                 * the previous segment (for example, 3 segments specified as
                                 * 0-1, 1-2, 2-3) */
    rwPRIMTYPETRILIST = 3,      /**<Unconnected triangles: each triangle is specified by
                                 * three indices, independently of other triangles (for example,
                                 * 3 triangles specified as 0-1-2, 3-4-5, 6-7-8) */
    rwPRIMTYPETRISTRIP = 4,     /**<Connected triangles sharing an edge with, at most, one
                                 * other forming a series (for example, 3 triangles specified
                                 * as 0-2-1, 1-2-3-, 2-4-3) */
    rwPRIMTYPETRIFAN = 5 ,      /**<Connected triangles sharing an edge with, at most,
                                 * two others forming a fan (for example, 3 triangles specified
                                 * as 0-2-1, 0-3-2, 0-4-3) */
    rwPRIMTYPEPOINTLIST = 6,    /**<Points 1, 2, 3, etc. This is not
                                 * supported by the default RenderWare
                                 * immediate or retained-mode pipelines
                                 * (except on PlayStation 2), it is intended
                                 * for use by user-created pipelines */
    rwPRIMITIVETYPEFORCEENUMSIZEINT = RWFORCEENUMSIZEINT
};
typedef enum RwPrimitiveType RwPrimitiveType;




/****************************************************************************
 Function prototypes
 */

#ifdef    __cplusplus
extern "C"
{
#endif                          /* __cplusplus */

/* Expose Z buffer range */
extern RwReal RwIm2DGetNearScreenZ(void);
extern RwReal RwIm2DGetFarScreenZ(void);

extern RwBool RwRenderStateGet(RwRenderState state, void *value);
extern RwBool RwRenderStateSet(RwRenderState state, void *value);

extern RwBool RwIm2DRenderLine(RwIm2DVertex *vertices, RwInt32 numVertices, RwInt32 vert1, RwInt32 vert2);
extern RwBool RwIm2DRenderTriangle(RwIm2DVertex *vertices, RwInt32 numVertices,
                                   RwInt32 vert1, RwInt32 vert2, RwInt32 vert3 );
extern RwBool RwIm2DRenderPrimitive(RwPrimitiveType primType, RwIm2DVertex *vertices, RwInt32 numVertices);
extern RwBool RwIm2DRenderIndexedPrimitive(RwPrimitiveType primType, RwIm2DVertex *vertices, RwInt32 numVertices,
                                                             RwImVertexIndex *indices, RwInt32 numIndices);

#ifdef    __cplusplus
}
#endif                          /* __cplusplus */


/*--- Automatically derived from: C:/daily/rwsdk/src/plcore/badevice.h ---*/
/***************************************************************************/
/************************* System Requests *********************************/
/***************************************************************************/

/* Device controls:
 *
 * rwDEVICESYSTEMOPEN(NULL, RwEngineOpenParams *openParams, 0)
 * rwDEVICESYSTEMCLOSE(NULL, NULL, 0)
 * rwDEVICESYSTEMSTART(NULL, NULL, 0)
 * rwDEVICESYSTEMSTOP(NULL, NULL, 0)
 * rwDEVICESYSTEMREGISTER(RwDevice *coreDeviceBlock, RwMemoryFunctions *memFuncs, 0)
 * rwDEVICESYSTEMGETNUMMODES(RwInt32 *numModes, NULL, 0)
 * rwDEVICESYSTEMGETMODEINFO(RwVideoMode *modeinfo, NULL, RwInt32 modeNum)
 * rwDEVICESYSTEMUSEMODE(NULL, NULL, RwInt32 modeNum)
 * rwDEVICESYSTEMFOCUS(NULL, NULL, RwBool gainFocus)
 * rwDEVICESYSTEMINITPIPELINE(NULL, NULL, 0)
 * rwDEVICESYSTEMGETMODE(RwInt32 *curMode, NULL, 0)
 * rwDEVICESYSTEMSTANDARDS(RwStandardFunc *fnPtrArray, NULL, RwInt32 arraySize)
 * rwDEVICESYSTEMGETTEXMEMSIZE(RwInt32 *texMemSizeOut, NULL, 0)
 * rwDEVICESYSTEMGETNUMSUBSYSTEMS(RwInt32 *numSubSystemsOut, NULL, 0)
 * rwDEVICESYSTEMGETSUBSYSTEMINFO(RwSubSystemInfo *subSystemInfo, NULL, RwInt32 subSystemNum)
 * rwDEVICESYSTEMGETCURRENTSUBSYSTEM(RwInt32 *curSubSystem, NULL, 0)
 * rwDEVICESYSTEMSETSUBSYSTEM(NULL, NULL, RwInt32 subSystemNum)
 * rwDEVICESYSTEMFINALIZESTART(NULL, NULL, 0)
 * rwDEVICESYSTEMINITIATESTOP(NULL, NULL, 0)
 * rwDEVICESYSTEMRXPIPELINEREQUESTPIPE(RxPipeline **pipelineRef, NULL, RwInt32 pipeType)
 * rwDEVICESYSTEMDD         - start of device specific controls
 */

#define rwPIPETYPEMATERIAL            0
#define rwPIPETYPEWORLDSECTORINSTANCE 1
#define rwPIPETYPEATOMICINSTANCE      2

enum RwCoreDeviceSystemFn
{
    rwDEVICESYSTEMOPEN                  = 0x00,
    rwDEVICESYSTEMCLOSE,
    rwDEVICESYSTEMSTART,
    rwDEVICESYSTEMSTOP,
    rwDEVICESYSTEMREGISTER,
    rwDEVICESYSTEMGETNUMMODES,
    rwDEVICESYSTEMGETMODEINFO,
    rwDEVICESYSTEMUSEMODE,
    rwDEVICESYSTEMFOCUS,
    rwDEVICESYSTEMINITPIPELINE,
    rwDEVICESYSTEMGETMODE,
    rwDEVICESYSTEMSTANDARDS,
    rwDEVICESYSTEMGETTEXMEMSIZE,
    rwDEVICESYSTEMGETNUMSUBSYSTEMS,
    rwDEVICESYSTEMGETSUBSYSTEMINFO,
    rwDEVICESYSTEMGETCURRENTSUBSYSTEM,
    rwDEVICESYSTEMSETSUBSYSTEM,
    rwDEVICESYSTEMFINALIZESTART,
    rwDEVICESYSTEMINITIATESTOP,
    rwDEVICESYSTEMGETMAXTEXTURESIZE,
    rwDEVICESYSTEMRXPIPELINEREQUESTPIPE,
    rwDEVICESYSTEMGETMETRICBLOCK,
    rwDEVICESYSTEMDD                    = 0x1000,
    rwCOREDEVICESYSTEMFNFORCEENUMSIZEINT = RWFORCEENUMSIZEINT
};
typedef enum RwCoreDeviceSystemFn RwCoreDeviceSystemFn;

/******************************************************************************/
/********************* Standard functions *************************************/
/******************************************************************************/

#define rwSTANDARDNASTANDARD            0
#define rwSTANDARDCAMERABEGINUPDATE     1   /* Start 3d camera update */
#define rwSTANDARDRGBTOPIXEL            2   /* For an RGB value return a pixel value */
#define rwSTANDARDPIXELTORGB            3   /* For a pixel value returns a RGB value */
#define rwSTANDARDRASTERCREATE          4   /* Create an raster */
#define rwSTANDARDRASTERDESTROY         5   /* Raster destroy */
#define rwSTANDARDIMAGEGETRASTER        6   /* Get image from a raster */
#define rwSTANDARDRASTERSETIMAGE        7   /* Set raster from an image */
#define rwSTANDARDTEXTURESETRASTER      8   /* Set texture's raster */
#define rwSTANDARDIMAGEFINDRASTERFORMAT 9   /* Find a suitable raster format for an image */
#define rwSTANDARDCAMERAENDUPDATE       10  /* End 3d camera update */
#define rwSTANDARDSETRASTERCONTEXT      11  /* Start destination of 2d operations */
#define rwSTANDARDRASTERSUBRASTER       12  /* Make a raster inside another raster */
#define rwSTANDARDRASTERCLEARRECT       13  /* Clear a rectangle of the current dest raster */
#define rwSTANDARDRASTERCLEAR           14  /* Clear the current dest raster */
#define rwSTANDARDRASTERLOCK            15  /* Lock a raster to get it's pixels */
#define rwSTANDARDRASTERUNLOCK          16  /* Unlock a raster to return it's pixels */
#define rwSTANDARDRASTERRENDER          17  /* Render a raster (not scaled, but masked) */
#define rwSTANDARDRASTERRENDERSCALED    18  /* Render a raster (scaled and masked) */
#define rwSTANDARDRASTERRENDERFAST      19  /* Render a raster (not scaled or masked) */
#define rwSTANDARDRASTERSHOWRASTER      20  /* Show a camera raster */
#define rwSTANDARDCAMERACLEAR           21  /* Clear a camera's raster and/or Z raster */
#define rwSTANDARDHINTRENDERF2B         22  /* Set hint for rendering direction in the world */
#define rwSTANDARDRASTERLOCKPALETTE     23  /* Lock a raster to get it's palette */
#define rwSTANDARDRASTERUNLOCKPALETTE   24  /* Unlock a raster to return it's palette */
#define rwSTANDARDNATIVETEXTUREGETSIZE  25  /* Get size of native texture when written to a stream */
#define rwSTANDARDNATIVETEXTUREREAD     26  /* Read native texture from the stream */
#define rwSTANDARDNATIVETEXTUREWRITE    27  /* Write native texture to the stream */
#define rwSTANDARDRASTERGETMIPLEVELS    28  /* Get the number of mip levels in a raster */
#define rwSTANDARDNUMOFSTANDARD         29

/****************************************************************************
 Global Types
 */

/* Standard functions */
typedef RwBool (*RwStandardFunc)(void *pOut,void *pInOut,RwInt32 nI);

typedef struct RwEngineOpenParams RwEngineOpenParams;

/**
 * \ingroup datatypes
 * \struct RwEngineOpenParams
 * This type is used to specify device dependent parameters
 * for use by the API function \ref RwEngineOpen.
 * For a Windows application the displayID field
 * should be set to the window's handle (of type HWND).
 * For NULL and sky libraries displayID=0:
 */
struct RwEngineOpenParams
{
    void    *displayID;     /**< Display Identifier */
};

/* nOption is one of a list of possible System defines (see above) */
typedef RwBool
    (*RwSystemFunc)(RwInt32 nOption,
                    void *pOut,
                    void *pInOut,
                    RwInt32 nIn);

/* Device block */
typedef RwBool
    (*RwRenderStateSetFunction)(RwRenderState nState,void *pParam);

typedef RwBool
    (*RwRenderStateGetFunction)(RwRenderState nState,void *pParam);

typedef RwBool
    (*RwIm2DRenderLineFunction)(RwIm2DVertex *vertices,
                                RwInt32 numVertices,
                                RwInt32 vert1,
                                RwInt32 vert2);

typedef RwBool
    (*RwIm2DRenderTriangleFunction)(RwIm2DVertex *vertices,
                                    RwInt32 numVertices,
                                    RwInt32 vert1,
                                    RwInt32 vert2,
                                    RwInt32 vert3);

typedef RwBool
    (*RwIm2DRenderPrimitiveFunction)(RwPrimitiveType primType,
                                     RwIm2DVertex *vertices,
                                     RwInt32 numVertices);

typedef RwBool
    (*RwIm2DRenderIndexedPrimitiveFunction)(RwPrimitiveType primType,
                                            RwIm2DVertex *vertices,
                                            RwInt32 numVertices,
                                            RwImVertexIndex *indices,
                                            RwInt32 numIndices);

typedef RwBool
    (*RwIm3DRenderLineFunction)(RwInt32 vert1,
                                RwInt32 vert2);

typedef RwBool
    (*RwIm3DRenderTriangleFunction)(RwInt32 vert1,
                                    RwInt32 vert2,
                                    RwInt32 vert3);

typedef RwBool
    (*RwIm3DRenderPrimitiveFunction)(RwPrimitiveType primType);

typedef RwBool
    (*RwIm3DRenderIndexedPrimitiveFunction)(RwPrimitiveType primtype,
                                            RwImVertexIndex *indices,
                                            RwInt32 numIndices);


typedef struct RwDevice RwDevice;
/**
 * \ingroup datatypes
 * \struct RwDevice
 * Structure describing a display device
 */
struct RwDevice
{
    RwReal                                  gammaCorrection; /**<Gamma correction  */
    RwSystemFunc                            fpSystem;  /**< System handler */
    RwReal                                  zBufferNear; /**< Near Z buffer value */
    RwReal                                  zBufferFar; /**< Far Z buffer value */

    /* Immediate mode functions */
    RwRenderStateSetFunction                fpRenderStateSet; /**< Internal Use */
    RwRenderStateGetFunction                fpRenderStateGet; /**< Internal Use */

    /* Render functions */
    RwIm2DRenderLineFunction                fpIm2DRenderLine; /**< Internal Use */
    RwIm2DRenderTriangleFunction            fpIm2DRenderTriangle; /**< Internal Use */
    RwIm2DRenderPrimitiveFunction           fpIm2DRenderPrimitive; /**< Internal Use */
    RwIm2DRenderIndexedPrimitiveFunction    fpIm2DRenderIndexedPrimitive; /**< Internal Use */

    RwIm3DRenderLineFunction                fpIm3DRenderLine; /**< Internal Use */
    RwIm3DRenderTriangleFunction            fpIm3DRenderTriangle; /**< Internal Use */
    RwIm3DRenderPrimitiveFunction           fpIm3DRenderPrimitive; /**< Internal Use */
    RwIm3DRenderIndexedPrimitiveFunction    fpIm3DRenderIndexedPrimitive; /**< Internal Use */
};

typedef struct RwMetrics RwMetrics;
/**
 * \ingroup datatypes
 * \struct RwMetrics
 * This structure provides information about the performance
 * of the application.  The metrics are recorded only in the metrics
 * libraries.  To use metrics you should compile with the RWMETRICS
 * preprocessor symbol defines, and link with the metrics libraries
 * that ship with the SDK.  The metrics are recorded on a per-frame
 * basis.  Each platform may provide additional information that
 * is specific to that platform.  Note that either the
 * \ref numTextureUploads or \ref numResourceAllocs being non-zero can
 * be considered as bad news and will indicate a significantly
 * reduced rendering performance.
 */
struct RwMetrics
{
    RwUInt32    numTriangles;           /**< Number of triangles processed */
    RwUInt32    numProcTriangles;       /**< Number of mesh triangles processed */
    RwUInt32    numVertices;            /**< Number of vertices processed */
    RwUInt32    numTextureUploads;      /**< Number of textures swapped */
    RwUInt32    sizeTextureUploads;     /**< Size of textures swapped */
    RwUInt32    numResourceAllocs;      /**< Number of resource blocks swapped */
    void        *devSpecificMetrics;    /**< Device specific metrics */
};

#define SUBSYSTEMNAME_MAXLEN 80

typedef struct RwSubSystemInfo RwSubSystemInfo;
/**
 * \ingroup datatypes
 * \struct RwSubSystemInfo
 * This type is used to represent information about a device.
 * The only available field specifies a character string
 * which identifies the subsystem
 * (see API function \ref RwEngineGetSubSystemInfo). */
struct RwSubSystemInfo
{
    RwChar  name[SUBSYSTEMNAME_MAXLEN]; /**< Sub system string */
};


/* Video modes */
/* These are flag bits which may be ORd */

/**
 * \ingroup datatypes
 * RwVideoModeFlag
 * These flags specify the type of display that RenderWare
 * will use.  The flags may be OR'd together to build composite modes.
 * Note that not all modes are supported on all platforms.
 */
enum RwVideoModeFlag
{
    rwVIDEOMODEEXCLUSIVE  = 0x1,    /**<Exclusive (i.e. full-screen) */
    rwVIDEOMODEINTERLACE  = 0x2,    /**<Interlaced */
    rwVIDEOMODEFFINTERLACE  = 0x4,  /**<Flicker Free Interlaced */
    rwVIDEOMODEFSAA0 = 0x8,         /**< \if sky2
                                     *      Full-screen antialiasing mode 0
                                     *   \endif */
    rwVIDEOMODEFSAA1 = 0x10,        /**< \if sky2
                                     *      Full-screen antialiasing mode 1
                                     *   \endif */
    rwVIDEOMODEFLAGFORCEENUMSIZEINT = RWFORCEENUMSIZEINT
};
typedef enum RwVideoModeFlag RwVideoModeFlag;

typedef struct RwVideoMode RwVideoMode;
/**
 * \ingroup datatypes
 * \struct RwVideoMode
 * This type represents a video mode available on a device specified
 * by the frame buffer resolution (width and height) and depth,
 * and a flag indicating  whether the device has exclusive use of
 * the mode (see API function \ref RwEngineGetVideoModeInfo): */
struct RwVideoMode
{
        RwInt32         width;  /**< Width */
        RwInt32         height; /**< Height */
        RwInt32         depth;  /**< Depth */
        RwVideoModeFlag flags;  /**< Flags */
};

/**
 * \ingroup datatypes
 * RwEngineInitFlag
 * Engine initialization flags.  An application can use
 * these to control the memory manager that RenderWare uses for dynamic
 * memory management.  By default RenderWare uses FreeLists.  This is the
 * preferred way of using RenderWare.  If the application does not want
 * RenderWare to use the memory manager, then the application can pass
 * rwENGINEINITNOFREELISTS as the argument to \ref RwEngineInit and
 * RenderWare will replace freelist calls to corresponding calls to
 * RwMalloc and RwFree.  This will result in more memory management
 * related calls.
 */
enum RwEngineInitFlag
{
    rwENGINEINITFREELISTS = 0,      /**<Use Freelists */
    rwENGINEINITNOFREELISTS = 0x1,  /**<Don't use Freelists */
    rwENGINEINITFLAGFORCEENUMSIZEINT = RWFORCEENUMSIZEINT
};
typedef enum RwEngineInitFlag RwEngineInitFlag;

/****************************************************************************
 Function prototypes
 */

#ifdef    __cplusplus
extern "C"
{
#endif                          /* __cplusplus */

/* Get the library binary version */
extern RwInt32 RwEngineGetVersion(void);

/* Sequence of events to get RenderWare up and running */
extern RwBool RwEngineInit(RwMemoryFunctions *memFuncs, RwUInt32 initFlags, RwUInt32 resArenaSize);
extern RwInt32 RwEngineRegisterPlugin(RwInt32 size, RwUInt32 pluginID,
                                  RwPluginObjectConstructor initCB,
                                  RwPluginObjectDestructor termCB);
extern RwInt32 RwEngineGetPluginOffset(RwUInt32 pluginID);
extern RwBool RwEngineOpen(RwEngineOpenParams *initParams);
extern RwBool RwEngineStart(void);
extern RwBool RwEngineStop(void);
extern RwBool RwEngineClose(void);
extern RwBool RwEngineTerm(void);

/* Finding out about the rendering sub systems available */
extern RwInt32 RwEngineGetNumSubSystems(void);
extern RwSubSystemInfo *RwEngineGetSubSystemInfo(RwSubSystemInfo *subSystemInfo, RwInt32 subSystemIndex);
extern RwInt32 RwEngineGetCurrentSubSystem(void);
extern RwBool RwEngineSetSubSystem(RwInt32 subSystemIndex);

/* Finding out about the modes available */
extern RwInt32 RwEngineGetNumVideoModes(void);
extern RwVideoMode *RwEngineGetVideoModeInfo(RwVideoMode *modeinfo, RwInt32 modeIndex);
extern RwInt32 RwEngineGetCurrentVideoMode(void);
extern RwBool RwEngineSetVideoMode(RwInt32 modeIndex);

/* Finding out how much texture memory is available */
extern RwInt32 RwEngineGetTextureMemorySize(void);
extern RwInt32 RwEngineGetMaxTextureSize(void);

/* Getting/Releasing the focus */
extern RwBool RwEngineSetFocus(RwBool enable);

/* Getting metrics */
extern RwMetrics *RwEngineGetMetrics(void);

#ifdef    __cplusplus
}
#endif                          /* __cplusplus */


/*--- Automatically derived from: C:/daily/rwsdk/src/plcore/bafsys.h ---*/

/****************************************************************************
 Global Types
 */

/*
 * \typedef rwFnFexist
 * Returns TRUE if file with given name exists, FALSE if it doesn't.
 */
typedef RwBool  (*rwFnFexist)(const RwChar *name);

/*
 * \typedef rwFnFopen
 * Mimics ANSI C Standard Library fopen.
 */
typedef void   *(*rwFnFopen)(const RwChar *name, const RwChar *mode);

/*
 * \typedef rwFnFclose
 * Mimics ANSI C Standard Library fclose.
 */
typedef int     (*rwFnFclose)(void *fptr);

/*
 * \typedef rwFnFread
 * Mimics ANSI C Standard Library fread.
 */
typedef size_t  (*rwFnFread)(void *addr, size_t size, size_t count, void *fptr);

/*
 * \typedef rwFnFwrite
 * Mimics ANSI C Standard Library fwrite.
 */
typedef size_t  (*rwFnFwrite)(const void *addr, size_t size, size_t count, void *fptr);

/*
 * \typedef rwFnFgets
 * Mimics ANSI C Standard Library fgets.
 */
typedef RwChar *(*rwFnFgets)(RwChar *buffer, int maxLen, void *fptr);

/*
 * \typedef rwFnFputs
 * Mimics ANSI C Standard Library fputs.
 */
typedef int     (*rwFnFputs)(const RwChar *buffer, void *fptr);

/*
 * \typedef rwFnFeof
 * Mimics ANSI C Standard Library feof.
 */
typedef int     (*rwFnFeof)(void *fptr);

/*
 * \typedef rwFnFseek
 * Mimics ANSI C Standard Library fseek.
 */
typedef int     (*rwFnFseek)(void *fptr, long offset, int origin);

/*
 * \typedef rwFnFflush
 * Mimics ANSI C Standard Library fflush.
 */
typedef int     (*rwFnFflush)(void *fptr);

/*
 * \typedef rwFnFtell
 * Mimics ANSI C Standard Library ftell.
 */
typedef int     (*rwFnFtell)(void *fptr);


/**
 * \ingroup datatypes
 * \typedef RwFileFunctions
 * typedef for struct RwFileFunctions
 */
typedef struct RwFileFunctions RwFileFunctions;
/**
 * \ingroup datatypes
 * \struct RwFileFunctions
 * This type is used to specify the file access
 * functions used by RenderWare. The default file system uses the standard
 * ANSI functions. The application may install an alternative file system
 * providing it complies with the ANSI interface -- see API 
 * function \ref RwOsGetFileInterface.
 *
 * The function types associated with this type are defined as follows 
 *
 * \verbatim
   RwBool (*rwFnFexist)(const RwChar *name)
       void *(*rwFnFopen)(const RwChar *name, const RwChar *access) 
       int (*rwFnFclose)(void *fptr)
       size_t (*rwFnFread)(void *addr, size_t size, size_t count, void *fptr)                        
       size_t (*rwFnFwrite)(const void *addr, size_t size, size_t count,
                 void *fptr)
   RwChar *(*rwFnFgets)(RwChar *buffer, int maxLen, void *fptr)
       int (*rwFnFputs)(const RwChar *buffer, void *fptr)
       int (*rwFnFeof)(void *fptr)
       int (*rwFnFseek)(void *fptr, long offset, int origin)
       int (*rwFnFflush)(void *fptr)
       int (*rwFnFtell)(void *fptr)
 \endverbatim
 * Note the function argument lists are consistent with the ANSI
 * standard file access interface:
 */
struct RwFileFunctions
{
    rwFnFexist  rwfexist; /**< Pointer to fexist function */
    rwFnFopen   rwfopen;  /**< Pointer to fopen function */
    rwFnFclose  rwfclose; /**< Pointer to fclose function */
    rwFnFread   rwfread;  /**< Pointer to fread function */
    rwFnFwrite  rwfwrite; /**< Pointer to fwrite function */
    rwFnFgets   rwfgets;  /**< Pointer to fgets function */
    rwFnFputs   rwfputs;  /**< Pointer to puts function */
    rwFnFeof    rwfeof;   /**< Pointer to feof function */
    rwFnFseek   rwfseek;  /**< Pointer to fseek function */
    rwFnFflush  rwfflush; /**< Pointer to fflush function */
    rwFnFtell   rwftell;  /**< Pointer to ftell function */  
};


#ifdef    __cplusplus
extern "C"
{
#endif                          /* __cplusplus */

extern RwFileFunctions *RwOsGetFileInterface(void);

#ifdef    __cplusplus
}
#endif                          /* __cplusplus */


/*--- Automatically derived from: C:/daily/rwsdk/src/plcore/baerr.h ---*/
/****************************************************************************
 Global Types
 */

typedef struct RwError RwError;
/**
 * \ingroup datatypes
 * \struct RwError
 * This type represents a RenderWare error specified by the
 * ID of the plugin that the error was issued from (pluginID) and the error
 * code itself (errorCode) (see API function \ref RwErrorGet).
 * \param pluginID The ID of the plugin that issued the error.
 * \param errorCode A value representing the error code.
 */
struct RwError
{
    RwInt32     pluginID;  /**< Internal Use */
    RwInt32     errorCode; /**< Internal Use */
};

#define RWECODE(a,b) a,

/* common errors have the MSB set */

enum RwErrorCodeCommon
{
    E_RW_NOERROR = (int)0x80000000L,
#include "errcom.def"
    E_RW_LASTERROR = RWFORCEENUMSIZEINT
};
typedef enum RwErrorCodeCommon RwErrorCodeCommon;

#undef RWECODE

/****************************************************************************
 Function prototypes
 */

#ifdef    __cplusplus
extern "C"
{
#endif                          /* __cplusplus */

extern RwError *RwErrorGet(RwError *code);
extern RwError *RwErrorSet(RwError *code);
extern RwInt32 _rwerror(RwInt32 code, ...);

#ifdef    __cplusplus
}
#endif                          /* __cplusplus */


/*--- Automatically derived from: C:/daily/rwsdk/src/plcore/badebug.h ---*/

/****************************************************************************
 Global Types
 */

/**
 * \ingroup datatypes
 * RwDebugType 
 * This type represents the different types of debug and 
 * trace messages that can be sent to the currently installed debug handler 
 * (see API function \ref RwDebugSendMessage)*/
enum RwDebugType
{
    rwNADEBUGTYPE = 0,          /**<Invalid */
    rwDEBUGASSERT,              /**<Send an assert message */
    rwDEBUGERROR,               /**<Send an error message */
    rwDEBUGMESSAGE,             /**<Send an informational message */
    rwDEBUGTRACE,               /**<Send a trace message */
    rwDEBUGTYPEFORCEENUMSIZEINT = RWFORCEENUMSIZEINT
};
typedef enum RwDebugType RwDebugType;

/**
 * \ingroup datatypes
 * \ref RwDebugHandler
 * This type represents the
 * function called from \ref RwDebugSendMessage for sending a message to the
 * RenderWare debug stream.
 *
 * \param  type   Type of debug message (assert, error, etc.).
 *
 * \param  string   Pointer to a string containing the error
 * message.
 *
 * \see RwDebugSetHandler
 */
typedef void        (*RwDebugHandler) (RwDebugType type,

                                       const RwChar * string);

#ifdef RWDEBUG

#define RwDebugSendMessage(type, funcName, message)     \
        _rwDebugSendMessage(type,                       \
                            RWSTRING(__FILE__),         \
                            __LINE__,                   \
                            funcName,                   \
                            message)

/****************************************************************************
 Function prototypes
 */

#ifdef    __cplusplus
extern              "C"
{
#endif                          /* __cplusplus */

/* Setting the debug message handler */
extern RwDebugHandler RwDebugSetHandler(RwDebugHandler handler);
extern void         RwDebugSetTraceState(RwBool state);

extern void         _rwDebugSendMessage(RwDebugType type,
                                        const RwChar * file,
                                        const RwInt32 line,
                                        const RwChar * funcName,
                                        const RwChar * message);

/* Sending a message */
extern RwChar      *_rwdberrcommon(RwInt32 code, ...);
extern RwChar      *_rwdbsprintf(const RwChar * format,
                                 ...) __RWFORMAT__(printf, 1, 2);


#ifdef    __cplusplus
}
#endif                          /* __cplusplus */

#else /* RWDEBUG */

#define RwDebugSetHandler(handler)
#define RwDebugSetTraceState(state)
#define RwDebugSendMessage(type, funcName, message)

#if (!defined(RWREGSETDEBUGTRACE))
#define RWREGSETDEBUGTRACE(_name) /* No op */
#endif /* (!defined(RWREGSETDEBUGTRACE)) */

#endif /* RWDEBUG */


/*--- Automatically derived from: C:/daily/rwsdk/src/plcore/balibtyp.h ---*/
/* Finding MSBs */

#define RWBYTEFINDMSB(a) \
   (_rwMsbBit[(a)]-1)

#define RWWORDFINDMSB(a) \
   (((a)&0xff00)?RWBYTEFINDMSB((a)>>8)+8: RWBYTEFINDMSB(a))

#define RWLONGFINDMSB(a) \
   (((a)&0xffff0000UL)?RWWORDFINDMSB((a)>>16)+16: RWWORDFINDMSB(a))

/****************************************************************************
 Defines
 */

/* macros used to access plugin data in objects */
#define RWPLUGINOFFSET(_type, _base, _offset)                   \
   ((_type *)((RwUInt8 *)(_base) + (_offset)))

#define RWPLUGINOFFSETCONST(_type, _base, _offset)              \
   ((const _type *)((const RwUInt8 *)(_base) + (_offset)))

/* macro used to access global data structure (the root type is RwGlobals) */
#define RWSRCGLOBAL(variable) \
   (((RwGlobals *)RwEngineInstance)->variable)

#define RWASSERTISTYPE(_f, _t) \
   RWASSERT((((const RwObject *)(_f))->type)==(_t))

/****************************************************************************
 Global Types
 */

enum RwEngineStatus
{
    rwENGINESTATUSIDLE = 0,                     /* This needs to be zero */
    rwENGINESTATUSINITED = 1,
    rwENGINESTATUSOPENED = 2,
    rwENGINESTATUSSTARTED = 3,
    rwENGINESTATUSFORCEENUMSIZEINT = RWFORCEENUMSIZEINT
};
typedef enum RwEngineStatus RwEngineStatus;

typedef struct RwGlobals RwGlobals;
struct RwGlobals
{
#ifdef RWDEBUG
        RwDebugHandler      debugFunction;          /* debug string handler */
        void                *debugFile;             /* debug output file */
        RwInt32             debugStackDepth;        /* current depth of function stack */
        RwBool              debugTrace;             /* is function tracing enabled */
#endif

        /* Current entities */
        void                *curCamera;             /* Current camera */
        void                *curWorld;              /* Current World */

        /* Checking counters */
        RwUInt16            renderFrame;            /* Camera display count */
        RwUInt16            lightFrame;             /* Used to ensure each light is applied only once. */
        RwUInt16            pad[2];                 /* Longword align it again */

        /* For the currently accessed device */
        RwDevice            dOpenDevice;

        /* Standard renderers and functions */
        RwStandardFunc      stdFunc[rwSTANDARDNUMOFSTANDARD];

        /* All of the frames which have been updated */
        RwLinkList          dirtyFrameList;

        /* The file functions */
        RwFileFunctions     fileFuncs;

        /* The string functions */
        RwStringFunctions   stringFuncs;

        /* The memory allocation functions */
        RwMemoryFunctions   memoryFuncs;
#ifdef RWDEBUG
        RwBool              freeListExtraDebug;
#endif /* RWDEBUG */

        /* virtual memory alloc/free functions */
        RwMemoryAllocFn         memoryAlloc;
        RwMemoryFreeFn          memoryFree;

        RwMetrics           *metrics;

        /* Current engine status */
        RwEngineStatus      engineStatus;

        /* Resource arena init size. */
        RwUInt32            resArenaInitSize;
};

typedef struct RwModuleInfo RwModuleInfo;
struct RwModuleInfo
{
        RwInt32     globalsOffset;
        RwInt32     numInstances;
};



/****************************************************************************
 Program wide globals
 */

#ifdef    __cplusplus
extern "C"
{
#endif                          /* __cplusplus */

#ifdef RWGLOBALSIZE
extern RwUInt32     ourGlobals[RWGLOBALSIZE / sizeof(RwUInt32)];
#define RwEngineInstance ourGlobals
#else /* RWGLOBALSIZE */
extern void         *RwEngineInstance;
#endif /* RWGLOBALSIZE */

extern RwInt8 _rwMsbBit[];

#ifdef    __cplusplus
}
#endif                          /* __cplusplus */


/*--- Automatically derived from: C:/daily/rwsdk/src/plcore/baresour.h ---*/

#define RWRESOURCESGLOBAL(var) (RWPLUGINOFFSET(rwResourcesGlobals,  \
    RwEngineInstance, resourcesModule.globalsOffset)->var)

/**
 * \ingroup datatypes
 * \typedef RwResEntry
 * RwResEntry object. Instanced data block in resources arena.
 * This should be considered an opaque
 * type. Use the RwResEntry API functions to access.
 */
typedef struct RwResEntry RwResEntry;

/**
 * \ingroup datatypes
 * \typedef RwResEntryDestroyNotify
 * This type represents the function
 * called from \ref RwResourcesFreeResEntry (and indirectly from
 * \ref RwResourcesEmptyArena) immediately before the memory used by the
 * specified resources entry is released.
 *
 * \param  resEntry   Pointer to the instanced data.
 */
typedef void        (*RwResEntryDestroyNotify) (RwResEntry * resEntry);

#if (!defined(DOXYGEN))
struct RwResEntry
{
    RwLLLink            link;   /**< Node in the list of resource elements */
    RwInt32             size;   /**< Size of this node */
    void               *owner;  /**< Owner of this node */
    RwResEntry        **ownerRef; /**< Pointer to pointer to this (enables de-alloc) */
    RwResEntryDestroyNotify destroyNotify; /**< This is called right before destruction */
};
#endif /* (!defined(DOXYGEN)) */

typedef struct rwResources rwResources;
struct rwResources
{
    RwInt32             maxSize;
    RwInt32             currentSize;
    RwInt32             reusageSize;

    void               *memHeap;

    RwLinkList          entriesA;
    RwLinkList          entriesB;

    RwLinkList         *freeEntries;
    RwLinkList         *usedEntries;
};

typedef struct rwResourcesGlobals rwResourcesGlobals;
struct rwResourcesGlobals
{
    rwResources         res;
};


#ifdef    __cplusplus
extern              "C"
{
#endif                          /* __cplusplus */

/* Setting the resources arena size */
extern RwBool       RwResourcesSetArenaSize(RwUInt32 size);
extern RwInt32      RwResourcesGetArenaSize(void);
extern RwInt32      RwResourcesGetArenaUsage(void);
extern RwBool       RwResourcesEmptyArena(void);

/* Allocate */
extern RwResEntry  *RwResourcesAllocateResEntry(void *owner,
                                                RwResEntry **ownerRef,
                                                RwInt32 size,
                                                RwResEntryDestroyNotify
                                                destroyNotify);
/* Deallocate */
extern RwBool       RwResourcesFreeResEntry(RwResEntry * entry);
/* Mark all as unused */
extern void         _rwResourcesPurge(void);
#if ((defined(RWDEBUG)) || (defined(RWSUPPRESSINLINE)))
/* Mark as used */
extern RwResEntry  *RwResourcesUseResEntry(RwResEntry * entry);
#endif /* ((defined(RWDEBUG)) || (defined(RWSUPPRESSINLINE))) */

extern RwModuleInfo resourcesModule;


#ifdef    __cplusplus
}
#endif                          /* __cplusplus */

#if ((!defined(RWDEBUG)) && (!defined(RWSUPPRESSINLINE)))
#define RwResourcesUseResEntry(_ntry)                               \
    ((((_ntry)->link.next)?                                         \
          (rwLinkListRemoveLLLink(&((_ntry)->link)),                \
           rwLinkListAddLLLink(RWRESOURCESGLOBAL(res.usedEntries),  \
                               &((_ntry)->link))):                  \
          NULL),                                                    \
     (_ntry))
#endif /* ((!defined(RWDEBUG)) && (!defined(RWSUPPRESSINLINE))) */


/*--- Automatically derived from: C:/daily/rwsdk/src/plcore/bacolor.h ---*/
/****************************************************************************
 Global Types
 */

typedef struct RwRGBAReal RwRGBAReal;
/**
 * \ingroup datatypes
 * \struct RwRGBAReal
 * This structure represents a RGBA color which has 
 * components specified as real values. 
 *
 * A color component of an RwRGBA with the value 255 generally corresponds 
 * to the associated component in an RwRGBAReal with the value 1.0f. 
 * However, any values can be substituted to denormalize/normalize 
 * RwRGBAReal and create different effects. For example, while light colors
 * are expressed as normalized RGBA, interesting effects can be gained using 
 * larger values. 
 *
 * It should also be noted that a color component of an RwRGBA with the 
 * value 0 generally corresponds to the associcated component in an 
 * RwRGBAReal with the value 0.0.
 */
struct RwRGBAReal
{
    RwReal red;     /**< red component */
    RwReal green;   /**< green component */
    RwReal blue;    /**< blue component */
    RwReal alpha;   /**< alpha component */
};

#if (!defined(RwRGBARealAssign))
#define RwRGBARealAssign(_target, _source)        \
    ( *(_target) = *(_source) )
#endif /* (!defined(RwRGBARealAssign)) */

typedef struct RwRGBA RwRGBA;
/**
 * \ingroup datatypes
 * \struct RwRGBA
 * This structure represents a RGBA color 
 * which has integer components specified in the range 0 to 255. */
struct RwRGBA
{
    RwUInt8 red;    /**< red component */
    RwUInt8 green;  /**< green component */
    RwUInt8 blue;   /**< blue component */
    RwUInt8 alpha;  /**< alpha component */
};

#if (!defined(RwRGBAAssign))
#define RwRGBAAssign(_target, _source)             \
    ( *(_target) = *(_source) )
#endif /* (!defined(RwRGBAAssign)) */

#define RwRGBARealAddMacro(o,a,b)                                        \
MACRO_START                                                              \
{                                                                        \
    (o)->red   = (((a)->red) + (   (b)->red));                           \
    (o)->green = (((a)->green) + ( (b)->green));                         \
    (o)->blue  = (((a)->blue) + (  (b)->blue));                          \
    (o)->alpha = (((a)->alpha) + ( (b)->alpha));                         \
}                                                                        \
MACRO_STOP

#define RwRGBARealSubMacro(o,a,b)                                        \
MACRO_START                                                              \
{                                                                        \
    (o)->red   = (((a)->red) - (   (b)->red));                           \
    (o)->green = (((a)->green) - ( (b)->green));                         \
    (o)->blue  = (((a)->blue) - (  (b)->blue));                          \
    (o)->alpha = (((a)->alpha) - ( (b)->alpha));                         \
}                                                                        \
MACRO_STOP

#define RwRGBARealScaleMacro(o,a,scale)                                  \
MACRO_START                                                              \
{                                                                        \
    (o)->red   = (((a)->red) * (   scale));                              \
    (o)->green = (((a)->green) * ( scale));                              \
    (o)->blue  = (((a)->blue) * (  scale));                              \
    (o)->alpha = (((a)->alpha) * ( scale));                              \
}                                                                        \
MACRO_STOP

/* Conversion macros */
#define RwRGBAFromRwRGBARealMacro(o, i)                                  \
MACRO_START                                                              \
{                                                                        \
    RwInt32 quantize;                                                    \
                                                                         \
    quantize = RwInt32FromRealMacro( ((i)->red   * (RwReal)255.0)             \
                                + (RwReal)0.5 );                         \
    (o)->red   = (RwUInt8) quantize;                                     \
    quantize = RwInt32FromRealMacro( ((i)->green * (RwReal)255.0)             \
                                + (RwReal)0.5 );                         \
    (o)->green = (RwUInt8) quantize;                                     \
    quantize = RwInt32FromRealMacro( ((i)->blue  * (RwReal)255.0)             \
                                + (RwReal)0.5 );                         \
    (o)->blue  = (RwUInt8) quantize;                                     \
    quantize = RwInt32FromRealMacro( ((i)->alpha * (RwReal)255.0)             \
                                + (RwReal)0.5 );                         \
    (o)->alpha = (RwUInt8) quantize;                                     \
                                                                         \
}                                                                        \
MACRO_STOP

#define RwRGBARealFromRwRGBAMacro(o, i)                                  \
MACRO_START                                                              \
{                                                                        \
    (o)->red   =                                                         \
        (((RwReal)(((i)->red))) * (   (RwReal)((1.0/255.0))));           \
    (o)->green =                                                         \
        (((RwReal)(((i)->green))) * ( (RwReal)((1.0/255.0))));           \
    (o)->blue  =                                                         \
        (((RwReal)(((i)->blue))) * (  (RwReal)((1.0/255.0))));           \
    (o)->alpha =                                                         \
        (((RwReal)(((i)->alpha))) * ( (RwReal)((1.0/255.0))));           \
}                                                                        \
MACRO_STOP

/****************************************************************************
 Function prototypes
 */

#ifdef    __cplusplus
extern "C"
{
#endif                          /* __cplusplus */


#if (! ( defined(RWDEBUG) || defined(RWSUPPRESSINLINE) ))

#define RwRGBARealAdd(o,a,b) \
        RwRGBARealAddMacro(o,a,b)

#define RwRGBARealSub(o,a,b) \
        RwRGBARealSubMacro(o,a,b)

#define RwRGBARealScale(o,a,scale) \
        RwRGBARealScaleMacro(o,a,scale)

#define RwRGBAFromRwRGBAReal(o, i) \
        RwRGBAFromRwRGBARealMacro(o, i)

#define RwRGBARealFromRwRGBA(o, i) \
        RwRGBARealFromRwRGBAMacro(o, i)

#else /* (! ( defined(RWDEBUG) || defined(RWSUPPRESSINLINE) )) */

/* Function versions for debug */
extern void RwRGBARealAdd(RwRGBAReal *result,
                          const RwRGBAReal *source1,
                          const RwRGBAReal *source2);

extern void RwRGBARealSub(RwRGBAReal *result,
                          const RwRGBAReal *source1,
                          const RwRGBAReal *source2);

extern void RwRGBARealScale(RwRGBAReal *result,
                            const RwRGBAReal *source,
                            RwReal scalar);

extern void RwRGBAFromRwRGBAReal(RwRGBA *result,
                                 const RwRGBAReal *source);

extern void RwRGBARealFromRwRGBA(RwRGBAReal *result,
                                 RwRGBA *source);

#endif /* (! ( defined(RWDEBUG) || defined(RWSUPPRESSINLINE) )) */

#ifdef    __cplusplus
}
#endif                          /* __cplusplus */


/*--- Automatically derived from: C:/daily/rwsdk/src/plcore/babinmtx.h ---*/

/****************************************************************************
 Global types
 */

/* Matrix stream format */
typedef struct rwStreamMatrix RwMatrixChunkInfo;
typedef struct rwStreamMatrix rwStreamMatrix;
struct rwStreamMatrix
{
    RwV3d               right;
    RwV3d               up;
    RwV3d               at;
    RwV3d               pos;
    RwInt32             type;
};


/****************************************************************************
 Function prototypes
 */

#ifdef    __cplusplus
extern              "C"
{
#endif                          /* __cplusplus */

/* Matrix binary format */
extern RwUInt32     RwMatrixStreamGetSize(const RwMatrix * matrix);
extern RwMatrix    *RwMatrixStreamRead(RwStream * stream,
                                       RwMatrix * matrix);
extern const RwMatrix *RwMatrixStreamWrite(const RwMatrix * matrix,
                                           RwStream * stream);
extern RwMatrixChunkInfo *RwMatrixChunkInfoRead(RwStream * stream,
                                                RwMatrixChunkInfo *
                                                matrixChunkInfo,
                                                RwInt32 * bytesRead);

#ifdef    __cplusplus
}
#endif                          /* __cplusplus */


/*--- Automatically derived from: C:/daily/rwsdk/src/plcore/babinary.h ---*/
/****************************************************************************
 Defines
 */

#ifndef rwCHUNKHEADERSIZE
#define rwCHUNKHEADERSIZE (sizeof(RwUInt32)*3)
#endif /* rwCHUNKHEADERSIZE */

/* Compatibility macro */
#define RwStreamWriteInt(_stream, _ints, _numBytes) \
        RwStreamWriteInt32(_stream, _ints, _numBytes)

#define RwStreamReadInt(_stream, _ints, _numBytes) \
        RwStreamReadInt32(_stream, _ints, _numBytes)

#define RwMemLittleEndian(_mem, _size) \
        RwMemLittleEndian32(_mem, _size)

#define RwMemNative(_mem, _size) \
        RwMemNative32(_mem, _size)

/****************************************************************************
 Global Types
 */

typedef struct RwChunkHeaderInfo RwChunkHeaderInfo;
/**
 * \ingroup datatypes
 * \struct RwChunkHeaderInfo
 * Holds data for a chunk header read from a
 * stream with \ref RwStreamReadChunkHeaderInfo. */
struct RwChunkHeaderInfo
{
    RwUInt32 type;      /**< chunk ID - see \ref RwStreamFindChunk */
    RwUInt32 length;    /**< length of the chunk data in bytes */
    RwUInt32 version;   /**< version of the chunk data. 
                         *   See \ref RwEngineGetVersion. */
    RwUInt32 buildNum;  /**< build number of the RenderWare libraries
                         *   previously used to stream out the data */
    RwBool isComplex;   /**< Internal Use */
};

/****************************************************************************
 Function prototypes
 */

#ifdef    __cplusplus
extern "C"
{
#endif                          /* __cplusplus */

/* Chunk header stuff */
extern RwBool RwStreamFindChunk(RwStream *stream, RwUInt32 type,
                                RwUInt32 *lengthOut, RwUInt32 *versionOut);

#define RwStreamWriteChunkHeader(stream, type, size) \
    _rwStreamWriteVersionedChunkHeader(         \
        stream, type, size, rwLIBRARYCURRENTVERSION, RWBUILDNUMBER)

extern RwStream *_rwStreamWriteVersionedChunkHeader(RwStream *stream,
                                                   RwInt32 type,
                                                   RwInt32 size,
                                                   RwUInt32 version,
                                                   RwUInt32 buildNum);

extern RwStream *RwStreamWriteReal(RwStream *stream, const RwReal *reals,
                                   RwUInt32 numBytes);
extern RwStream *RwStreamWriteInt32(RwStream *stream, const RwInt32 *ints,
                                    RwUInt32 numBytes);
extern RwStream *RwStreamWriteInt16(RwStream *stream, const RwInt16 *ints,
                                    RwUInt32 numBytes);

extern RwStream *RwStreamReadReal(RwStream *stream, RwReal *reals,
                                  RwUInt32 numBytes);
extern RwStream *RwStreamReadInt32(RwStream *stream, RwInt32 *ints,
                                   RwUInt32 numBytes);
extern RwStream *RwStreamReadInt16(RwStream *stream, RwInt16 *ints,
                                   RwUInt32 numBytes);

/* Binary Portability Functions */
extern void *RwMemLittleEndian16(void *mem, RwUInt32 size);
extern void *RwMemLittleEndian32(void *mem, RwUInt32 size);
extern void *RwMemNative16(void *mem, RwUInt32 size);
extern void *RwMemNative32(void *mem, RwUInt32 size);
extern void *RwMemRealToFloat32(void *mem, RwUInt32 size);
extern void *RwMemFloat32ToReal(void *mem, RwUInt32 size);

extern RwStream *
RwStreamReadChunkHeaderInfo(RwStream *stream, RwChunkHeaderInfo *chunkHeaderInfo);

#ifdef    __cplusplus
}
#endif                          /* __cplusplus */

#endif /* RWPLCORE_H */
