// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Utils.h"
#include "IIDChainSubWallet.h"
#include "nlohmann/json.hpp"

using namespace Elastos::ElaWallet;

#define JNI_CreateIDTransaction "(JLjava/lang/String;Ljava/lang/String;)Ljava/lang/String;"

static jstring JNICALL CreateIDTransaction(JNIEnv *env, jobject clazz, jlong instance,
                                           jstring jpayloadJson,
                                           jstring jmemo) {
    bool exception = false;
    std::string msgException;

    const char *payloadJson = env->GetStringUTFChars(jpayloadJson, NULL);
    const char *memo = env->GetStringUTFChars(jmemo, NULL);

    IIDChainSubWallet *wallet = (IIDChainSubWallet *) instance;
    jstring tx = NULL;

    try {
        nlohmann::json txJson = wallet->CreateIDTransaction(nlohmann::json::parse(payloadJson),
                                                            memo);
        tx = env->NewStringUTF(txJson.dump().c_str());
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jpayloadJson, payloadJson);
    env->ReleaseStringUTFChars(jmemo, memo);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return tx;
}

#define JNI_GetAllDID "(JII)Ljava/lang/String;"

static jstring JNICALL GetAllDID(JNIEnv *env, jobject clazz, jlong instance,
                                 jint jStart,
                                 jint jCount) {
    jstring didString = NULL;

    try {
        IIDChainSubWallet *wallet = (IIDChainSubWallet *) instance;
        nlohmann::json didJson = wallet->GetAllDID(jStart, jCount);
        didString = env->NewStringUTF(didJson.dump().c_str());
    } catch (const std::exception &e) {
        ThrowWalletException(env, e.what());
    }

    return didString;
}

#define JNI_GetAllCID "(JII)Ljava/lang/String;"

static jstring JNICALL GetAllCID(JNIEnv *env, jobject clazz, jlong instance,
                                 jint jStart,
                                 jint jCount) {
    jstring cidString = NULL;

    try {
        IIDChainSubWallet *wallet = (IIDChainSubWallet *) instance;
        nlohmann::json didJson = wallet->GetAllCID(jStart, jCount);
        cidString = env->NewStringUTF(didJson.dump().c_str());
    } catch (const std::exception &e) {
        ThrowWalletException(env, e.what());
    }

    return cidString;
}

#define JNI_Sign "(JLjava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;"

static jstring JNICALL Sign(JNIEnv *env, jobject clazz, jlong instance,
                            jstring jdid,
                            jstring jmessage,
                            jstring jpayPassword) {
    bool exception = false;
    std::string msgException;
    jstring jsignature = NULL;

    const char *did = env->GetStringUTFChars(jdid, NULL);
    const char *message = env->GetStringUTFChars(jmessage, NULL);
    const char *paypasswd = env->GetStringUTFChars(jpayPassword, NULL);

    try {
        IIDChainSubWallet *wallet = (IIDChainSubWallet *) instance;
        std::string signature = wallet->Sign(did, message, paypasswd);
        jsignature = env->NewStringUTF(signature.c_str());
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jdid, did);
    env->ReleaseStringUTFChars(jmessage, message);
    env->ReleaseStringUTFChars(jpayPassword, paypasswd);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return jsignature;
}

#define JNI_VerifySignature "(JLjava/lang/String;Ljava/lang/String;Ljava/lang/String;)Z"

static jboolean JNICALL VerifySignature(JNIEnv *env, jobject clazz, jlong instance,
                                        jstring jpublicKey,
                                        jstring jmessage,
                                        jstring jsignature) {
    bool exception = false;
    std::string msgException;
    bool result = false;

    const char *publicKey = env->GetStringUTFChars(jpublicKey, NULL);
    const char *message = env->GetStringUTFChars(jmessage, NULL);
    const char *signatuure = env->GetStringUTFChars(jsignature, NULL);

    try {
        IIDChainSubWallet *wallet = (IIDChainSubWallet *) instance;
        result = wallet->VerifySignature(publicKey, message, signatuure);
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jpublicKey, publicKey);
    env->ReleaseStringUTFChars(jmessage, message);
    env->ReleaseStringUTFChars(jsignature, signatuure);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return jboolean(result);
}

#define JNI_GetPublicKeyDID "(JLjava/lang/String;)Ljava/lang/String;"

static jstring JNICALL GetPublicKeyDID(JNIEnv *env, jobject clazz, jlong instance,
                                         jstring jpublicKey) {
    bool exception = false;
    std::string msgException;
    jstring jdid = NULL;

    const char *publicKey = env->GetStringUTFChars(jpublicKey, NULL);

    try {
        IIDChainSubWallet *wallet = (IIDChainSubWallet *) instance;
        std::string did = wallet->GetPublicKeyDID(publicKey);
        jdid = env->NewStringUTF(did.c_str());
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jpublicKey, publicKey);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return jdid;
}

#define JNI_GetPublicKeyCID "(JLjava/lang/String;)Ljava/lang/String;"

static jstring JNICALL GetPublicKeyCID(JNIEnv *env, jobject clazz, jlong instance,
                                       jstring jpublicKey) {
    bool exception = false;
    std::string msgException;
    jstring jcid = NULL;

    const char *publicKey = env->GetStringUTFChars(jpublicKey, NULL);

    try {
        IIDChainSubWallet *wallet = (IIDChainSubWallet *) instance;
        std::string cid = wallet->GetPublicKeyCID(publicKey);
        jcid = env->NewStringUTF(cid.c_str());
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jpublicKey, publicKey);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return jcid;
}

#define JNI_SignDigest "(JLjava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;"

static jstring JNICALL SignDigest(JNIEnv *env, jobject clazz, jlong instance,
                                    jstring jdid,
                                    jstring jdigest,
                                    jstring jpayPasswd) {
    bool exception = false;
    std::string msgException;
    jstring signature = NULL;

    const char *did = env->GetStringUTFChars(jdid, NULL);
    const char *digest = env->GetStringUTFChars(jdigest, NULL);
    const char *payPasswd = env->GetStringUTFChars(jpayPasswd, NULL);

    try {
        IIDChainSubWallet *wallet = (IIDChainSubWallet *) instance;
        std::string sig = wallet->SignDigest(did, digest, payPasswd);
        signature = env->NewStringUTF(sig.c_str());
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jdid, did);
    env->ReleaseStringUTFChars(jdigest, digest);
    env->ReleaseStringUTFChars(jpayPasswd, payPasswd);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }
    return signature;
}

static const JNINativeMethod methods[] = {
        REGISTER_METHOD(CreateIDTransaction),
        REGISTER_METHOD(GetAllDID),
        REGISTER_METHOD(GetAllCID),
        REGISTER_METHOD(Sign),
        REGISTER_METHOD(VerifySignature),
        REGISTER_METHOD(GetPublicKeyDID),
        REGISTER_METHOD(GetPublicKeyCID),
        REGISTER_METHOD(SignDigest),
};

jint RegisterIDChainSubWallet(JNIEnv *env, const std::string &path) {
    return RegisterNativeMethods(env, path + "IDChainSubWallet", methods, NELEM(methods));
}

