// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "SubWalletCallback.h"

namespace Elastos {
    namespace ElaWallet {

        SubWalletCallback::SubWalletCallback(JNIEnv *env, jobject jobj) {
            _obj = env->NewGlobalRef(jobj);
            env->GetJavaVM(&_jvm);

        }

        SubWalletCallback::~SubWalletCallback() {
            JNIEnv *env = GetEnv();

//            _jvm->DetachCurrentThread();
            env->DeleteGlobalRef(_obj);
        }

        JNIEnv *SubWalletCallback::GetEnv() {
            JNIEnv *env;
            _jvm->AttachCurrentThread(&env, NULL);
            return env;
        }

        void SubWalletCallback::OnTransactionStatusChanged(const std::string &txid,
                                                           const std::string &status,
                                                           const nlohmann::json &desc,
                                                           uint32_t confirms) {
            JNIEnv *env = GetEnv();

            jclass clazz = env->GetObjectClass(_obj);
            jmethodID methodId = env->GetMethodID(clazz, "OnTransactionStatusChanged",
                                                  "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;I)V");
            jstring jtxid = env->NewStringUTF(txid.c_str());
            jstring jstatus = env->NewStringUTF(status.c_str());
            jstring jdesc = env->NewStringUTF(desc.dump().c_str());

            env->CallVoidMethod(_obj, methodId, jtxid, jstatus, jdesc, confirms);

            env->DeleteLocalRef(jtxid);
            env->DeleteLocalRef(jstatus);
            env->DeleteLocalRef(jdesc);
            env->DeleteLocalRef(clazz);
//            Detach();
        }

        void SubWalletCallback::OnBlockSyncProgress(const nlohmann::json &progressInfo) {
            JNIEnv *env = GetEnv();

            jstring info = env->NewStringUTF(progressInfo.dump().c_str());
            jclass clazz = env->GetObjectClass(_obj);
            jmethodID methodId = env->GetMethodID(clazz, "OnBlockSyncProgress", "(Ljava/lang/String;)V");
            env->CallVoidMethod(_obj, methodId, info);

            env->DeleteLocalRef(info);
            env->DeleteLocalRef(clazz);
//            Detach();
        }

        void SubWalletCallback::OnBalanceChanged(const std::string &asset, const std::string &balance) {
            JNIEnv *env = GetEnv();

            jstring assetID = env->NewStringUTF(asset.c_str());
            jstring value = env->NewStringUTF(balance.c_str());

            jclass clazz = env->GetObjectClass(_obj);
            jmethodID methodId = env->GetMethodID(clazz, "OnBalanceChanged",
                                                  "(Ljava/lang/String;Ljava/lang/String;)V");
            env->CallVoidMethod(_obj, methodId, assetID, value);

            env->DeleteLocalRef(assetID);
            env->DeleteLocalRef(value);
            env->DeleteLocalRef(clazz);
//            Detach();
        }

        void
        SubWalletCallback::OnTxPublished(const std::string &hash, const nlohmann::json &result) {
            JNIEnv *env = GetEnv();

            jstring jResult = env->NewStringUTF(result.dump().c_str());
            jstring jHash = env->NewStringUTF(hash.c_str());

            jclass clazz = env->GetObjectClass(_obj);
            jmethodID methodId = env->GetMethodID(clazz, "OnTxPublished",
                                                  "(Ljava/lang/String;Ljava/lang/String;)V");
            env->CallVoidMethod(_obj, methodId, jHash, jResult);

            env->DeleteLocalRef(jResult);
            env->DeleteLocalRef(jHash);
            env->DeleteLocalRef(clazz);
//            Detach();
        }

        void
        SubWalletCallback::OnAssetRegistered(const std::string &asset, const nlohmann::json &info) {
            JNIEnv *env = GetEnv();

            jstring jasset = env->NewStringUTF(asset.c_str());
            jstring jinfo = env->NewStringUTF(info.dump().c_str());

            jclass clazz = env->GetObjectClass(_obj);
            jmethodID methodId = env->GetMethodID(clazz, "OnAssetRegistered",
                                                  "(Ljava/lang/String;Ljava/lang/String;)V");
            env->CallVoidMethod(_obj, methodId, jasset, jinfo);

            env->DeleteLocalRef(jasset);
            env->DeleteLocalRef(jinfo);
            env->DeleteLocalRef(clazz);
        }

        void SubWalletCallback::OnConnectStatusChanged(const std::string &status) {
            JNIEnv *env = GetEnv();

            jstring jstatus = env->NewStringUTF(status.c_str());

            jclass clazz = env->GetObjectClass(_obj);
            jmethodID methodID = env->GetMethodID(clazz, "OnConnectStatusChanged", "(Ljava/lang/String;)V");

            env->CallVoidMethod(_obj, methodID, jstatus);

            env->DeleteLocalRef(jstatus);
            env->DeleteLocalRef(clazz);
        }


    }
}

using namespace Elastos::ElaWallet;

#define JNI_InitSubWalletCallback "()J"

static jlong JNICALL InitSubWalletCallback(JNIEnv *env, jobject clazz) {
    SubWalletCallback *callback = NULL;

    try {
        callback = new SubWalletCallback(env, clazz);
    } catch (const std::exception &e) {
        ThrowWalletException(env, e.what());
    }

    return (jlong) callback;
}

#define JNI_DisposeNative "(J)V"

static void JNICALL DisposeNative(JNIEnv *env, jobject clazz, jlong instance) {
    SubWalletCallback *callback = (SubWalletCallback *) instance;
    delete callback;
}

static const JNINativeMethod methods[] = {
        REGISTER_METHOD(InitSubWalletCallback),
        REGISTER_METHOD(DisposeNative),
};

jint RegisterSubWalletCallback(JNIEnv *env, const std::string &path) {
    return RegisterNativeMethods(env, path + "SubWalletCallback", methods, NELEM(methods));
}
