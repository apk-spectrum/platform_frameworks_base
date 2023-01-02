#ifndef __NATIVE_ASSETMANAGER_H
#define __NATIVE_ASSETMANAGER_H

#include <stdio.h>
#include <stdint.h>
#include <utils/Vector.h>
#include <utils/String8.h>

#include <utils/Errors.h>

using namespace android;

#ifndef SPECTRUM_DLL_IMPORT

#ifdef SPECTRUM_NATIVE_DLL
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT extern
#endif

#ifdef __cplusplus
extern "C" {
#endif

DLLEXPORT void setAppender(void (*appender)(char*));

DLLEXPORT void nativeInit();

DLLEXPORT void nativeRealease();

DLLEXPORT void nativeFree(void* p);

DLLEXPORT int64_t createAssetManager();

DLLEXPORT void realeaseAssetManager(int64_t handle);

DLLEXPORT int getPackageId(const char* path);

DLLEXPORT bool addPackage(int64_t handle, const char* path);

DLLEXPORT bool addResPackage(int64_t handle, const char* path);

DLLEXPORT const char* getResourceName(int64_t handle, uint32_t resId);

DLLEXPORT const char* getResourceType(int64_t handle, uint32_t resId);

DLLEXPORT ssize_t getResourceValues(int64_t handle, uint32_t resId
        , Vector<String8>** outValues, Vector<String8>** outConfigs
        , char** outName);

#ifdef __cplusplus
}
#endif

#else // !SPECTRUM_DLL_IMPORT

extern void nativeInit();

extern void nativeRealease();

typedef void (*nativeFreeFunc)(void* p);

typedef int (*mainFunc)(int, char* const []);

typedef void (*appenderFunc)(char*);

typedef void (*setAppenderFunc)(appenderFunc);

typedef int64_t (*createAssetManagerFunc)();

typedef void (*realeaseAssetManagerFunc)(int64_t handle);

typedef int (*getPackageIdFunc)(const char* path);

typedef bool (*addPackageFunc)(int64_t handle, const char* path);

typedef const char* (*getResourceFunc)(int64_t handle, uint32_t resId);

typedef ssize_t (*getResourceValuesFunc)(int64_t handle, uint32_t resId
        , Vector<String8>** outValues, Vector<String8>** outConfigs
        , char** outName);

extern nativeFreeFunc nativeFree;
extern mainFunc nativeMain;
extern createAssetManagerFunc createAssetManager;
extern realeaseAssetManagerFunc realeaseAssetManager;
extern getPackageIdFunc getPackageId;
extern addPackageFunc addPackage;
extern addPackageFunc addResPackage;
extern getResourceFunc getResourceName;
extern getResourceFunc getResourceType;
extern getResourceValuesFunc getResourceValues;

#endif // SPECTRUM_DLL_IMPORT
#endif // __NATIVE_ASSETMANAGER_H