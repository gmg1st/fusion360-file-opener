// F3DThumbnailProvider - Native C COM IThumbnailProvider for .f3d/.f3z files
// Pure C implementation - no .NET, no C++ runtime dependency
// Compiled with TCC (Tiny C Compiler) for maximum compatibility

#define WIN32_LEAN_AND_MEAN
#define UNICODE
#define _UNICODE
#define COBJMACROS
#define INITGUID

#include <windows.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

// Ensure __MIDL_CONST is defined (needed for REFIID in C mode)
#ifndef __MIDL_CONST
#define __MIDL_CONST
#endif

// ========== Forward declarations for TCC missing headers ==========

// We define everything we need manually since TCC lacks objbase.h/gdiplus.h

// Basic COM types are defined by TCC's guiddef.h (included via windows.h)
// We just need HRESULT and ULONG if not already defined
#ifndef _HRESULT_DEFINED
typedef long HRESULT;
#endif
#ifndef _ULONG_DEFINED
typedef unsigned long ULONG;
#endif

#define S_OK     ((HRESULT)0)
#define S_FALSE  ((HRESULT)1)
#define E_FAIL   ((HRESULT)0x80004005)
#define E_NOINTERFACE ((HRESULT)0x80004002)
#define E_POINTER ((HRESULT)0x80004003)
#define E_INVALIDARG ((HRESULT)0x80070057)
#define E_OUTOFMEMORY ((HRESULT)0x8007000E)
#define E_NOTIMPL ((HRESULT)0x80004001)
#define CLASS_E_CLASSNOTAVAILABLE ((HRESULT)0x80040111)
#define CLASS_E_NOAGGREGATION ((HRESULT)0x80040110)

#ifndef STDMETHODCALLTYPE
#define STDMETHODCALLTYPE __stdcall
#endif

// IUnknown interface
typedef struct IUnknown IUnknown;
typedef struct IUnknownVtbl {
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(IUnknown*, REFIID, void**);
    ULONG (STDMETHODCALLTYPE *AddRef)(IUnknown*);
    ULONG (STDMETHODCALLTYPE *Release)(IUnknown*);
} IUnknownVtbl;
struct IUnknown { IUnknownVtbl* lpVtbl; };

// IStream forward decl (we only need it as opaque type)
typedef struct IStream IStream;
typedef struct IStreamVtbl {
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(IStream*, REFIID, void**);
    ULONG (STDMETHODCALLTYPE *AddRef)(IStream*);
    ULONG (STDMETHODCALLTYPE *Release)(IStream*);
    // ISequentialStream
    HRESULT (STDMETHODCALLTYPE *Read)(IStream*, void*, ULONG, ULONG*);
    HRESULT (STDMETHODCALLTYPE *Write)(IStream*, const void*, ULONG, ULONG*);
    // IStream
    HRESULT (STDMETHODCALLTYPE *Seek)(IStream*, LARGE_INTEGER, DWORD, ULARGE_INTEGER*);
    HRESULT (STDMETHODCALLTYPE *SetSize)(IStream*, ULARGE_INTEGER);
    HRESULT (STDMETHODCALLTYPE *CopyTo)(IStream*, IStream*, ULARGE_INTEGER, ULARGE_INTEGER*, ULARGE_INTEGER*);
    HRESULT (STDMETHODCALLTYPE *Commit)(IStream*, DWORD);
    HRESULT (STDMETHODCALLTYPE *Revert)(IStream*);
    HRESULT (STDMETHODCALLTYPE *LockRegion)(IStream*, ULARGE_INTEGER, ULARGE_INTEGER, DWORD);
    HRESULT (STDMETHODCALLTYPE *UnlockRegion)(IStream*, ULARGE_INTEGER, ULARGE_INTEGER, DWORD);
    HRESULT (STDMETHODCALLTYPE *Stat)(IStream*, void*, DWORD);
    HRESULT (STDMETHODCALLTYPE *Clone)(IStream*, IStream**);
} IStreamVtbl;
struct IStream { IStreamVtbl* lpVtbl; };

// IClassFactory interface
typedef struct IClassFactory IClassFactory;
typedef struct IClassFactoryVtbl {
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(IClassFactory*, REFIID, void**);
    ULONG (STDMETHODCALLTYPE *AddRef)(IClassFactory*);
    ULONG (STDMETHODCALLTYPE *Release)(IClassFactory*);
    HRESULT (STDMETHODCALLTYPE *CreateInstance)(IClassFactory*, IUnknown*, REFIID, void**);
    HRESULT (STDMETHODCALLTYPE *LockServer)(IClassFactory*, BOOL);
} IClassFactoryVtbl;
struct IClassFactory { IClassFactoryVtbl* lpVtbl; };

// WTS_ALPHATYPE
typedef enum { WTSAT_UNKNOWN = 0, WTSAT_RGB = 1, WTSAT_ARGB = 2 } WTS_ALPHATYPE;

// IInitializeWithFile interface
typedef struct IInitializeWithFile IInitializeWithFile;
typedef struct IInitializeWithFileVtbl {
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(IInitializeWithFile*, REFIID, void**);
    ULONG (STDMETHODCALLTYPE *AddRef)(IInitializeWithFile*);
    ULONG (STDMETHODCALLTYPE *Release)(IInitializeWithFile*);
    HRESULT (STDMETHODCALLTYPE *Initialize)(IInitializeWithFile*, LPCWSTR, DWORD);
} IInitializeWithFileVtbl;
struct IInitializeWithFile { IInitializeWithFileVtbl* lpVtbl; };

// IInitializeWithStream interface
typedef struct IInitializeWithStream IInitializeWithStream;
typedef struct IInitializeWithStreamVtbl {
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(IInitializeWithStream*, REFIID, void**);
    ULONG (STDMETHODCALLTYPE *AddRef)(IInitializeWithStream*);
    ULONG (STDMETHODCALLTYPE *Release)(IInitializeWithStream*);
    HRESULT (STDMETHODCALLTYPE *Initialize)(IInitializeWithStream*, IStream*, DWORD);
} IInitializeWithStreamVtbl;
struct IInitializeWithStream { IInitializeWithStreamVtbl* lpVtbl; };

// IThumbnailProvider interface
typedef struct IThumbnailProvider IThumbnailProvider;
typedef struct IThumbnailProviderVtbl {
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(IThumbnailProvider*, REFIID, void**);
    ULONG (STDMETHODCALLTYPE *AddRef)(IThumbnailProvider*);
    ULONG (STDMETHODCALLTYPE *Release)(IThumbnailProvider*);
    HRESULT (STDMETHODCALLTYPE *GetThumbnail)(IThumbnailProvider*, UINT, HBITMAP*, WTS_ALPHATYPE*);
} IThumbnailProviderVtbl;
struct IThumbnailProvider { IThumbnailProviderVtbl* lpVtbl; };

// ========== GUIDs ==========

static const GUID IID_IUnknown = 
    { 0x00000000, 0x0000, 0x0000, { 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 } };
static const GUID IID_IClassFactory = 
    { 0x00000001, 0x0000, 0x0000, { 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 } };
// Correct IIDs from Windows SDK (verified via HKCR\Interface registry)
// IID_IThumbnailProvider = {E357FCCD-A995-4576-B01F-234630154E96}
static const GUID IID_IThumbnailProvider = 
    { 0xe357fccd, 0xa995, 0x4576, { 0xb0, 0x1f, 0x23, 0x46, 0x30, 0x15, 0x4e, 0x96 } };
// IID_IInitializeWithFile = {B7D14566-0509-4CCE-A71F-0A554233BD9B}
static const GUID IID_IInitializeWithFile = 
    { 0xb7d14566, 0x0509, 0x4cce, { 0xa7, 0x1f, 0x0a, 0x55, 0x42, 0x33, 0xbd, 0x9b } };
// IID_IInitializeWithStream = {B824B49D-22AC-4161-AC8A-9916E8FA3F7F}
static const GUID IID_IInitializeWithStream = 
    { 0xb824b49d, 0x22ac, 0x4161, { 0xac, 0x8a, 0x99, 0x16, 0xe8, 0xfa, 0x3f, 0x7f } };
// Our CLSID: {C7A3F5D2-8E4B-4F6A-9D1E-3B5C7F8A2E6D}
static const GUID CLSID_F3DThumbnailProvider = 
    { 0xc7a3f5d2, 0x8e4b, 0x4f6a, { 0x9d, 0x1e, 0x3b, 0x5c, 0x7f, 0x8a, 0x2e, 0x6d } };

static int IsEqualGUID_(const GUID* a, const GUID* b) {
    return memcmp(a, b, sizeof(GUID)) == 0;
}

// ========== Global Debug Logging ==========
static void DebugLog(const char* msg) {
    HANDLE hLog = CreateFileW(L"C:\\Users\\G11\\AppData\\Local\\Temp\\f3d_thumb.log",
        FILE_APPEND_DATA, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hLog != INVALID_HANDLE_VALUE) {
        DWORD written;
        WriteFile(hLog, msg, (DWORD)strlen(msg), &written, NULL);
        WriteFile(hLog, "\r\n", 2, &written, NULL);
        CloseHandle(hLog);
    }
}

static void DebugLogFmt(const char* fmt, ...) {
    char buf[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    DebugLog(buf);
}

// ========== GDI+ C Interface (manual declarations) ==========

typedef struct GpBitmap GpBitmap;
typedef struct GpImage GpImage;
typedef struct GpGraphics GpGraphics;

typedef struct GdiplusStartupInput {
    UINT32 GdiplusVersion;
    void* DebugEventCallback;
    BOOL SuppressBackgroundThread;
    BOOL SuppressExternalCodecs;
} GdiplusStartupInput;

typedef ULONG_PTR GdiplusStartupOutput; // simplified

// PixelFormat
#define PixelFormat32bppARGB 0x0026200A

// InterpolationMode
typedef enum {
    InterpolationModeHighQualityBicubic = 8
} InterpolationMode;

// GDI+ function pointers
typedef int (WINAPI *GdiplusStartup_t)(ULONG_PTR*, const GdiplusStartupInput*, void*);
typedef void (WINAPI *GdiplusShutdown_t)(ULONG_PTR);
typedef int (WINAPI *GdipCreateBitmapFromStream_t)(IStream*, GpBitmap**);
typedef int (WINAPI *GdipGetImageWidth_t)(GpImage*, UINT*);
typedef int (WINAPI *GdipGetImageHeight_t)(GpImage*, UINT*);
typedef int (WINAPI *GdipCreateBitmapFromScan0_t)(INT, INT, INT, INT, void*, GpBitmap**);
typedef int (WINAPI *GdipGetImageGraphicsContext_t)(GpImage*, GpGraphics**);
typedef int (WINAPI *GdipSetInterpolationMode_t)(GpGraphics*, InterpolationMode);
typedef int (WINAPI *GdipDrawImageRectI_t)(GpGraphics*, GpImage*, INT, INT, INT, INT);
typedef int (WINAPI *GdipCreateHBITMAPFromBitmap_t)(GpBitmap*, HBITMAP*, DWORD);
typedef int (WINAPI *GdipDisposeImage_t)(GpImage*);
typedef int (WINAPI *GdipDeleteGraphics_t)(GpGraphics*);

static GdiplusStartup_t g_GdiplusStartup = NULL;
static GdiplusShutdown_t g_GdiplusShutdown = NULL;
static GdipCreateBitmapFromStream_t g_GdipCreateBitmapFromStream = NULL;
static GdipGetImageWidth_t g_GdipGetImageWidth = NULL;
static GdipGetImageHeight_t g_GdipGetImageHeight = NULL;
static GdipCreateBitmapFromScan0_t g_GdipCreateBitmapFromScan0 = NULL;
static GdipGetImageGraphicsContext_t g_GdipGetImageGraphicsContext = NULL;
static GdipSetInterpolationMode_t g_GdipSetInterpolationMode = NULL;
static GdipDrawImageRectI_t g_GdipDrawImageRectI = NULL;
static GdipCreateHBITMAPFromBitmap_t g_GdipCreateHBITMAPFromBitmap = NULL;
static GdipDisposeImage_t g_GdipDisposeImage = NULL;
static GdipDeleteGraphics_t g_GdipDeleteGraphics = NULL;

static HMODULE g_hGdiplus = NULL;
static ULONG_PTR g_gdiplusToken = 0;

static BOOL InitGdiplus() {
    if (g_hGdiplus) return TRUE;
    g_hGdiplus = LoadLibraryW(L"gdiplus.dll");
    if (!g_hGdiplus) return FALSE;
    
    g_GdiplusStartup = (GdiplusStartup_t)GetProcAddress(g_hGdiplus, "GdiplusStartup");
    g_GdiplusShutdown = (GdiplusShutdown_t)GetProcAddress(g_hGdiplus, "GdiplusShutdown");
    g_GdipCreateBitmapFromStream = (GdipCreateBitmapFromStream_t)GetProcAddress(g_hGdiplus, "GdipCreateBitmapFromStream");
    g_GdipGetImageWidth = (GdipGetImageWidth_t)GetProcAddress(g_hGdiplus, "GdipGetImageWidth");
    g_GdipGetImageHeight = (GdipGetImageHeight_t)GetProcAddress(g_hGdiplus, "GdipGetImageHeight");
    g_GdipCreateBitmapFromScan0 = (GdipCreateBitmapFromScan0_t)GetProcAddress(g_hGdiplus, "GdipCreateBitmapFromScan0");
    g_GdipGetImageGraphicsContext = (GdipGetImageGraphicsContext_t)GetProcAddress(g_hGdiplus, "GdipGetImageGraphicsContext");
    g_GdipSetInterpolationMode = (GdipSetInterpolationMode_t)GetProcAddress(g_hGdiplus, "GdipSetInterpolationMode");
    g_GdipDrawImageRectI = (GdipDrawImageRectI_t)GetProcAddress(g_hGdiplus, "GdipDrawImageRectI");
    g_GdipCreateHBITMAPFromBitmap = (GdipCreateHBITMAPFromBitmap_t)GetProcAddress(g_hGdiplus, "GdipCreateHBITMAPFromBitmap");
    g_GdipDisposeImage = (GdipDisposeImage_t)GetProcAddress(g_hGdiplus, "GdipDisposeImage");
    g_GdipDeleteGraphics = (GdipDeleteGraphics_t)GetProcAddress(g_hGdiplus, "GdipDeleteGraphics");
    
    if (!g_GdiplusStartup || !g_GdiplusShutdown || !g_GdipCreateBitmapFromStream ||
        !g_GdipGetImageWidth || !g_GdipGetImageHeight || !g_GdipCreateBitmapFromScan0 ||
        !g_GdipGetImageGraphicsContext || !g_GdipSetInterpolationMode ||
        !g_GdipDrawImageRectI || !g_GdipCreateHBITMAPFromBitmap ||
        !g_GdipDisposeImage || !g_GdipDeleteGraphics) {
        return FALSE;
    }
    
    GdiplusStartupInput input = { 1, NULL, FALSE, FALSE };
    if (g_GdiplusStartup(&g_gdiplusToken, &input, NULL) != 0) return FALSE;
    
    return TRUE;
}

// ========== CreateStreamOnHGlobal declaration ==========

typedef HRESULT (WINAPI *CreateStreamOnHGlobal_t)(HGLOBAL, BOOL, IStream**);
static CreateStreamOnHGlobal_t g_CreateStreamOnHGlobal = NULL;

static BOOL InitOle32() {
    HMODULE hOle32 = LoadLibraryW(L"ole32.dll");
    if (!hOle32) return FALSE;
    g_CreateStreamOnHGlobal = (CreateStreamOnHGlobal_t)GetProcAddress(hOle32, "CreateStreamOnHGlobal");
    return g_CreateStreamOnHGlobal != NULL;
}

// ========== libzstd dynamic loading ==========

typedef unsigned long long (*ZSTD_decompress_t)(void*, size_t, const void*, size_t);
typedef unsigned long long (*ZSTD_getFrameContentSize_t)(const void*, size_t);
typedef unsigned long long (*ZSTD_isError_t)(unsigned long long);

static ZSTD_decompress_t g_ZSTD_decompress = NULL;
static ZSTD_getFrameContentSize_t g_ZSTD_getFrameContentSize = NULL;
static ZSTD_isError_t g_ZSTD_isError = NULL;
static HMODULE g_hZstd = NULL;

static BOOL InitZstd() {
    if (g_hZstd) return TRUE;
    
    const wchar_t* paths[] = {
        L"C:\\Users\\G11\\AppData\\Local\\F3DThumbnailProvider\\libzstd.dll",
        L"libzstd.dll",
        L"zstd.dll",
        NULL
    };
    
    for (int i = 0; paths[i]; i++) {
        g_hZstd = LoadLibraryW(paths[i]);
        if (g_hZstd) break;
    }
    if (!g_hZstd) return FALSE;
    
    g_ZSTD_decompress = (ZSTD_decompress_t)GetProcAddress(g_hZstd, "ZSTD_decompress");
    g_ZSTD_getFrameContentSize = (ZSTD_getFrameContentSize_t)GetProcAddress(g_hZstd, "ZSTD_getFrameContentSize");
    g_ZSTD_isError = (ZSTD_isError_t)GetProcAddress(g_hZstd, "ZSTD_isError");
    
    if (!g_ZSTD_decompress || !g_ZSTD_isError) {
        FreeLibrary(g_hZstd);
        g_hZstd = NULL;
        return FALSE;
    }
    return TRUE;
}

// ========== Windows 8+ Decompress API (for Deflate) ==========
// RtlDecompressBuffer uses LZNT1 (value 2), NOT standard Deflate!
// Windows 8+ provides CreateDecompressor/Decompress which supports COMPRESS_ALGORITHM_DEFLATE
// Also try RtlDecompressBuffer with COMPRESSION_FORMAT_DEFLATED (15)

typedef LONG NTSTATUS;
// RtlDecompressBuffer
typedef NTSTATUS (WINAPI *RtlDecompressBuffer_t)(USHORT, PUCHAR, ULONG, PUCHAR, ULONG, PULONG);
static RtlDecompressBuffer_t g_RtlDecompressBuffer = NULL;
static BOOL g_rtlInit = FALSE;

// Windows 8+ API
typedef HANDLE DECOMPRESSOR_HANDLE;
typedef BOOL (WINAPI *CreateDecompressor_t)(DWORD, PVOID, DECOMPRESSOR_HANDLE*);
typedef BOOL (WINAPI *Decompress_t)(DECOMPRESSOR_HANDLE, const void*, SIZE_T, void*, SIZE_T, SIZE_T*);
typedef void (WINAPI *CloseDecompressor_t)(DECOMPRESSOR_HANDLE);

#define COMPRESS_ALGORITHM_DEFLATE 2
#define COMPRESSION_FORMAT_DEFLATED 15

static CreateDecompressor_t g_CreateDecompressor = NULL;
static Decompress_t g_Decompress = NULL;
static CloseDecompressor_t g_CloseDecompressor = NULL;
static BOOL g_win8DecompressInit = FALSE;
static BOOL g_win8DecompressAvail = FALSE;

// ========== zlib (for Deflate) ==========
// z_stream structure for zlib inflate API
typedef struct {
    const unsigned char *next_in;
    unsigned int avail_in;
    unsigned long total_in;
    unsigned char *next_out;
    unsigned int avail_out;
    unsigned long total_out;
    char *msg;
    void *state;
    void *zalloc;
    void *zfree;
    void *opaque;
    int data_type;
    unsigned long adler;
    unsigned long reserved;
} z_stream;

typedef int (*zlib_inflateInit2__t)(z_stream*, int, const char*, int);
typedef int (*zlib_inflate_t)(z_stream*, int);
typedef int (*zlib_inflateEnd_t)(z_stream*);
typedef const char* (*zlib_zlibVersion_t)(void);

static zlib_inflateInit2__t g_inflateInit2_ = NULL;
static zlib_inflate_t g_inflate = NULL;
static zlib_inflateEnd_t g_inflateEnd = NULL;
static zlib_zlibVersion_t g_zlibVersion = NULL;
static HMODULE g_hZlib = NULL;
static BOOL g_zlibInit = FALSE;
static BOOL g_zlibAvail = FALSE;

#define Z_OK 0
#define Z_STREAM_END 1
#define Z_FINISH 4
#define ZLIB_VERSION_LEN 32

static BOOL InitZlib() {
    if (g_zlibInit) return g_zlibAvail;
    g_zlibInit = TRUE;
    
    const wchar_t* paths[] = {
        L"C:\\Users\\G11\\AppData\\Local\\F3DThumbnailProvider\\zlib1.dll",
        L"zlib1.dll",
        NULL
    };
    for (int i = 0; paths[i]; i++) {
        g_hZlib = LoadLibraryW(paths[i]);
        if (g_hZlib) break;
    }
    if (!g_hZlib) {
        DebugLog("[InitZlib] FAIL: zlib1.dll not found");
        return FALSE;
    }
    
    g_inflateInit2_ = (zlib_inflateInit2__t)GetProcAddress(g_hZlib, "inflateInit2_");
    g_inflate = (zlib_inflate_t)GetProcAddress(g_hZlib, "inflate");
    g_inflateEnd = (zlib_inflateEnd_t)GetProcAddress(g_hZlib, "inflateEnd");
    g_zlibVersion = (zlib_zlibVersion_t)GetProcAddress(g_hZlib, "zlibVersion");
    
    if (!g_inflateInit2_ || !g_inflate || !g_inflateEnd) {
        DebugLog("[InitZlib] FAIL: missing zlib functions");
        return FALSE;
    }
    
    if (g_zlibVersion) {
        const char* ver = g_zlibVersion();
        DebugLogFmt("[InitZlib] zlib version: %s", ver);
    }
    g_zlibAvail = TRUE;
    return TRUE;
}

static unsigned char* DecompressDeflateZlib(const unsigned char* compData, uint32_t compSize, uint32_t uncompSize, uint32_t* outSize) {
    if (!InitZlib()) return NULL;
    
    // Try both raw deflate (windowBits=-15) and zlib-wrapped (windowBits=15)
    // Also try with 2-byte skip in case there's a custom header
    int windowBitsOptions[] = { -15, 15 };  // raw deflate first, then zlib
    int skipOptions[] = { 0, 2 };  // no skip first, then 2-byte skip
    
    for (int wi = 0; wi < 2; wi++) {
        for (int si = 0; si < 2; si++) {
            int skip = skipOptions[si];
            if ((uint32_t)skip >= compSize) continue;
            
            uint32_t dataLen = compSize - skip;
            const unsigned char* dataPtr = compData + skip;
            
            uint32_t bufSize = uncompSize;
            if (bufSize == 0) bufSize = compSize * 20;
            
            unsigned char* output = (unsigned char*)malloc(bufSize);
            if (!output) continue;
            
            z_stream stream;
            memset(&stream, 0, sizeof(stream));
            stream.next_in = (unsigned char*)dataPtr;
            stream.avail_in = dataLen;
            stream.next_out = output;
            stream.avail_out = bufSize;
            
            int ret = g_inflateInit2_(&stream, windowBitsOptions[wi], "1.3.1", sizeof(z_stream));
            if (ret != Z_OK) {
                DebugLogFmt("[DecompressDeflateZlib] inflateInit2_ failed wb=%d skip=%d ret=%d", windowBitsOptions[wi], skip, ret);
                free(output);
                continue;
            }
            
            ret = g_inflate(&stream, Z_FINISH);
            g_inflateEnd(&stream);
            
            if (ret == Z_STREAM_END || (ret == Z_OK && stream.total_out > 0)) {
                DebugLogFmt("[DecompressDeflateZlib] SUCCESS wb=%d skip=%d total_out=%lu", windowBitsOptions[wi], skip, stream.total_out);
                *outSize = (uint32_t)stream.total_out;
                return output;
            }
            
            DebugLogFmt("[DecompressDeflateZlib] inflate failed wb=%d skip=%d ret=%d total_out=%lu msg=%s",
                        windowBitsOptions[wi], skip, ret, stream.total_out, stream.msg ? stream.msg : "(null)");
            free(output);
        }
    }
    
    return NULL;
}

static BOOL InitRtlDecompress() {
    if (g_rtlInit) return g_RtlDecompressBuffer != NULL;
    g_rtlInit = TRUE;
    HMODULE hNtdll = GetModuleHandleW(L"ntdll.dll");
    if (!hNtdll) return FALSE;
    g_RtlDecompressBuffer = (RtlDecompressBuffer_t)GetProcAddress(hNtdll, "RtlDecompressBuffer");
    return g_RtlDecompressBuffer != NULL;
}

static BOOL InitWin8Decompress() {
    if (g_win8DecompressInit) return g_win8DecompressAvail;
    g_win8DecompressInit = TRUE;
    
    // Try kernel32.dll, kernelbase.dll, and API set DLL
    const wchar_t* dlls[] = { 
        L"kernel32.dll", 
        L"kernelbase.dll",
        L"api-ms-win-core-compression-l1-1-0.dll",
        NULL 
    };
    for (int i = 0; dlls[i]; i++) {
        HMODULE hMod = GetModuleHandleW(dlls[i]);
        if (!hMod) hMod = LoadLibraryW(dlls[i]);
        if (!hMod) continue;
        
        g_CreateDecompressor = (CreateDecompressor_t)GetProcAddress(hMod, "CreateDecompressor");
        g_Decompress = (Decompress_t)GetProcAddress(hMod, "Decompress");
        g_CloseDecompressor = (CloseDecompressor_t)GetProcAddress(hMod, "CloseDecompressor");
        
        DebugLogFmt("[InitWin8Decompress] Trying %S: CreateDecompressor=%p Decompress=%p CloseDecompressor=%p",
                    dlls[i], g_CreateDecompressor, g_Decompress, g_CloseDecompressor);
        
        if (g_CreateDecompressor && g_Decompress && g_CloseDecompressor) {
            g_win8DecompressAvail = TRUE;
            DebugLogFmt("[InitWin8Decompress] Found all functions in %S", dlls[i]);
            return TRUE;
        }
    }
    
    DebugLog("[InitWin8Decompress] FAIL: CreateDecompressor/Decompress not found in any DLL");
    return FALSE;
}

// ========== F3D Preview Extraction ==========

static unsigned char* DecompressZstd(const unsigned char* compData, uint32_t compSize, uint32_t uncompSize, uint32_t* outSize) {
    if (!InitZstd()) return NULL;
    
    if (uncompSize == 0 && g_ZSTD_getFrameContentSize) {
        unsigned long long contentSize = g_ZSTD_getFrameContentSize(compData, compSize);
        if (contentSize != 0xFFFFFFFFFFFFFFFFULL && contentSize != 0xFFFFFFFFFFFFFFFEULL) {
            uncompSize = (uint32_t)contentSize;
        }
        if (uncompSize == 0) uncompSize = compSize * 10;
    }
    
    unsigned char* output = (unsigned char*)malloc(uncompSize);
    if (!output) return NULL;
    
    unsigned long long result = g_ZSTD_decompress(output, uncompSize, compData, compSize);
    if (g_ZSTD_isError(result)) {
        free(output);
        return NULL;
    }
    
    *outSize = (uint32_t)result;
    return output;
}

static unsigned char* DecompressDeflate(const unsigned char* compData, uint32_t compSize, uint32_t uncompSize, uint32_t* outSize) {
    DebugLogFmt("[DecompressDeflate] ENTER compSize=%u uncompSize=%u firstBytes=%02X%02X%02X%02X",
                compSize, uncompSize, compData[0], compData[1], compData[2], compData[3]);
    
    // Method 0: zlib (preferred - most reliable)
    DebugLog("[DecompressDeflate] Trying zlib...");
    unsigned char* result = DecompressDeflateZlib(compData, compSize, uncompSize, outSize);
    if (result) {
        DebugLogFmt("[DecompressDeflate] zlib SUCCESS, size=%u", *outSize);
        return result;
    }
    DebugLog("[DecompressDeflate] zlib failed, trying fallbacks...");

    // Method 1: Windows 8+ CreateDecompressor/Decompress API
    if (InitWin8Decompress()) {
        for (int skip = 0; skip <= 2; skip += 2) {
            if ((uint32_t)skip >= compSize) break;
            uint32_t bufSize = uncompSize > 0 ? uncompSize : compSize * 20;
            for (int attempt = 0; attempt < 2; attempt++) {
                unsigned char* output = (unsigned char*)malloc(bufSize);
                if (!output) break;
                DECOMPRESSOR_HANDLE hDec = NULL;
                if (!g_CreateDecompressor(COMPRESS_ALGORITHM_DEFLATE, NULL, &hDec)) { free(output); break; }
                SIZE_T finalSize = 0;
                BOOL ok = g_Decompress(hDec, compData + skip, compSize - skip, output, bufSize, &finalSize);
                g_CloseDecompressor(hDec);
                if (ok && finalSize > 0) { *outSize = (uint32_t)finalSize; return output; }
                free(output); bufSize = compSize * 50;
            }
        }
    }

    // Method 2: RtlDecompressBuffer with COMPRESSION_FORMAT_DEFLATED (15)
    if (InitRtlDecompress()) {
        for (int skip = 0; skip <= 2; skip += 2) {
            if ((uint32_t)skip >= compSize) break;
            uint32_t bufSize = uncompSize > 0 ? uncompSize : compSize * 20;
            for (int attempt = 0; attempt < 2; attempt++) {
                unsigned char* output = (unsigned char*)malloc(bufSize);
                if (!output) break;
                ULONG finalSize = 0;
                NTSTATUS status = g_RtlDecompressBuffer(COMPRESSION_FORMAT_DEFLATED,
                    output, bufSize, (PUCHAR)(compData + skip), compSize - skip, &finalSize);
                if (status == 0 && finalSize > 0) { *outSize = finalSize; return output; }
                free(output); bufSize = compSize * 50;
            }
        }
    }

    DebugLog("[DecompressDeflate] FAIL: all methods failed");
    return NULL;
}

// Extract preview PNG from a memory buffer (ZIP parsing)
static unsigned char* ExtractPreviewFromBuffer(unsigned char* data, uint32_t fileSize, uint32_t* outSize) {
    DebugLog("[ExtractPreview] ENTER buffer parsing");
    
    uint32_t offset = 0;
    unsigned char* result = NULL;
    int entryCount = 0;
    
    while (offset < fileSize - 30) {
        if (data[offset] != 0x50 || data[offset+1] != 0x4B ||
            data[offset+2] != 0x03 || data[offset+3] != 0x04) {
            offset++;
            continue;
        }
        
        uint16_t compMethod = *(uint16_t*)(data + offset + 8);
        uint32_t compSize = *(uint32_t*)(data + offset + 18);
        uint32_t uncompSize = *(uint32_t*)(data + offset + 22);
        uint16_t nameLen = *(uint16_t*)(data + offset + 26);
        uint16_t extraLen = *(uint16_t*)(data + offset + 28);
        
        uint32_t dataOffset = offset + 30 + nameLen + extraLen;
        if (dataOffset + compSize > fileSize) { offset++; continue; }
        
        char name[1024];
        int copyLen = nameLen < 1023 ? nameLen : 1023;
        memcpy(name, data + offset + 30, copyLen);
        name[copyLen] = 0;
        
        // Check for "preview" (case-insensitive) and ".png"
        BOOL hasPreview = FALSE;
        BOOL hasPng = FALSE;
        int nameLenInt = (int)strlen(name);
        
        for (int i = 0; i <= nameLenInt - 7; i++) {
            if (_strnicmp(name + i, "preview", 7) == 0) {
                hasPreview = TRUE;
                break;
            }
        }
        
        if (nameLenInt >= 4 && _stricmp(name + nameLenInt - 4, ".png") == 0) {
            hasPng = TRUE;
        }
        
        DebugLogFmt("[ExtractPreview] Entry #%d: name='%s' method=%u compSize=%u uncompSize=%u preview=%d png=%d",
                    entryCount, name, compMethod, compSize, uncompSize, hasPreview, hasPng);
        
        if (hasPreview && hasPng && compSize > 0) {
            unsigned char* compData = data + dataOffset;
            unsigned char* pngData = NULL;
            uint32_t pngSize = 0;
            
            DebugLogFmt("[ExtractPreview] Found preview PNG! method=%u, trying decompress...", compMethod);
            
            if (compMethod == 93) {
                pngData = DecompressZstd(compData, compSize, uncompSize, &pngSize);
                DebugLogFmt("[ExtractPreview] Zstd result: pngData=%p pngSize=%u", pngData, pngSize);
            } else if (compMethod == 0) {
                pngData = (unsigned char*)malloc(compSize);
                if (pngData) { memcpy(pngData, compData, compSize); pngSize = compSize; }
                DebugLogFmt("[ExtractPreview] Store result: pngData=%p pngSize=%u", pngData, pngSize);
            } else if (compMethod == 8) {
                pngData = DecompressDeflate(compData, compSize, uncompSize, &pngSize);
                DebugLogFmt("[ExtractPreview] Deflate result: pngData=%p pngSize=%u", pngData, pngSize);
            } else {
                DebugLogFmt("[ExtractPreview] Unsupported compression method %u", compMethod);
            }
            
            if (pngData && pngSize > 8) {
                DebugLogFmt("[ExtractPreview] Decompressed first 8 bytes: %02X%02X%02X%02X%02X%02X%02X%02X",
                            pngData[0],pngData[1],pngData[2],pngData[3],pngData[4],pngData[5],pngData[6],pngData[7]);
                if (pngData[0] == 0x89 && pngData[1] == 0x50 &&
                    pngData[2] == 0x4E && pngData[3] == 0x47) {
                    DebugLogFmt("[ExtractPreview] PNG signature CONFIRMED! size=%u", pngSize);
                    *outSize = pngSize;
                    result = pngData;
                    break;
                } else {
                    DebugLog("[ExtractPreview] NOT a PNG signature");
                }
            } else if (pngData) {
                DebugLogFmt("[ExtractPreview] Data too small: %u bytes", pngSize);
            } else {
                DebugLog("[ExtractPreview] Decompression returned NULL");
            }
            if (pngData) free(pngData);
        }
        
        entryCount++;
        offset = dataOffset + compSize;
    }
    
    DebugLogFmt("[ExtractPreview] Parsed %d entries, result=%p", entryCount, result);
    return result;
}

static unsigned char* ExtractPreviewFromF3D(const wchar_t* filePath, uint32_t* outSize) {
    DebugLog("[ExtractPreview] ENTER file path mode");
    
    HANDLE hFile = CreateFileW(filePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        DebugLogFmt("[ExtractPreview] FAIL: CreateFileW error=%lu", GetLastError());
        return NULL;
    }
    DebugLog("[ExtractPreview] File opened OK");
    
    LARGE_INTEGER fileSize;
    fileSize.QuadPart = 0;
    DWORD lowPart = GetFileSize(hFile, (LPDWORD)&fileSize.HighPart);
    if (lowPart == INVALID_FILE_SIZE && GetLastError() != NO_ERROR) {
        DebugLogFmt("[ExtractPreview] FAIL: GetFileSize error=%lu", GetLastError());
        CloseHandle(hFile);
        return NULL;
    }
    fileSize.LowPart = lowPart;
    DebugLogFmt("[ExtractPreview] File size: %lld bytes", fileSize.QuadPart);
    
    if (fileSize.QuadPart > 100 * 1024 * 1024) {
        DebugLog("[ExtractPreview] FAIL: File too large (>100MB)");
        CloseHandle(hFile);
        return NULL;
    }
    
    DWORD fileSize32 = (DWORD)fileSize.QuadPart;
    unsigned char* data = (unsigned char*)malloc(fileSize32);
    if (!data) { DebugLog("[ExtractPreview] FAIL: malloc failed"); CloseHandle(hFile); return NULL; }
    
    DWORD bytesRead = 0;
    if (!ReadFile(hFile, data, fileSize32, &bytesRead, NULL) || bytesRead != fileSize32) {
        DebugLogFmt("[ExtractPreview] FAIL: ReadFile bytesRead=%lu expected=%u", bytesRead, fileSize32);
        free(data);
        CloseHandle(hFile);
        return NULL;
    }
    CloseHandle(hFile);
    DebugLog("[ExtractPreview] File read OK, parsing ZIP entries...");
    
    unsigned char* result = ExtractPreviewFromBuffer(data, fileSize32, outSize);
    free(data);
    return result;
}

// ========== COM Object Implementation ==========

// Object structure - IThumbnailProvider is the primary interface (first vtable)
// IInitializeWithFile is the secondary interface (second vtable)
// IInitializeWithStream is the tertiary interface (third vtable)
typedef struct {
    const IThumbnailProviderVtbl* lpThumbVtbl;
    const IInitializeWithFileVtbl* lpInitVtbl;
    const IInitializeWithStreamVtbl* lpStreamVtbl;
    LONG refCount;
    wchar_t filePath[MAX_PATH];
    // Stream data (used when initialized via IInitializeWithStream)
    unsigned char* streamData;
    uint32_t streamSize;
} F3DThumbnailProvider;

// Forward declarations
static HRESULT STDMETHODCALLTYPE Thumbnail_QueryInterface(IThumbnailProvider*, REFIID, void**);
static ULONG STDMETHODCALLTYPE Thumbnail_AddRef(IThumbnailProvider*);
static ULONG STDMETHODCALLTYPE Thumbnail_Release(IThumbnailProvider*);
static HRESULT STDMETHODCALLTYPE Thumbnail_GetThumbnail(IThumbnailProvider*, UINT, HBITMAP*, WTS_ALPHATYPE*);

static HRESULT STDMETHODCALLTYPE Init_QueryInterface(IInitializeWithFile*, REFIID, void**);
static ULONG STDMETHODCALLTYPE Init_AddRef(IInitializeWithFile*);
static ULONG STDMETHODCALLTYPE Init_Release(IInitializeWithFile*);
static HRESULT STDMETHODCALLTYPE Init_Initialize(IInitializeWithFile*, LPCWSTR, DWORD);

// IInitializeWithStream methods
static HRESULT STDMETHODCALLTYPE Stream_QueryInterface(IInitializeWithStream*, REFIID, void**);
static ULONG STDMETHODCALLTYPE Stream_AddRef(IInitializeWithStream*);
static ULONG STDMETHODCALLTYPE Stream_Release(IInitializeWithStream*);
static HRESULT STDMETHODCALLTYPE Stream_Initialize(IInitializeWithStream*, IStream*, DWORD);

// Vtables
static const IThumbnailProviderVtbl g_ThumbnailVtbl = {
    Thumbnail_QueryInterface, Thumbnail_AddRef, Thumbnail_Release, Thumbnail_GetThumbnail
};

static const IInitializeWithFileVtbl g_InitVtbl = {
    Init_QueryInterface, Init_AddRef, Init_Release, Init_Initialize
};

static const IInitializeWithStreamVtbl g_StreamVtbl = {
    Stream_QueryInterface, Stream_AddRef, Stream_Release, Stream_Initialize
};

// Helper: get object from IInitializeWithFile pointer
// Since IInitializeWithFile* points to lpInitVtbl (second member), subtract offset
#define INIT_THIS(p) ((F3DThumbnailProvider*)((char*)(p) - offsetof(F3DThumbnailProvider, lpInitVtbl)))
#define STREAM_THIS(p) ((F3DThumbnailProvider*)((char*)(p) - offsetof(F3DThumbnailProvider, lpStreamVtbl)))
#define THUMB_THIS(p) ((F3DThumbnailProvider*)(p))

// IThumbnailProvider methods
static HRESULT STDMETHODCALLTYPE Thumbnail_QueryInterface(IThumbnailProvider* This, REFIID riid, void** ppv) {
    F3DThumbnailProvider* obj = THUMB_THIS(This);
    if (!ppv) return E_POINTER;
    
    if (IsEqualGUID_(riid, &IID_IUnknown) || IsEqualGUID_(riid, &IID_IThumbnailProvider)) {
        *ppv = &obj->lpThumbVtbl;
    } else if (IsEqualGUID_(riid, &IID_IInitializeWithFile)) {
        *ppv = &obj->lpInitVtbl;
    } else if (IsEqualGUID_(riid, &IID_IInitializeWithStream)) {
        *ppv = &obj->lpStreamVtbl;
    } else {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
    InterlockedIncrement(&obj->refCount);
    return S_OK;
}

static ULONG STDMETHODCALLTYPE Thumbnail_AddRef(IThumbnailProvider* This) {
    return InterlockedIncrement(&THUMB_THIS(This)->refCount);
}

static ULONG STDMETHODCALLTYPE Thumbnail_Release(IThumbnailProvider* This) {
    F3DThumbnailProvider* obj = THUMB_THIS(This);
    LONG count = InterlockedDecrement(&obj->refCount);
    if (count == 0) {
        if (obj->streamData) free(obj->streamData);
        free(obj);
    }
    return count;
}

static HRESULT STDMETHODCALLTYPE Thumbnail_GetThumbnail(IThumbnailProvider* This, UINT cx, HBITMAP* phbmp, WTS_ALPHATYPE* pdwAlpha) {
    F3DThumbnailProvider* obj = THUMB_THIS(This);
    
    // Debug logging
    #define LOG(msg) do { \
        HANDLE hLog = CreateFileW(L"C:\\Users\\G11\\AppData\\Local\\Temp\\f3d_thumb.log", \
            FILE_APPEND_DATA, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL); \
        if (hLog != INVALID_HANDLE_VALUE) { \
            DWORD written; WriteFile(hLog, msg, strlen(msg), &written, NULL); \
            WriteFile(hLog, "\r\n", 2, &written, NULL); CloseHandle(hLog); \
        } \
    } while(0)
    
    LOG("[GetThumbnail] ENTER");
    
    if (!phbmp || !pdwAlpha) { LOG("[GetThumbnail] FAIL: bad pointer"); return E_POINTER; }
    *phbmp = NULL;
    *pdwAlpha = WTSAT_UNKNOWN;
    
    if (obj->filePath[0] == 0 && !obj->streamData) { LOG("[GetThumbnail] FAIL: no file path and no stream data"); return E_FAIL; }
    if (obj->filePath[0] != 0) { LOG("[GetThumbnail] file path set"); }
    if (obj->streamData) { LOG("[GetThumbnail] stream data available"); }
    
    // Initialize GDI+
    if (!InitGdiplus()) { LOG("[GetThumbnail] FAIL: InitGdiplus failed"); return E_FAIL; }
    LOG("[GetThumbnail] GDI+ init OK");
    
    if (!InitOle32()) { LOG("[GetThumbnail] FAIL: InitOle32 failed"); return E_FAIL; }
    LOG("[GetThumbnail] OLE32 init OK");
    
    // Extract preview PNG - use stream data if available, otherwise read from file
    uint32_t pngSize = 0;
    unsigned char* pngData = NULL;
    if (obj->streamData && obj->streamSize > 0) {
        LOG("[GetThumbnail] Using stream data for extraction");
        pngData = ExtractPreviewFromBuffer(obj->streamData, obj->streamSize, &pngSize);
    } else {
        LOG("[GetThumbnail] Using file path for extraction");
        pngData = ExtractPreviewFromF3D(obj->filePath, &pngSize);
    }
    if (!pngData || pngSize == 0) { 
        LOG("[GetThumbnail] FAIL: ExtractPreview returned null");
        if (pngData) free(pngData); 
        return E_FAIL; 
    }
    { char buf[128]; sprintf(buf, "[GetThumbnail] PNG extracted: %u bytes, sig=%02X%02X%02X%02X", pngSize, pngData[0],pngData[1],pngData[2],pngData[3]); LOG(buf); }
    
    // Create IStream from memory
    HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, pngSize);
    if (!hGlobal) { LOG("[GetThumbnail] FAIL: GlobalAlloc failed"); free(pngData); return E_FAIL; }
    
    void* pGlobal = GlobalLock(hGlobal);
    memcpy(pGlobal, pngData, pngSize);
    GlobalUnlock(hGlobal);
    free(pngData);
    
    IStream* pStream = NULL;
    if (FAILED(g_CreateStreamOnHGlobal(hGlobal, TRUE, &pStream))) {
        LOG("[GetThumbnail] FAIL: CreateStreamOnHGlobal failed");
        GlobalFree(hGlobal);
        return E_FAIL;
    }
    LOG("[GetThumbnail] IStream created");
    
    // Load bitmap
    GpBitmap* pBitmap = NULL;
    int gdipStatus = g_GdipCreateBitmapFromStream(pStream, &pBitmap);
    if (gdipStatus != 0) {
        { char buf[128]; sprintf(buf, "[GetThumbnail] FAIL: GdipCreateBitmapFromStream status=%d", gdipStatus); LOG(buf); }
        pStream->lpVtbl->Release(pStream);
        return E_FAIL;
    }
    pStream->lpVtbl->Release(pStream);
    LOG("[GetThumbnail] Bitmap loaded from stream");
    
    // Get dimensions
    UINT srcW = 0, srcH = 0;
    g_GdipGetImageWidth((GpImage*)pBitmap, &srcW);
    g_GdipGetImageHeight((GpImage*)pBitmap, &srcH);
    { char buf[128]; sprintf(buf, "[GetThumbnail] Dimensions: %ux%u", srcW, srcH); LOG(buf); }
    if (srcW == 0 || srcH == 0) { LOG("[GetThumbnail] FAIL: zero dimensions"); g_GdipDisposeImage((GpImage*)pBitmap); return E_FAIL; }
    
    // Calculate thumbnail size
    UINT dstW, dstH;
    if (srcW >= srcH) {
        dstW = cx;
        dstH = (UINT)((double)srcH / srcW * cx);
    } else {
        dstH = cx;
        dstW = (UINT)((double)srcW / srcH * cx);
    }
    if (dstW < 1) dstW = 1;
    if (dstH < 1) dstH = 1;
    { char buf[128]; sprintf(buf, "[GetThumbnail] Thumb size: %ux%u", dstW, dstH); LOG(buf); }
    
    // Create scaled bitmap
    GpBitmap* pScaled = NULL;
    int scaleStatus = g_GdipCreateBitmapFromScan0((INT)dstW, (INT)dstH, 0, PixelFormat32bppARGB, NULL, &pScaled);
    if (!pScaled) { 
        { char buf[128]; sprintf(buf, "[GetThumbnail] FAIL: GdipCreateBitmapFromScan0 status=%d", scaleStatus); LOG(buf); }
        g_GdipDisposeImage((GpImage*)pBitmap); 
        return E_FAIL; 
    }
    LOG("[GetThumbnail] Scaled bitmap created");
    
    GpGraphics* pGraphics = NULL;
    g_GdipGetImageGraphicsContext((GpImage*)pScaled, &pGraphics);
    g_GdipSetInterpolationMode(pGraphics, InterpolationModeHighQualityBicubic);
    g_GdipDrawImageRectI(pGraphics, (GpImage*)pBitmap, 0, 0, (INT)dstW, (INT)dstH);
    g_GdipDeleteGraphics(pGraphics);
    g_GdipDisposeImage((GpImage*)pBitmap);
    LOG("[GetThumbnail] Image drawn");
    
    // Convert to HBITMAP - use white background (0xFFFFFFFF) for transparency
    HBITMAP hBmp = NULL;
    int hbStatus = g_GdipCreateHBITMAPFromBitmap(pScaled, &hBmp, 0xFFFFFFFF);
    g_GdipDisposeImage((GpImage*)pScaled);
    
    { char buf[128]; sprintf(buf, "[GetThumbnail] GdipCreateHBITMAPFromBitmap status=%d hBmp=%p", hbStatus, hBmp); LOG(buf); }
    
    if (!hBmp || hbStatus != 0) { 
        { char buf[128]; sprintf(buf, "[GetThumbnail] FAIL: GdipCreateHBITMAPFromBitmap status=%d hBmp=%p", hbStatus, hBmp); LOG(buf); }
        return E_FAIL; 
    }
    
    // Verify bitmap is valid
    BITMAP bmInfo;
    if (GetObject(hBmp, sizeof(bmInfo), &bmInfo)) {
        char buf[128];
        sprintf(buf, "[GetThumbnail] HBITMAP: %dx%d %dbpp planes=%d", bmInfo.bmWidth, bmInfo.bmHeight, bmInfo.bmBitsPixel, bmInfo.bmPlanes);
        LOG(buf);
    }
    
    *phbmp = hBmp;
    *pdwAlpha = WTSAT_RGB;
    LOG("[GetThumbnail] Returning S_OK with WTSAT_RGB");
    return S_OK;
}

// IInitializeWithFile methods
static HRESULT STDMETHODCALLTYPE Init_QueryInterface(IInitializeWithFile* This, REFIID riid, void** ppv) {
    F3DThumbnailProvider* obj = INIT_THIS(This);
    return Thumbnail_QueryInterface((IThumbnailProvider*)&obj->lpThumbVtbl, riid, ppv);
}

static ULONG STDMETHODCALLTYPE Init_AddRef(IInitializeWithFile* This) {
    return InterlockedIncrement(&INIT_THIS(This)->refCount);
}

static ULONG STDMETHODCALLTYPE Init_Release(IInitializeWithFile* This) {
    F3DThumbnailProvider* obj = INIT_THIS(This);
    LONG count = InterlockedDecrement(&obj->refCount);
    if (count == 0) free(obj);
    return count;
}

static HRESULT STDMETHODCALLTYPE Init_Initialize(IInitializeWithFile* This, LPCWSTR pszFilePath, DWORD grfMode) {
    F3DThumbnailProvider* obj = INIT_THIS(This);
    if (!pszFilePath) return E_INVALIDARG;
    wcsncpy(obj->filePath, pszFilePath, MAX_PATH - 1);
    obj->filePath[MAX_PATH - 1] = 0;
    DebugLog("[Init_Initialize] File path set via IInitializeWithFile");
    return S_OK;
}

// ========== IInitializeWithStream Implementation ==========

static HRESULT STDMETHODCALLTYPE Stream_QueryInterface(IInitializeWithStream* This, REFIID riid, void** ppv) {
    F3DThumbnailProvider* obj = STREAM_THIS(This);
    return Thumbnail_QueryInterface((IThumbnailProvider*)&obj->lpThumbVtbl, riid, ppv);
}

static ULONG STDMETHODCALLTYPE Stream_AddRef(IInitializeWithStream* This) {
    return InterlockedIncrement(&STREAM_THIS(This)->refCount);
}

static ULONG STDMETHODCALLTYPE Stream_Release(IInitializeWithStream* This) {
    F3DThumbnailProvider* obj = STREAM_THIS(This);
    LONG count = InterlockedDecrement(&obj->refCount);
    if (count == 0) {
        if (obj->streamData) free(obj->streamData);
        free(obj);
    }
    return count;
}

static HRESULT STDMETHODCALLTYPE Stream_Initialize(IInitializeWithStream* This, IStream* pStream, DWORD grfMode) {
    F3DThumbnailProvider* obj = STREAM_THIS(This);
    if (!pStream) return E_INVALIDARG;
    
    DebugLog("[Stream_Initialize] ENTER");
    
    // Read the entire stream into memory
    // First, get the stream size by seeking to end
    LARGE_INTEGER zero;
    zero.QuadPart = 0;
    ULARGE_INTEGER streamPos;
    HRESULT hr = pStream->lpVtbl->Seek(pStream, zero, 2, &streamPos); // STREAM_SEEK_END = 2
    if (hr != S_OK) {
        DebugLogFmt("[Stream_Initialize] FAIL: Seek to end failed hr=0x%08X", (unsigned)hr);
        return E_FAIL;
    }
    
    uint32_t streamSize = (uint32_t)streamPos.QuadPart;
    DebugLogFmt("[Stream_Initialize] Stream size: %u bytes", streamSize);
    
    if (streamSize == 0 || streamSize > 100 * 1024 * 1024) {
        DebugLog("[Stream_Initialize] FAIL: Invalid stream size");
        return E_FAIL;
    }
    
    // Seek back to beginning
    hr = pStream->lpVtbl->Seek(pStream, zero, 0, NULL); // STREAM_SEEK_SET = 0
    if (hr != S_OK) {
        DebugLogFmt("[Stream_Initialize] FAIL: Seek to beginning failed hr=0x%08X", (unsigned)hr);
        return E_FAIL;
    }
    
    // Allocate buffer and read
    unsigned char* data = (unsigned char*)malloc(streamSize);
    if (!data) {
        DebugLog("[Stream_Initialize] FAIL: malloc failed");
        return E_OUTOFMEMORY;
    }
    
    uint32_t totalRead = 0;
    while (totalRead < streamSize) {
        ULONG bytesRead = 0;
        hr = pStream->lpVtbl->Read(pStream, data + totalRead, streamSize - totalRead, &bytesRead);
        if (hr != S_OK || bytesRead == 0) {
            DebugLogFmt("[Stream_Initialize] FAIL: Read failed hr=0x%08X bytesRead=%lu totalRead=%u", (unsigned)hr, bytesRead, totalRead);
            free(data);
            return E_FAIL;
        }
        totalRead += bytesRead;
    }
    
    DebugLogFmt("[Stream_Initialize] Read %u bytes successfully", totalRead);
    
    // Save the buffer in the object
    obj->streamData = data;
    obj->streamSize = totalRead;
    
    DebugLog("[Stream_Initialize] OK");
    return S_OK;
}

// ========== Class Factory ==========

typedef struct {
    const IClassFactoryVtbl* lpVtbl;
    LONG refCount;
} F3DClassFactory;

static HRESULT STDMETHODCALLTYPE Factory_QueryInterface(IClassFactory* This, REFIID riid, void** ppv) {
    if (!ppv) return E_POINTER;
    if (IsEqualGUID_(riid, &IID_IUnknown) || IsEqualGUID_(riid, &IID_IClassFactory)) {
        *ppv = This;
        InterlockedIncrement(&((F3DClassFactory*)This)->refCount);
        return S_OK;
    }
    *ppv = NULL;
    return E_NOINTERFACE;
}

static ULONG STDMETHODCALLTYPE Factory_AddRef(IClassFactory* This) {
    return InterlockedIncrement(&((F3DClassFactory*)This)->refCount);
}

static ULONG STDMETHODCALLTYPE Factory_Release(IClassFactory* This) {
    F3DClassFactory* obj = (F3DClassFactory*)This;
    LONG count = InterlockedDecrement(&obj->refCount);
    if (count == 0) free(obj);
    return count;
}

static HRESULT STDMETHODCALLTYPE Factory_CreateInstance(IClassFactory* This, IUnknown* pUnkOuter, REFIID riid, void** ppv) {
    if (!ppv) return E_POINTER;
    if (pUnkOuter) return CLASS_E_NOAGGREGATION;
    
    F3DThumbnailProvider* pProvider = (F3DThumbnailProvider*)calloc(1, sizeof(F3DThumbnailProvider));
    if (!pProvider) return E_OUTOFMEMORY;
    
    pProvider->lpThumbVtbl = &g_ThumbnailVtbl;
    pProvider->lpInitVtbl = &g_InitVtbl;
    pProvider->lpStreamVtbl = &g_StreamVtbl;
    pProvider->refCount = 1;
    pProvider->streamData = NULL;
    pProvider->streamSize = 0;
    pProvider->filePath[0] = 0;
    
    HRESULT hr = Thumbnail_QueryInterface((IThumbnailProvider*)&pProvider->lpThumbVtbl, riid, ppv);
    Thumbnail_Release((IThumbnailProvider*)&pProvider->lpThumbVtbl);
    return hr;
}

static HRESULT STDMETHODCALLTYPE Factory_LockServer(IClassFactory* This, BOOL fLock) {
    return S_OK;
}

static const IClassFactoryVtbl g_FactoryVtbl = {
    Factory_QueryInterface, Factory_AddRef, Factory_Release,
    Factory_CreateInstance, Factory_LockServer
};

// ========== DLL Exports ==========

HINSTANCE g_hInstance = NULL;

__declspec(dllexport) HRESULT __stdcall DllGetClassObject(REFCLSID rclsid, REFIID riid, void** ppv) {
    if (!ppv) return E_POINTER;
    if (!IsEqualGUID_(rclsid, &CLSID_F3DThumbnailProvider)) return CLASS_E_CLASSNOTAVAILABLE;
    
    F3DClassFactory* pFactory = (F3DClassFactory*)calloc(1, sizeof(F3DClassFactory));
    if (!pFactory) return E_OUTOFMEMORY;
    
    pFactory->lpVtbl = &g_FactoryVtbl;
    pFactory->refCount = 1;
    
    HRESULT hr = Factory_QueryInterface((IClassFactory*)pFactory, riid, ppv);
    Factory_Release((IClassFactory*)pFactory);
    return hr;
}

__declspec(dllexport) HRESULT __stdcall DllCanUnloadNow() {
    return S_OK;
}

// ========== DLL Main ==========

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved) {
    switch (dwReason) {
    case DLL_PROCESS_ATTACH:
        g_hInstance = hModule;
        DisableThreadLibraryCalls(hModule);
        break;
    case DLL_PROCESS_DETACH:
        if (g_hZstd) FreeLibrary(g_hZstd);
        if (g_gdiplusToken && g_GdiplusShutdown) g_GdiplusShutdown(g_gdiplusToken);
        if (g_hGdiplus) FreeLibrary(g_hGdiplus);
        break;
    }
    return TRUE;
}
