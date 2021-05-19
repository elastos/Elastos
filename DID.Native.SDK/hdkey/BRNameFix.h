/*
 * Copyright (c) 2019 Elastos Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __BRNAMEFIX_H__
#define __BRNAMEFIX_H__

#define BRBIP39Encode		DID_BRBIP39Encode
#define BRBIP39Decode		DID_BRBIP39Decode
#define BRBIP39PhraseIsValid		DID_BRBIP39PhraseIsValid
#define BRBIP39DeriveKey		DID_BRBIP39DeriveKey
#define BRSHA1		DID_BRSHA1
#define BRSHA256		DID_BRSHA256
#define BRSHA224		DID_BRSHA224
#define BRSHA256_2		DID_BRSHA256_2
#define BRSHA384		DID_BRSHA384
#define BRSHA512		DID_BRSHA512
#define BRRMD160		DID_BRRMD160
#define BRHash160		DID_BRHash160
#define BRSHA3_256		DID_BRSHA3_256
#define BRKeccak256		DID_BRKeccak256
#define BRMD5		DID_BRMD5
#define BRMurmur3_32		DID_BRMurmur3_32
#define BRSip64		DID_BRSip64
#define BRHMAC		DID_BRHMAC
#define BRHMACDRBG		DID_BRHMACDRBG
#define BRPoly1305		DID_BRPoly1305
#define BRChacha20		DID_BRChacha20
#define BRChacha20Poly1305AEADEncrypt		DID_BRChacha20Poly1305AEADEncrypt
#define BRChacha20Poly1305AEADDecrypt		DID_BRChacha20Poly1305AEADDecrypt
#define BRAESECBEncrypt		DID_BRAESECBEncrypt
#define BRAESECBDecrypt		DID_BRAESECBDecrypt
#define BRAESCTR		DID_BRAESCTR
#define BRAESCTR_OFFSET		DID_BRAESCTR_OFFSET
#define BRPBKDF2		DID_BRPBKDF2
#define BRScrypt		DID_BRScrypt
#define BRRand		DID_BRRand
#define BRSecp256k1ModAdd		DID_BRSecp256k1ModAdd
#define BRSecp256k1ModMul		DID_BRSecp256k1ModMul
#define BRSecp256k1PointGen		DID_BRSecp256k1PointGen
#define BRSecp256k1PointAdd		DID_BRSecp256k1PointAdd
#define BRSecp256k1PointMul		DID_BRSecp256k1PointMul
#define BRPrivKeyIsValid		DID_BRPrivKeyIsValid
#define BRKeySetSecret		DID_BRKeySetSecret
#define BRKeySetPrivKey		DID_BRKeySetPrivKey
#define BRKeySetPubKey		DID_BRKeySetPubKey
#define BRKeyPrivKey		DID_BRKeyPrivKey
#define BRKeyPubKey		DID_BRKeyPubKey
#define BRKeyHash160		DID_BRKeyHash160
#define BRKeyAddress		DID_BRKeyAddress
#define BRKeyLegacyAddr		DID_BRKeyLegacyAddr
#define BRKeySign		DID_BRKeySign
#define BRKeyVerify		DID_BRKeyVerify
#define BRKeyClean		DID_BRKeyClean
#define BRKeyCompactSign		DID_BRKeyCompactSign
#define BRKeyRecoverPubKey		DID_BRKeyRecoverPubKey
#define BRKeyECDH		DID_BRKeyECDH
#define BRKeyCompactSignEthereum		DID_BRKeyCompactSignEthereum
#define BRKeyRecoverPubKeyEthereum		DID_BRKeyRecoverPubKeyEthereum
#define BRBase58Encode		DID_BRBase58Encode
#define BRBase58Decode		DID_BRBase58Decode
#define BRBase58CheckEncode		DID_BRBase58CheckEncode
#define BRBase58CheckDecode		DID_BRBase58CheckDecode
#define BRBech32Decode		DID_BRBech32Decode
#define BRBech32Encode		DID_BRBech32Encode
#define BRBIP32MasterPubKey		DID_BRBIP32MasterPubKey
#define BRBIP32PubKey		DID_BRBIP32PubKey
#define BRBIP32PrivKey		DID_BRBIP32PrivKey
#define BRBIP32PrivKeyList		DID_BRBIP32PrivKeyList
#define BRBIP44PrivKeyList		DID_BRBIP44PrivKeyList
#define BRBIP32PubKeyPath		DID_BRBIP32PubKeyPath
#define BRBIP32vPubKeyPath		DID_BRBIP32vPubKeyPath
#define BRBIP32PrivKeyPath		DID_BRBIP32PrivKeyPath
#define BRBIP32vPrivKeyPath		DID_BRBIP32vPrivKeyPath
#define BRBIP32SerializeMasterPrivKey		DID_BRBIP32SerializeMasterPrivKey
#define BRBIP32ParseMasterPrivKey		DID_BRBIP32ParseMasterPrivKey
#define BRBIP32SerializeMasterPubKey		DID_BRBIP32SerializeMasterPubKey
#define BRBIP32ParseMasterPubKey		DID_BRBIP32ParseMasterPubKey
#define BRBIP32APIAuthKey		DID_BRBIP32APIAuthKey
#define BRBIP32BitIDKey		DID_BRBIP32BitIDKey
#define BRBIP32vRootFromSeed		DID_BRBIP32vRootFromSeed
#define BRBIP32PrivKeyPathFromRoot		DID_BRBIP32PrivKeyPathFromRoot
#define BRBIP32vPrivKeyPathFromRoot		DID_BRBIP32vPrivKeyPathFromRoot
#define BRBIP32vPubKeyPathWithParentKey		DID_BRBIP32vPubKeyPathWithParentKey
#define BRVarInt		DID_BRVarInt
#define BRVarIntSet		DID_BRVarIntSet
#define BRVarIntSize		DID_BRVarIntSize
#define BRScriptElements		DID_BRScriptElements
#define BRScriptData		DID_BRScriptData
#define BRScriptPushData		DID_BRScriptPushData
#define BRScriptPKH		DID_BRScriptPKH
#define BRAddressFromScriptPubKey		DID_BRAddressFromScriptPubKey
#define BRAddressFromScriptSig		DID_BRAddressFromScriptSig
#define BRAddressFromWitness		DID_BRAddressFromWitness
#define BRAddressFromHash160		DID_BRAddressFromHash160
#define BRAddressScriptPubKey		DID_BRAddressScriptPubKey
#define BRAddressHash160		DID_BRAddressHash160
#define BRAddressIsValid		DID_BRAddressIsValid
#define BRAddressHash		DID_BRAddressHash
#define BRMurmur3_32		DID_BRMurmur3_32
#define BRAddressEq		DID_BRAddressEq
#define BRSetFree		DID_BRSetFree
#define BRSetNew		DID_BRSetNew
#define BRSetAdd		DID_BRSetAdd
#define BRSetRemove		DID_BRSetRemove
#define BRSetClear		DID_BRSetClear
#define BRSetCount		DID_BRSetCount
#define BRSetContains		DID_BRSetContains
#define BRSetIntersects		DID_BRSetIntersects
#define BRSetGet		DID_BRSetGet
#define BRSetIterate		DID_BRSetIterate
#define BRSetAll		DID_BRSetAll
#define BRSetApply		DID_BRSetApply
#define BRSetUnion		DID_BRSetUnion
#define BRSetMinus		DID_BRSetMinus
#define BRSetIntersect		DID_BRSetIntersect
#define BRSetFree		DID_BRSetFree
#define BRSetIterate		DID_BRSetIterate
#define BRSetIterate		DID_BRSetIterate
#define BRKeyECIESAES128SHA256Encrypt		DID_BRKeyECIESAES128SHA256Encrypt
#define BRKeyECIESAES128SHA256Decrypt		DID_BRKeyECIESAES128SHA256Decrypt
#define BRKeyPigeonPairingKey		DID_BRKeyPigeonPairingKey
#define BRKeyPigeonEncrypt		DID_BRKeyPigeonEncrypt
#define BRKeyPigeonDecrypt		DID_BRKeyPigeonDecrypt

#endif /* __BRNAMEFIX_H__ */
