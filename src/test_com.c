// Test program to verify F3DThumbnailProviderNative.dll COM interface
// Compiled with TCC - standalone diagnostic tool

#define WIN32_LEAN_AND_MEAN
#define UNICODE
#define _UNICODE
#define COBJMACROS
#define INITGUID

#include <windows.h>
#include <stdint.h>
#include <stdio.h>

// Disable stdout buffering
static void out(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    fflush(stdout);
}

// Ensure __MIDL_CONST is defined
#ifndef __MIDL_CONST
#define __MIDL_CONST
#endif

// Basic COM types
#ifndef _HRESULT_DEFINED
typedef long HRESULT;
#endif
typedef unsigned long ULONG;

#define S_OK     ((HRESULT)0)
#define S_FALSE  ((HRESULT)1)
#define E_FAIL   ((HRESULT)0x80004005)
#define E_NOINTERFACE ((HRESULT)0x80004002)
#define E_POINTER ((HRESULT)0x80004003)
#define E_INVALIDARG ((HRESULT)0x80070057)

#ifndef STDMETHODCALLTYPE
#define STDMETHODCALLTYPE __stdcall
#endif

// GUIDs
static const GUID IID_IUnknown =
    { 0x00000000, 0x0000, 0x0000, { 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 } };
static const GUID IID_IClassFactory =
    { 0x00000001, 0x0000, 0x0000, { 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 } };
// Correct IIDs from Windows SDK (verified via HKCR\Interface registry)
static const GUID IID_IThumbnailProvider =
    { 0xe357fccd, 0xa995, 0x4576, { 0xb0, 0x1f, 0x23, 0x46, 0x30, 0x15, 0x4e, 0x96 } };
static const GUID IID_IInitializeWithFile =
    { 0xb7d14566, 0x0509, 0x4cce, { 0xa7, 0x1f, 0x0a, 0x55, 0x42, 0x33, 0xbd, 0x9b } };
static const GUID CLSID_F3DThumbnailProvider =
    { 0xc7a3f5d2, 0x8e4b, 0x4f6a, { 0x9d, 0x1e, 0x3b, 0x5c, 0x7f, 0x8a, 0x2e, 0x6d } };

static int IsEqualGUID_(const GUID* a, const GUID* b) {
    return memcmp(a, b, sizeof(GUID)) == 0;
}

// IUnknown
typedef struct IUnknown IUnknown;
typedef struct IUnknownVtbl {
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(IUnknown*, const GUID*, void**);
    ULONG (STDMETHODCALLTYPE *AddRef)(IUnknown*);
    ULONG (STDMETHODCALLTYPE *Release)(IUnknown*);
} IUnknownVtbl;
struct IUnknown { IUnknownVtbl* lpVtbl; };

// IClassFactory
typedef struct IClassFactory IClassFactory;
typedef struct IClassFactoryVtbl {
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(IClassFactory*, const GUID*, void**);
    ULONG (STDMETHODCALLTYPE *AddRef)(IClassFactory*);
    ULONG (STDMETHODCALLTYPE *Release)(IClassFactory*);
    HRESULT (STDMETHODCALLTYPE *CreateInstance)(IClassFactory*, IUnknown*, const GUID*, void**);
    HRESULT (STDMETHODCALLTYPE *LockServer)(IClassFactory*, BOOL);
} IClassFactoryVtbl;
struct IClassFactory { IClassFactoryVtbl* lpVtbl; };

// IInitializeWithFile
typedef struct IInitializeWithFile IInitializeWithFile;
typedef struct IInitializeWithFileVtbl {
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(IInitializeWithFile*, const GUID*, void**);
    ULONG (STDMETHODCALLTYPE *AddRef)(IInitializeWithFile*);
    ULONG (STDMETHODCALLTYPE *Release)(IInitializeWithFile*);
    HRESULT (STDMETHODCALLTYPE *Initialize)(IInitializeWithFile*, LPCWSTR, DWORD);
} IInitializeWithFileVtbl;
struct IInitializeWithFile { IInitializeWithFileVtbl* lpVtbl; };

// IThumbnailProvider
typedef enum { WTSAT_UNKNOWN = 0, WTSAT_RGB = 1, WTSAT_ARGB = 2 } WTS_ALPHATYPE;
typedef struct IThumbnailProvider IThumbnailProvider;
typedef struct IThumbnailProviderVtbl {
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(IThumbnailProvider*, const GUID*, void**);
    ULONG (STDMETHODCALLTYPE *AddRef)(IThumbnailProvider*);
    ULONG (STDMETHODCALLTYPE *Release)(IThumbnailProvider*);
    HRESULT (STDMETHODCALLTYPE *GetThumbnail)(IThumbnailProvider*, UINT, HBITMAP*, WTS_ALPHATYPE*);
} IThumbnailProviderVtbl;
struct IThumbnailProvider { IThumbnailProviderVtbl* lpVtbl; };

// DLL exports typedef
typedef HRESULT (__stdcall *DllGetClassObjectFunc)(const GUID*, const GUID*, void**);

int main() {
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
    // Resolve DLL path from %LOCALAPPDATA% (works for any user)
    wchar_t dllPath[MAX_PATH];
    DWORD nEnv = GetEnvironmentVariableW(L"LOCALAPPDATA", dllPath, MAX_PATH);
    if (nEnv == 0 || nEnv >= MAX_PATH - 60) {
        printf("FAILED: LOCALAPPDATA not set or too long\n");
        return 1;
    }
    wcscat(dllPath, L"\\F3DThumbnailProvider\\F3DThumbnailProviderNative.dll");
    const wchar_t* testFile = L"D:\\fusion360\\Installation\\Local\\Autodesk\\webdeploy\\production\\61bf25b220a2d0307c84c301e65a59ac225d9a1e\\Fusion\\Server\\Fusion\\Configuration\\Mechanical\\Screws\\FlatHex.f3d";

    printf("=== F3D Thumbnail Provider COM Test ===\n");

    // Step 1: Load DLL
    printf("[1] Loading DLL: %S\n", dllPath);
    HMODULE hDll = LoadLibraryW(dllPath);
    if (!hDll) {
        printf("FAILED: LoadLibrary error %lu\n", GetLastError());
        return 1;
    }
    printf("  OK: DLL loaded at %p\n", hDll);

    // Step 2: Get DllGetClassObject
    printf("[2] Getting DllGetClassObject...\n");
    DllGetClassObjectFunc pDllGetClassObject = (DllGetClassObjectFunc)GetProcAddress(hDll, "DllGetClassObject");
    if (!pDllGetClassObject) {
        printf("FAILED: DllGetClassObject not found! Error %lu\n", GetLastError());
        FreeLibrary(hDll);
        return 1;
    }
    printf("  OK: DllGetClassObject at %p\n", pDllGetClassObject);

    // Step 3: Get Class Factory
    printf("[3] Getting IClassFactory...\n");
    IClassFactory* pFactory = NULL;
    HRESULT hr = pDllGetClassObject(&CLSID_F3DThumbnailProvider, &IID_IClassFactory, (void**)&pFactory);
    if (hr != S_OK || !pFactory) {
        printf("FAILED: DllGetClassObject returned hr=0x%08X\n", (unsigned)hr);
        FreeLibrary(hDll);
        return 1;
    }
    printf("  OK: IClassFactory at %p\n", pFactory);

    // Step 4: Create Instance
    printf("[4] Creating IThumbnailProvider instance...\n");
    IThumbnailProvider* pThumb = NULL;
    hr = pFactory->lpVtbl->CreateInstance(pFactory, NULL, &IID_IThumbnailProvider, (void**)&pThumb);
    pFactory->lpVtbl->Release(pFactory);
    if (hr != S_OK || !pThumb) {
        printf("FAILED: CreateInstance returned hr=0x%08X\n", (unsigned)hr);
        FreeLibrary(hDll);
        return 1;
    }
    printf("  OK: IThumbnailProvider at %p\n", pThumb);

    // Step 5: QueryInterface for IInitializeWithFile
    printf("[5] Querying IInitializeWithFile...\n");
    IInitializeWithFile* pInit = NULL;
    hr = pThumb->lpVtbl->QueryInterface(pThumb, &IID_IInitializeWithFile, (void**)&pInit);
    if (hr != S_OK || !pInit) {
        printf("FAILED: QueryInterface(IInitializeWithFile) returned hr=0x%08X\n", (unsigned)hr);
        pThumb->lpVtbl->Release(pThumb);
        FreeLibrary(hDll);
        return 1;
    }
    printf("  OK: IInitializeWithFile at %p\n", pInit);

    // Step 6: Initialize with file
    printf("[6] Initializing with file: %S\n", testFile);
    hr = pInit->lpVtbl->Initialize(pInit, testFile, 0);
    pInit->lpVtbl->Release(pInit);
    if (hr != S_OK) {
        printf("FAILED: Initialize returned hr=0x%08X\n", (unsigned)hr);
        pThumb->lpVtbl->Release(pThumb);
        FreeLibrary(hDll);
        return 1;
    }
    printf("  OK: Initialize succeeded\n");

    // Step 7: GetThumbnail
    printf("[7] Getting thumbnail (256x256)...\n");
    HBITMAP hBmp = NULL;
    WTS_ALPHATYPE alphaType = WTSAT_UNKNOWN;
    hr = pThumb->lpVtbl->GetThumbnail(pThumb, 256, &hBmp, &alphaType);
    pThumb->lpVtbl->Release(pThumb);
    if (hr != S_OK || !hBmp) {
        printf("FAILED: GetThumbnail returned hr=0x%08X, hBmp=%p\n", (unsigned)hr, hBmp);
        FreeLibrary(hDll);
        return 1;
    }
    printf("  OK: Thumbnail created! hBmp=%p, alphaType=%d\n", hBmp, alphaType);

    // Verify bitmap
    BITMAP bm;
    if (GetObject(hBmp, sizeof(bm), &bm)) {
        printf("  Bitmap: %dx%d, %d bpp, %d planes\n", bm.bmWidth, bm.bmHeight, bm.bmBitsPixel, bm.bmPlanes);
    }
    DeleteObject(hBmp);

    printf("\n=== ALL TESTS PASSED ===\n");
    FreeLibrary(hDll);
    return 0;
}
