// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <sstream>
#include "Utils.h"
#include "ISubWallet.h"
#include "nlohmann/json.hpp"
#include "SubWalletCallback.h"

#define TAG "SubWallet"

using namespace Elastos::ElaWallet;

#define JNI_GetChainID "(J)Ljava/lang/String;"

static jstring JNICALL GetChainID(JNIEnv *env, jobject clazz, jlong jSubProxy) {
    jstring chainId = NULL;

    try {
        ISubWallet *subWallet = (ISubWallet *) jSubProxy;
        std::string result = subWallet->GetChainID();
        chainId = env->NewStringUTF(result.c_str());
    } catch (const std::exception &e) {
        ThrowWalletException(env, e.what());
    }

    return chainId;
}

#define JNI_GetBasicInfo "(J)Ljava/lang/String;"

static jstring JNICALL GetBasicInfo(JNIEnv *env, jobject clazz, jlong jSubProxy) {
    jstring info = NULL;

    try {
        ISubWallet *subWallet = (ISubWallet *) jSubProxy;
        nlohmann::json result = subWallet->GetBasicInfo();
        info = env->NewStringUTF(result.dump().c_str());
    } catch (const std::exception &e) {
        ThrowWalletException(env, e.what());
    }

    return info;
}

#define JNI_GetBalanceInfo "(J)Ljava/lang/String;"

static jstring JNICALL GetBalanceInfo(JNIEnv *env, jobject clazz, jlong jSubProxy) {
    jstring info = NULL;

    try {
        ISubWallet *subWallet = (ISubWallet *) jSubProxy;
        nlohmann::json result = subWallet->GetBalanceInfo();
        info = env->NewStringUTF(result.dump().c_str());
    } catch (const std::exception &e) {
        ThrowWalletException(env, e.what());
    }

    return info;
}

#define JNI_GetBalance "(J)Ljava/lang/String;"

static jstring JNICALL GetBalance(JNIEnv *env, jobject clazz, jlong jSubProxy) {
    jstring balance = NULL;

    try {
        ISubWallet *subWallet = (ISubWallet *) jSubProxy;
        std::string amount = subWallet->GetBalance();
        balance = env->NewStringUTF(amount.c_str());
    } catch (const std::exception &e) {
        ThrowWalletException(env, e.what());
    }

    return balance;
}

#define JNI_CreateAddress "(J)Ljava/lang/String;"

static jstring JNICALL CreateAddress(JNIEnv *env, jobject clazz, jlong jSubProxy) {
    jstring addr = NULL;

    try {
        ISubWallet *subWallet = (ISubWallet *) jSubProxy;
        std::string result = subWallet->CreateAddress();
        addr = env->NewStringUTF(result.c_str());
    } catch (const std::exception &e) {
        ThrowWalletException(env, e.what());
    }

    return addr;
}

#define JNI_GetAllAddress "(JII)Ljava/lang/String;"

static jstring JNICALL GetAllAddress(JNIEnv *env, jobject clazz, jlong jSubProxy,
                                     jint jStart,
                                     jint jCount) {
    jstring addresses = NULL;

    try {
        ISubWallet *subWallet = (ISubWallet *) jSubProxy;
        nlohmann::json addressesJson = subWallet->GetAllAddress(jStart, jCount);
        addresses = env->NewStringUTF(addressesJson.dump().c_str());
    } catch (const std::exception &e) {
        ThrowWalletException(env, e.what());
    }

    return addresses;
}

#define JNI_GetAllPublicKeys "(JII)Ljava/lang/String;"

static jstring JNICALL GetAllPublicKeys(JNIEnv *env, jobject clazz, jlong jSubProxy,
                                        jint jStart,
                                        jint jCount) {
    jstring publicKeys = NULL;

    try {
        ISubWallet *subWallet = (ISubWallet *) jSubProxy;
        nlohmann::json pubKeyJson = subWallet->GetAllPublicKeys(jStart, jCount);
        publicKeys = env->NewStringUTF(pubKeyJson.dump().c_str());
    } catch (const std::exception &e) {
        ThrowWalletException(env, e.what());
    }

    return publicKeys;
}

#define JNI_GetBalanceWithAddress "(JLjava/lang/String;)Ljava/lang/String;"

static jstring JNICALL GetBalanceWithAddress(JNIEnv *env, jobject clazz, jlong jSubProxy,
                                             jstring jaddress) {
    bool exception = false;
    std::string msgException;

    const char *address = env->GetStringUTFChars(jaddress, NULL);
    jstring result = NULL;

    try {
        ISubWallet *subWallet = (ISubWallet *) jSubProxy;
        std::string amount = subWallet->GetBalanceWithAddress(address);
        result = env->NewStringUTF(amount.c_str());
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jaddress, address);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return result;
}

#define JNI_AddCallback "(JJ)V"

static void JNICALL AddCallback(JNIEnv *env, jobject clazz, jlong jSubProxy,
                                jlong jSubWalletCallbackInstance) {

    try {
        ISubWallet *subWallet = (ISubWallet *) jSubProxy;
        SubWalletCallback *subWalletCallback = (SubWalletCallback *) jSubWalletCallbackInstance;

        subWallet->AddCallback(subWalletCallback);
    } catch (const std::exception &e) {
        ThrowWalletException(env, e.what());
    }
}

#define JNI_RemoveCallback "(J)V"

static void JNICALL RemoveCallback(JNIEnv *env, jobject clazz, jlong jSubProxy) {
    try {
        ISubWallet *subWallet = (ISubWallet *) jSubProxy;
        subWallet->RemoveCallback();
    } catch (const std::exception &e) {
        ThrowWalletException(env, e.what());
    }
}

#define JNI_CreateTransaction "(JLjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;"

static jstring JNICALL CreateTransaction(JNIEnv *env, jobject clazz, jlong jSubProxy,
                                         jstring jfromAddress,
                                         jstring jtoAddress,
                                         jstring jamount,
                                         jstring jmemo) {
    bool exception = false;
    std::string msgException;

    const char *fromAddress = env->GetStringUTFChars(jfromAddress, NULL);
    const char *toAddress = env->GetStringUTFChars(jtoAddress, NULL);
    const char *amount = env->GetStringUTFChars(jamount, NULL);
    const char *memo = env->GetStringUTFChars(jmemo, NULL);

    ISubWallet *subWallet = (ISubWallet *) jSubProxy;
    jstring tx = NULL;

    try {
        nlohmann::json result = subWallet->CreateTransaction(fromAddress, toAddress, amount, memo);
        tx = env->NewStringUTF(result.dump().c_str());
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jfromAddress, fromAddress);
    env->ReleaseStringUTFChars(jtoAddress, toAddress);
    env->ReleaseStringUTFChars(jamount, amount);
    env->ReleaseStringUTFChars(jmemo, memo);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return tx;
}

#define JNI_GetAllUTXOs "(JIILjava/lang/String;)Ljava/lang/String;"

static jstring JNICALL GetAllUTXOs(JNIEnv *env, jobject clazz, jlong jSubProxy,
                                   jint start,
                                   jint count,
                                   jstring jaddress) {
    bool exception = false;
    std::string msgException;

    const char *address = env->GetStringUTFChars(jaddress, NULL);

    ISubWallet *subWallet = (ISubWallet *) jSubProxy;

    jstring result = NULL;

    try {
        nlohmann::json utxoJson = subWallet->GetAllUTXOs(start, count, address);
        result = env->NewStringUTF(utxoJson.dump().c_str());
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jaddress, address);

    return result;
}

#define JNI_CreateConsolidateTransaction "(JLjava/lang/String;)Ljava/lang/String;"

static jstring JNICALL CreateConsolidateTransaction(JNIEnv *env, jobject clazz, jlong jSubProxy,
                                                    jstring jmemo) {
    bool exception = false;
    std::string msgException;

    const char *memo = env->GetStringUTFChars(jmemo, NULL);

    ISubWallet *subWallet = (ISubWallet *) jSubProxy;
    jstring tx = NULL;

    try {
        nlohmann::json result = subWallet->CreateConsolidateTransaction(memo);
        tx = env->NewStringUTF(result.dump().c_str());
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jmemo, memo);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return tx;

}

#define JNI_SignTransaction "(JLjava/lang/String;Ljava/lang/String;)Ljava/lang/String;"

static jstring JNICALL SignTransaction(JNIEnv *env, jobject clazz, jlong jSubProxy,
                                       jstring jRawTransaction,
                                       jstring jPayPassword) {
    bool exception = false;
    std::string msgException;

    const char *rawTransaction = env->GetStringUTFChars(jRawTransaction, NULL);
    const char *payPassword = env->GetStringUTFChars(jPayPassword, NULL);

    ISubWallet *subWallet = (ISubWallet *) jSubProxy;
    jstring tx = NULL;

    try {
        nlohmann::json result = subWallet->SignTransaction(nlohmann::json::parse(rawTransaction),
                                                           payPassword);
        tx = env->NewStringUTF(result.dump().c_str());
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jRawTransaction, rawTransaction);
    env->ReleaseStringUTFChars(jPayPassword, payPassword);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return tx;
}

#define JNI_GetTransactionSignedInfo "(JLjava/lang/String;)Ljava/lang/String;"

static jstring JNICALL GetTransactionSignedInfo(JNIEnv *env, jobject clazz, jlong jSubProxy,
                                                jstring jTransactionJson) {
    bool exception = false;
    std::string msgException;

    const char *transactionJson = env->GetStringUTFChars(jTransactionJson, NULL);
    jstring result = NULL;

    try {
        ISubWallet *subWallet = (ISubWallet *) jSubProxy;
        nlohmann::json signers = subWallet->GetTransactionSignedInfo(
                nlohmann::json::parse(transactionJson));
        result = env->NewStringUTF(signers.dump().c_str());
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jTransactionJson, transactionJson);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return result;
}

#define JNI_PublishTransaction "(JLjava/lang/String;)Ljava/lang/String;"

static jstring JNICALL PublishTransaction(JNIEnv *env, jobject clazz, jlong jSubProxy,
                                          jstring jTransactionJson) {
    bool exception = false;
    std::string msgException;

    const char *transactionJson = env->GetStringUTFChars(jTransactionJson, NULL);

    jstring result = NULL;

    try {
        ISubWallet *subWallet = (ISubWallet *) jSubProxy;
        nlohmann::json r = subWallet->PublishTransaction(nlohmann::json::parse(transactionJson));
        result = env->NewStringUTF(r.dump().c_str());
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jTransactionJson, transactionJson);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return result;
}

#define JNI_GetAllTransaction "(JIILjava/lang/String;)Ljava/lang/String;"

static jstring JNICALL GetAllTransaction(JNIEnv *env, jobject clazz, jlong jSubProxy,
                                         jint start,
                                         jint count,
                                         jstring jaddressOrTxid) {
    bool exception = false;
    std::string msgException;

    const char *addressOrTxid = env->GetStringUTFChars(jaddressOrTxid, NULL);
    jstring tx = NULL;

    try {
        ISubWallet *subWallet = (ISubWallet *) jSubProxy;
        nlohmann::json result = subWallet->GetAllTransaction(start, count, addressOrTxid);
        tx = env->NewStringUTF(result.dump().c_str());
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jaddressOrTxid, addressOrTxid);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return tx;
}

#define JNI_SyncStart "(J)V"

static void JNICALL SyncStart(JNIEnv *env, jobject clazz, jlong jproxy) {
    bool exception = false;
    std::string msgException;

    try {
        ISubWallet *subWallet = (ISubWallet *) jproxy;
        subWallet->SyncStart();
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    if (exception)
        ThrowWalletException(env, msgException.c_str());
}

#define JNI_SyncStop "(J)V"

static void JNICALL SyncStop(JNIEnv *env, jobject clazz, jlong jproxy) {
    bool exception = false;
    std::string msgException;

    try {
        ISubWallet *subWallet = (ISubWallet *) jproxy;
        subWallet->SyncStop();
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    if (exception)
        ThrowWalletException(env, msgException.c_str());
}

#define JNI_Resync "(J)V"

static void JNICALL Resync(JNIEnv *env, jobject clazz, jlong jproxy) {
    bool exception = false;
    std::string msgException;

    try {
        ISubWallet *subWallet = (ISubWallet *) jproxy;
        subWallet->Resync();
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    if (exception)
        ThrowWalletException(env, msgException.c_str());
}

#define JNI_GetAllCoinBaseTransaction "(JIILjava/lang/String;)Ljava/lang/String;"

static jstring JNICALL GetAllCoinBaseTransaction(JNIEnv *env, jobject clazz, jlong jSubProxy,
                                                 jint start, jint count, jstring jtxID) {
    bool exception = false;
    std::string msgException;
    jstring tx = NULL;

    const char *txid = env->GetStringUTFChars(jtxID, NULL);

    try {
        ISubWallet *subWallet = (ISubWallet *) jSubProxy;
        nlohmann::json txJson = subWallet->GetAllCoinBaseTransaction(start, count, txid);
        tx = env->NewStringUTF(txJson.dump().c_str());

    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jtxID, txid);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return tx;
}

#define JNI_GetAssetInfo "(JLjava/lang/String;)Ljava/lang/String;"

static jstring JNICALL GetAssetInfo(JNIEnv *env, jobject clazz, jlong jSubProxy,
                                    jstring jassetID) {
    bool exception = false;
    std::string msgException;
    jstring info = NULL;

    const char *assetid = env->GetStringUTFChars(jassetID, NULL);

    try {
        ISubWallet *subWallet = (ISubWallet *) jSubProxy;
        nlohmann::json jsonInfo = subWallet->GetAssetInfo(assetid);
        info = env->NewStringUTF(jsonInfo.dump().c_str());

    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jassetID, assetid);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return info;
}

#define JNI_SetFixedPeer "(JLjava/lang/String;I)Z"

static jbyte JNICALL SetFixedPeer(JNIEnv *env, jobject clazz, jlong jSubProxy,
                                  jstring jaddress,
                                  jint jport) {
    bool exception = false;
    std::string msgException;
    jboolean result = false;

    const char *address = env->GetStringUTFChars(jaddress, NULL);

    try {
        ISubWallet *subWallet = (ISubWallet *) jSubProxy;
        result = subWallet->SetFixedPeer(address, jport);
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jaddress, address);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return result;
}

static const JNINativeMethod methods[] = {
        REGISTER_METHOD(GetChainID),
        REGISTER_METHOD(GetBasicInfo),
        REGISTER_METHOD(GetBalanceInfo),
        REGISTER_METHOD(GetBalance),
        REGISTER_METHOD(CreateAddress),
        REGISTER_METHOD(GetAllAddress),
        REGISTER_METHOD(GetAllPublicKeys),
        REGISTER_METHOD(GetBalanceWithAddress),
        REGISTER_METHOD(AddCallback),
        REGISTER_METHOD(RemoveCallback),
        REGISTER_METHOD(CreateTransaction),
        REGISTER_METHOD(GetAllUTXOs),
        REGISTER_METHOD(CreateConsolidateTransaction),
        REGISTER_METHOD(SignTransaction),
        REGISTER_METHOD(GetTransactionSignedInfo),
        REGISTER_METHOD(PublishTransaction),
        REGISTER_METHOD(GetAllTransaction),
        REGISTER_METHOD(GetAllCoinBaseTransaction),
        REGISTER_METHOD(GetAssetInfo),
        REGISTER_METHOD(SetFixedPeer),
        REGISTER_METHOD(SyncStart),
        REGISTER_METHOD(SyncStop),
        REGISTER_METHOD(Resync)
};

jint RegisterSubWallet(JNIEnv *env, const std::string &path) {
    return RegisterNativeMethods(env, path + "SubWallet", methods, NELEM(methods));
}