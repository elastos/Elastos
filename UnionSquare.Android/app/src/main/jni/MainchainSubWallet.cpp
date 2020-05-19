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

#define JNI_GetVoteInfo "(JLjava/lang/String;)Ljava/lang/String;"

static jstring JNICALL GetVoteInfo(JNIEnv *env, jobject clazz, jlong jSubWalletProxy,
        jstring jtype) {
    bool exception = false;
    std::string msgException;

    const char *type = env->GetStringUTFChars(jtype, NULL);

    IMainchainSubWallet *subWallet = (IMainchainSubWallet *) jSubWalletProxy;
    jstring list = NULL;

    try {
        nlohmann::json listJson = subWallet->GetVoteInfo(type);
        list = env->NewStringUTF(listJson.dump().c_str());
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jtype, type);

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

#define JNI_GenerateCRInfoPayload "(JLjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;J)Ljava/lang/String;"

static jstring JNICALL GenerateCRInfoPayload(JNIEnv *env, jobject clazz, jlong jProxy,
                                             jstring jCRPublicKey,
                                             jstring jdid,
                                             jstring jNickName,
                                             jstring jURL,
                                             jlong location) {
    bool exception = false;
    std::string msgException;
    jstring payload = NULL;

    const char *publicKey = env->GetStringUTFChars(jCRPublicKey, NULL);
    const char *did = env->GetStringUTFChars(jdid, NULL);
    const char *nickName = env->GetStringUTFChars(jNickName, NULL);
    const char *url = env->GetStringUTFChars(jURL, NULL);

    try {
        IMainchainSubWallet *wallet = (IMainchainSubWallet *) jProxy;
        nlohmann::json payloadJson = wallet->GenerateCRInfoPayload(publicKey, did, nickName, url,
                                                                   location);
        payload = env->NewStringUTF(payloadJson.dump().c_str());
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jCRPublicKey, publicKey);
    env->ReleaseStringUTFChars(jdid, did);
    env->ReleaseStringUTFChars(jNickName, nickName);
    env->ReleaseStringUTFChars(jURL, url);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return payload;
}

#define JNI_GenerateUnregisterCRPayload "(JLjava/lang/String;)Ljava/lang/String;"

static jstring JNICALL GenerateUnregisterCRPayload(JNIEnv *env, jobject clazz, jlong jProxy,
                                                   jstring jCID) {
    bool exception = false;
    std::string msgException;
    jstring payload = NULL;

    const char *cID = env->GetStringUTFChars(jCID, NULL);
    try {
        IMainchainSubWallet *wallet = (IMainchainSubWallet *) jProxy;
        nlohmann::json payloadJson = wallet->GenerateUnregisterCRPayload(cID);
        payload = env->NewStringUTF(payloadJson.dump().c_str());
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jCID, cID);

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

#define JNI_ProposalOwnerDigest "(JLjava/lang/String;)Ljava/lang/String;"

static jstring JNICALL ProposalOwnerDigest(JNIEnv *env, jobject clazz, jlong jSubWalletProxy,
                                             jstring jpayload) {
    bool exception = false;
    std::string msgException;
    jstring result = NULL;

    const char *payload = env->GetStringUTFChars(jpayload, NULL);

    IMainchainSubWallet *subWallet = (IMainchainSubWallet *) jSubWalletProxy;

    try {
        std::string digest = subWallet->ProposalOwnerDigest(nlohmann::json::parse(payload));
        result = env->NewStringUTF(digest.c_str());
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jpayload, payload);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return result;
}

#define JNI_ProposalCRCouncilMemberDigest "(JLjava/lang/String;)Ljava/lang/String;"

static jstring JNICALL ProposalCRCouncilMemberDigest(JNIEnv *env, jobject clazz, jlong jSubWalletProxy,
                                               jstring jpayload) {
    bool exception = false;
    std::string msgException;
    jstring result = NULL;

    const char *payload = env->GetStringUTFChars(jpayload, NULL);

    IMainchainSubWallet *subWallet = (IMainchainSubWallet *) jSubWalletProxy;

    try {
        std::string digest = subWallet->ProposalCRCouncilMemberDigest(nlohmann::json::parse(payload));
        result = env->NewStringUTF(digest.c_str());
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jpayload, payload);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return result;
}

#define JNI_CalculateProposalHash "(JLjava/lang/String;)Ljava/lang/String;"

static jstring JNICALL
CalculateProposalHash(JNIEnv *env, jobject clazz, jlong jSubWalletProxy,
                          jstring jpayload) {
    bool exception = false;
    std::string msgException;
    jstring result = NULL;

    const char *payload = env->GetStringUTFChars(jpayload, NULL);

    IMainchainSubWallet *subWallet = (IMainchainSubWallet *) jSubWalletProxy;

    try {
        std::string hash = subWallet->CalculateProposalHash(nlohmann::json::parse(payload));
        result = env->NewStringUTF(hash.c_str());
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jpayload, payload);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return result;
}

#define JNI_CreateProposalTransaction "(JLjava/lang/String;Ljava/lang/String;)Ljava/lang/String;"

static jstring JNICALL
CreateProposalTransaction(JNIEnv *env, jobject clazz, jlong jSubWalletProxy,
                             jstring jpayload,
                             jstring jmemo) {
    bool exception = false;
    std::string msgException;
    jstring result = NULL;

    const char *payload = env->GetStringUTFChars(jpayload, NULL);
    const char *memo = env->GetStringUTFChars(jmemo, NULL);

    IMainchainSubWallet *subWallet = (IMainchainSubWallet *) jSubWalletProxy;

    try {
        nlohmann::json j = subWallet->CreateProposalTransaction(nlohmann::json::parse(payload), memo);
        result = env->NewStringUTF(j.dump().c_str());
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jpayload, payload);
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

#define JNI_ProposalReviewDigest "(JLjava/lang/String;)Ljava/lang/String;"

static jstring JNICALL ProposalReviewDigest(JNIEnv *env, jobject clazz, jlong jSubWalletProxy,
                          jstring jpayload) {

    bool exception = false;
    std::string msgException;
    jstring result = NULL;

    const char *payload = env->GetStringUTFChars(jpayload, NULL);

    IMainchainSubWallet *subWallet = (IMainchainSubWallet *) jSubWalletProxy;

    try {
        std::string digest = subWallet->ProposalReviewDigest(nlohmann::json::parse(payload));
        result = env->NewStringUTF(digest.c_str());
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jpayload, payload);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return result;
}

#define JNI_CreateProposalReviewTransaction "(JLjava/lang/String;Ljava/lang/String;)Ljava/lang/String;"

static jstring JNICALL
CreateProposalReviewTransaction(JNIEnv *env, jobject clazz, jlong jSubWalletProxy,
                                   jstring jpayload,
                                   jstring jmemo) {
    bool exception = false;
    std::string msgException;
    jstring result = NULL;

    const char *payload = env->GetStringUTFChars(jpayload, NULL);
    const char *memo = env->GetStringUTFChars(jmemo, NULL);

    IMainchainSubWallet *subWallet = (IMainchainSubWallet *) jSubWalletProxy;

    try {
        nlohmann::json j = subWallet->CreateProposalReviewTransaction(
                nlohmann::json::parse(payload), memo);
        result = env->NewStringUTF(j.dump().c_str());
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jpayload, payload);
    env->ReleaseStringUTFChars(jmemo, memo);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return result;
}

#define JNI_ProposalTrackingOwnerDigest "(JLjava/lang/String;)Ljava/lang/String;"

static jstring JNICALL
ProposalTrackingOwnerDigest(JNIEnv *env, jobject clazz, jlong jSubWalletProxy,
                          jstring jpayload) {
    bool exception = false;
    std::string msgException;
    jstring result = NULL;

    const char *payload = env->GetStringUTFChars(jpayload, NULL);

    IMainchainSubWallet *subWallet = (IMainchainSubWallet *) jSubWalletProxy;

    try {
        std::string digest = subWallet->ProposalTrackingOwnerDigest(nlohmann::json::parse(payload));
        result = env->NewStringUTF(digest.c_str());
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jpayload, payload);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return result;
}

#define JNI_ProposalTrackingNewOwnerDigest "(JLjava/lang/String;)Ljava/lang/String;"

static jstring JNICALL
ProposalTrackingNewOwnerDigest(JNIEnv *env, jobject clazz, jlong jSubWalletProxy,
        jstring jpayload) {
    bool exception = false;
    std::string msgException;
    jstring result = NULL;

    const char *payload = env->GetStringUTFChars(jpayload, NULL);

    IMainchainSubWallet *subWallet = (IMainchainSubWallet *) jSubWalletProxy;

    try {
        std::string digest = subWallet->ProposalTrackingNewOwnerDigest(nlohmann::json::parse(payload));
        result = env->NewStringUTF(digest.c_str());
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jpayload, payload);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return result;
}

#define JNI_ProposalTrackingSecretaryDigest "(JLjava/lang/String;)Ljava/lang/String;"

static jstring JNICALL
ProposalTrackingSecretaryDigest(JNIEnv *env, jobject clazz, jlong jSubWalletProxy,
                             jstring jpayload) {
    bool exception = false;
    std::string msgException;
    jstring result = NULL;

    const char *payload = env->GetStringUTFChars(jpayload, NULL);

    IMainchainSubWallet *subWallet = (IMainchainSubWallet *) jSubWalletProxy;

    try {
        std::string digest = subWallet->ProposalTrackingSecretaryDigest(nlohmann::json::parse(payload));
        result = env->NewStringUTF(digest.c_str());
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jpayload, payload);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return result;
}

#define JNI_CreateProposalTrackingTransaction "(JLjava/lang/String;Ljava/lang/String;)Ljava/lang/String;"

static jstring JNICALL
CreateProposalTrackingTransaction(JNIEnv *env, jobject clazz, jlong jSubWalletProxy,
                                   jstring jpayload,
                                   jstring jmemo) {
    bool exception = false;
    std::string msgException;
    jstring result = NULL;

    const char *payload = env->GetStringUTFChars(jpayload, NULL);
    const char *memo = env->GetStringUTFChars(jmemo, NULL);

    IMainchainSubWallet *subWallet = (IMainchainSubWallet *) jSubWalletProxy;

    try {
        nlohmann::json j = subWallet->CreateProposalTrackingTransaction(nlohmann::json::parse(payload), memo);
        result = env->NewStringUTF(j.dump().c_str());
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jpayload, payload);
    env->ReleaseStringUTFChars(jmemo, memo);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return result;
}

#define JNI_ProposalWithdrawDigest "(JLjava/lang/String;)Ljava/lang/String;"

static jstring JNICALL
ProposalWithdrawDigest(JNIEnv *env, jobject clazz, jlong jSubWalletProxy,
                                jstring jpayload) {
    bool exception = false;
    std::string msgException;
    jstring result = NULL;

    const char *payload = env->GetStringUTFChars(jpayload, NULL);

    IMainchainSubWallet *subWallet = (IMainchainSubWallet *) jSubWalletProxy;

    try {
        std::string digest = subWallet->ProposalWithdrawDigest(nlohmann::json::parse(payload));
        result = env->NewStringUTF(digest.c_str());
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jpayload, payload);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return result;
}

#define JNI_CreateProposalWithdrawTransaction "(JLjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;"

static jstring JNICALL
CreateProposalWithdrawTransaction(JNIEnv *env, jobject clazz, jlong jSubWalletProxy,
                                  jstring jrecipient,
                                  jstring jamount,
                                  jstring jutxo,
                                  jstring jpayload,
                                  jstring jmemo) {
    bool exception = false;
    std::string msgException;
    jstring result = NULL;

    const char *recipient = env->GetStringUTFChars(jrecipient, NULL);
    const char *amount = env->GetStringUTFChars(jamount, NULL);
    const char *utxo = env->GetStringUTFChars(jutxo, NULL);
    const char *payload = env->GetStringUTFChars(jpayload, NULL);
    const char *memo = env->GetStringUTFChars(jmemo, NULL);

    IMainchainSubWallet *subWallet = (IMainchainSubWallet *) jSubWalletProxy;

    try {
        nlohmann::json j = subWallet->CreateProposalWithdrawTransaction(recipient, amount,
                                                                        nlohmann::json::parse(utxo),
                                                                        nlohmann::json::parse(payload), memo);
        result = env->NewStringUTF(j.dump().c_str());
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jrecipient, recipient);
    env->ReleaseStringUTFChars(jamount, amount);
    env->ReleaseStringUTFChars(jutxo, utxo);
    env->ReleaseStringUTFChars(jpayload, payload);
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
        REGISTER_METHOD(GetVoteInfo),
        REGISTER_METHOD(GetRegisteredCRInfo),
        REGISTER_METHOD(ProposalOwnerDigest),
        REGISTER_METHOD(ProposalCRCouncilMemberDigest),
        REGISTER_METHOD(CalculateProposalHash),
        REGISTER_METHOD(CreateProposalTransaction),
        REGISTER_METHOD(CreateVoteCRCProposalTransaction),
        REGISTER_METHOD(CreateImpeachmentCRCTransaction),
        REGISTER_METHOD(ProposalReviewDigest),
        REGISTER_METHOD(CreateProposalReviewTransaction),
        REGISTER_METHOD(ProposalTrackingOwnerDigest),
        REGISTER_METHOD(ProposalTrackingNewOwnerDigest),
        REGISTER_METHOD(ProposalTrackingSecretaryDigest),
        REGISTER_METHOD(CreateProposalTrackingTransaction),
        REGISTER_METHOD(ProposalWithdrawDigest),
        REGISTER_METHOD(CreateProposalWithdrawTransaction),
};

jint RegisterMainchainSubWallet(JNIEnv *env, const std::string &path) {
    return RegisterNativeMethods(env, path + "MainchainSubWallet", methods, NELEM(methods));
}



