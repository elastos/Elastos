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

        void SubWalletCallback::OnETHSCEventHandled(const nlohmann::json &event) {
            JNIEnv *env = GetEnv();

            jstring jevent = env->NewStringUTF(event.dump().c_str());

            jclass clazz = env->GetObjectClass(_obj);
            jmethodID methodId = env->GetMethodID(clazz, "OnETHSCEventHandled", "(Ljava/lang/String;)V");

            env->CallVoidMethod(_obj, methodId, jevent);

            env->DeleteLocalRef(jevent);
            env->DeleteLocalRef(clazz);
        }

        /**
         * @param id request id
         * @return If successful, return below. Otherwise {} or null will be returned to indicate the error.
         * {
         *   "id": 0,
         *   "result": "0x1dfd14000" // 8049999872 Wei
         * }
         */
        nlohmann::json SubWalletCallback::GasPrice(int id) {
            JNIEnv *env = GetEnv();

            jclass clazz = env->GetObjectClass(_obj);
            jmethodID methodId = env->GetMethodID(clazz, "GasPrice", "(I)Ljava/lang/String;");

            jstring result = (jstring)env->CallObjectMethod(_obj, methodId, id);

            const char *cstr = env->GetStringUTFChars(result, NULL);
            std::string str = std::string(cstr);
            env->ReleaseStringUTFChars(result, cstr);
            env->DeleteLocalRef(result);
            env->DeleteLocalRef(clazz);

            return nlohmann::json::parse(str);
        }

        /**
         * @param from
         * @param to
         * @param amount
         * @param gasPrice
         * @param data
         * @param id request id
         * @return If successful, return below. Otherwise {} or null will be returned to indicate the error.
         * {
         *   "id": 0,
         *   "result": "0x5208" // 21000
         * }
         */
        nlohmann::json SubWalletCallback::EstimateGas(const std::string &from,
                                                      const std::string &to,
                                                      const std::string &amount,
                                                      const std::string &gasPrice,
                                                      const std::string &data,
                                                      int id) {
            JNIEnv *env = GetEnv();

            jclass clazz = env->GetObjectClass(_obj);
            jmethodID methodId = env->GetMethodID(clazz, "EstimateGas", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;I)Ljava/lang/String;");

            jstring jfrom = env->NewStringUTF(from.c_str());
            jstring jto = env->NewStringUTF(to.c_str());
            jstring jamount = env->NewStringUTF(amount.c_str());
            jstring jgasPrice = env->NewStringUTF(gasPrice.c_str());
            jstring jdata = env->NewStringUTF(data.c_str());

            jstring result = (jstring)env->CallObjectMethod(_obj, methodId, jfrom, jto, jamount, jgasPrice, jdata, id);

            const char *cstr = env->GetStringUTFChars(result, NULL);
            std::string str = std::string(cstr);
            env->ReleaseStringUTFChars(result, cstr);
            env->DeleteLocalRef(result);
            env->DeleteLocalRef(clazz);

            env->DeleteLocalRef(jfrom);
            env->DeleteLocalRef(jto);
            env->DeleteLocalRef(jamount);
            env->DeleteLocalRef(jgasPrice);
            env->DeleteLocalRef(jdata);

            return nlohmann::json::parse(str);
        }

        /**
         * @param address
         * @param id
         * @return
         * {
         *   "id": 0,
         *   "result": "0x0234c8a3397aab58" // 158972490234375000
         * }
         */
        nlohmann::json SubWalletCallback::GetBalance(const std::string &address, int id) {
            JNIEnv *env = GetEnv();

            jclass clazz = env->GetObjectClass(_obj);
            jmethodID methodId = env->GetMethodID(clazz, "GetBalance", "(Ljava/lang/String;I)Ljava/lang/String;");

            jstring jaddress = env->NewStringUTF(address.c_str());

            jstring result = (jstring)env->CallObjectMethod(_obj, methodId, jaddress, id);

            const char *cstr = env->GetStringUTFChars(result, NULL);
            std::string str = std::string(cstr);
            env->ReleaseStringUTFChars(result, cstr);
            env->DeleteLocalRef(result);
            env->DeleteLocalRef(clazz);

            env->DeleteLocalRef(jaddress);

            return nlohmann::json::parse(str);
        }

        /**
         * @param tx Signed raw transaction.
         * @param id
         * @return If successful, return below. Otherwise {} or null will be returned to indicate the error.
         * {
         *   "id": 0,
         *   "result": "0xe670ec64341771606e55d6b4ca35a1a6b75ee3d5145a99d05921026d1527331"
         * }
         */
        nlohmann::json SubWalletCallback::SubmitTransaction(const std::string &tx, int id) {
            JNIEnv *env = GetEnv();

            jclass clazz = env->GetObjectClass(_obj);
            jmethodID methodId = env->GetMethodID(clazz, "SubmitTransaction", "(Ljava/lang/String;I)Ljava/lang/String;");

            jstring jtx = env->NewStringUTF(tx.c_str());

            jstring result = (jstring)env->CallObjectMethod(_obj, methodId, jtx, id);

            const char *cstr = env->GetStringUTFChars(result, NULL);
            std::string str = std::string(cstr);
            env->ReleaseStringUTFChars(result, cstr);
            env->DeleteLocalRef(result);
            env->DeleteLocalRef(clazz);

            env->DeleteLocalRef(jtx);

            return nlohmann::json::parse(str);
        }

         /**
          * @param begBlockNumber
          * @param endBlockNumber
          * @return If successful, return below. Otherwise {} or null will be returned to indicate the error.
          * {
          *   "id": 0,
          *   "result": [{
          *     "hash":"0x88df016429689c079f3b2f6ad39fa052532c56795b733da78a91ebe6a713944b",
          *     "from":"0xa7d9ddbe1f17865597fbd27ec712455208b6b76d",
          *     "to":"0xf02c1c8e6114b1dbe8937a39260b5b0a374432bb",
          *     "contract":"0xb60e8dd61c5d32be8058bb8eb970870f07233155", // or "", if none was created
          *     "amount":"0xf3dbb76162000", // 4290000000000000
          *     "gasLimit":"0x1388",
          *     "gasPrice":"0x4a817c800", // 20000000000
          *     "data":"0x68656c6c6f21", // input
          *     "nonce":"0x15", // 21
          *     "gasUsed":"0xc350", // 50000
          *     "blockNumber":"0x5daf3b", // 6139707
          *     "blockHash":"0x1d59ff54b1eb26b013ce3cb5fc9dab3705b415a67127a003c3e61eb445bb8df2",
          *     "blockConfirmations":"0x100", // 256
          *     "blockTransactionIndex":"0x41", // 65
          *     "blockTimestamp":"0x55ba467c",
          *     "isError": "0"
          *   },
          *   {
          *     ...
          *   }]
          * }
          */
         nlohmann::json SubWalletCallback::GetTransactions(const std::string &address,
                                                           uint64_t begBlockNumber,
                                                           uint64_t endBlockNumber,
                                                           int id) {
             JNIEnv *env = GetEnv();

             jclass clazz = env->GetObjectClass(_obj);
             jmethodID methodId = env->GetMethodID(clazz, "GetTransactions", "(Ljava/lang/String;JJI)Ljava/lang/String;");

             jstring jaddress = env->NewStringUTF(address.c_str());
             jlong jbegBlockNumber = (jlong)begBlockNumber;
             jlong jendBlockNumber = (jlong)endBlockNumber;

             jstring result = (jstring)env->CallObjectMethod(_obj, methodId, jaddress, jbegBlockNumber, jendBlockNumber, id);

             const char *cstr = env->GetStringUTFChars(result, NULL);
             std::string str = std::string(cstr);
             env->ReleaseStringUTFChars(result, cstr);
             env->DeleteLocalRef(result);
             env->DeleteLocalRef(clazz);

             env->DeleteLocalRef(jaddress);

             return nlohmann::json::parse(str);
        }

        /**
         * @param contract
         * @param address
         * @param event
         * @param begBlockNumber
         * @param endBlockNumber
         * @param id
         * @return If successful, return below. Otherwise {} or null will be returned to indicate the error.
         * {
         *   "id": 0,
         *   "result": [{
         *     "hash":"0xdf829c5a142f1fccd7d8216c5785ac562ff41e2dcfdf5785ac562ff41e2dcf",
         *     "contract":"0xb60e8dd61c5d32be8058bb8eb970870f07233155", // or "", if none was created
         *     "topics":["0x59ebeb90bc63057b6515673c3ecf9438e5058bca0f92585014eced636878c9a5"]
         *     "data":"0x0000000000000000000000000000000000000000000000000000000000000000",
         *     "gasPrice":"0x4a817c800", // 20000000000
         *     "gasUsed":"0x4dc", // 1244
         *     "logIndex":"0x1", // 1
         *     "blockNumber":"0x1b4", // 436
         *     "blockTransactionIndex":"0x0", // 0
         *     "blockTimestamp":"0x55ba467c",
         *   },{
         *    ...
         *   }]
         * }
         */
        nlohmann::json SubWalletCallback::GetLogs(const std::string &contract,
                                                  const std::string &address,
                                                  const std::string &event,
                                                  uint64_t begBlockNumber,
                                                  uint64_t endBlockNumber,
                                                  int id) {
            JNIEnv *env = GetEnv();

            jclass clazz = env->GetObjectClass(_obj);
            jmethodID methodId = env->GetMethodID(clazz, "GetLogs", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;JJI)Ljava/lang/String;");

            jstring jcontract = env->NewStringUTF(contract.c_str());
            jstring jaddress = env->NewStringUTF(address.c_str());
            jstring jevent = env->NewStringUTF(event.c_str());
            jlong jbegBlockNumber = (jlong)begBlockNumber;
            jlong jendBlockNumber = (jlong)endBlockNumber;

            jstring result = (jstring)env->CallObjectMethod(_obj, methodId, jcontract, jaddress, jevent, jbegBlockNumber, jendBlockNumber, id);

            const char *cstr = env->GetStringUTFChars(result, NULL);
            std::string str = std::string(cstr);
            env->ReleaseStringUTFChars(result, cstr);

            env->DeleteLocalRef(result);
            env->DeleteLocalRef(clazz);

            env->DeleteLocalRef(jcontract);
            env->DeleteLocalRef(jaddress);
            env->DeleteLocalRef(jevent);

            return nlohmann::json::parse(str);
        }

        /**
         * @param id
         * @return If successful, return below. Otherwise {} or null will be returned to indicate the error.
         * [{
         *   "id":0,
         *   "address": "0x407d73d8a49eeb85d32cf465507dd71d507100c1",
         *   "symbol": "ELA",
         *   "name": "elastos",
         *   "description": "desc",
         *   "decimals": 18,
         *   "defaultGasLimit": "0x1388",
         *   "defaultGasPrice": "0x1dfd14000" // 8049999872 Wei
         * },{
         *   ...
         * }]
         */
        nlohmann::json SubWalletCallback::GetTokens(int id) {
            JNIEnv *env = GetEnv();

            jclass clazz = env->GetObjectClass(_obj);
            jmethodID methodId = env->GetMethodID(clazz, "GetTokens", "(I)Ljava/lang/String;");

            jstring result = (jstring)env->CallObjectMethod(_obj, methodId, id);

            const char *cstr = env->GetStringUTFChars(result, NULL);
            std::string str = std::string(cstr);
            env->ReleaseStringUTFChars(result, cstr);

            env->DeleteLocalRef(result);
            env->DeleteLocalRef(clazz);

            return nlohmann::json::parse(str);
        }

        /**
         * @param id
         * @return If successful, return below. Otherwise {} or null will be returned to indicate the error.
         * {
         *   "id":0,
         *   "result": "0x4b7" // 1207
         * }
         */
        nlohmann::json SubWalletCallback::GetBlockNumber(int id) {
            JNIEnv *env = GetEnv();

            jclass clazz = env->GetObjectClass(_obj);
            jmethodID methodId = env->GetMethodID(clazz, "GetBlockNumber", "(I)Ljava/lang/String;");

            jstring result = (jstring)env->CallObjectMethod(_obj, methodId, id);

            const char *cstr = env->GetStringUTFChars(result, NULL);
            std::string str = std::string(cstr);
            env->ReleaseStringUTFChars(result, cstr);

            env->DeleteLocalRef(result);
            env->DeleteLocalRef(clazz);

            return nlohmann::json::parse(str);
        }

        /**
         * @param address
         * @param id
         * @return If successful, return below. Otherwise {} or null will be returned to indicate the error.
         * {
         *   "id": 0,
         *   "result": "0x1" // 1
         * }
         */
        nlohmann::json SubWalletCallback::GetNonce(const std::string &address, int id) {
            JNIEnv *env = GetEnv();

            jclass clazz = env->GetObjectClass(_obj);
            jmethodID methodId = env->GetMethodID(clazz, "GetNonce", "(Ljava/lang/String;I)Ljava/lang/String;");

            jstring jaddress = env->NewStringUTF(address.c_str());

            jstring result = (jstring)env->CallObjectMethod(_obj, methodId, jaddress, id);

            const char *cstr = env->GetStringUTFChars(result, NULL);
            std::string str = std::string(cstr);
            env->ReleaseStringUTFChars(result, cstr);
            env->DeleteLocalRef(result);
            env->DeleteLocalRef(clazz);

            env->DeleteLocalRef(jaddress);

            return nlohmann::json::parse(str);
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
