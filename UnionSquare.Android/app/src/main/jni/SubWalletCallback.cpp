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

//            Detach();
        }

        void SubWalletCallback::OnBlockSyncStarted() {
            JNIEnv *env = GetEnv();

            jclass clazz = env->GetObjectClass(_obj);
            jmethodID methodId = env->GetMethodID(clazz, "OnBlockSyncStarted", "()V");
            env->CallVoidMethod(_obj, methodId);

//            Detach();
        }

        void SubWalletCallback::OnBlockSyncProgress(uint32_t currentBlockHeight,
                                                    uint32_t estimatedHeight,
                                                    time_t lastBlockTime) {
            JNIEnv *env = GetEnv();

            jclass clazz = env->GetObjectClass(_obj);
            jmethodID methodId = env->GetMethodID(clazz, "OnBlockSyncProgress", "(IIJ)V");
            env->CallVoidMethod(_obj, methodId, currentBlockHeight, estimatedHeight, lastBlockTime);

//            Detach();
        }

        void SubWalletCallback::OnBlockSyncStopped() {
            JNIEnv *env = GetEnv();

            jclass clazz = env->GetObjectClass(_obj);
            jmethodID methodId = env->GetMethodID(clazz, "OnBlockSyncStopped", "()V");
            env->CallVoidMethod(_obj, methodId);

//            Detach();
        }

        void SubWalletCallback::OnBalanceChanged(const std::string &asset, uint64_t balance) {
            JNIEnv *env = GetEnv();

            jstring assetID = env->NewStringUTF(asset.c_str());

            jclass clazz = env->GetObjectClass(_obj);
            jmethodID methodId = env->GetMethodID(clazz, "OnBalanceChanged",
                                                  "(Ljava/lang/String;J)V");
            env->CallVoidMethod(_obj, methodId, assetID, balance);

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

//            Detach();
        }

        void SubWalletCallback::OnTxDeleted(const std::string &hash, bool notifyUser,
                                            bool recommendRescan) {
            JNIEnv *env = GetEnv();

            jstring jHash = env->NewStringUTF(hash.c_str());

            jclass clazz = env->GetObjectClass(_obj);
            jmethodID methodId = env->GetMethodID(clazz, "OnTxDeleted", "(Ljava/lang/String;ZZ)V");
            env->CallVoidMethod(_obj, methodId, jHash, notifyUser, recommendRescan);

//            Detach();
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
