// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Utils.h"
#include "IIdChainSubWallet.h"
#include "nlohmann/json.hpp"

using namespace Elastos::ElaWallet;

#define JNI_CreateIdTransaction "(JLjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;"

static jstring JNICALL CreateIdTransaction(JNIEnv *env, jobject clazz, jlong instance,
                                           jstring jfromAddress,
                                           jstring jpayloadJson,
                                           jstring jprogramJson,
                                           jstring jmemo,
                                           jstring jremark) {
    bool exception = false;
    std::string msgException;

    const char *fromAddress = env->GetStringUTFChars(jfromAddress, NULL);
    const char *payloadJson = env->GetStringUTFChars(jpayloadJson, NULL);
    const char *programJson = env->GetStringUTFChars(jprogramJson, NULL);
    const char *memo = env->GetStringUTFChars(jmemo, NULL);
    const char *remark = env->GetStringUTFChars(jremark, NULL);

    IIdChainSubWallet *wallet = (IIdChainSubWallet *) instance;
    jstring tx = NULL;

    try {
        nlohmann::json txJson = wallet->CreateIdTransaction(fromAddress,
                                                            nlohmann::json::parse(payloadJson),
                                                            nlohmann::json::parse(programJson),
                                                            memo, remark);
        tx = env->NewStringUTF(txJson.dump().c_str());
    } catch (const std::exception &e) {
        exception = true;
        msgException = e.what();
    }

    env->ReleaseStringUTFChars(jfromAddress, fromAddress);
    env->ReleaseStringUTFChars(jpayloadJson, payloadJson);
    env->ReleaseStringUTFChars(jprogramJson, programJson);
    env->ReleaseStringUTFChars(jmemo, memo);
    env->ReleaseStringUTFChars(jremark, remark);

    if (exception) {
        ThrowWalletException(env, msgException.c_str());
    }

    return tx;
}


static const JNINativeMethod methods[] = {
        REGISTER_METHOD(CreateIdTransaction),
};

jint RegisterIDChainSubWallet(JNIEnv *env, const std::string &path) {
    return RegisterNativeMethods(env, path + "IDChainSubWallet", methods, NELEM(methods));
}

