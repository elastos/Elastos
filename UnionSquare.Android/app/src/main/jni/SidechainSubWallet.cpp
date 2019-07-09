// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Utils.h"
#include "ISidechainSubWallet.h"
#include "nlohmann/json.hpp"

using namespace Elastos::ElaWallet;

#define JNI_CreateWithdrawTransaction "(JLjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;"

static jstring JNICALL
CreateWithdrawTransaction(JNIEnv *env, jobject clazz, jlong jSideSubWalletProxy,
                          jstring jfromAddress,
                          jstring jamount,
                          jstring jmainChainAddress,
                          jstring jmemo) {
    bool exception = false;
    std::string msgException;

    const char *fromAddress = env->GetStringUTFChars(jfromAddress, NULL);
    const char *amount = env->GetStringUTFChars(jamount, NULL);
    const char *mainChainAddress = env->GetStringUTFChars(jmainChainAddress, NULL);
    const char *memo = env->GetStringUTFChars(jmemo, NULL);

    ISidechainSubWallet *wallet = (ISidechainSubWallet *) jSideSubWalletProxy;
    jstring tx = NULL;

    try {
        nlohmann::json txJson = wallet->CreateWithdrawTransaction(fromAddress, amount,
                                                                  mainChainAddress, memo);

        tx = env->NewStringUTF(txJson.dump().c_str());
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jfromAddress, fromAddress);
    env->ReleaseStringUTFChars(jamount, amount);
    env->ReleaseStringUTFChars(jmainChainAddress, mainChainAddress);
    env->ReleaseStringUTFChars(jmemo, memo);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return tx;
}

#define JNI_GetGenesisAddress "(J)Ljava/lang/String;"

static jstring JNICALL GetGenesisAddress(JNIEnv *env, jobject clazz, jlong jSideSubWalletProxy) {
    bool exception = false;
    std::string msgException;

    ISidechainSubWallet *wallet = (ISidechainSubWallet *) jSideSubWalletProxy;
    jstring addr = NULL;

    try {
        std::string address = wallet->GetGenesisAddress();
        addr = env->NewStringUTF(address.c_str());
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return addr;
}


static const JNINativeMethod methods[] = {
        REGISTER_METHOD(CreateWithdrawTransaction),
        REGISTER_METHOD(GetGenesisAddress),
};

jint RegisterSidechainSubWallet(JNIEnv *env, const std::string &path) {
    return RegisterNativeMethods(env, path + "SidechainSubWallet", methods, NELEM(methods));
}
