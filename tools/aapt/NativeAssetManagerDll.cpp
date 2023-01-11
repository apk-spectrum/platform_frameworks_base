#include "NativeAssetManager.h"

#include <stdio.h>

#include <windows.h>

#include "OutLineBuffer.h"

HINSTANCE hDllInst = NULL;

appenderFunc appendStringStream = NULL;
mainFunc nativeMain = NULL;

nativeFreeFunc nativeFree = NULL;
createAssetManagerFunc createAssetManager = NULL;
realeaseAssetManagerFunc realeaseAssetManager = NULL;
getPackageIdFunc getPackageId = NULL;
addPackageFunc addPackage = NULL;
addPackageFunc addResPackage = NULL;
getResourceFunc getResourceName = NULL;
getResourceFunc getResourceType = NULL;
getResourceValuesFunc getResourceValues = NULL;

int main(int argc, char* const argv[]) {
    if (nativeMain == NULL) nativeInit();
    if (nativeMain == NULL) {
        fprintf(stderr, "Unable to find main function.\n");
        return 1;
    }
    return nativeMain(argc, argv);
}

void nativeInit() {
    if (hDllInst != NULL) {
        fprintf(stderr, "Already to load native dll %p \n", hDllInst);
        return;
    }

    int ret = NO_ERROR;
    char path[MAX_PATH];
    HMODULE hm = NULL;

    if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | 
            GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            (LPCSTR) &nativeInit, &hm) == 0) {
        ret = GetLastError();
        fprintf(stderr, "GetModuleHandle failed, error = %d\n", ret);
    }

    if (ret == NO_ERROR && GetModuleFileName(hm, path, sizeof(path)) == 0) {
        ret = GetLastError();
        fprintf(stderr, "GetModuleFileName failed, error = %d\n", ret);
    }

    char* file = path;
    if (ret == NO_ERROR) {
        int i;
        for (i = 0; i < MAX_PATH && path[i] != '\0'; ++i) {
            if (path[i] == '\\') file = path + i + 1;
        }
        if (i == MAX_PATH - 1 && strcmp(&path[MAX_PATH - 5], ".dll") != 0) {
            ret = NOT_ENOUGH_DATA;
            fprintf(stderr, "Failure : Not enough path buf.\n");
        } else {
            if (strncmp(file, "lib", 3) == 0) file += 3;
            if ((file - path) + 16 < MAX_PATH) {
                strcpy(file, "AaptNative32.dll");
            } else {
                ret = NOT_ENOUGH_DATA;
                fprintf(stderr, "Failure : Not enough path buf.\n");
            }
        }
    }

    if (ret != NO_ERROR) {
        strcpy(path, "tool\\windows\\AaptNative32.dll");
    }
    fprintf(stderr, "Native library path : %s, ret %d\n", path, ret);

    hDllInst = LoadLibrary(path);
    if (hDllInst == NULL) {
        fprintf(stderr, "Unable to load library. err : %lu \n", GetLastError());
        return;
    }

    setAppenderFunc setAppender = (setAppenderFunc) GetProcAddress(hDllInst, "setAppender");
    if (setAppender == NULL) {
        fprintf(stderr, "Unable to find setAppenderFunc function. err : %lu \n", GetLastError());
    } else {
        setAppender(&OutLineBuffer::appendStringStream);
    }

    nativeMain = (mainFunc) GetProcAddress(hDllInst, "main");
    if (nativeMain == NULL) {
        fprintf(stderr, "Unable to find main function. err : %lu \n", GetLastError());
    }

    nativeFree = (nativeFreeFunc) GetProcAddress(hDllInst, "nativeFree");
    if (nativeFree == NULL) {
        fprintf(stderr, "Unable to find nativeFree function. err : %lu \n", GetLastError());
    }

    createAssetManager = (createAssetManagerFunc) GetProcAddress(hDllInst, "createAssetManager");
    if (createAssetManager == NULL) {
        fprintf(stderr, "Unable to find createAssetManager function. err : %lu \n", GetLastError());
    }

    realeaseAssetManager = (realeaseAssetManagerFunc) GetProcAddress(hDllInst, "realeaseAssetManager");
    if (realeaseAssetManager == NULL) {
        fprintf(stderr, "Unable to find realeaseAssetManager function. err : %lu \n", GetLastError());
    }

    getPackageId = (getPackageIdFunc) GetProcAddress(hDllInst, "getPackageId");
    if (getPackageId == NULL) {
        fprintf(stderr, "Unable to find getPackageId function. err : %lu \n", GetLastError());
    }

    addPackage = (addPackageFunc) GetProcAddress(hDllInst, "addPackage");
    if (addPackage == NULL) {
        fprintf(stderr, "Unable to find addPackage function. err : %lu \n", GetLastError());
    }

    addResPackage = (addPackageFunc) GetProcAddress(hDllInst, "addResPackage");
    if (addResPackage == NULL) {
        fprintf(stderr, "Unable to find addResPackage function. err : %lu \n", GetLastError());
    }

    getResourceName = (getResourceFunc) GetProcAddress(hDllInst, "getResourceName");
    if (getResourceName == NULL) {
        fprintf(stderr, "Unable to find getResourceName function. err : %lu \n", GetLastError());
    }

    getResourceType = (getResourceFunc) GetProcAddress(hDllInst, "getResourceType");
    if (getResourceType == NULL) {
        fprintf(stderr, "Unable to find getResourceType function. err : %lu \n", GetLastError());
    }

    getResourceValues = (getResourceValuesFunc) GetProcAddress(hDllInst, "getResourceValues");
    if (getResourceValues == NULL) {
        fprintf(stderr, "Unable to find getResourceValues function. err : %lu \n", GetLastError());
    }
}

void nativeRealease() {
    if (hDllInst == NULL) return;
    FreeLibrary(hDllInst);
    hDllInst = NULL;
}