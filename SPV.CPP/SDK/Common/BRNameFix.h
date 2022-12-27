/*
 * Copyright (c) 2020 Elastos Foundation
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

#ifndef __ELASTOS_SDK_BRNAME_FIX_H__
#define __ELASTOS_SDK_BRNAME_FIX_H__

#define BRTxInputSetAddress		SPV_BRTxInputSetAddress
#define BRTxInputSetScript		SPV_BRTxInputSetScript
#define BRTxInputSetSignature		SPV_BRTxInputSetSignature
#define BRTxInputSetWitness		SPV_BRTxInputSetWitness
#define BRTxOutputSetAddress		SPV_BRTxOutputSetAddress
#define BRTxOutputSetScript		SPV_BRTxOutputSetScript
#define BRTransactionFree		SPV_BRTransactionFree
#define BRTransactionNew		SPV_BRTransactionNew
#define BRTransactionCopy		SPV_BRTransactionCopy
#define BRTransactionParse		SPV_BRTransactionParse
#define BRTransactionSerialize		SPV_BRTransactionSerialize
#define BRTransactionAddInput		SPV_BRTransactionAddInput
#define BRTransactionAddOutput		SPV_BRTransactionAddOutput
#define BRTransactionShuffleOutputs		SPV_BRTransactionShuffleOutputs
#define BRTransactionSize		SPV_BRTransactionSize
#define BRTransactionVSize		SPV_BRTransactionVSize
#define BRTransactionStandardFee		SPV_BRTransactionStandardFee
#define BRTransactionIsSigned		SPV_BRTransactionIsSigned
#define BRTransactionSign		SPV_BRTransactionSign
#define BRTransactionIsStandard		SPV_BRTransactionIsStandard
#define BRTransactionHash		SPV_BRTransactionHash
#define BRTransactionEq		SPV_BRTransactionEq
#define BRMerkleBlockFree		SPV_BRMerkleBlockFree
#define BRMerkleBlockNew		SPV_BRMerkleBlockNew
#define BRMerkleBlockCopy		SPV_BRMerkleBlockCopy
#define BRMerkleBlockParse		SPV_BRMerkleBlockParse
#define BRMerkleBlockSerialize		SPV_BRMerkleBlockSerialize
#define BRMerkleBlockTxHashes		SPV_BRMerkleBlockTxHashes
#define BRMerkleBlockSetTxHashes		SPV_BRMerkleBlockSetTxHashes
#define BRMerkleBlockVerifyDifficulty		SPV_BRMerkleBlockVerifyDifficulty
#define BRMerkleBlockIsValid		SPV_BRMerkleBlockIsValid
#define BRMerkleBlockContainsTxHash		SPV_BRMerkleBlockContainsTxHash
#define BRMerkleBlockHash		SPV_BRMerkleBlockHash
#define BRMerkleBlockEq		SPV_BRMerkleBlockEq
#define BRBIP38KeyIsValid		SPV_BRBIP38KeyIsValid
#define BRKeySetBIP38Key		SPV_BRKeySetBIP38Key
#define BRKeyBIP38ItermediateCode		SPV_BRKeyBIP38ItermediateCode
#define BRKeyBIP38ItermediateCodeLS		SPV_BRKeyBIP38ItermediateCodeLS
#define BRKeySetBIP38ItermediateCode		SPV_BRKeySetBIP38ItermediateCode
#define BRKeyBIP38Key		SPV_BRKeyBIP38Key
#define BRBIP39Encode		SPV_BRBIP39Encode
#define BRBIP39Decode		SPV_BRBIP39Decode
#define BRBIP39PhraseIsValid		SPV_BRBIP39PhraseIsValid
#define BRBIP39DeriveKey		SPV_BRBIP39DeriveKey
#define BRSHA1		SPV_BRSHA1
#define BRSHA256		SPV_BRSHA256
#define BRSHA224		SPV_BRSHA224
#define BRSHA256_2		SPV_BRSHA256_2
#define BRSHA384		SPV_BRSHA384
#define BRSHA512		SPV_BRSHA512
#define BRRMD160		SPV_BRRMD160
#define BRHash160		SPV_BRHash160
#define BRSHA3_256		SPV_BRSHA3_256
#define BRKeccak256		SPV_BRKeccak256
#define BRMD5		SPV_BRMD5
#define BRMurmur3_32		SPV_BRMurmur3_32
#define BRSip64		SPV_BRSip64
#define BRHMAC		SPV_BRHMAC
#define BRHMACDRBG		SPV_BRHMACDRBG
#define BRPoly1305		SPV_BRPoly1305
#define BRChacha20		SPV_BRChacha20
#define BRChacha20Poly1305AEADEncrypt		SPV_BRChacha20Poly1305AEADEncrypt
#define BRChacha20Poly1305AEADDecrypt		SPV_BRChacha20Poly1305AEADDecrypt
#define BRAESECBEncrypt		SPV_BRAESECBEncrypt
#define BRAESECBDecrypt		SPV_BRAESECBDecrypt
#define BRAESCTR		SPV_BRAESCTR
#define BRAESCTR_OFFSET		SPV_BRAESCTR_OFFSET
#define BRPBKDF2		SPV_BRPBKDF2
#define BRScrypt		SPV_BRScrypt
#define BRRand		SPV_BRRand
#define BRSecp256k1ModAdd		SPV_BRSecp256k1ModAdd
#define BRSecp256k1ModMul		SPV_BRSecp256k1ModMul
#define BRSecp256k1PointGen		SPV_BRSecp256k1PointGen
#define BRSecp256k1PointAdd		SPV_BRSecp256k1PointAdd
#define BRSecp256k1PointMul		SPV_BRSecp256k1PointMul
#define BRPrivKeyIsValid		SPV_BRPrivKeyIsValid
#define BRKeySetSecret		SPV_BRKeySetSecret
#define BRKeySetPrivKey		SPV_BRKeySetPrivKey
#define BRKeySetPubKey		SPV_BRKeySetPubKey
#define BRKeyPrivKey		SPV_BRKeyPrivKey
#define BRKeyPubKey		SPV_BRKeyPubKey
#define BRKeyHash160		SPV_BRKeyHash160
#define BRKeyAddress		SPV_BRKeyAddress
#define BRKeyLegacyAddr		SPV_BRKeyLegacyAddr
#define BRKeySign		SPV_BRKeySign
#define BRKeyVerify		SPV_BRKeyVerify
#define BRKeyClean		SPV_BRKeyClean
#define BRKeyCompactSign		SPV_BRKeyCompactSign
#define BRKeyRecoverPubKey		SPV_BRKeyRecoverPubKey
#define BRKeyECDH		SPV_BRKeyECDH
#define BRKeyCompactSignEthereum		SPV_BRKeyCompactSignEthereum
#define BRKeyRecoverPubKeyEthereum		SPV_BRKeyRecoverPubKeyEthereum
#define BRBase58Encode		SPV_BRBase58Encode
#define BRBase58Decode		SPV_BRBase58Decode
#define BRBase58CheckEncode		SPV_BRBase58CheckEncode
#define BRBase58CheckDecode		SPV_BRBase58CheckDecode
#define BRAssertInstall		SPV_BRAssertInstall
#define BRAssertUninstall		SPV_BRAssertUninstall
#define BRAssertRemoveRecovery		SPV_BRAssertRemoveRecovery
#define BRPeerManagerDisconnect		SPV_BRPeerManagerDisconnect
#define BREthereumEWMDisconnect		SPV_BREthereumEWMDisconnect
#define BRAssertDefineRecover		SPV_BRAssertDefineRecover
#define BRBech32Decode		SPV_BRBech32Decode
#define BRBech32Encode		SPV_BRBech32Encode
#define BRBIP32MasterPubKey		SPV_BRBIP32MasterPubKey
#define BRBIP32PubKey		SPV_BRBIP32PubKey
#define BRBIP32PrivKey		SPV_BRBIP32PrivKey
#define BRBIP32PrivKeyList		SPV_BRBIP32PrivKeyList
#define BRBIP32PrivKeyPath		SPV_BRBIP32PrivKeyPath
#define BRBIP32vPrivKeyPath		SPV_BRBIP32vPrivKeyPath
#define BRBIP32SerializeMasterPrivKey		SPV_BRBIP32SerializeMasterPrivKey
#define BRBIP32ParseMasterPrivKey		SPV_BRBIP32ParseMasterPrivKey
#define BRBIP32SerializeMasterPubKey		SPV_BRBIP32SerializeMasterPubKey
#define BRBIP32ParseMasterPubKey		SPV_BRBIP32ParseMasterPubKey
#define BRBIP32APIAuthKey		SPV_BRBIP32APIAuthKey
#define BRBIP32BitIDKey		SPV_BRBIP32BitIDKey
#define BRVarInt		SPV_BRVarInt
#define BRVarIntSet		SPV_BRVarIntSet
#define BRVarIntSize		SPV_BRVarIntSize
#define BRScriptElements		SPV_BRScriptElements
#define BRScriptData		SPV_BRScriptData
#define BRScriptPushData		SPV_BRScriptPushData
#define BRScriptPKH		SPV_BRScriptPKH
#define BRAddressFromScriptPubKey		SPV_BRAddressFromScriptPubKey
#define BRAddressFromScriptSig		SPV_BRAddressFromScriptSig
#define BRAddressFromWitness		SPV_BRAddressFromWitness
#define BRAddressFromHash160		SPV_BRAddressFromHash160
#define BRAddressScriptPubKey		SPV_BRAddressScriptPubKey
#define BRAddressHash160		SPV_BRAddressHash160
#define BRAddressIsValid		SPV_BRAddressIsValid
#define BRAddressHash		SPV_BRAddressHash
#define BRSetFree		SPV_BRSetFree
#define BRSetNew		SPV_BRSetNew
#define BRSetAdd		SPV_BRSetAdd
#define BRSetRemove		SPV_BRSetRemove
#define BRSetClear		SPV_BRSetClear
#define BRSetCount		SPV_BRSetCount
#define BRSetContains		SPV_BRSetContains
#define BRSetIntersects		SPV_BRSetIntersects
#define BRSetGet		SPV_BRSetGet
#define BRSetIterate		SPV_BRSetIterate
#define BRSetAll		SPV_BRSetAll
#define BRSetApply		SPV_BRSetApply
#define BRSetUnion		SPV_BRSetUnion
#define BRSetMinus		SPV_BRSetMinus
#define BRSetIntersect		SPV_BRSetIntersect
#define BRKeyECIESAES128SHA256Encrypt		SPV_BRKeyECIESAES128SHA256Encrypt
#define BRKeyECIESAES128SHA256Decrypt		SPV_BRKeyECIESAES128SHA256Decrypt
#define BRKeyPigeonPairingKey		SPV_BRKeyPigeonPairingKey
#define BRKeyPigeonEncrypt		SPV_BRKeyPigeonEncrypt
#define BRKeyPigeonDecrypt		SPV_BRKeyPigeonDecrypt

#endif
