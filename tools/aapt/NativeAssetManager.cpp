#include "NativeAssetManager.h"

#include <stdio.h>

#include <androidfw/ResourceTypes.h>

#include "AaptXml.h"
#include "XMLNode.h"

extern void (*appendStringStream)(char*);
void setAppender(void (*appender)(char*)) {
    appendStringStream = appender;
}

void nativeInit() { }

void nativeRealease() { }

void nativeFree(void* p) {
    if (p != NULL) free(p);
}

int64_t createAssetManager() {
    AssetManager *assetManager = new AssetManager();
    return reinterpret_cast<int64_t>(assetManager);
}

void realeaseAssetManager(int64_t handle) {
    if (handle == 0) return;
    AssetManager *assetManager = reinterpret_cast<AssetManager*>(handle);
    delete assetManager;
}

int getPackageId(const char* path) {
    if (path == NULL) {
        fprintf(stderr, "ERROR: path(%p) is null\n", path);
        return 0;
    }

    int32_t assetsCookie;
    AssetManager assets;
    assets.addAssetPath(String8(path), &assetsCookie);
    return (int) assets.getResources(false).getPackageId(assetsCookie);
}

bool addPackage(int64_t handle, const char* path) {
    if (handle == 0 || path == NULL) {
        fprintf(stderr, "ERROR: handle(%p) or path(%p) is null\n", (void*) handle, path);
        return false;
    }

    int32_t assetsCookie;
    AssetManager *assets = reinterpret_cast<AssetManager*>(handle);
    if (!assets->addAssetPath(String8(path), &assetsCookie)) {
        fprintf(stderr, "ERROR: dump failed because assets could not be loaded : %s\n", path);
        return false;
    }
    return true;
}

bool addResPackage(int64_t handle, const char* path) {
    if (handle == 0 || path == 0) {
        fprintf(stderr, "ERROR: handle(%p) or path(%p) is null\n", (void*) handle, path);
        return false;
    }

    AssetManager *assets = reinterpret_cast<AssetManager*>(handle);
    const ResTable& res = assets->getResources(false);
    if (res.getError() != NO_ERROR) {
        fprintf(stderr, "ERROR: dump failed because the resource table is invalid/corrupt.\n");
        return false;
    }

    int32_t packId = getPackageId(path);
    if (res.isExistPackageId(packId)) {
        fprintf(stderr, "WARRING: Existed packageId(%d) : %s\n", packId, path);
        return false;
    }

    int32_t assetsCookie;
    if (!assets->addAssetPath(String8(path), &assetsCookie)) {
        fprintf(stderr, "ERROR: dump failed because assets could not be loaded : %s\n", path);
        return false;
    }
    return true;
}

const char* getResourceName(int64_t handle, uint32_t resId) {
    if (handle == 0) return NULL;

    AssetManager *assets = reinterpret_cast<AssetManager*>(handle);
    const ResTable& res = assets->getResources(false);
    if (res.getError() != NO_ERROR) {
        fprintf(stderr, "ERROR: dump failed because the resource table is invalid/corrupt.\n");
        return NULL;
    }

    android::ResTable::resource_name rname;
    if (!res.getResourceName(resId, true, &rname)) {
        return NULL;
    }

    String8 name8;
    if (rname.name8 != NULL) {
        name8 = String8(rname.name8, rname.nameLen);
    } else {
        name8 = String8(rname.name, rname.nameLen);
    }
    return name8.string();
}

const char* getResourceType(int64_t handle, uint32_t resId) {
    if (handle == 0) return NULL;

    AssetManager *assets = reinterpret_cast<AssetManager*>(handle);
    const ResTable& res = assets->getResources(false);
    if (res.getError() != NO_ERROR) {
        fprintf(stderr, "ERROR: dump failed because the resource table is invalid/corrupt.\n");
        return NULL;
    }

    android::ResTable::resource_name rname;
    if (!res.getResourceName(resId, true, &rname)) {
        return NULL;
    }

    String8 type8;
    if (rname.type8 != NULL) {
        type8 = String8(rname.type8, rname.typeLen);
    } else {
        type8 = String8(rname.type, rname.typeLen);
    }
    return type8.string();
}

ssize_t getResourceValues(int64_t handle, uint32_t resId
        , Vector<String8>** resValues, Vector<String8>** resConfigs
        , char** outName) {
    if (handle == 0) return DEAD_OBJECT;

    AssetManager *assets = reinterpret_cast<AssetManager*>(handle);
    const ResTable& res = assets->getResources(false);
    if (res.getError() != NO_ERROR) {
        fprintf(stderr, "ERROR: dump failed because the resource table is invalid/corrupt.\n");
        return DEAD_OBJECT;
    }

    *resValues = new Vector<String8>();
    if (*resValues == NULL) {
        return NO_MEMORY;
    }

    *resConfigs = new Vector<String8>();
    if (*resConfigs == NULL) {
        free(*resValues);
        *resValues = NULL;
        return NO_MEMORY;
    }

    android::ResTable::resource_name rname;
    ssize_t ret = res.getResource(resId, *resValues, *resConfigs, &rname);

    int valCount = (*resValues)->size();
    int confCount = (*resConfigs)->size();
    if (valCount != confCount) {
        fprintf(stderr, "WARRING: resValues is different size with resConfigs\n");
    }

    if (outName != NULL) {
        String8 name8;
        if (rname.name8 != NULL) {
            name8 = String8(rname.name8, rname.nameLen);
        } else {
            name8 = String8(rname.name, rname.nameLen);
        }
        *outName = strdup(name8.string());
    }

    return ret;
}
