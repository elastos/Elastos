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
const std::string CHAINID_IDCHAIN = "IdChain";

#define JNI_GetID "(J)Ljava/lang/String;"

static jstring JNICALL GetID(JNIEnv *env, jobject clazz, jlong instance) {
    jstring id = NULL;
    try {
        IMasterWallet *masterWallet = (IMasterWallet *) instance;
        std::string key = masterWallet->GetId();
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
            std::string id = allSubWallets[i]->GetChainId();
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

#define JNI_CreateSubWallet "(JLjava/lang/String;J)J"

static jlong JNICALL CreateSubWallet(JNIEnv *env, jobject clazz, jlong instance, jstring jChainID,
                                     jlong jFeePerKb) {

    bool exception = false;
    std::string msgException;

    const char *chainID = env->GetStringUTFChars(jChainID, NULL);

    IMasterWallet *masterWallet = (IMasterWallet *) instance;
    ISubWallet *subWallet = NULL;

    try {
        subWallet = masterWallet->CreateSubWallet(chainID, jFeePerKb);
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

#define JNI_DestroyWallet "(JJ)V"

static void JNICALL DestroyWallet(JNIEnv *env, jobject clazz, jlong jMasterInstance,
                                  jlong jSubWalletInstance) {
    try {
        IMasterWallet *masterWallet = (IMasterWallet *) jMasterInstance;
        ISubWallet *subWallet = (ISubWallet *) jSubWalletInstance;
        masterWallet->DestroyWallet(subWallet);
    } catch (const std::exception &e) {
        ThrowWalletException(env, e.what());
    }
}

#define JNI_GetPublicKey "(J)Ljava/lang/String;"

static jstring JNICALL GetPublicKey(JNIEnv *env, jobject clazz, jlong instance) {
    jstring key = NULL;

    try {
        IMasterWallet *masterWallet = (IMasterWallet *) instance;
        std::string k = masterWallet->GetPublicKey();
        key = env->NewStringUTF(k.c_str());
    } catch (const std::exception &e) {
        ThrowWalletException(env, e.what());
    }

    return key;
}

#define JNI_Sign "(JLjava/lang/String;Ljava/lang/String;)Ljava/lang/String;"

static jstring JNICALL Sign(JNIEnv *env, jobject clazz, jlong instance, jstring jmessage,
                            jstring jpayPassword) {
    bool exception = false;
    std::string msgException;

    const char *message = env->GetStringUTFChars(jmessage, NULL);
    const char *payPassword = env->GetStringUTFChars(jpayPassword, NULL);

    IMasterWallet *masterWallet = (IMasterWallet *) instance;
    jstring result = NULL;

    try {
        std::string r = masterWallet->Sign(message, payPassword);
        result = env->NewStringUTF(r.c_str());
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jmessage, message);
    env->ReleaseStringUTFChars(jpayPassword, payPassword);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return result;
}

#define JNI_CheckSign "(JLjava/lang/String;Ljava/lang/String;Ljava/lang/String;)Z"

static jboolean JNICALL CheckSign(JNIEnv *env, jobject clazz, jlong instance,
                                  jstring jaddress, jstring jmessage,
                                  jstring jsignature) {
    bool exception = false;
    std::string msgException;

    const char *address = env->GetStringUTFChars(jaddress, NULL);
    const char *message = env->GetStringUTFChars(jmessage, NULL);
    const char *signature = env->GetStringUTFChars(jsignature, NULL);

    IMasterWallet *masterWallet = (IMasterWallet *) instance;
    jboolean result = 0;

    try {
        result = masterWallet->CheckSign(address, message, signature);
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jaddress, address);
    env->ReleaseStringUTFChars(jmessage, message);
    env->ReleaseStringUTFChars(jsignature, signature);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return result;
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

static const JNINativeMethod methods[] = {
        REGISTER_METHOD(GetID),
        REGISTER_METHOD(GetBasicInfo),
        REGISTER_METHOD(GetAllSubWallets),
        REGISTER_METHOD(CreateSubWallet),
        REGISTER_METHOD(DestroyWallet),
        REGISTER_METHOD(GetPublicKey),
        REGISTER_METHOD(Sign),
        REGISTER_METHOD(CheckSign),
        REGISTER_METHOD(IsAddressValid),
        REGISTER_METHOD(GetSupportedChains),
        REGISTER_METHOD(ChangePassword),
};

jint RegisterMasterWallet(JNIEnv *env, const std::string &path) {
    return RegisterNativeMethods(env, path + "MasterWallet", methods, NELEM(methods));
}

