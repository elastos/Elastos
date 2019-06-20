// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Utils.h"
#include "IMainchainSubWallet.h"
#include "nlohmann/json.hpp"

using namespace Elastos::ElaWallet;

#define JNI_CreateDepositTransaction "(JLjava/lang/String;Ljava/lang/String;JLjava/lang/String;Ljava/lang/String;Z)Ljava/lang/String;"

static jstring JNICALL CreateDepositTransaction(JNIEnv *env, jobject clazz, jlong instance,
                                                jstring jfromAddress,
                                                jstring jlockedAddress,
                                                jlong amount,
                                                jstring jsideChainAddress,
                                                jstring jmemo,
                                                jboolean useVotedUTXO) {
    bool exception = false;
    std::string msgException;

    const char *fromAddress = env->GetStringUTFChars(jfromAddress, NULL);
    const char *lockedAddress = env->GetStringUTFChars(jlockedAddress, NULL);
    const char *sideChainAddress = env->GetStringUTFChars(jsideChainAddress, NULL);
    const char *memo = env->GetStringUTFChars(jmemo, NULL);

    IMainchainSubWallet *wallet = (IMainchainSubWallet *) instance;
    jstring tx = NULL;

    try {
        nlohmann::json txJson = wallet->CreateDepositTransaction(fromAddress, lockedAddress, amount,
                                                                 sideChainAddress, memo,
                                                                 useVotedUTXO);
        tx = env->NewStringUTF(txJson.dump().c_str());
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jfromAddress, fromAddress);
    env->ReleaseStringUTFChars(jlockedAddress, lockedAddress);
    env->ReleaseStringUTFChars(jsideChainAddress, sideChainAddress);
    env->ReleaseStringUTFChars(jmemo, memo);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return tx;
}

#define JNI_GenerateProducerPayload "(JLjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;JLjava/lang/String;)Ljava/lang/String;"

static jstring JNICALL GenerateProducerPayload(JNIEnv *env, jobject clazz, jlong jProxy,
                                               jstring jPublicKey,
                                               jstring jNodePublicKey,
                                               jstring jNickName,
                                               jstring jURL,
                                               jstring jIPAddress,
                                               jlong location,
                                               jstring jPayPasswd) {
    bool exception = false;
    std::string msgException;
    jstring payload = NULL;

    const char *publicKey = env->GetStringUTFChars(jPublicKey, NULL);
    const char *nodePublicKey = env->GetStringUTFChars(jNodePublicKey, NULL);
    const char *nickName = env->GetStringUTFChars(jNickName, NULL);
    const char *url = env->GetStringUTFChars(jURL, NULL);
    const char *ipAddress = env->GetStringUTFChars(jIPAddress, NULL);
    const char *payPasswd = env->GetStringUTFChars(jPayPasswd, NULL);

    try {
        IMainchainSubWallet *wallet = (IMainchainSubWallet *) jProxy;
        nlohmann::json payloadJson = wallet->GenerateProducerPayload(publicKey, nodePublicKey,
                                                                     nickName, url, ipAddress,
                                                                     location, payPasswd);
        payload = env->NewStringUTF(payloadJson.dump().c_str());
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jPublicKey, publicKey);
    env->ReleaseStringUTFChars(jNodePublicKey, nodePublicKey);
    env->ReleaseStringUTFChars(jNickName, nickName);
    env->ReleaseStringUTFChars(jURL, url);
    env->ReleaseStringUTFChars(jIPAddress, ipAddress);
    env->ReleaseStringUTFChars(jPayPasswd, payPasswd);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return payload;
}

#define JNI_GenerateCancelProducerPayload "(JLjava/lang/String;Ljava/lang/String;)Ljava/lang/String;"

static jstring JNICALL GenerateCancelProducerPayload(JNIEnv *env, jobject clazz, jlong jProxy,
                                                     jstring jPublicKey,
                                                     jstring jPayPasswd) {
    bool exception = false;
    std::string msgException;
    jstring payload = NULL;

    const char *publicKey = env->GetStringUTFChars(jPublicKey, NULL);
    const char *payPasswd = env->GetStringUTFChars(jPayPasswd, NULL);

    try {
        IMainchainSubWallet *wallet = (IMainchainSubWallet *) jProxy;
        nlohmann::json payloadJson = wallet->GenerateCancelProducerPayload(publicKey, payPasswd);
        payload = env->NewStringUTF(payloadJson.dump().c_str());
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jPublicKey, publicKey);
    env->ReleaseStringUTFChars(jPayPasswd, payPasswd);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return payload;
}

#define JNI_CreateRegisterProducerTransaction "(JLjava/lang/String;Ljava/lang/String;JLjava/lang/String;Z)Ljava/lang/String;"

static jstring JNICALL CreateRegisterProducerTransaction(JNIEnv *env, jobject clazz, jlong jProxy,
                                                         jstring jFromAddress,
                                                         jstring jPayloadJson,
                                                         jlong amount,
                                                         jstring jMemo,
                                                         jboolean useVotedUTXO) {
    bool exception = false;
    std::string msgException;
    jstring tx = NULL;

    const char *fromAddress = env->GetStringUTFChars(jFromAddress, NULL);
    const char *payloadJson = env->GetStringUTFChars(jPayloadJson, NULL);
    const char *memo = env->GetStringUTFChars(jMemo, NULL);

    try {
        IMainchainSubWallet *wallet = (IMainchainSubWallet *) jProxy;
        nlohmann::json payload = nlohmann::json::parse(payloadJson);
        nlohmann::json txJson = wallet->CreateRegisterProducerTransaction(fromAddress, payload,
                                                                          amount, memo, useVotedUTXO);
        tx = env->NewStringUTF(txJson.dump().c_str());
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jFromAddress, fromAddress);
    env->ReleaseStringUTFChars(jPayloadJson, payloadJson);
    env->ReleaseStringUTFChars(jMemo, memo);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return tx;
}

#define JNI_CreateUpdateProducerTransaction "(JLjava/lang/String;Ljava/lang/String;Ljava/lang/String;Z)Ljava/lang/String;"

static jstring JNICALL CreateUpdateProducerTransaction(JNIEnv *env, jobject clazz, jlong jProxy,
                                                       jstring jFromAddress,
                                                       jstring jPayloadJson,
                                                       jstring jMemo,
                                                       jboolean useVotedUTXO) {
    bool exception = false;
    std::string msgException;
    jstring tx = NULL;

    const char *fromAddress = env->GetStringUTFChars(jFromAddress, NULL);
    const char *payloadJson = env->GetStringUTFChars(jPayloadJson, NULL);
    const char *memo = env->GetStringUTFChars(jMemo, NULL);

    try {
        IMainchainSubWallet *wallet = (IMainchainSubWallet *) jProxy;
        nlohmann::json payload = nlohmann::json::parse(payloadJson);
        nlohmann::json txJson = wallet->CreateUpdateProducerTransaction(fromAddress, payload, memo,
                                                                        useVotedUTXO);
        tx = env->NewStringUTF(txJson.dump().c_str());
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jFromAddress, fromAddress);
    env->ReleaseStringUTFChars(jPayloadJson, payloadJson);
    env->ReleaseStringUTFChars(jMemo, memo);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return tx;
}

#define JNI_CreateCancelProducerTransaction "(JLjava/lang/String;Ljava/lang/String;Ljava/lang/String;Z)Ljava/lang/String;"

static jstring JNICALL CreateCancelProducerTransaction(JNIEnv *env, jobject clazz, jlong jProxy,
                                                       jstring jFromAddress,
                                                       jstring jPayloadJson,
                                                       jstring jMemo,
                                                       jboolean useVotedUTXO) {
    bool exception = false;
    std::string msgException;
    jstring tx = NULL;

    const char *fromAddress = env->GetStringUTFChars(jFromAddress, NULL);
    const char *payloadJson = env->GetStringUTFChars(jPayloadJson, NULL);
    const char *memo = env->GetStringUTFChars(jMemo, NULL);

    try {
        IMainchainSubWallet *wallet = (IMainchainSubWallet *) jProxy;
        nlohmann::json payload = nlohmann::json::parse(payloadJson);
        nlohmann::json txJson = wallet->CreateCancelProducerTransaction(fromAddress, payload, memo,
                                                                        useVotedUTXO);
        tx = env->NewStringUTF(txJson.dump().c_str());
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jFromAddress, fromAddress);
    env->ReleaseStringUTFChars(jPayloadJson, payloadJson);
    env->ReleaseStringUTFChars(jMemo, memo);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return tx;
}

#define JNI_CreateRetrieveDepositTransaction "(JJLjava/lang/String;)Ljava/lang/String;"

static jstring JNICALL CreateRetrieveDepositTransaction(JNIEnv *env, jobject clazz, jlong jProxy,
                                                        jlong amount,
                                                        jstring jMemo) {
    bool exception = false;
    std::string msgException;
    jstring tx = NULL;

    const char *memo = env->GetStringUTFChars(jMemo, NULL);

    try {
        IMainchainSubWallet *wallet = (IMainchainSubWallet *) jProxy;
        nlohmann::json txJson = wallet->CreateRetrieveDepositTransaction(amount, memo);
        tx = env->NewStringUTF(txJson.dump().c_str());
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jMemo, memo);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return tx;
}

#define JNI_GetOwnerPublicKey "(J)Ljava/lang/String;"

static jstring JNICALL GetOwnerPublicKey(JNIEnv *env, jobject clazz, jlong jProxy) {
    bool exception = false;
    std::string msgException;
    jstring publicKey = NULL;

    try {
        IMainchainSubWallet *wallet = (IMainchainSubWallet *) jProxy;
        std::string pubKey = wallet->GetOwnerPublicKey();
        publicKey = env->NewStringUTF(pubKey.c_str());
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return publicKey;
}

#define JNI_CreateVoteProducerTransaction "(JLjava/lang/String;JLjava/lang/String;Ljava/lang/String;Z)Ljava/lang/String;"

static jstring JNICALL CreateVoteProducerTransaction(JNIEnv *env, jobject clazz, jlong jProxy,
                                                     jstring jfromAddress,
                                                     jlong stake,
                                                     jstring jPublicKeys,
                                                     jstring jMemo,
                                                     jboolean useVotedUTXO) {

    bool exception = false;
    std::string msgException;

    const char *fromAddress = env->GetStringUTFChars(jfromAddress, NULL);
    const char *publicKeys = env->GetStringUTFChars(jPublicKeys, NULL);
    const char *memo = env->GetStringUTFChars(jMemo, NULL);

    jstring tx = NULL;

    try {
        IMainchainSubWallet *wallet = (IMainchainSubWallet *) jProxy;
        nlohmann::json txJson = wallet->CreateVoteProducerTransaction(fromAddress, stake,
                                                                      nlohmann::json::parse(
                                                                              publicKeys), memo,
                                                                      useVotedUTXO);
        tx = env->NewStringUTF(txJson.dump().c_str());
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jfromAddress, fromAddress);
    env->ReleaseStringUTFChars(jPublicKeys, publicKeys);
    env->ReleaseStringUTFChars(jMemo, memo);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return tx;
}

#define JNI_GetVotedProducerList "(J)Ljava/lang/String;"

static jstring JNICALL GetVotedProducerList(JNIEnv *env, jobject clazz, jlong jSubWalletProxy) {
    bool exception = false;
    std::string msgException;

    IMainchainSubWallet *subWallet = (IMainchainSubWallet *) jSubWalletProxy;
    jstring list = NULL;

    try {
        nlohmann::json listJson = subWallet->GetVotedProducerList();
        list = env->NewStringUTF(listJson.dump().c_str());
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return list;
}

#define JNI_GetRegisteredProducerInfo "(J)Ljava/lang/String;"

static jstring JNICALL
GetRegisteredProducerInfo(JNIEnv *env, jobject clazz, jlong jSubWalletProxy) {
    bool exception = false;
    jstring info = NULL;
    std::string msgException;

    IMainchainSubWallet *subWallet = (IMainchainSubWallet *) jSubWalletProxy;

    try {
        nlohmann::json infoJson = subWallet->GetRegisteredProducerInfo();
        info = env->NewStringUTF(infoJson.dump().c_str());
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return info;
}

#define JNI_GetOwnerAddress "(J)Ljava/lang/String;"

static jstring JNICALL GetOwnerAddress(JNIEnv *env, jobject clazz, jlong jSubWalletProxy) {
    bool exception = false;
    std::string msgException;
    jstring ownerAddress = NULL;

    try {
        IMainchainSubWallet *subWallet = (IMainchainSubWallet *) jSubWalletProxy;
        std::string address = subWallet->GetOwnerAddress();
        ownerAddress = env->NewStringUTF(address.c_str());

    }catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }
    return ownerAddress;
}


static const JNINativeMethod methods[] = {
        REGISTER_METHOD(CreateDepositTransaction),
        REGISTER_METHOD(GenerateProducerPayload),
        REGISTER_METHOD(GenerateCancelProducerPayload),
        REGISTER_METHOD(CreateRegisterProducerTransaction),
        REGISTER_METHOD(CreateUpdateProducerTransaction),
        REGISTER_METHOD(CreateCancelProducerTransaction),
        REGISTER_METHOD(CreateRetrieveDepositTransaction),
        REGISTER_METHOD(GetOwnerPublicKey),
        REGISTER_METHOD(CreateVoteProducerTransaction),
        REGISTER_METHOD(GetVotedProducerList),
        REGISTER_METHOD(GetRegisteredProducerInfo),
        REGISTER_METHOD(GetOwnerAddress),
};

jint RegisterMainchainSubWallet(JNIEnv *env, const std::string &path) {
    return RegisterNativeMethods(env, path + "MainchainSubWallet", methods, NELEM(methods));
}



