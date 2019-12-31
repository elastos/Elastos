// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Utils.h"
#include "IMasterWallet.h"
#include "nlohmann/json.hpp"

using namespace Elastos::ElaWallet;
const std::string CLASS_MCSUBWALLET = CLASS_PACKAGE_PATH + "MainchainSubWallet";
const std::string CLASS_IDSUBWALLET = CLASS_PACKAGE_PATH + "IDChainSubWallet";
const std::string CHAINID_MAINCHAIN = "ELA";
const std::string CHAINID_IDCHAIN = "IDChain";

#define JNI_GetID "(J)Ljava/lang/String;"

static jstring JNICALL GetID(JNIEnv *env, jobject clazz, jlong instance) {
    jstring id = NULL;
    try {
        IMasterWallet *masterWallet = (IMasterWallet *) instance;
        std::string key = masterWallet->GetID();
        id = env->NewStringUTF(key.c_str());
    } catch (const std::exception &e) {
        ThrowWalletException(env, e.what());
    }

    return id;
}

#define JNI_GetBasicInfo "(J)Ljava/lang/String;"

static jstring JNICALL GetBasicInfo(JNIEnv *env, jobject clazz, jlong instance) {
    IMasterWallet *masterWallet = (IMasterWallet *) instance;
    jstring info = NULL;

    try {
        nlohmann::json basicInfo = masterWallet->GetBasicInfo();
        info = env->NewStringUTF(basicInfo.dump().c_str());
    } catch (const std::exception &e) {
        ThrowWalletException(env, e.what());
    }

    return info;
}

#define JNI_GetAllSubWallets "(J)[Ljava/lang/Object;"

static jobjectArray JNICALL GetAllSubWallets(JNIEnv *env, jobject clazz, jlong instance) {
    try {
        IMasterWallet *masterWallet = (IMasterWallet *) instance;
        std::vector<ISubWallet *> allSubWallets = masterWallet->GetAllSubWallets();

        jclass clazzSubWallet;
        jmethodID subWalletConstructor;
        jobject subWallet;

        jclass clazzObject = env->FindClass("java/lang/Object");
        jobjectArray subWalletArray = env->NewObjectArray(allSubWallets.size(), clazzObject, NULL);

        for (int i = 0; i < allSubWallets.size(); i++) {
            std::string id = allSubWallets[i]->GetChainID();
            if (id == CHAINID_MAINCHAIN) {
                clazzSubWallet = env->FindClass(CLASS_MCSUBWALLET.c_str());
                subWalletConstructor = env->GetMethodID(clazzSubWallet, "<init>", "(J)V");
                subWallet = env->NewObject(clazzSubWallet, subWalletConstructor,
                                           (jlong) allSubWallets[i]);
                env->SetObjectArrayElement(subWalletArray, i, subWallet);
            } else if (id == CHAINID_IDCHAIN) {
                clazzSubWallet = env->FindClass(CLASS_IDSUBWALLET.c_str());
                subWalletConstructor = env->GetMethodID(clazzSubWallet, "<init>", "(J)V");
                subWallet = env->NewObject(clazzSubWallet, subWalletConstructor,
                                           (jlong) allSubWallets[i]);
                env->SetObjectArrayElement(subWalletArray, i, subWallet);
            } else {
                ThrowWalletException(env, "Unknow chain id");
                return NULL;
            }
        }

        return subWalletArray;
    } catch (const std::exception &e) {
        ThrowWalletException(env, e.what());
        return NULL;
    }
}

#define JNI_GetSubWallet "(JLjava/lang/String;)J"

static jlong JNICALL GetSubWallet(JNIEnv *env, jobject clazz, jlong instance, jstring jChainID) {

    bool exception = false;
    std::string msgException;
    ISubWallet *subWallet = NULL;

    const char *chainID = env->GetStringUTFChars(jChainID, NULL);

    IMasterWallet *masterWallet = (IMasterWallet *) instance;
    try {
        subWallet = masterWallet->GetSubWallet(chainID);
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jChainID, chainID);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return (jlong) subWallet;
}

#define JNI_CreateSubWallet "(JLjava/lang/String;)J"

static jlong JNICALL CreateSubWallet(JNIEnv *env, jobject clazz, jlong instance, jstring jChainID) {

    bool exception = false;
    std::string msgException;

    const char *chainID = env->GetStringUTFChars(jChainID, NULL);

    IMasterWallet *masterWallet = (IMasterWallet *) instance;
    ISubWallet *subWallet = NULL;

    try {
        subWallet = masterWallet->CreateSubWallet(chainID);
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jChainID, chainID);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return (jlong) subWallet;
}

#define JNI_DestroyWallet "(JLjava/lang/String;)V"

static void JNICALL DestroyWallet(JNIEnv *env, jobject clazz, jlong jMasterInstance,
                                  jstring jchainID) {
    bool exception = false;
    std::string msgException;

    const char *chainID = env->GetStringUTFChars(jchainID, NULL);

    try {
        IMasterWallet *masterWallet = (IMasterWallet *) jMasterInstance;
        masterWallet->DestroyWallet(chainID);
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jchainID, chainID);

    if (exception)
        ThrowWalletException(env, msgException.c_str());
}

#define JNI_GetPubKeyInfo "(J)Ljava/lang/String;"

static jstring JNICALL GetPubKeyInfo(JNIEnv *env, jobject clazz, jlong instance) {
    jstring info = NULL;

    try {
        IMasterWallet *masterWallet = (IMasterWallet *) instance;
        nlohmann::json k = masterWallet->GetPubKeyInfo();
        info = env->NewStringUTF(k.dump().c_str());
    } catch (const std::exception &e) {
        ThrowWalletException(env, e.what());
    }

    return info;
}

#define JNI_IsAddressValid "(JLjava/lang/String;)Z"

static jboolean JNICALL
IsAddressValid(JNIEnv *env, jobject clazz, jlong instance, jstring jaddress) {
    bool exception = false;
    std::string msgException;

    const char *address = env->GetStringUTFChars(jaddress, NULL);

    IMasterWallet *masterWallet = (IMasterWallet *) instance;
    bool valid = false;

    try {
        valid = masterWallet->IsAddressValid(address);
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jaddress, address);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return (jboolean) valid;
}

#define JNI_GetSupportedChains "(J)[Ljava/lang/String;"

static jobjectArray JNICALL GetSupportedChains(JNIEnv *env, jobject clazz, jlong instance) {
    IMasterWallet *masterWallet = (IMasterWallet *) instance;
    std::vector<std::string> chains;

    try {
        chains = masterWallet->GetSupportedChains();
    } catch (const std::exception &e) {
        ThrowWalletException(env, e.what());
        return NULL;
    }

    jclass objClass = env->FindClass("java/lang/String");
    jobjectArray objArray = env->NewObjectArray(chains.size(), objClass, 0);
    for (int i = 0; i < chains.size(); ++i) {
        env->SetObjectArrayElement(objArray, i, env->NewStringUTF(chains[i].c_str()));
    }

    return objArray;
}

#define JNI_ChangePassword "(JLjava/lang/String;Ljava/lang/String;)V"

static void JNICALL
ChangePassword(JNIEnv *env, jobject clazz, jlong instance, jstring joldPassword,
               jstring jnewPassword) {
    bool exception = false;
    std::string msgException;

    const char *oldPassword = env->GetStringUTFChars(joldPassword, NULL);
    const char *newPassword = env->GetStringUTFChars(jnewPassword, NULL);

    IMasterWallet *masterWallet = (IMasterWallet *) instance;

    try {
        masterWallet->ChangePassword(oldPassword, newPassword);
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(joldPassword, oldPassword);
    env->ReleaseStringUTFChars(jnewPassword, newPassword);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }
}

#define JNI_VerifyPrivateKey "(JLjava/lang/String;Ljava/lang/String;)Z"

static jboolean JNICALL
VerifyPrivateKey(JNIEnv *env, jobject clazz, jlong instance, jstring jmnemonic,
                 jstring jpassphrase) {
    bool exception = false;
    std::string msgException;
    bool valid = false;

    const char *mnemonic = env->GetStringUTFChars(jmnemonic, NULL);
    const char *passphrase = env->GetStringUTFChars(jpassphrase, NULL);

    IMasterWallet *masterWallet = (IMasterWallet *) instance;

    try {
        valid = masterWallet->VerifyPrivateKey(mnemonic, passphrase);
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jmnemonic, mnemonic);
    env->ReleaseStringUTFChars(jpassphrase, passphrase);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return (jboolean) valid;
}

#define JNI_VerifyPassPhrase "(JLjava/lang/String;Ljava/lang/String;)Z"

static jboolean JNICALL VerifyPassPhrase(JNIEnv *env, jobject clazz, jlong instance,
                                         jstring jpassphrase, jstring jpayPasswd) {
    bool exception = false;
    std::string msgException;
    bool valid = false;

    const char *passphrase = env->GetStringUTFChars(jpassphrase, NULL);
    const char *payPasswd = env->GetStringUTFChars(jpayPasswd, NULL);

    IMasterWallet *masterWallet = (IMasterWallet *) instance;

    try {
        valid = masterWallet->VerifyPassPhrase(passphrase, payPasswd);
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jpassphrase, passphrase);
    env->ReleaseStringUTFChars(jpayPasswd, payPasswd);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return (jboolean) valid;
}

#define JNI_VerifyPayPassword "(JLjava/lang/String;)Z"

static jboolean JNICALL
VerifyPayPassword(JNIEnv *env, jobject clazz, jlong instance, jstring jpayPasswd) {
    bool exception = false;
    std::string msgException;
    bool valid = false;

    const char *payPasswd = env->GetStringUTFChars(jpayPasswd, NULL);

    IMasterWallet *masterWallet = (IMasterWallet *) instance;

    try {
        valid = masterWallet->VerifyPayPassword(payPasswd);
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jpayPasswd, payPasswd);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return (jboolean) valid;
}

#define JNI_ExportKeystore "(JLjava/lang/String;Ljava/lang/String;)Ljava/lang/String;"

static jstring JNICALL ExportKeystore(JNIEnv *env, jobject clazz, jlong instance,
                                      jstring jbackPassword,
                                      jstring jpayPassword) {
    bool exception = false;
    std::string msgException;
    jstring result = NULL;

    const char *backPassword = env->GetStringUTFChars(jbackPassword, NULL);
    const char *payPassword = env->GetStringUTFChars(jpayPassword, NULL);

    IMasterWallet *masterWallet = (IMasterWallet *) instance;

    try {
        nlohmann::json keyStore = masterWallet->ExportKeystore(backPassword, payPassword);
        result = env->NewStringUTF(keyStore.dump().c_str());
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jbackPassword, backPassword);
    env->ReleaseStringUTFChars(jpayPassword, payPassword);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return result;
}

#define JNI_ExportMnemonic "(JLjava/lang/String;)Ljava/lang/String;"

static jstring JNICALL ExportMnemonic(JNIEnv *env, jobject clazz, jlong instance,
                                      jstring jpayPassword) {
    bool exception = false;
    std::string msgException;
    jstring jmnemonic = NULL;

    const char *payPassword = env->GetStringUTFChars(jpayPassword, NULL);

    IMasterWallet *masterWallet = (IMasterWallet *) instance;

    try {
        std::string result = masterWallet->ExportMnemonic(payPassword);
        jmnemonic = env->NewStringUTF(result.c_str());
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jpayPassword, payPassword);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return jmnemonic;
}

#define JNI_ExportReadonlyWallet "(J)Ljava/lang/String;"

static jstring JNICALL ExportReadonlyWallet(JNIEnv *env, jobject clazz, jlong instance) {
    jstring result = NULL;

    IMasterWallet *masterWallet = (IMasterWallet *) instance;

    try {
        nlohmann::json j = masterWallet->ExportReadonlyWallet();
        result = env->NewStringUTF(j.dump().c_str());
    } catch (const std::exception &e) {
        ThrowWalletException(env, e.what());
    }

    return result;
}

#define JNI_ExportPrivateKey "(JLjava/lang/String;)Ljava/lang/String;"

static jstring JNICALL ExportPrivateKey(JNIEnv *env, jobject clazz, jlong instance,
                                        jstring jpayPassword) {
    bool exception = false;
    std::string msgException;
    jstring result = NULL;

    const char *payPassword = env->GetStringUTFChars(jpayPassword, NULL);

    IMasterWallet *masterWallet = (IMasterWallet *) instance;

    try {
        std::string privateKey = masterWallet->ExportPrivateKey(payPassword);
        result = env->NewStringUTF(privateKey.c_str());
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jpayPassword, payPassword);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return result;
}

#define JNI_ExportMasterPublicKey "(J)Ljava/lang/String;"

static jstring JNICALL ExportMasterPublicKey(JNIEnv *env, jobject clazz, jlong instance) {
    jstring result = NULL;

    IMasterWallet *masterWallet = (IMasterWallet *) instance;

    try {
        std::string pubKey = masterWallet->ExportMasterPublicKey();
        result = env->NewStringUTF(pubKey.c_str());
    } catch (const std::exception &e) {
        ThrowWalletException(env, e.what());
    }

    return result;
}

static const JNINativeMethod methods[] = {
        REGISTER_METHOD(GetID),
        REGISTER_METHOD(GetBasicInfo),
        REGISTER_METHOD(GetAllSubWallets),
        REGISTER_METHOD(GetSubWallet),
        REGISTER_METHOD(CreateSubWallet),
        REGISTER_METHOD(DestroyWallet),
        REGISTER_METHOD(IsAddressValid),
        REGISTER_METHOD(GetSupportedChains),
        REGISTER_METHOD(ChangePassword),
        REGISTER_METHOD(GetPubKeyInfo),
        REGISTER_METHOD(VerifyPrivateKey),
        REGISTER_METHOD(VerifyPassPhrase),
        REGISTER_METHOD(VerifyPayPassword),
        REGISTER_METHOD(ExportKeystore),
        REGISTER_METHOD(ExportMnemonic),
        REGISTER_METHOD(ExportReadonlyWallet),
        REGISTER_METHOD(ExportPrivateKey),
        REGISTER_METHOD(ExportMasterPublicKey),
};

jint RegisterMasterWallet(JNIEnv *env, const std::string &path) {
    return RegisterNativeMethods(env, path + "MasterWallet", methods, NELEM(methods));
}

