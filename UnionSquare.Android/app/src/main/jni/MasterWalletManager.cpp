// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Utils.h"
#include "MasterWalletManager.h"

using namespace Elastos::ElaWallet;

#define JNI_GenerateMnemonic "(JLjava/lang/String;I)Ljava/lang/String;"

static jstring JNICALL GenerateMnemonic(JNIEnv *env, jobject clazz, jlong instance,
                                        jstring jlanguage, jint wordCount) {
    bool exception = false;
    std::string msgException;

    MasterWalletManager *walletManager = (MasterWalletManager *) instance;
    const char *language = env->GetStringUTFChars(jlanguage, NULL);
    jstring mnemonic = NULL;

    try {
        std::string str = walletManager->GenerateMnemonic(language, wordCount);
        mnemonic = env->NewStringUTF(str.c_str());
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jlanguage, language);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return mnemonic;
}

#define JNI_CreateMasterWallet "(JLjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Z)J"

static jlong JNICALL CreateMasterWallet(JNIEnv *env, jobject clazz, jlong instance,
                                        jstring jmasterWalletId,
                                        jstring jmnemonic,
                                        jstring jphrasePassword,
                                        jstring jpayPassword,
                                        jboolean jSingleAddress) {
    bool exception = false;
    std::string msgException;

    const char *masterWalletId = env->GetStringUTFChars(jmasterWalletId, NULL);
    const char *mnemonic = env->GetStringUTFChars(jmnemonic, NULL);
    const char *phrasePassword = env->GetStringUTFChars(jphrasePassword, NULL);
    const char *payPassword = env->GetStringUTFChars(jpayPassword, NULL);

    MasterWalletManager *walletManager = (MasterWalletManager *) instance;
    IMasterWallet *masterWallet = NULL;

    try {
        masterWallet = walletManager->CreateMasterWallet(masterWalletId, mnemonic,
                                                         phrasePassword, payPassword,
                                                         jSingleAddress);
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jmasterWalletId, masterWalletId);
    env->ReleaseStringUTFChars(jmnemonic, mnemonic);
    env->ReleaseStringUTFChars(jphrasePassword, phrasePassword);
    env->ReleaseStringUTFChars(jpayPassword, payPassword);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return (jlong) masterWallet;
}

#define JNI_CreateMultiSignMasterWallet "(JLjava/lang/String;Ljava/lang/String;IZZJ)J"

static jlong JNICALL CreateMultiSignMasterWallet(JNIEnv *env, jobject clazz, jlong instance,
                                                 jstring jMasterWalletId,
                                                 jstring jCoSigners,
                                                 jint jRequiredSignCount,
                                                 jboolean jsingleAddress,
                                                 jboolean jcompatible,
                                                 jlong timestamp) {
    bool exception = false;
    std::string msgException;

    const char *masterWalletId = env->GetStringUTFChars(jMasterWalletId, NULL);
    const char *coSigners = env->GetStringUTFChars(jCoSigners, NULL);

    MasterWalletManager *walletManager = (MasterWalletManager *) instance;
    IMasterWallet *masterWallet = NULL;
    nlohmann::json coSignersJson = nlohmann::json::parse(coSigners);

    try {
        masterWallet = walletManager->CreateMultiSignMasterWallet(masterWalletId, coSignersJson,
                                                                  jRequiredSignCount,
                                                                  jsingleAddress,
                                                                  jcompatible, (time_t) timestamp);
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jMasterWalletId, masterWalletId);
    env->ReleaseStringUTFChars(jCoSigners, coSigners);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return (jlong) masterWallet;
}

#define JNI_CreateMultiSignMasterWalletWithPrivKey "(JLjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;IZZJ)J"

static jlong JNICALL
CreateMultiSignMasterWalletWithPrivKey(JNIEnv *env, jobject clazz, jlong instance,
                                       jstring jMasterWalletId,
                                       jstring jxprv,
                                       jstring jPayPassword,
                                       jstring jCoSigners,
                                       jint jRequiredSignCount,
                                       jboolean jsingleAddress,
                                       jboolean jcompatible,
                                       jlong timestamp) {
    bool exception = false;
    std::string msgException;

    const char *masterWalletId = env->GetStringUTFChars(jMasterWalletId, NULL);
    const char *xprv = env->GetStringUTFChars(jxprv, NULL);
    const char *payPassword = env->GetStringUTFChars(jPayPassword, NULL);
    const char *coSigners = env->GetStringUTFChars(jCoSigners, NULL);

    MasterWalletManager *walletManager = (MasterWalletManager *) instance;
    IMasterWallet *masterWallet = NULL;

    try {
        nlohmann::json coSignersJson = nlohmann::json::parse(coSigners);
        masterWallet = walletManager->CreateMultiSignMasterWallet(masterWalletId, xprv,
                                                                  payPassword, coSignersJson,
                                                                  jRequiredSignCount,
                                                                  jsingleAddress,
                                                                  jcompatible,
                                                                  (time_t) timestamp);
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jMasterWalletId, masterWalletId);
    env->ReleaseStringUTFChars(jxprv, xprv);
    env->ReleaseStringUTFChars(jPayPassword, payPassword);
    env->ReleaseStringUTFChars(jCoSigners, coSigners);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return (jlong) masterWallet;
}

#define JNI_CreateMultiSignMasterWalletWithMnemonic "(JLjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;IZZJ)J"

static jlong JNICALL
CreateMultiSignMasterWalletWithMnemonic(JNIEnv *env, jobject clazz, jlong instance,
                                        jstring jMasterWalletId,
                                        jstring jMnemonic,
                                        jstring jPhrasePassword,
                                        jstring jPayPassword,
                                        jstring jCoSigners,
                                        jint jRequiredSignCount,
                                        jboolean jsingleAddress,
                                        jboolean jcompatible,
                                        jlong timestamp) {
    bool exception = false;
    std::string msgException;

    const char *masterWalletId = env->GetStringUTFChars(jMasterWalletId, NULL);
    const char *mnemonic = env->GetStringUTFChars(jMnemonic, NULL);
    const char *phrasePassword = env->GetStringUTFChars(jPhrasePassword, NULL);
    const char *payPassword = env->GetStringUTFChars(jPayPassword, NULL);
    const char *coSigners = env->GetStringUTFChars(jCoSigners, NULL);

    MasterWalletManager *walletManager = (MasterWalletManager *) instance;
    IMasterWallet *masterWallet = NULL;

    try {
        nlohmann::json coSignersJson = nlohmann::json::parse(coSigners);
        masterWallet = walletManager->CreateMultiSignMasterWallet(masterWalletId, mnemonic,
                                                                  phrasePassword, payPassword,
                                                                  coSignersJson,
                                                                  jRequiredSignCount,
                                                                  jsingleAddress,
                                                                  jcompatible,
                                                                  (time_t) timestamp);
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jMasterWalletId, masterWalletId);
    env->ReleaseStringUTFChars(jMnemonic, mnemonic);
    env->ReleaseStringUTFChars(jPhrasePassword, phrasePassword);
    env->ReleaseStringUTFChars(jPayPassword, payPassword);
    env->ReleaseStringUTFChars(jCoSigners, coSigners);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return (jlong) masterWallet;
}

#define JNI_DestroyWallet "(JLjava/lang/String;)V"

static void JNICALL DestroyWallet(JNIEnv *env, jobject clazz, jlong instance,
                                  jstring jMasterWalletId) {
    bool exception = false;
    std::string msgException;

    const char *masterWalletId = env->GetStringUTFChars(jMasterWalletId, NULL);

    MasterWalletManager *walletManager = (MasterWalletManager *) instance;

    try {
        walletManager->DestroyWallet(masterWalletId);
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jMasterWalletId, masterWalletId);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }
}

#define JNI_ImportWalletWithKeystore "(JLjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)J"

static jlong JNICALL ImportWalletWithKeystore(JNIEnv *env, jobject clazz, jlong instance,
                                              jstring jmasterWalletId,
                                              jstring jkeystoreContent,
                                              jstring jbackupPassword,
                                              jstring jpayPassword) {
    bool exception = false;
    std::string msgException;

    const char *masterWalletId = env->GetStringUTFChars(jmasterWalletId, NULL);
    const char *keystoreContent = env->GetStringUTFChars(jkeystoreContent, NULL);
    const char *backupPassword = env->GetStringUTFChars(jbackupPassword, NULL);
    const char *payPassword = env->GetStringUTFChars(jpayPassword, NULL);

    MasterWalletManager *walletManager = (MasterWalletManager *) instance;
    IMasterWallet *masterWallet = NULL;

    try {
        masterWallet = walletManager->ImportWalletWithKeystore(masterWalletId,
                                                               nlohmann::json::parse(
                                                                       keystoreContent),
                                                               backupPassword, payPassword);
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jmasterWalletId, masterWalletId);
    env->ReleaseStringUTFChars(jkeystoreContent, keystoreContent);
    env->ReleaseStringUTFChars(jbackupPassword, backupPassword);
    env->ReleaseStringUTFChars(jpayPassword, payPassword);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return (jlong) masterWallet;
}

#define JNI_ImportWalletWithMnemonic "(JLjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;ZJ)J"

static jlong JNICALL ImportWalletWithMnemonic(JNIEnv *env, jobject clazz, jlong instance,
                                              jstring jmasterWalletId,
                                              jstring jmnemonic,
                                              jstring jphrasePassword,
                                              jstring jpayPassword,
                                              jboolean jSingleAddress,
                                              jlong timestamp) {
    bool exception = false;
    std::string msgException;

    const char *masterWalletId = env->GetStringUTFChars(jmasterWalletId, NULL);
    const char *mnemonic = env->GetStringUTFChars(jmnemonic, NULL);
    const char *phrasePassword = env->GetStringUTFChars(jphrasePassword, NULL);
    const char *payPassword = env->GetStringUTFChars(jpayPassword, NULL);

    MasterWalletManager *walletManager = (MasterWalletManager *) instance;
    IMasterWallet *masterWallet = NULL;

    try {
        masterWallet = walletManager->ImportWalletWithMnemonic(masterWalletId, mnemonic,
                                                               phrasePassword, payPassword,
                                                               jSingleAddress, (time_t) timestamp);
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jmasterWalletId, masterWalletId);
    env->ReleaseStringUTFChars(jmnemonic, mnemonic);
    env->ReleaseStringUTFChars(jphrasePassword, phrasePassword);
    env->ReleaseStringUTFChars(jpayPassword, payPassword);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return (jlong) masterWallet;
}

#define JNI_ImportReadonlyWallet "(JLjava/lang/String;Ljava/lang/String;)J"

static jlong JNICALL ImportReadonlyWallet(JNIEnv *env, jobject clazz, jlong instance,
                                          jstring jmasterWalletID,
                                          jstring jwalletJson) {
    bool exception = false;
    std::string msgException;

    const char *masterWalletID = env->GetStringUTFChars(jmasterWalletID, NULL);
    const char *walletJosn = env->GetStringUTFChars(jwalletJson, NULL);

    MasterWalletManager *manager = (MasterWalletManager *) instance;
    IMasterWallet *masterWallet = NULL;

    try {
        masterWallet = manager->ImportReadonlyWallet(masterWalletID,
                                                     nlohmann::json::parse(walletJosn));
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return (jlong) masterWallet;
}

#define JNI_GetVersion "(J)Ljava/lang/String;"

static jstring JNICALL GetVersion(JNIEnv *env, jobject clazz, jlong instance) {
    bool exception = false;
    std::string msgException;

    MasterWalletManager *manager = (MasterWalletManager *) instance;

    jstring result = NULL;

    try {
        std::string str = manager->GetVersion();
        result = env->NewStringUTF(str.c_str());
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return result;
}

#define JNI_FlushData "(J)V"

static void JNICALL FlushData(JNIEnv *env, jobject clazz, jlong instance) {
    bool exception = false;
    std::string msgException;

    MasterWalletManager *manager = (MasterWalletManager *) instance;

    try {
        manager->FlushData();
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }
}

#define JNI_GetAllMasterWallets "(J)[J"

static jlongArray JNICALL GetAllMasterWallets(JNIEnv *env, jobject clazz, jlong instance) {
    MasterWalletManager *walletManager = (MasterWalletManager *) instance;
    std::vector<IMasterWallet *> array;

    try {
        array = walletManager->GetAllMasterWallets();
    } catch (const std::exception &e) {
        ThrowWalletException(env, e.what());
        return NULL;
    }

    const int length = array.size();
    jlongArray jarray = env->NewLongArray(length);

    if (length > 0) {
        jlong masterWallets[length];
        for (int i = 0; i < length; i++) {
            masterWallets[i] = (jlong) array[i];
        }
        env->SetLongArrayRegion(jarray, 0, length, masterWallets);
    }

    return jarray;
}

#define JNI_GetAllMasterWalletID "(J)[Ljava/lang/String;"

static jobjectArray JNICALL GetAllMasterWalletID(JNIEnv *env, jobject clazz, jlong instance) {
    try {
        MasterWalletManager *walletManager = (MasterWalletManager *) instance;
        std::vector<std::string> allIds = walletManager->GetAllMasterWalletID();

        jclass objClass = env->FindClass("java/lang/String");
        jobjectArray objArray = env->NewObjectArray(allIds.size(), objClass, 0);
        for (int i = 0; i < allIds.size(); i++) {
            env->SetObjectArrayElement(objArray, i, env->NewStringUTF(allIds[i].c_str()));
        }

        return objArray;
    } catch (const std::exception &e) {
        ThrowWalletException(env, e.what());
        return NULL;
    }
}

#define JNI_GetMasterWallet "(JLjava/lang/String;)J"

static jlong JNICALL
GetMasterWallet(JNIEnv *env, jobject clazz, jlong instance, jstring jMasterWalletId) {
    bool exception = false;
    std::string msgException;

    const char *masterWalletId = env->GetStringUTFChars(jMasterWalletId, NULL);
    IMasterWallet *masterWallet = NULL;

    try {
        MasterWalletManager *walletManager = (MasterWalletManager *) instance;
        masterWallet = walletManager->GetMasterWallet(masterWalletId);
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jMasterWalletId, masterWalletId);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return (jlong) masterWallet;
}

#define JNI_InitMasterWalletManager "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)J"

static jlong JNICALL InitMasterWalletManager(JNIEnv *env, jobject clazz,
                                             jstring jrootPath,
                                             jstring jnetType,
                                             jstring jconfig,
                                             jstring jdataPath) {
    bool exception = false;
    std::string msgException;

    MasterWalletManager *walletManager = NULL;
    const char *rootPath = env->GetStringUTFChars(jrootPath, NULL);
    const char *netType = env->GetStringUTFChars(jnetType, NULL);
    const char *config = env->GetStringUTFChars(jconfig, NULL);
    const char *dataPath = env->GetStringUTFChars(jdataPath, NULL);

    try {
        nlohmann::json configJSON;
        if (config && strlen(config) > 0)
            configJSON = nlohmann::json::parse(config);

        walletManager = new MasterWalletManager(rootPath, netType, configJSON, dataPath);
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jrootPath, rootPath);
    env->ReleaseStringUTFChars(jnetType, netType);
    env->ReleaseStringUTFChars(jconfig, config);
    env->ReleaseStringUTFChars(jdataPath, dataPath);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return (jlong) walletManager;
}

#define JNI_DisposeNative "(J)V"

static void JNICALL DisposeNative(JNIEnv *env, jobject clazz, jlong instance) {
    MasterWalletManager *walletManager = (MasterWalletManager *) instance;
    delete walletManager;
}

static const JNINativeMethod methods[] = {
        REGISTER_METHOD(GenerateMnemonic),
        REGISTER_METHOD(CreateMasterWallet),
        REGISTER_METHOD(CreateMultiSignMasterWallet),
        REGISTER_METHOD(CreateMultiSignMasterWalletWithPrivKey),
        REGISTER_METHOD(CreateMultiSignMasterWalletWithMnemonic),
        REGISTER_METHOD(GetAllMasterWallets),
        REGISTER_METHOD(GetAllMasterWalletID),
        REGISTER_METHOD(GetMasterWallet),
        REGISTER_METHOD(DestroyWallet),
        REGISTER_METHOD(ImportWalletWithKeystore),
        REGISTER_METHOD(ImportWalletWithMnemonic),
        REGISTER_METHOD(ImportReadonlyWallet),
        REGISTER_METHOD(GetVersion),
        REGISTER_METHOD(InitMasterWalletManager),
        REGISTER_METHOD(DisposeNative),
        REGISTER_METHOD(FlushData),
};

jint RegisterMasterWalletManager(JNIEnv *env, const std::string &path) {
    return RegisterNativeMethods(env, path + "MasterWalletManager", methods, NELEM(methods));
}

