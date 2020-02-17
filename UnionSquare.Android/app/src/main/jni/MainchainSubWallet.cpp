// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Utils.h"
#include "IMainchainSubWallet.h"
#include "nlohmann/json.hpp"

using namespace Elastos::ElaWallet;

#define JNI_CreateDepositTransaction "(JLjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;"

static jstring JNICALL CreateDepositTransaction(JNIEnv *env, jobject clazz, jlong instance,
                                                jstring jfromAddress,
                                                jstring jsideChainID,
                                                jstring jamount,
                                                jstring jsideChainAddress,
                                                jstring jmemo) {
    bool exception = false;
    std::string msgException;

    const char *fromAddress = env->GetStringUTFChars(jfromAddress, NULL);
    const char *sideChainID = env->GetStringUTFChars(jsideChainID, NULL);
    const char *amount = env->GetStringUTFChars(jamount, NULL);
    const char *sideChainAddress = env->GetStringUTFChars(jsideChainAddress, NULL);
    const char *memo = env->GetStringUTFChars(jmemo, NULL);

    IMainchainSubWallet *wallet = (IMainchainSubWallet *) instance;
    jstring tx = NULL;

    try {
        nlohmann::json txJson = wallet->CreateDepositTransaction(fromAddress, sideChainID, amount,
                                                                 sideChainAddress, memo);
        tx = env->NewStringUTF(txJson.dump().c_str());
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jfromAddress, fromAddress);
    env->ReleaseStringUTFChars(jsideChainID, sideChainID);
    env->ReleaseStringUTFChars(jamount, amount);
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

#define JNI_CreateRegisterProducerTransaction "(JLjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;"

static jstring JNICALL CreateRegisterProducerTransaction(JNIEnv *env, jobject clazz, jlong jProxy,
                                                         jstring jFromAddress,
                                                         jstring jPayloadJson,
                                                         jstring jamount,
                                                         jstring jMemo) {
    bool exception = false;
    std::string msgException;
    jstring tx = NULL;

    const char *fromAddress = env->GetStringUTFChars(jFromAddress, NULL);
    const char *payloadJson = env->GetStringUTFChars(jPayloadJson, NULL);
    const char *amount = env->GetStringUTFChars(jamount, NULL);
    const char *memo = env->GetStringUTFChars(jMemo, NULL);

    try {
        IMainchainSubWallet *wallet = (IMainchainSubWallet *) jProxy;
        nlohmann::json payload = nlohmann::json::parse(payloadJson);
        nlohmann::json txJson = wallet->CreateRegisterProducerTransaction(fromAddress, payload,
                                                                          amount, memo);
        tx = env->NewStringUTF(txJson.dump().c_str());
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jFromAddress, fromAddress);
    env->ReleaseStringUTFChars(jPayloadJson, payloadJson);
    env->ReleaseStringUTFChars(jamount, amount);
    env->ReleaseStringUTFChars(jMemo, memo);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return tx;
}

#define JNI_CreateUpdateProducerTransaction "(JLjava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;"

static jstring JNICALL CreateUpdateProducerTransaction(JNIEnv *env, jobject clazz, jlong jProxy,
                                                       jstring jFromAddress,
                                                       jstring jPayloadJson,
                                                       jstring jMemo) {
    bool exception = false;
    std::string msgException;
    jstring tx = NULL;

    const char *fromAddress = env->GetStringUTFChars(jFromAddress, NULL);
    const char *payloadJson = env->GetStringUTFChars(jPayloadJson, NULL);
    const char *memo = env->GetStringUTFChars(jMemo, NULL);

    try {
        IMainchainSubWallet *wallet = (IMainchainSubWallet *) jProxy;
        nlohmann::json payload = nlohmann::json::parse(payloadJson);
        nlohmann::json txJson = wallet->CreateUpdateProducerTransaction(fromAddress, payload, memo);
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

#define JNI_CreateCancelProducerTransaction "(JLjava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;"

static jstring JNICALL CreateCancelProducerTransaction(JNIEnv *env, jobject clazz, jlong jProxy,
                                                       jstring jFromAddress,
                                                       jstring jPayloadJson,
                                                       jstring jMemo) {
    bool exception = false;
    std::string msgException;
    jstring tx = NULL;

    const char *fromAddress = env->GetStringUTFChars(jFromAddress, NULL);
    const char *payloadJson = env->GetStringUTFChars(jPayloadJson, NULL);
    const char *memo = env->GetStringUTFChars(jMemo, NULL);

    try {
        IMainchainSubWallet *wallet = (IMainchainSubWallet *) jProxy;
        nlohmann::json payload = nlohmann::json::parse(payloadJson);
        nlohmann::json txJson = wallet->CreateCancelProducerTransaction(fromAddress, payload, memo);
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

#define JNI_CreateRetrieveDepositTransaction "(JLjava/lang/String;Ljava/lang/String;)Ljava/lang/String;"

static jstring JNICALL CreateRetrieveDepositTransaction(JNIEnv *env, jobject clazz, jlong jProxy,
                                                        jstring jamount,
                                                        jstring jMemo) {
    bool exception = false;
    std::string msgException;
    jstring tx = NULL;

    const char *memo = env->GetStringUTFChars(jMemo, NULL);
    const char *amount = env->GetStringUTFChars(jamount, NULL);

    try {
        IMainchainSubWallet *wallet = (IMainchainSubWallet *) jProxy;
        nlohmann::json txJson = wallet->CreateRetrieveDepositTransaction(amount, memo);
        tx = env->NewStringUTF(txJson.dump().c_str());
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jMemo, memo);
    env->ReleaseStringUTFChars(jamount, amount);

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

#define JNI_CreateVoteProducerTransaction "(JLjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;"

static jstring JNICALL CreateVoteProducerTransaction(JNIEnv *env, jobject clazz, jlong jProxy,
                                                     jstring jfromAddress,
                                                     jstring jstake,
                                                     jstring jPublicKeys,
                                                     jstring jMemo,
                                                     jstring jinvalidCandidates) {

    bool exception = false;
    std::string msgException;

    const char *fromAddress = env->GetStringUTFChars(jfromAddress, NULL);
    const char *stake = env->GetStringUTFChars(jstake, NULL);
    const char *publicKeys = env->GetStringUTFChars(jPublicKeys, NULL);
    const char *memo = env->GetStringUTFChars(jMemo, NULL);
    const char *invalidCandidates = env->GetStringUTFChars(jinvalidCandidates, NULL);

    jstring tx = NULL;

    try {
        IMainchainSubWallet *wallet = (IMainchainSubWallet *) jProxy;
        nlohmann::json txJson = wallet->CreateVoteProducerTransaction(fromAddress, stake,
                                                                      nlohmann::json::parse(
                                                                              publicKeys), memo,
                                                                      nlohmann::json::parse(
                                                                              invalidCandidates));
        tx = env->NewStringUTF(txJson.dump().c_str());
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jfromAddress, fromAddress);
    env->ReleaseStringUTFChars(jstake, stake);
    env->ReleaseStringUTFChars(jPublicKeys, publicKeys);
    env->ReleaseStringUTFChars(jMemo, memo);
    env->ReleaseStringUTFChars(jinvalidCandidates, invalidCandidates);

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

    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }
    return ownerAddress;
}

#define JNI_GenerateCRInfoPayload "(JLjava/lang/String;Ljava/lang/String;Ljava/lang/String;J)Ljava/lang/String;"

static jstring JNICALL GenerateCRInfoPayload(JNIEnv *env, jobject clazz, jlong jProxy,
                                             jstring jCRPublicKey,
                                             jstring jNickName,
                                             jstring jURL,
                                             jlong location) {
    bool exception = false;
    std::string msgException;
    jstring payload = NULL;

    const char *publicKey = env->GetStringUTFChars(jCRPublicKey, NULL);
    const char *nickName = env->GetStringUTFChars(jNickName, NULL);
    const char *url = env->GetStringUTFChars(jURL, NULL);

    try {
        IMainchainSubWallet *wallet = (IMainchainSubWallet *) jProxy;
        nlohmann::json payloadJson = wallet->GenerateCRInfoPayload(publicKey, nickName, url,
                                                                   location);
        payload = env->NewStringUTF(payloadJson.dump().c_str());
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jCRPublicKey, publicKey);
    env->ReleaseStringUTFChars(jNickName, nickName);
    env->ReleaseStringUTFChars(jURL, url);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return payload;
}

#define JNI_GenerateUnregisterCRPayload "(JLjava/lang/String;)Ljava/lang/String;"

static jstring JNICALL GenerateUnregisterCRPayload(JNIEnv *env, jobject clazz, jlong jProxy,
                                                   jstring jCRDID) {
    bool exception = false;
    std::string msgException;
    jstring payload = NULL;

    const char *crDID = env->GetStringUTFChars(jCRDID, NULL);
    try {
        IMainchainSubWallet *wallet = (IMainchainSubWallet *) jProxy;
        nlohmann::json payloadJson = wallet->GenerateUnregisterCRPayload(crDID);
        payload = env->NewStringUTF(payloadJson.dump().c_str());
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jCRDID, crDID);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return payload;
}

#define JNI_CreateRegisterCRTransaction "(JLjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;"

static jstring JNICALL CreateRegisterCRTransaction(JNIEnv *env, jobject clazz, jlong jProxy,
                                                   jstring jfromAddress,
                                                   jstring jpayload,
                                                   jstring jamount,
                                                   jstring jmemo) {
    bool exception = false;
    std::string msgException;

    const char *fromAddress = env->GetStringUTFChars(jfromAddress, NULL);
    const char *payload = env->GetStringUTFChars(jpayload, NULL);
    const char *amount = env->GetStringUTFChars(jamount, NULL);
    const char *memo = env->GetStringUTFChars(jmemo, NULL);

    jstring tx = NULL;

    try {
        IMainchainSubWallet *wallet = (IMainchainSubWallet *) jProxy;
        nlohmann::json txJson = wallet->CreateRegisterCRTransaction(fromAddress,
                                                                    nlohmann::json::parse(payload),
                                                                    amount, memo);
        tx = env->NewStringUTF(txJson.dump().c_str());
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jfromAddress, fromAddress);
    env->ReleaseStringUTFChars(jpayload, payload);
    env->ReleaseStringUTFChars(jamount, amount);
    env->ReleaseStringUTFChars(jmemo, memo);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return tx;
}

#define JNI_CreateUpdateCRTransaction "(JLjava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;"

static jstring JNICALL CreateUpdateCRTransaction(JNIEnv *env, jobject clazz, jlong jProxy,
                                                 jstring jfromAddress,
                                                 jstring jpayload,
                                                 jstring jmemo) {
    bool exception = false;
    std::string msgException;

    const char *fromAddress = env->GetStringUTFChars(jfromAddress, NULL);
    const char *payload = env->GetStringUTFChars(jpayload, NULL);
    const char *memo = env->GetStringUTFChars(jmemo, NULL);

    jstring tx = NULL;

    try {
        IMainchainSubWallet *wallet = (IMainchainSubWallet *) jProxy;
        nlohmann::json txJson = wallet->CreateUpdateCRTransaction(fromAddress,
                                                                  nlohmann::json::parse(payload),
                                                                  memo);
        tx = env->NewStringUTF(txJson.dump().c_str());
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jfromAddress, fromAddress);
    env->ReleaseStringUTFChars(jpayload, payload);
    env->ReleaseStringUTFChars(jmemo, memo);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return tx;
}

#define JNI_CreateUnregisterCRTransaction "(JLjava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;"

static jstring JNICALL CreateUnregisterCRTransaction(JNIEnv *env, jobject clazz, jlong jProxy,
                                                     jstring jfromAddress,
                                                     jstring jpayload,
                                                     jstring jmemo) {
    bool exception = false;
    std::string msgException;

    const char *fromAddress = env->GetStringUTFChars(jfromAddress, NULL);
    const char *payload = env->GetStringUTFChars(jpayload, NULL);
    const char *memo = env->GetStringUTFChars(jmemo, NULL);

    jstring tx = NULL;

    try {
        IMainchainSubWallet *wallet = (IMainchainSubWallet *) jProxy;
        nlohmann::json txJson = wallet->CreateUnregisterCRTransaction(fromAddress,
                                                                      nlohmann::json::parse(
                                                                              payload),
                                                                      memo);
        tx = env->NewStringUTF(txJson.dump().c_str());
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jfromAddress, fromAddress);
    env->ReleaseStringUTFChars(jpayload, payload);
    env->ReleaseStringUTFChars(jmemo, memo);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return tx;
}

#define JNI_CreateRetrieveCRDepositTransaction "(JLjava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;"

static jstring JNICALL CreateRetrieveCRDepositTransaction(JNIEnv *env, jobject clazz, jlong jProxy,
                                                          jstring jcrPublicKey,
                                                          jstring jamount,
                                                          jstring jmemo) {
    bool exception = false;
    std::string msgException;

    const char *crPublicKey = env->GetStringUTFChars(jcrPublicKey, NULL);
    const char *amount = env->GetStringUTFChars(jamount, NULL);
    const char *memo = env->GetStringUTFChars(jmemo, NULL);

    jstring tx = NULL;

    try {
        IMainchainSubWallet *wallet = (IMainchainSubWallet *) jProxy;
        nlohmann::json txJson = wallet->CreateRetrieveCRDepositTransaction(crPublicKey, amount, memo);
        tx = env->NewStringUTF(txJson.dump().c_str());
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jcrPublicKey, crPublicKey);
    env->ReleaseStringUTFChars(jamount, amount);
    env->ReleaseStringUTFChars(jmemo, memo);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return tx;
}

#define JNI_CreateVoteCRTransaction "(JLjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;"

static jstring JNICALL CreateVoteCRTransaction(JNIEnv *env, jobject clazz, jlong jProxy,
                                               jstring jfromAddress,
                                               jstring jvotes,
                                               jstring jmemo,
                                               jstring jinvalidCandidates) {
    bool exception = false;
    std::string msgException;

    const char *fromAddress = env->GetStringUTFChars(jfromAddress, NULL);
    const char *votes = env->GetStringUTFChars(jvotes, NULL);
    const char *memo = env->GetStringUTFChars(jmemo, NULL);
    const char *invalidCandidates = env->GetStringUTFChars(jinvalidCandidates, NULL);

    jstring tx = NULL;

    try {
        IMainchainSubWallet *wallet = (IMainchainSubWallet *) jProxy;
        nlohmann::json txJson = wallet->CreateVoteCRTransaction(fromAddress,
                                                                nlohmann::json::parse(votes),
                                                                memo,
                                                                nlohmann::json::parse(
                                                                        invalidCandidates));
        tx = env->NewStringUTF(txJson.dump().c_str());
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jfromAddress, fromAddress);
    env->ReleaseStringUTFChars(jvotes, votes);
    env->ReleaseStringUTFChars(jmemo, memo);
    env->ReleaseStringUTFChars(jinvalidCandidates, invalidCandidates);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return tx;
}

#define JNI_GetVotedCRList "(J)Ljava/lang/String;"

static jstring JNICALL GetVotedCRList(JNIEnv *env, jobject clazz, jlong jSubWalletProxy) {
    bool exception = false;
    std::string msgException;

    IMainchainSubWallet *subWallet = (IMainchainSubWallet *) jSubWalletProxy;
    jstring list = NULL;

    try {
        nlohmann::json listJson = subWallet->GetVotedCRList();
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

#define JNI_GetRegisteredCRInfo "(J)Ljava/lang/String;"

static jstring JNICALL
GetRegisteredCRInfo(JNIEnv *env, jobject clazz, jlong jSubWalletProxy) {
    bool exception = false;
    jstring info = NULL;
    std::string msgException;

    IMainchainSubWallet *subWallet = (IMainchainSubWallet *) jSubWalletProxy;

    try {
        nlohmann::json infoJson = subWallet->GetRegisteredCRInfo();
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

#define JNI_SponsorProposalDigest "(JCLjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;"

static jstring JNICALL SponsorProposalDigest(JNIEnv *env, jobject clazz, jlong jSubWalletProxy,
                                             jchar jtype,
                                             jstring jcategoryData,
                                             jstring jsponsorPublickey,
                                             jstring jdraftHash,
                                             jstring jbudgets,
                                             jstring jrecipient) {
    bool exception = false;
    std::string msgException;
    jstring result = NULL;

    const char *categoryData = env->GetStringUTFChars(jcategoryData, NULL);
    const char *sponsorPublickey = env->GetStringUTFChars(jsponsorPublickey, NULL);
    const char *draftHash = env->GetStringUTFChars(jdraftHash, NULL);
    const char *budgets = env->GetStringUTFChars(jbudgets, NULL);
    const char *recipent = env->GetStringUTFChars(jrecipient, NULL);

    IMainchainSubWallet *subWallet = (IMainchainSubWallet *) jSubWalletProxy;

    try {
        nlohmann::json digest = subWallet->SponsorProposalDigest(jtype, categoryData, sponsorPublickey,
                                                              draftHash,
                                                              nlohmann::json::parse(budgets),
                                                              recipent);
        result = env->NewStringUTF(digest.dump().c_str());
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jcategoryData, categoryData);
    env->ReleaseStringUTFChars(jsponsorPublickey, sponsorPublickey);
    env->ReleaseStringUTFChars(jdraftHash, draftHash);
    env->ReleaseStringUTFChars(jbudgets, budgets);
    env->ReleaseStringUTFChars(jrecipient, recipent);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return result;
}

#define JNI_CRSponsorProposalDigest "(JLjava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;"

static jstring JNICALL CRSponsorProposalDigest(JNIEnv *env, jobject clazz, jlong jSubWalletProxy,
                                               jstring jsponsorSignedProposal,
                                               jstring jcrSponsorDID,
                                               jstring jcrOpinionHash) {
    bool exception = false;
    std::string msgException;
    jstring result = NULL;

    const char *sponsorSignedProposal = env->GetStringUTFChars(jsponsorSignedProposal, NULL);
    const char *crSponsorDID = env->GetStringUTFChars(jcrSponsorDID, NULL);
    const char *crOpinionHash = env->GetStringUTFChars(jcrOpinionHash, NULL);

    IMainchainSubWallet *subWallet = (IMainchainSubWallet *) jSubWalletProxy;

    try {
        nlohmann::json digest = subWallet->CRSponsorProposalDigest(nlohmann::json::parse(sponsorSignedProposal),
                                                                crSponsorDID, crOpinionHash);
        result = env->NewStringUTF(digest.dump().c_str());
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jsponsorSignedProposal, sponsorSignedProposal);
    env->ReleaseStringUTFChars(jcrSponsorDID, crSponsorDID);
    env->ReleaseStringUTFChars(jcrOpinionHash, crOpinionHash);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return result;
}

#define JNI_CreateCRCProposalTransaction "(JLjava/lang/String;Ljava/lang/String;)Ljava/lang/String;"

static jstring JNICALL
CreateCRCProposalTransaction(JNIEnv *env, jobject clazz, jlong jSubWalletProxy,
                             jstring jcrSignedProposal,
                             jstring jmemo) {
    bool exception = false;
    std::string msgException;
    jstring result = NULL;

    const char *crSignedProposal = env->GetStringUTFChars(jcrSignedProposal, NULL);
    const char *memo = env->GetStringUTFChars(jmemo, NULL);

    IMainchainSubWallet *subWallet = (IMainchainSubWallet *) jSubWalletProxy;

    try {
        nlohmann::json j = subWallet->CreateCRCProposalTransaction(nlohmann::json::parse(crSignedProposal),
                                                                   memo);
        result = env->NewStringUTF(j.dump().c_str());
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jcrSignedProposal, crSignedProposal);
    env->ReleaseStringUTFChars(jmemo, memo);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return result;
}

#define JNI_CreateVoteCRCProposalTransaction "(JLjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;"

static jstring JNICALL
CreateVoteCRCProposalTransaction(JNIEnv *env, jobject clazz, jlong jSubWalletProxy,
                                 jstring jfromAddress,
                                 jstring jvotes,
                                 jstring jmemo,
                                 jstring jinvalidCandidates) {

    bool exception = false;
    std::string msgException;
    jstring result = NULL;

    const char *fromAddress = env->GetStringUTFChars(jfromAddress, NULL);
    const char *votes = env->GetStringUTFChars(jvotes, NULL);
    const char *memo = env->GetStringUTFChars(jmemo, NULL);
    const char *invalidCandidates = env->GetStringUTFChars(jinvalidCandidates, NULL);

    IMainchainSubWallet *subWallet = (IMainchainSubWallet *) jSubWalletProxy;

    try {
        nlohmann::json j = subWallet->CreateVoteCRCProposalTransaction(fromAddress,
                                                                       nlohmann::json::parse(votes),
                                                                       memo,
                                                                       nlohmann::json::parse(
                                                                               invalidCandidates));
        result = env->NewStringUTF(j.dump().c_str());
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jfromAddress, fromAddress);
    env->ReleaseStringUTFChars(jvotes, votes);
    env->ReleaseStringUTFChars(jmemo, memo);
    env->ReleaseStringUTFChars(jinvalidCandidates, invalidCandidates);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return result;

}

#define JNI_CreateImpeachmentCRCTransaction "(JLjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;"

static jstring JNICALL
CreateImpeachmentCRCTransaction(JNIEnv *env, jobject clazz, jlong jSubWalletProxy,
                                jstring jfromAddress,
                                jstring jvotes,
                                jstring jmemo,
                                jstring jinvalidCandidates) {
    bool exception = false;
    std::string msgException;
    jstring result = NULL;

    const char *fromAddress = env->GetStringUTFChars(jfromAddress, NULL);
    const char *votes = env->GetStringUTFChars(jvotes, NULL);
    const char *memo = env->GetStringUTFChars(jmemo, NULL);
    const char *invalidCandidates = env->GetStringUTFChars(jinvalidCandidates, NULL);

    IMainchainSubWallet *subWallet = (IMainchainSubWallet *) jSubWalletProxy;

    try {
        nlohmann::json j = subWallet->CreateImpeachmentCRCTransaction(fromAddress,
                                                                      nlohmann::json::parse(votes),
                                                                      memo,
                                                                      nlohmann::json::parse(
                                                                              invalidCandidates));
        result = env->NewStringUTF(j.dump().c_str());
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jfromAddress, fromAddress);
    env->ReleaseStringUTFChars(jvotes, votes);
    env->ReleaseStringUTFChars(jmemo, memo);
    env->ReleaseStringUTFChars(jinvalidCandidates, invalidCandidates);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return result;
}

#define JNI_GenerateCRCProposalReview "(JLjava/lang/String;BLjava/lang/String;Ljava/lang/String;)Ljava/lang/String;"

static jstring JNICALL
GenerateCRCProposalReview(JNIEnv *env, jobject clazz, jlong jSubWalletProxy,
                          jstring jproposalHash,
                          jbyte voteResult,
                          jstring jcrDID) {

    bool exception = false;
    std::string msgException;
    jstring result = NULL;

    const char *proposalHash = env->GetStringUTFChars(jproposalHash, NULL);
    const char *crDID = env->GetStringUTFChars(jcrDID, NULL);

    IMainchainSubWallet *subWallet = (IMainchainSubWallet *) jSubWalletProxy;

    try {
        nlohmann::json j = subWallet->GenerateCRCProposalReview(proposalHash, voteResult, crDID);
        result = env->NewStringUTF(j.dump().c_str());
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jproposalHash, proposalHash);
    env->ReleaseStringUTFChars(jcrDID, crDID);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return result;
}

#define JNI_CreateCRCProposalReviewTransaction "(JLjava/lang/String;Ljava/lang/String;)Ljava/lang/String;"

static jstring JNICALL
CreateCRCProposalReviewTransaction(JNIEnv *env, jobject clazz, jlong jSubWalletProxy,
                                   jstring jproposalReview,
                                   jstring jmemo) {
    bool exception = false;
    std::string msgException;
    jstring result = NULL;

    const char *proposalReview = env->GetStringUTFChars(jproposalReview, NULL);
    const char *memo = env->GetStringUTFChars(jmemo, NULL);

    IMainchainSubWallet *subWallet = (IMainchainSubWallet *) jSubWalletProxy;

    try {
        nlohmann::json j = subWallet->CreateCRCProposalReviewTransaction(
                nlohmann::json::parse(proposalReview), memo);
        result = env->NewStringUTF(j.dump().c_str());
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jproposalReview, proposalReview);
    env->ReleaseStringUTFChars(jmemo, memo);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return result;
}

#define JNI_LeaderProposalTrackDigest "(JBLjava/lang/String;Ljava/lang/String;BLjava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;"

static jstring JNICALL
LeaderProposalTrackDigest(JNIEnv *env, jobject clazz, jlong jSubWalletProxy,
                          jbyte jtype,
                          jstring jproposalHash,
                          jstring jdocumentHash,
                          jbyte jstage,
                          jstring jappropriation,
                          jstring jleaderPubKey,
                          jstring jnewLeaderPubKey) {
    bool exception = false;
    std::string msgException;
    jstring result = NULL;

    const char *proposalHash = env->GetStringUTFChars(jproposalHash, NULL);
    const char *documentHash = env->GetStringUTFChars(jdocumentHash, NULL);
    const char *appropriation = env->GetStringUTFChars(jappropriation, NULL);
    const char *leaderPubKey = env->GetStringUTFChars(jleaderPubKey, NULL);
    const char *newLeaderPubKey = env->GetStringUTFChars(jnewLeaderPubKey, NULL);

    IMainchainSubWallet *subWallet = (IMainchainSubWallet *) jSubWalletProxy;

    try {
        nlohmann::json j = subWallet->LeaderProposalTrackDigest(jtype,proposalHash, documentHash,
                                                                jstage, appropriation, leaderPubKey,
                                                                newLeaderPubKey);
        result = env->NewStringUTF(j.dump().c_str());
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jproposalHash, proposalHash);
    env->ReleaseStringUTFChars(jdocumentHash, documentHash);
    env->ReleaseStringUTFChars(jappropriation, appropriation);
    env->ReleaseStringUTFChars(jleaderPubKey, leaderPubKey);
    env->ReleaseStringUTFChars(jnewLeaderPubKey, newLeaderPubKey);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return result;
}

#define JNI_NewLeaderProposalTrackDigest "(JLjava/lang/String;)Ljava/lang/String;"

static jstring JNICALL
NewLeaderProposalTrackDigest(JNIEnv *env, jobject clazz, jlong jSubWalletProxy,
        jstring jleaderSignedProposalTracking) {
    bool exception = false;
    std::string msgException;
    jstring result = NULL;

    const char *leaderSignedProposalTracking = env->GetStringUTFChars(jleaderSignedProposalTracking,
            NULL);

    IMainchainSubWallet *subWallet = (IMainchainSubWallet *) jSubWalletProxy;

    try {
        nlohmann::json j = subWallet->NewLeaderProposalTrackDigest(leaderSignedProposalTracking);
        result = env->NewStringUTF(j.dump().c_str());
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jleaderSignedProposalTracking, leaderSignedProposalTracking);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return result;
}

#define JNI_SecretaryGeneralProposalTrackDigest "(JLjava/lang/String;)Ljava/lang/String;"

static jstring JNICALL
SecretaryGeneralProposalTrackDigest(JNIEnv *env, jobject clazz, jlong jSubWalletProxy,
                             jstring jleaderSignedProposalTracking) {
    bool exception = false;
    std::string msgException;
    jstring result = NULL;

    const char *leaderSignedProposalTracking = env->GetStringUTFChars(jleaderSignedProposalTracking,
                                                                      NULL);

    IMainchainSubWallet *subWallet = (IMainchainSubWallet *) jSubWalletProxy;

    try {
        nlohmann::json j = subWallet->SecretaryGeneralProposalTrackDigest(leaderSignedProposalTracking);
        result = env->NewStringUTF(j.dump().c_str());
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jleaderSignedProposalTracking, leaderSignedProposalTracking);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return result;
}

#define JNI_CreateProposalTrackingTransaction "(JLjava/lang/String;Ljava/lang/String;)Ljava/lang/String;"

static jstring JNICALL
CreateProposalTrackingTransaction(JNIEnv *env, jobject clazz, jlong jSubWalletProxy,
                                   jstring jsecretaryGeneralSignedPayload,
                                   jstring jmemo) {
    bool exception = false;
    std::string msgException;
    jstring result = NULL;

    const char *payload = env->GetStringUTFChars(jsecretaryGeneralSignedPayload, NULL);
    const char *memo = env->GetStringUTFChars(jmemo, NULL);

    IMainchainSubWallet *subWallet = (IMainchainSubWallet *) jSubWalletProxy;

    try {
        nlohmann::json j = subWallet->CreateProposalTrackingTransaction(
                nlohmann::json::parse(payload), memo);
        result = env->NewStringUTF(j.dump().c_str());
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jsecretaryGeneralSignedPayload, payload);
    env->ReleaseStringUTFChars(jmemo, memo);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return result;
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
        REGISTER_METHOD(GenerateCRInfoPayload),
        REGISTER_METHOD(GenerateUnregisterCRPayload),
        REGISTER_METHOD(CreateRegisterCRTransaction),
        REGISTER_METHOD(CreateUpdateCRTransaction),
        REGISTER_METHOD(CreateUnregisterCRTransaction),
        REGISTER_METHOD(CreateRetrieveCRDepositTransaction),
        REGISTER_METHOD(CreateVoteCRTransaction),
        REGISTER_METHOD(GetVotedCRList),
        REGISTER_METHOD(GetRegisteredCRInfo),
        REGISTER_METHOD(SponsorProposalDigest),
        REGISTER_METHOD(CRSponsorProposalDigest),
        REGISTER_METHOD(CreateCRCProposalTransaction),
        REGISTER_METHOD(CreateVoteCRCProposalTransaction),
        REGISTER_METHOD(CreateImpeachmentCRCTransaction),
        REGISTER_METHOD(GenerateCRCProposalReview),
        REGISTER_METHOD(CreateCRCProposalReviewTransaction),
        REGISTER_METHOD(LeaderProposalTrackDigest),
        REGISTER_METHOD(NewLeaderProposalTrackDigest),
        REGISTER_METHOD(SecretaryGeneralProposalTrackDigest),
        REGISTER_METHOD(CreateProposalTrackingTransaction),
};

jint RegisterMainchainSubWallet(JNIEnv *env, const std::string &path) {
    return RegisterNativeMethods(env, path + "MainchainSubWallet", methods, NELEM(methods));
}



