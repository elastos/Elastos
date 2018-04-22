//  Created by Ed Gamble on 1/23/2018
//  Copyright (c) 2018 breadwallet LLC.
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//  THE SOFTWARE.

#include <stdlib.h>
#include <malloc.h>
#include <BRPeer.h>
#include <BRInt.h>
#include "BRCoreJni.h"
#include "com_elastos_spvcore_CorePeer.h"

/*
 * Class:     com_elastos_spvcore_CorePeer
 * Method:    getAddress
 * Signature: ()[B
 */
JNIEXPORT jbyteArray JNICALL Java_com_elastos_spvcore_CorePeer_getAddress
(JNIEnv *env, jobject thisObject) {
	BRPeer *peer = (BRPeer *) getJNIReference (env, thisObject);

	jsize addressLen = sizeof(peer->address.u8);
	jbyteArray address = (*env)->NewByteArray (env, addressLen);
	(*env)->SetByteArrayRegion (env, address, 0, addressLen, (jbyte *) peer->address.u8);

	return address;
}


/*
 * Class:     com_elastos_spvcore_CorePeer
 * Method:    getPort
 * Signature: ()I;
 */
JNIEXPORT jint
JNICALL Java_com_elastos_spvcore_CorePeer_getPort
(JNIEnv *env, jobject thisObject) {
	BRPeer *peer = (BRPeer *) getJNIReference (env, thisObject);
	return peer->port;
}

/*
 * Class:     com_elastos_spvcore_CorePeer
 * Method:    getTimestamp
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_com_elastos_spvcore_CorePeer_getTimestamp
(JNIEnv *env, jobject thisObject) {
	BRPeer *peer = (BRPeer *) getJNIReference (env, thisObject);
	return peer->timestamp;
}

/*
 * Class:     com_elastos_spvcore_CorePeer
 * Method:    setEarliestKeyTime
 * Signature: (J)V
 */
JNIEXPORT void JNICALL
Java_com_elastos_spvcore_CorePeer_setEarliestKeyTime
(JNIEnv *env, jobject thisObject, jlong earliestKeyTime) {
	BRPeer *peer = (BRPeer *) getJNIReference (env, thisObject);
	BRPeerSetEarliestKeyTime (peer, (uint32_t) earliestKeyTime);
}

/*
 * Class:     com_elastos_spvcore_CorePeer
 * Method:    setCurrentBlockHeight
 * Signature: (J)V
 */
JNIEXPORT void JNICALL
Java_com_elastos_spvcore_CorePeer_setCurrentBlockHeight
(JNIEnv *env, jobject thisObject, jlong currentBlockHeight) {
	BRPeer *peer = (BRPeer *) getJNIReference (env, thisObject);
	BRPeerSetCurrentBlockHeight (peer, (uint32_t) currentBlockHeight);
}

/*
 * Class:     com_elastos_spvcore_CorePeer
 * Method:    getConnectStatus
 * Signature: ()I
 */
JNIEXPORT jint JNICALL
Java_com_elastos_spvcore_CorePeer_getConnectStatusValue
(JNIEnv *env, jobject thisObject) {
	BRPeer *peer = (BRPeer *) getJNIReference (env, thisObject);
	return BRPeerConnectStatus (peer);
}

/*
 * Class:     com_elastos_spvcore_CorePeer
 * Method:    connect
 * Signature: ()V
 */
JNIEXPORT void JNICALL
Java_com_elastos_spvcore_CorePeer_connect
(JNIEnv *env, jobject thisObject) {
	BRPeer *peer = (BRPeer *) getJNIReference (env, thisObject);
	BRPeerConnect (peer);
}

/*
 * Class:     com_elastos_spvcore_CorePeer
 * Method:    disconnect
 * Signature: ()V
 */
JNIEXPORT void JNICALL
Java_com_elastos_spvcore_CorePeer_disconnect
(JNIEnv *env, jobject thisObject) {
	BRPeer *peer = (BRPeer *) getJNIReference (env, thisObject);
	BRPeerDisconnect (peer);
}

/*
 * Class:     com_elastos_spvcore_CorePeer
 * Method:    scheduleDisconnect
 * Signature: (D)V
 */
JNIEXPORT void JNICALL
Java_com_elastos_spvcore_CorePeer_scheduleDisconnect
(JNIEnv *env, jobject thisObject, jdouble time) {
	BRPeer *peer = (BRPeer *) getJNIReference (env, thisObject);
	BRPeerScheduleDisconnect (peer, time);
}

/*
 * Class:     com_elastos_spvcore_CorePeer
 * Method:    setNeedsFilterUpdate
 * Signature: (Z)V
 */
JNIEXPORT void JNICALL
Java_com_elastos_spvcore_CorePeer_setNeedsFilterUpdate
(JNIEnv *env, jobject thisObject, jboolean needsFilterUpdate) {
	BRPeer *peer = (BRPeer *) getJNIReference (env, thisObject);
	BRPeerSetNeedsFilterUpdate (peer, needsFilterUpdate);
}

/*
 * Class:     com_elastos_spvcore_CorePeer
 * Method:    getHost
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL
Java_com_elastos_spvcore_CorePeer_getHost
(JNIEnv *env, jobject thisObject) {
	BRPeer *peer = (BRPeer *) getJNIReference (env, thisObject);
	const char *host = BRPeerHost(peer);
	return (*env)->NewStringUTF (env, host);
}

/*
 * Class:     com_elastos_spvcore_CorePeer
 * Method:    getVersion
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL
Java_com_elastos_spvcore_CorePeer_getVersion
(JNIEnv *env, jobject thisObject) {
	BRPeer *peer = (BRPeer *) getJNIReference (env, thisObject);
	return (jlong) BRPeerVersion (peer);
}

/*
 * Class:     com_elastos_spvcore_CorePeer
 * Method:    getUserAgent
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL
Java_com_elastos_spvcore_CorePeer_getUserAgent
(JNIEnv *env, jobject thisObject) {
	BRPeer *peer = (BRPeer *) getJNIReference (env, thisObject);
	const char *host = BRPeerUserAgent(peer);
	return (*env)->NewStringUTF (env, host);
}

/*
 * Class:     com_elastos_spvcore_CorePeer
 * Method:    getLastBlock
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL
Java_com_elastos_spvcore_CorePeer_getLastBlock
(JNIEnv *env, jobject thisObject) {
	BRPeer *peer = (BRPeer *) getJNIReference (env, thisObject);
	return (jlong) BRPeerLastBlock (peer);
}

/*
 * Class:     com_elastos_spvcore_CorePeer
 * Method:    getFeePerKb
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL
Java_com_elastos_spvcore_CorePeer_getFeePerKb
(JNIEnv *env, jobject thisObject) {
	BRPeer *peer = (BRPeer *) getJNIReference (env, thisObject);
	return (jlong) BRPeerFeePerKb (peer);
}

/*
 * Class:     com_elastos_spvcore_CorePeer
 * Method:    getPingTime
 * Signature: ()D
 */
JNIEXPORT jdouble JNICALL
Java_com_elastos_spvcore_CorePeer_getPingTime
(JNIEnv *env, jobject thisObject) {
	BRPeer *peer = (BRPeer *) getJNIReference (env, thisObject);
	return (jdouble) BRPeerPingTime (peer);
}

/*
 * Class:     com_elastos_spvcore_CorePeer
 * Method:    createJniCorePeerNatural
 * Signature: ([BIJ)J
 */
JNIEXPORT jlong JNICALL Java_com_elastos_spvcore_CorePeer_createJniCorePeerNatural
(JNIEnv *env, jclass thisClass,
 jbyteArray peerAddress,
 jint peerPort,
 jlong peerTimestamp) {
	BRPeer *result = (BRPeer *) calloc (1, sizeof (BRPeer));

	jbyte *byteAddr  = (*env)->GetByteArrayElements(env, peerAddress, 0);

	result->address = *(UInt128 *) byteAddr;
	result->port = (uint16_t) peerPort;
	result->timestamp = (uint64_t) peerTimestamp;
	result->services = SERVICES_NODE_NETWORK;
	result->flags = 0;

	return (jlong) result;
}


/*
 * Class:     com_elastos_spvcore_CorePeer
 * Method:    createJniCorePeer
 * Signature: ([B[B[B)J
 */
JNIEXPORT jlong JNICALL
Java_com_elastos_spvcore_CorePeer_createJniCorePeer
(JNIEnv *env, jclass thisClass,
 jbyteArray peerAddress,
 jbyteArray peerPort,
 jbyteArray peerTimeStamp ) {
	BRPeer *result = (BRPeer *) calloc (1, sizeof (BRPeer));

	jbyte *byteAddr  = (*env)->GetByteArrayElements(env, peerAddress, 0);
	jbyte *bytePort  = (*env)->GetByteArrayElements(env, peerPort, 0);
	jbyte *byteStamp = (*env)->GetByteArrayElements(env, peerTimeStamp, 0);

	result->address = *(UInt128 *) byteAddr;
	result->port = *(uint16_t *) bytePort;
	result->timestamp = *(uint64_t *) byteStamp;
	result->services = SERVICES_NODE_NETWORK;
	result->flags = 0;

	return (jlong) result;
}

/*
 * Class:     com_elastos_spvcore_CorePeer
 * Method:    createJniCorePeerMagic
 * Signature: (J)J
 */
JNIEXPORT jlong
JNICALL Java_com_elastos_spvcore_CorePeer_createJniCorePeerMagic
(JNIEnv *env, jclass thisClass,
 jlong magicNumber) {
	BRPeer *result = BRPeerNew((uint32_t) magicNumber);
	return (jlong) result;
}
