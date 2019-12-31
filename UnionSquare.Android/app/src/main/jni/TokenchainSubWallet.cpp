// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Utils.h"
#include "ITokenchainSubWallet.h"
#include "nlohmann/json.hpp"

using namespace Elastos::ElaWallet;

#define JNI_GetBalanceInfo "(JLjava/lang/String;)Ljava/lang/String;"

static jstring JNICALL GetBalanceInfo(JNIEnv *env, jobject clazz, jlong instance,
                                      jstring jassetID) {
    bool exception = false;
    std::string msgException;

    const char *assetID = env->GetStringUTFChars(jassetID, NULL);
    jstring balance = NULL;

    try {
        ITokenchainSubWallet *wallet = (ITokenchainSubWallet *) instance;

        nlohmann::json jsondata = wallet->GetBalanceInfo(assetID);
        balance = env->NewStringUTF(jsondata.dump().c_str());

    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jassetID, assetID);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return balance;
}

#define JNI_GetBalance "(JLjava/lang/String;)Ljava/lang/String;"

static jstring JNICALL GetBalance(JNIEnv *env, jobject clazz, jlong instance,
                                  jstring jassetID) {
    bool exception = false;
    std::string msgException;

    const char *assetID = env->GetStringUTFChars(jassetID, NULL);
    jstring balance = NULL;

    try {
        ITokenchainSubWallet *wallet = (ITokenchainSubWallet *) instance;

        std::string amount = wallet->GetBalance(assetID);
        balance = env->NewStringUTF(amount.c_str());

    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jassetID, assetID);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return balance;
}

#define JNI_GetBalanceWithAddress "(JLjava/lang/String;Ljava/lang/String;)Ljava/lang/String;"

static jstring JNICALL GetBalanceWithAddress(JNIEnv *env, jobject clazz, jlong instance,
                                             jstring jassetID,
                                             jstring jaddress) {
    bool exception = false;
    std::string msgException;

    const char *assetID = env->GetStringUTFChars(jassetID, NULL);
    const char *address = env->GetStringUTFChars(jaddress, NULL);
    jstring balance = NULL;

    try {
        ITokenchainSubWallet *wallet = (ITokenchainSubWallet *) instance;

        std::string amount = wallet->GetBalanceWithAddress(assetID, address);
        balance = env->NewStringUTF(amount.c_str());

    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jassetID, assetID);
    env->ReleaseStringUTFChars(jaddress, address);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return balance;
}

#define JNI_CreateRegisterAssetTransaction "(JLjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;BLjava/lang/String;)Ljava/lang/String;"

static jstring JNICALL CreateRegisterAssetTransaction(JNIEnv *env, jobject clazz, jlong instance,
                                                      jstring jname,
                                                      jstring jdescription,
                                                      jstring jregisterToAddress,
                                                      jstring jregisterAmount,
                                                      jbyte precision,
                                                      jstring jmemo) {
    bool exception = false;
    std::string msgException;

    const char *name = env->GetStringUTFChars(jname, NULL);
    const char *descript = env->GetStringUTFChars(jdescription, NULL);
    const char *registerToAddress = env->GetStringUTFChars(jregisterToAddress, NULL);
    const char *registerAmount = env->GetStringUTFChars(jregisterAmount, NULL);
    const char *memo = env->GetStringUTFChars(jmemo, NULL);
    jstring tx = NULL;

    try {
        ITokenchainSubWallet *wallet = (ITokenchainSubWallet *) instance;

        nlohmann::json txJson = wallet->CreateRegisterAssetTransaction(name, descript,
                                                                       registerToAddress,
                                                                       registerAmount,
                                                                       precision,
                                                                       memo);

        tx = env->NewStringUTF(txJson.dump().c_str());

    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jname, name);
    env->ReleaseStringUTFChars(jdescription, descript);
    env->ReleaseStringUTFChars(jregisterToAddress, registerToAddress);
    env->ReleaseStringUTFChars(jregisterAmount, registerAmount);
    env->ReleaseStringUTFChars(jmemo, memo);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return tx;

}

#define JNI_CreateTransaction "(JLjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;"

static jstring JNICALL CreateTransaction(JNIEnv *env, jobject clazz, jlong instance,
                                         jstring jfromAddress,
                                         jstring jtoAddress,
                                         jstring jamount,
                                         jstring jassetID,
                                         jstring jmemo) {
    bool exception = false;
    std::string msgException;

    const char *fromAddress = env->GetStringUTFChars(jfromAddress, NULL);
    const char *toAddress = env->GetStringUTFChars(jtoAddress, NULL);
    const char *amount = env->GetStringUTFChars(jamount, NULL);
    const char *assetID = env->GetStringUTFChars(jassetID, NULL);
    const char *memo = env->GetStringUTFChars(jmemo, NULL);

    jstring tx = NULL;

    try {
        ITokenchainSubWallet *wallet = (ITokenchainSubWallet *) instance;
        nlohmann::json result = wallet->CreateTransaction(fromAddress, toAddress, amount, assetID,
                                                          memo);
        tx = env->NewStringUTF(result.dump().c_str());
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jfromAddress, fromAddress);
    env->ReleaseStringUTFChars(jtoAddress, toAddress);
    env->ReleaseStringUTFChars(jamount, amount);
    env->ReleaseStringUTFChars(jassetID, assetID);
    env->ReleaseStringUTFChars(jmemo, memo);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return tx;
}

#define JNI_CreateConsolidateTransaction "(JLjava/lang/String;Ljava/lang/String;)Ljava/lang/String;"

static jstring JNICALL CreateConsolidateTransaction(JNIEnv *env, jobject clazz, jlong instance,
                                                    jstring jassetID,
                                                    jstring jmemo) {
    bool exception = false;
    std::string msgException;

    const char *assetID = env->GetStringUTFChars(jassetID, NULL);
    const char *memo = env->GetStringUTFChars(jmemo, NULL);

    jstring tx = NULL;

    try {
        ITokenchainSubWallet *wallet = (ITokenchainSubWallet *) instance;
        nlohmann::json result = wallet->CreateConsolidateTransaction(assetID, memo);

        tx = env->NewStringUTF(result.dump().c_str());
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jassetID, assetID);
    env->ReleaseStringUTFChars(jmemo, memo);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return tx;
}

#define JNI_GetAllAssets "(J)Ljava/lang/String;"

static jstring JNICALL GetAllAssets(JNIEnv *env, jobject clazz, jlong instance) {
    bool exception = false;
    std::string msgException;

    jstring assets = NULL;
    try {
        ITokenchainSubWallet *wallet = (ITokenchainSubWallet *) instance;
        nlohmann::json result = wallet->GetAllAssets();

        assets = env->NewStringUTF(result.dump().c_str());
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return assets;
}

static const JNINativeMethod methods[] = {
        REGISTER_METHOD(GetBalanceInfo),
        REGISTER_METHOD(GetBalance),
        REGISTER_METHOD(GetBalanceWithAddress),
        REGISTER_METHOD(CreateRegisterAssetTransaction),
        REGISTER_METHOD(CreateTransaction),
        REGISTER_METHOD(CreateConsolidateTransaction),
        REGISTER_METHOD(GetAllAssets),
};

jint RegisterTokenChainSubWallet(JNIEnv *env, const std::string &path) {
    return RegisterNativeMethods(env, path + "TokenchainSubWallet", methods, NELEM(methods));
}