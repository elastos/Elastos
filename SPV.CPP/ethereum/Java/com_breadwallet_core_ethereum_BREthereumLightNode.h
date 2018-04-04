/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class com_breadwallet_core_ethereum_BREthereumLightNode */

#ifndef _Included_com_breadwallet_core_ethereum_BREthereumLightNode
#define _Included_com_breadwallet_core_ethereum_BREthereumLightNode
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     com_breadwallet_core_ethereum_BREthereumLightNode
 * Method:    jniCreateLightNodeLES
 * Signature: (Lcom/breadwallet/core/ethereum/BREthereumLightNode/Client;JLjava/lang/String;)J
 */
JNIEXPORT jlong JNICALL Java_com_breadwallet_core_ethereum_BREthereumLightNode_jniCreateLightNodeLES
  (JNIEnv *, jclass, jobject, jlong, jstring);

/*
 * Class:     com_breadwallet_core_ethereum_BREthereumLightNode
 * Method:    jniCreateLightNodeLES_PublicKey
 * Signature: (Lcom/breadwallet/core/ethereum/BREthereumLightNode/Client;J[B)J
 */
JNIEXPORT jlong JNICALL Java_com_breadwallet_core_ethereum_BREthereumLightNode_jniCreateLightNodeLES_1PublicKey
  (JNIEnv *, jclass, jobject, jlong, jbyteArray);

/*
 * Class:     com_breadwallet_core_ethereum_BREthereumLightNode
 * Method:    jniCreateLightNodeJSON_RPC
 * Signature: (Lcom/breadwallet/core/ethereum/BREthereumLightNode/Client;JLjava/lang/String;)J
 */
JNIEXPORT jlong JNICALL Java_com_breadwallet_core_ethereum_BREthereumLightNode_jniCreateLightNodeJSON_1RPC
  (JNIEnv *, jclass, jobject, jlong, jstring);

/*
 * Class:     com_breadwallet_core_ethereum_BREthereumLightNode
 * Method:    jniCreateLightNodeJSON_RPC_PublicKey
 * Signature: (Lcom/breadwallet/core/ethereum/BREthereumLightNode/Client;J[B)J
 */
JNIEXPORT jlong JNICALL Java_com_breadwallet_core_ethereum_BREthereumLightNode_jniCreateLightNodeJSON_1RPC_1PublicKey
  (JNIEnv *, jclass, jobject, jlong, jbyteArray);

/*
 * Class:     com_breadwallet_core_ethereum_BREthereumLightNode
 * Method:    jniLightNodeGetAccount
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_com_breadwallet_core_ethereum_BREthereumLightNode_jniLightNodeGetAccount
  (JNIEnv *, jobject);

/*
 * Class:     com_breadwallet_core_ethereum_BREthereumLightNode
 * Method:    jniGetAccountPrimaryAddress
 * Signature: (J)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_breadwallet_core_ethereum_BREthereumLightNode_jniGetAccountPrimaryAddress
  (JNIEnv *, jobject, jlong);

/*
 * Class:     com_breadwallet_core_ethereum_BREthereumLightNode
 * Method:    jniGetAccountPrimaryAddressPublicKey
 * Signature: (J)[B
 */
JNIEXPORT jbyteArray JNICALL Java_com_breadwallet_core_ethereum_BREthereumLightNode_jniGetAccountPrimaryAddressPublicKey
  (JNIEnv *, jobject, jlong);

/*
 * Class:     com_breadwallet_core_ethereum_BREthereumLightNode
 * Method:    jniLightNodeGetWallet
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_com_breadwallet_core_ethereum_BREthereumLightNode_jniLightNodeGetWallet
  (JNIEnv *, jobject);

/*
 * Class:     com_breadwallet_core_ethereum_BREthereumLightNode
 * Method:    jniLightNodeGetWalletToken
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_com_breadwallet_core_ethereum_BREthereumLightNode_jniLightNodeGetWalletToken
  (JNIEnv *, jobject, jlong);

/*
 * Class:     com_breadwallet_core_ethereum_BREthereumLightNode
 * Method:    jniLightNodeCreateWalletToken
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_com_breadwallet_core_ethereum_BREthereumLightNode_jniLightNodeCreateWalletToken
  (JNIEnv *, jobject, jlong);

/*
 * Class:     com_breadwallet_core_ethereum_BREthereumLightNode
 * Method:    jniGetWalletBalance
 * Signature: (JJ)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_breadwallet_core_ethereum_BREthereumLightNode_jniGetWalletBalance
  (JNIEnv *, jobject, jlong, jlong);

/*
 * Class:     com_breadwallet_core_ethereum_BREthereumLightNode
 * Method:    jniEstimateWalletGasPrice
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_com_breadwallet_core_ethereum_BREthereumLightNode_jniEstimateWalletGasPrice
  (JNIEnv *, jobject, jlong);

/*
 * Class:     com_breadwallet_core_ethereum_BREthereumLightNode
 * Method:    jniForceWalletBalanceUpdate
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_com_breadwallet_core_ethereum_BREthereumLightNode_jniForceWalletBalanceUpdate
  (JNIEnv *, jobject, jlong);

/*
 * Class:     com_breadwallet_core_ethereum_BREthereumLightNode
 * Method:    jniAnnounceTransaction
 * Signature: (ILjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_breadwallet_core_ethereum_BREthereumLightNode_jniAnnounceTransaction
  (JNIEnv *, jobject, jint, jstring, jstring, jstring, jstring, jstring, jstring, jstring, jstring, jstring, jstring, jstring, jstring, jstring, jstring, jstring, jstring);

/*
 * Class:     com_breadwallet_core_ethereum_BREthereumLightNode
 * Method:    jniAnnounceBalance
 * Signature: (ILjava/lang/String;I)V
 */
JNIEXPORT void JNICALL Java_com_breadwallet_core_ethereum_BREthereumLightNode_jniAnnounceBalance
  (JNIEnv *, jobject, jint, jstring, jint);

/*
 * Class:     com_breadwallet_core_ethereum_BREthereumLightNode
 * Method:    jniAnnounceGasPrice
 * Signature: (ILjava/lang/String;I)V
 */
JNIEXPORT void JNICALL Java_com_breadwallet_core_ethereum_BREthereumLightNode_jniAnnounceGasPrice
  (JNIEnv *, jobject, jint, jstring, jint);

/*
 * Class:     com_breadwallet_core_ethereum_BREthereumLightNode
 * Method:    jniAnnounceGasEstimate
 * Signature: (ILjava/lang/String;I)V
 */
JNIEXPORT void JNICALL Java_com_breadwallet_core_ethereum_BREthereumLightNode_jniAnnounceGasEstimate
  (JNIEnv *, jobject, jint, jstring, jint);

/*
 * Class:     com_breadwallet_core_ethereum_BREthereumLightNode
 * Method:    jniAnnounceSubmitTransaction
 * Signature: (ILjava/lang/String;I)V
 */
JNIEXPORT void JNICALL Java_com_breadwallet_core_ethereum_BREthereumLightNode_jniAnnounceSubmitTransaction
  (JNIEnv *, jobject, jint, jstring, jint);

/*
 * Class:     com_breadwallet_core_ethereum_BREthereumLightNode
 * Method:    jniCreateTransaction
 * Signature: (JLjava/lang/String;Ljava/lang/String;J)J
 */
JNIEXPORT jlong JNICALL Java_com_breadwallet_core_ethereum_BREthereumLightNode_jniCreateTransaction
  (JNIEnv *, jobject, jlong, jstring, jstring, jlong);

/*
 * Class:     com_breadwallet_core_ethereum_BREthereumLightNode
 * Method:    jniSignTransaction
 * Signature: (JJLjava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_breadwallet_core_ethereum_BREthereumLightNode_jniSignTransaction
  (JNIEnv *, jobject, jlong, jlong, jstring);

/*
 * Class:     com_breadwallet_core_ethereum_BREthereumLightNode
 * Method:    jniSubmitTransaction
 * Signature: (JJ)V
 */
JNIEXPORT void JNICALL Java_com_breadwallet_core_ethereum_BREthereumLightNode_jniSubmitTransaction
  (JNIEnv *, jobject, jlong, jlong);

/*
 * Class:     com_breadwallet_core_ethereum_BREthereumLightNode
 * Method:    jniGetTransactions
 * Signature: (J)[J
 */
JNIEXPORT jlongArray JNICALL Java_com_breadwallet_core_ethereum_BREthereumLightNode_jniGetTransactions
  (JNIEnv *, jobject, jlong);

/*
 * Class:     com_breadwallet_core_ethereum_BREthereumLightNode
 * Method:    jniTransactionEstimateGas
 * Signature: (JJ)V
 */
JNIEXPORT void JNICALL Java_com_breadwallet_core_ethereum_BREthereumLightNode_jniTransactionEstimateGas
  (JNIEnv *, jobject, jlong, jlong);

/*
 * Class:     com_breadwallet_core_ethereum_BREthereumLightNode
 * Method:    jniTransactionEstimateFee
 * Signature: (JLjava/lang/String;JJ)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_breadwallet_core_ethereum_BREthereumLightNode_jniTransactionEstimateFee
  (JNIEnv *, jobject, jlong, jstring, jlong, jlong);

/*
 * Class:     com_breadwallet_core_ethereum_BREthereumLightNode
 * Method:    jniTransactionHasToken
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL Java_com_breadwallet_core_ethereum_BREthereumLightNode_jniTransactionHasToken
  (JNIEnv *, jobject, jlong);

/*
 * Class:     com_breadwallet_core_ethereum_BREthereumLightNode
 * Method:    jniTransactionGetAmount
 * Signature: (JJ)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_breadwallet_core_ethereum_BREthereumLightNode_jniTransactionGetAmount
  (JNIEnv *, jobject, jlong, jlong);

/*
 * Class:     com_breadwallet_core_ethereum_BREthereumLightNode
 * Method:    jniTransactionGetFee
 * Signature: (JJ)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_breadwallet_core_ethereum_BREthereumLightNode_jniTransactionGetFee
  (JNIEnv *, jobject, jlong, jlong);

/*
 * Class:     com_breadwallet_core_ethereum_BREthereumLightNode
 * Method:    jniTransactionSourceAddress
 * Signature: (J)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_breadwallet_core_ethereum_BREthereumLightNode_jniTransactionSourceAddress
  (JNIEnv *, jobject, jlong);

/*
 * Class:     com_breadwallet_core_ethereum_BREthereumLightNode
 * Method:    jniTransactionTargetAddress
 * Signature: (J)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_breadwallet_core_ethereum_BREthereumLightNode_jniTransactionTargetAddress
  (JNIEnv *, jobject, jlong);

/*
 * Class:     com_breadwallet_core_ethereum_BREthereumLightNode
 * Method:    jniTransactionGetHash
 * Signature: (J)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_breadwallet_core_ethereum_BREthereumLightNode_jniTransactionGetHash
  (JNIEnv *, jobject, jlong);

/*
 * Class:     com_breadwallet_core_ethereum_BREthereumLightNode
 * Method:    jniTransactionGetGasPrice
 * Signature: (JJ)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_breadwallet_core_ethereum_BREthereumLightNode_jniTransactionGetGasPrice
  (JNIEnv *, jobject, jlong, jlong);

/*
 * Class:     com_breadwallet_core_ethereum_BREthereumLightNode
 * Method:    jniTransactionGetGasLimit
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_com_breadwallet_core_ethereum_BREthereumLightNode_jniTransactionGetGasLimit
  (JNIEnv *, jobject, jlong);

/*
 * Class:     com_breadwallet_core_ethereum_BREthereumLightNode
 * Method:    jniTransactionGetGasUsed
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_com_breadwallet_core_ethereum_BREthereumLightNode_jniTransactionGetGasUsed
  (JNIEnv *, jobject, jlong);

/*
 * Class:     com_breadwallet_core_ethereum_BREthereumLightNode
 * Method:    jniTransactionGetNonce
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_com_breadwallet_core_ethereum_BREthereumLightNode_jniTransactionGetNonce
  (JNIEnv *, jobject, jlong);

/*
 * Class:     com_breadwallet_core_ethereum_BREthereumLightNode
 * Method:    jniTransactionGetBlockNumber
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_com_breadwallet_core_ethereum_BREthereumLightNode_jniTransactionGetBlockNumber
  (JNIEnv *, jobject, jlong);

/*
 * Class:     com_breadwallet_core_ethereum_BREthereumLightNode
 * Method:    jniTransactionGetBlockTimestamp
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_com_breadwallet_core_ethereum_BREthereumLightNode_jniTransactionGetBlockTimestamp
  (JNIEnv *, jobject, jlong);

/*
 * Class:     com_breadwallet_core_ethereum_BREthereumLightNode
 * Method:    jniTransactionIsConfirmed
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL Java_com_breadwallet_core_ethereum_BREthereumLightNode_jniTransactionIsConfirmed
  (JNIEnv *, jobject, jlong);

/*
 * Class:     com_breadwallet_core_ethereum_BREthereumLightNode
 * Method:    jniTransactionIsSubmitted
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL Java_com_breadwallet_core_ethereum_BREthereumLightNode_jniTransactionIsSubmitted
  (JNIEnv *, jobject, jlong);

/*
 * Class:     com_breadwallet_core_ethereum_BREthereumLightNode
 * Method:    jniLightNodeConnect
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_com_breadwallet_core_ethereum_BREthereumLightNode_jniLightNodeConnect
  (JNIEnv *, jobject);

/*
 * Class:     com_breadwallet_core_ethereum_BREthereumLightNode
 * Method:    jniLightNodeDisconnect
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_com_breadwallet_core_ethereum_BREthereumLightNode_jniLightNodeDisconnect
  (JNIEnv *, jobject);

/*
 * Class:     com_breadwallet_core_ethereum_BREthereumLightNode
 * Method:    initializeNative
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_breadwallet_core_ethereum_BREthereumLightNode_initializeNative
  (JNIEnv *, jclass);

#ifdef __cplusplus
}
#endif
#endif
