#include <stdio.h>
#include <stdlib.h>

#include <androidfw/ResourceTypes.h>

#include <android-base/utf8.h>
#include <android-base/file.h>  // for O_BINARY

#include "com_apkspectrum_core_scanner_AaptNativeScanner.h"
#include "JniCharacterSet.h"

#include "AaptXml.h"
#include "XMLNode.h"

JNIEXPORT jlong JNICALL Java_com_apkspectrum_core_scanner_AaptNativeScanner_nativeCreateAssetManager
  (JNIEnv *, jclass) {
    AssetManager *assetManager = new AssetManager();
    return reinterpret_cast<jlong>(assetManager);
}

JNIEXPORT void JNICALL Java_com_apkspectrum_core_scanner_AaptNativeScanner_nativeRealeaseAssetManager
  (JNIEnv *, jclass, jlong handle) {
    if (handle == 0) return;
    AssetManager *assetManager = reinterpret_cast<AssetManager*>(handle);
    delete assetManager;
}

JNIEXPORT jint JNICALL Java_com_apkspectrum_core_scanner_AaptNativeScanner_nativeGetPackageId
  (JNIEnv *env, jclass, jstring path) {
    if (path == NULL) {
        fprintf(stderr, "ERROR: path(%p) is null\n", path);
        return JNI_FALSE;
    }

    char *filepath = jstring2cstr(env, path);
    if (filepath == NULL) {
        fprintf(stderr, "Failure: encoding path is NULL\n");
        fflush(stderr);
        return JNI_FALSE;
    }
    int fd = -1;
#ifdef _WIN32
    char *utfpath = jstring2utfstr(env, path);
    if (strcmp(filepath, utfpath) != 0) {
        fd = ::android::base::utf8::open(utfpath, O_RDONLY | O_BINARY | O_CLOEXEC, 0);
        if (fd < 0) {
            fprintf(stderr, "ERROR: nativeAddResPackage() Unable to open '%s': %s, fd %d\n"
                    , utfpath, strerror(errno), fd);
            return JNI_FALSE;
        }
    }
    free(utfpath);
#endif

    AssetManager tmpAssets;
    int32_t assetsCookie;
    if (fd < 0) {
        tmpAssets.addAssetPath(String8(filepath), &assetsCookie);
    } else {
        tmpAssets.addAssetFd(fd, String8(filepath), &assetsCookie);
    }
    const ResTable& res = tmpAssets.getResources(false);
    jint packId = res.getPackageId(assetsCookie);

    free(filepath);

    return packId;
}

JNIEXPORT jboolean JNICALL Java_com_apkspectrum_core_scanner_AaptNativeScanner_nativeAddPackage
  (JNIEnv *env, jclass, jlong handle, jstring path) {
    if (handle == 0 || path == 0) {
        fprintf(stderr, "ERROR: handle(%lld) or path(%p) is null\n", static_cast<long long>(handle), path);
        return JNI_FALSE;
    }

    char *filepath = jstring2cstr(env, path);
    if (filepath == NULL) {
        fprintf(stderr, "Failure: encoding path is NULL\n");
        fflush(stderr);
        return JNI_FALSE;
    }

    int fd = -1;
#ifdef _WIN32
    char *utfpath = jstring2utfstr(env, path);
    if (strcmp(filepath, utfpath) != 0) {
        fd = ::android::base::utf8::open(utfpath, O_RDONLY | O_BINARY | O_CLOEXEC, 0);
        if (fd < 0) {
            fprintf(stderr, "ERROR: nativeAddResPackage() Unable to open '%s': %s, fd %d\n"
                    , utfpath, strerror(errno), fd);
            return JNI_FALSE;
        }
    }
    free(utfpath);
#endif

    AssetManager *assetManager = reinterpret_cast<AssetManager*>(handle);
    int32_t assetsCookie;
    jboolean result = JNI_TRUE;
    if (fd < 0) {
        if (!assetManager->addAssetPath(String8(filepath), &assetsCookie)) {
            fprintf(stderr, "ERROR: dump failed because assets could not be loaded : %s\n", filepath);
            fflush(stderr);
            result = JNI_FALSE;
        }
    } else {
        if (!assetManager->addAssetFd(fd, String8(filepath), &assetsCookie)) {
            fprintf(stderr, "ERROR: dump failed because assets could not be loaded : %s\n", filepath);
            fflush(stderr);
            result = JNI_FALSE;
        }
    }
    free(filepath);

    return result;
}

JNIEXPORT jboolean JNICALL Java_com_apkspectrum_core_scanner_AaptNativeScanner_nativeAddResPackage
  (JNIEnv * env, jclass, jlong handle, jstring path) {
    if (handle == 0 || path == 0) {
        fprintf(stderr, "ERROR: handle(%lld) or path(%p) is null\n", static_cast<long long>(handle), path);
        return JNI_FALSE;
    }

    char *filepath = jstring2cstr(env, path);
    if (filepath == NULL) {
        fprintf(stderr, "Failure: encoding path is NULL\n");
        fflush(stderr);
        return JNI_FALSE;
    }
    int fd = -1;
#ifdef _WIN32
    char *utfpath = jstring2utfstr(env, path);
    if (strcmp(filepath, utfpath) != 0) {
        fd = ::android::base::utf8::open(utfpath, O_RDONLY | O_BINARY | O_CLOEXEC, 0);
        if (fd < 0) {
            fprintf(stderr, "ERROR: nativeAddResPackage() Unable to open '%s': %s, fd %d\n"
                    , utfpath, strerror(errno), fd);
            return JNI_FALSE;
        }
    }
    free(utfpath);
#endif

    int32_t packId = -1;
    {
        AssetManager tmpAssets;
        int32_t assetsCookie;
        if (fd < 0) {
            if (!tmpAssets.addAssetPath(String8(filepath), &assetsCookie)) {
                fprintf(stderr, "ERROR: dump failed because assets could not be loaded : %s\n", filepath);
                return JNI_FALSE;
            }
        } else {
            if (!tmpAssets.addAssetFd(fd, String8(filepath), &assetsCookie)) {
                fprintf(stderr, "ERROR: dump failed because assets could not be loaded by fd\n");
                return JNI_FALSE;
            }
        }
        const ResTable& res = tmpAssets.getResources(false);
        packId = res.getPackageId(assetsCookie);
    }

    AssetManager *assetManager = reinterpret_cast<AssetManager*>(handle);

    const ResTable& res = assetManager->getResources(false);
    if (res.getError() != NO_ERROR) {
        fprintf(stderr, "ERROR: dump failed because the resource table is invalid/corrupt.\n");
        fflush(stderr);
        return JNI_FALSE;
    }

    jboolean result = JNI_TRUE;
    if (res.isExistPackageId(packId)) {
        fprintf(stderr, "WARRING: Existed packageId(%d) : %s\n", packId, filepath);
        result = JNI_FALSE;
    } else {
        int32_t assetsCookie;
        if (fd < 0) {
            if (!assetManager->addAssetPath(String8(filepath), &assetsCookie)) {
                fprintf(stderr, "ERROR: dump failed because assets could not be loaded : %s\n", filepath);
                result = JNI_FALSE;
            }
        } else {
            if (!assetManager->addAssetFd(fd, String8(filepath), &assetsCookie)) {
                fprintf(stderr, "ERROR: dump failed because assets could not be loaded by fd\n");
                return JNI_FALSE;
            }
        }
    }

    free(filepath);
    fflush(stderr);

    return result;
}

JNIEXPORT jstring JNICALL Java_com_apkspectrum_core_scanner_AaptNativeScanner_nativeGetResourceName
  (JNIEnv *env, jclass, jlong handle, jint resID) {
    if (handle == 0) return NULL;

    AssetManager *assetManager = reinterpret_cast<AssetManager*>(handle);

    const ResTable& res = assetManager->getResources(false);
    if (res.getError() != NO_ERROR) {
        fprintf(stderr, "ERROR: dump failed because the resource table is invalid/corrupt.\n");
        fflush(stderr);
        return NULL;
    }

    jstring resName = NULL;
    android::ResTable::resource_name rname;
    if (res.getResourceName(resID, true, &rname)) {
        String8 name8;
        if (rname.name8 != NULL) {
            name8 = String8(rname.name8, rname.nameLen);
        } else {
            name8 = String8(rname.name, rname.nameLen);
        }
        resName = env->NewStringUTF(name8.string());
    }

    return resName;
}

JNIEXPORT jstring JNICALL Java_com_apkspectrum_core_scanner_AaptNativeScanner_nativeGetResourceType
  (JNIEnv * env, jclass, jlong handle, jint resID) {
    if (handle == 0) return NULL;

    AssetManager *assetManager = reinterpret_cast<AssetManager*>(handle);

    const ResTable& res = assetManager->getResources(false);
    if (res.getError() != NO_ERROR) {
        fprintf(stderr, "ERROR: dump failed because the resource table is invalid/corrupt.\n");
        fflush(stderr);
        return NULL;
    }

    jstring resType = NULL;
    android::ResTable::resource_name rname;
    if (res.getResourceName(resID, true, &rname)) {
        String8 type8;
        if (rname.type8 != NULL) {
            type8 = String8(rname.type8, rname.typeLen);
        } else {
            type8 = String8(rname.type, rname.typeLen);
        }
        resType = env->NewStringUTF(type8.string());
    }

    return resType;
}

JNIEXPORT jobjectArray JNICALL Java_com_apkspectrum_core_scanner_AaptNativeScanner_nativeGetResourceValues
  (JNIEnv *env, jclass, jlong handle, jint resID) {
    if (handle == 0) return NULL;

    jclass apkinfo_ResourceInfo = env->FindClass("com/apkspectrum/data/apkinfo/ResourceInfo");
    if (apkinfo_ResourceInfo == NULL) {
        fprintf(stderr, "ERROR: failed find class \"com/apkspectrum/data/apkinfo/ResourceInfo\"\n");
        fflush(stderr);
        return NULL;
    }

    jmethodID apkinfo_ResourceInfo_ = env->GetMethodID(apkinfo_ResourceInfo, "<init>", "(Ljava/lang/String;Ljava/lang/String;)V");
    if (apkinfo_ResourceInfo_ == NULL) {
        fprintf(stderr, "ERROR: failed GetMethodID ResourceInfo<init>\n");
        fflush(stderr);
        env->DeleteLocalRef(apkinfo_ResourceInfo);
        return NULL;
    }

    Vector<String8> resValues;
    Vector<String8> resConfigs;

    AssetManager *assetManager = reinterpret_cast<AssetManager*>(handle);
    const ResTable& res = assetManager->getResources(false);
    if (res.getError() != NO_ERROR) {
        fprintf(stderr, "ERROR: dump failed because the resource table is invalid/corrupt.\n");
        fflush(stderr);
        env->DeleteLocalRef(apkinfo_ResourceInfo);
        return NULL;
    }
    res.getResource(resID, &resValues, &resConfigs);

    int valCount = resValues.size();
    int confCount = resConfigs.size();

    if (valCount != confCount) {
        fprintf(stderr, "WARRING: resValues is different size with resConfigs\n");
        fflush(stderr);
    }

    jobjectArray outputArray = env->NewObjectArray(valCount, apkinfo_ResourceInfo, NULL);
    if (outputArray == NULL) {
        fprintf(stderr, "ERROR: Can't create to arrary of ResourceInfo\n");
        fflush(stderr);
        env->DeleteLocalRef(apkinfo_ResourceInfo);
        return NULL;
    }

    for (int i = 0; i < valCount; i++) {
        jobject item = env->NewObject(apkinfo_ResourceInfo, apkinfo_ResourceInfo_,
                env->NewStringUTF(resValues[i].string()),
                env->NewStringUTF(i < confCount ? resConfigs[i].string() : ""));
        if (item == NULL) {
            fprintf(stderr, "WARRING: Can't create to object of ResourceInfo\n");
            fflush(stderr);
            continue;
        }
        env->SetObjectArrayElement(outputArray, i, item);
        env->DeleteLocalRef(item);
    }

    return outputArray;
}

JNIEXPORT jobject JNICALL Java_com_apkspectrum_core_scanner_AaptNativeScanner_nativeGetResourceString
  (JNIEnv *, jclass, jlong, jint, jstring) {
    return NULL;
}

// static JNINativeMethod sMethod[] = {
//     /* name, signature, funcPtr */
//     {"nativeCreateAssetManager", "()J", (jobjectArray*)Java_com_apkspectrum_core_scanner_AaptNativeScanner_nativeCreateAssetManager},
//     {"nativeRealeaseAssetManager", "(J)V", (jobjectArray*)Java_com_apkspectrum_core_scanner_AaptNativeScanner_nativeRealeaseAssetManager},
//     {"nativeGetPackageId", "(Ljava/lang/String;)I", (jobjectArray*)Java_com_apkspectrum_core_scanner_AaptNativeScanner_nativeGetPackageId},
//     {"nativeAddPackage", "(JLjava/lang/String;)Z", (jobjectArray*)Java_com_apkspectrum_core_scanner_AaptNativeScanner_nativeAddPackage},
//     {"nativeAddResPackage", "(JLjava/lang/String;)Z", (jobjectArray*)Java_com_apkspectrum_core_scanner_AaptNativeScanner_nativeAddResPackage},
//     {"nativeGetResourceName", "(JI)Ljava/lang/String;", (jobjectArray*)Java_com_apkspectrum_core_scanner_AaptNativeScanner_nativeGetResourceName},
//     {"nativeGetResourceType", "(JI)Ljava/lang/String;", (jobjectArray*)Java_com_apkspectrum_core_scanner_AaptNativeScanner_nativeGetResourceType},
//     {"nativeGetResourceValues", "(JI)[Lcom/apkspectrum/data/apkinfo/ResourceInfo;", (jobjectArray*)Java_com_apkspectrum_core_scanner_AaptNativeScanner_nativeGetResourceValues},
//     {"nativeGetResourceString", "(JILjava/lang/String;)Lcom/apkspectrum/data/apkinfo/ResourceInfo;", (jobjectArray*)Java_com_apkspectrum_core_scanner_AaptNativeScanner_nativeGetResourceString}
// };

/*
int jniRegisterNativMethod(JNIEnv* env, const char* className, const JNINativeMethod* gMethods, int numMethods ) {
    jclass clazz;

    clazz = env->FindClass(className);

    if (clazz == NULL) {
        return -1;
    }
    if (env->RegisterNatives(clazz, gMethods, numMethods) < 0) {
        return -1;
    }
    return 0;
}
*/
