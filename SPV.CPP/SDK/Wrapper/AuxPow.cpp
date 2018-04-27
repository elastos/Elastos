// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "BRTransaction.h"
#include "BRMerkleBlock.h"
#include "BRAddress.h"

#include "AuxPow.h"

namespace Elastos {
	namespace SDK {

		uint64_t ReadVarInt(ByteStream &istream, uint8_t h)
		{
			if (h < 0xFD) {
				return (uint64_t)h;
			} else if (h == 0xfd) {
				uint8_t txInCountData[16 / 8];
				istream.getBytes(txInCountData, 16 / 8);
				return (uint64_t)UInt16GetLE(txInCountData);
			} else if (h == 0xfe) {
				uint8_t txInCountData[32 / 8];
				istream.getBytes(txInCountData, 32 / 8);
				return (uint64_t)UInt32GetLE(txInCountData);
			} else if (h == 0xff) {
				uint8_t txInCountData[64 / 8];
				istream.getBytes(txInCountData, 64 / 8);
				return (uint64_t)UInt64GetLE(txInCountData);
			}

			return 0;
		}

		AuxPow::AuxPow() {
			_btcTransaction = BRTransactionNew();
			_parBlockHeader = BRMerkleBlockNew();
		}

		AuxPow::~AuxPow() {
			BRTransactionFree(_btcTransaction);
			BRMerkleBlockFree(_parBlockHeader);
		}

		void AuxPow::Serialize(ByteStream &ostream) const {
			serializeBtcTransaction(ostream);

			uint8_t parentHashData[256 / 8];
			UInt256Set(parentHashData, _parentHash);
			ostream.putBytes(parentHashData, 256 / 8);

			uint8_t auxMerkleIndexData[32 / 8];
			UInt32SetLE(auxMerkleIndexData, _auxMerkleIndex);
			ostream.putBytes(auxMerkleIndexData, 32 / 8);

			uint8_t auxMerkleBranchCountData[64 / 8];
			UInt64SetLE(auxMerkleBranchCountData, uint64_t(_auxMerkleBranch.size()));
			ostream.putBytes(auxMerkleBranchCountData, 64 / 8);

			uint8_t auxMerkleBranchData[256 / 8];
			for (uint64_t i = 0; i < _auxMerkleBranch.size(); ++i) {
				UInt256Set(auxMerkleBranchData, _auxMerkleBranch[i]);
				ostream.putBytes(auxMerkleBranchData, 256 / 8);
			}

			uint8_t parMerkleIndexData[32 / 8];
			UInt32SetLE(parMerkleIndexData, _parMerkleIndex);
			ostream.putBytes(parMerkleIndexData, 32 / 8);

			uint8_t parCoinBaseMerkleCountData[64 / 8];
			UInt64SetLE(parCoinBaseMerkleCountData, uint64_t(_parCoinBaseMerkle.size()));
			ostream.putBytes(parCoinBaseMerkleCountData, 64 / 8);

			uint8_t parCoinBaseMerkleData[256 / 8];
			for (uint64_t i = 0; i < _parCoinBaseMerkle.size(); ++i) {
				UInt256Set(parCoinBaseMerkleData, _parCoinBaseMerkle[i]);
				ostream.putBytes(parCoinBaseMerkleData, 256 / 8);
			}

			serializeBtcBlockHeader(ostream);
		}

		void AuxPow::Deserialize(ByteStream &istream) {
			deserializeBtcTransaction(istream);

			uint8_t parentHashData[256 / 8];
			istream.getBytes(parentHashData, 256 / 8);
			UInt256Get(&_parentHash, parentHashData);

			uint8_t auxMerkleIndexData[32 / 8];
			istream.getBytes(auxMerkleIndexData, 32 / 8);
			_auxMerkleIndex = UInt32GetLE(auxMerkleIndexData);

			uint8_t auxMerkleBranchCountHead;
			istream.getBytes(&auxMerkleBranchCountHead, 8 / 8);
			uint64_t auxMerkleBranchCount = ReadVarInt(istream, auxMerkleBranchCountHead);

			_auxMerkleBranch.resize(auxMerkleBranchCount);
			uint8_t auxMerkleBranchData[256 / 8];
			for (uint64_t i = 0; i < auxMerkleBranchCount; ++i) {
				istream.getBytes(auxMerkleBranchData, 256 / 8);
				UInt256Get(&_auxMerkleBranch[i], auxMerkleBranchData);
			}

			uint8_t parMerkleIndexData[32 / 8];
			istream.getBytes(parMerkleIndexData, 32 / 8);
			_parMerkleIndex = UInt32GetLE(parMerkleIndexData);

			uint8_t parCoinBaseMerkleCountHead;
			istream.getBytes(&parCoinBaseMerkleCountHead, 8 / 8);
			uint64_t parCoinBaseMerkleCount = ReadVarInt(istream, parCoinBaseMerkleCountHead);


			_parCoinBaseMerkle.resize(parCoinBaseMerkleCount);
			uint8_t parCoinBaseMerkleData[256 / 8];
			for (uint64_t i = 0; i < parCoinBaseMerkleCount; ++i) {
				istream.getBytes(parCoinBaseMerkleData, 256 / 8);
				UInt256Get(&_parCoinBaseMerkle[i], parCoinBaseMerkleData);
			}

			deserializeBtcBlockHeader(istream);
		}

		void AuxPow::serializeBtcTransaction(ByteStream &ostream) const {
			uint8_t versionData[32 / 8];
			UInt32SetLE(versionData, _btcTransaction->version);
			ostream.putBytes(versionData, 32 / 8);

			uint8_t txInCountData[64 / 8];
			UInt64SetLE(txInCountData, uint64_t(_btcTransaction->inCount));
			ostream.putBytes(txInCountData, 64 / 8);

			for (uint64_t i = 0; i < _btcTransaction->inCount; ++i) {
				serializeBtcTxIn(ostream, _btcTransaction->inputs[i]);
			}

			uint8_t txOutCountData[64 / 8];
			UInt64SetLE(txOutCountData, uint64_t(_btcTransaction->outCount));
			ostream.putBytes(txOutCountData, 64 / 8);

			for (uint64_t i = 0; i < _btcTransaction->outCount; ++i) {
				serializeBtcTxOut(ostream, _btcTransaction->outputs[i]);
			}

			uint8_t lockTimeData[32 / 8];
			UInt32SetLE(lockTimeData, _btcTransaction->lockTime);
			ostream.putBytes(lockTimeData, 32 / 8);
		}

		void AuxPow::deserializeBtcTransaction(ByteStream &istream) {
			uint8_t versionData[32 / 8];
			istream.getBytes(versionData, 32 / 8);
			_btcTransaction->version = UInt32GetLE(versionData);

			uint8_t txInCountHead;
			istream.getBytes(&txInCountHead, 8 / 8);
			_btcTransaction->inCount = (size_t)ReadVarInt(istream, txInCountHead);

			for (uint64_t i = 0; i < _btcTransaction->inCount; ++i) {
				deserializeBtcTxIn(istream, _btcTransaction->inputs[i]);
			}

			uint8_t txOutCountHead;
			istream.getBytes(&txOutCountHead, 8 / 8);
			_btcTransaction->outCount = (size_t)ReadVarInt(istream, txOutCountHead);

			for (uint64_t i = 0; i < _btcTransaction->outCount; ++i) {
				deserializeBtcTxOut(istream, _btcTransaction->outputs[i]);
			}

			uint8_t lockTimeData[32 / 8];
			istream.getBytes(lockTimeData, 32 / 8);
			_btcTransaction->lockTime = UInt32GetLE(lockTimeData);
		}

		void AuxPow::serializeBtcTxIn(ByteStream &ostream, const BRTxInput &input) const {
			uint8_t hashData[256 / 8];
			UInt256Set(hashData, input.txHash);
			ostream.putBytes(hashData, 256 / 8);

			uint8_t indexData[32 / 8];
			UInt32SetLE(indexData, input.index);
			ostream.putBytes(indexData, 32 / 8);

			uint8_t signatureScriptLengthData[64 / 8];
			UInt64SetLE(signatureScriptLengthData, uint64_t(input.sigLen));
			ostream.putBytes(indexData, 64 / 8);

			ostream.putBytes(input.signature, input.sigLen);

			uint8_t sequenceData[32 / 8];
			UInt32SetLE(sequenceData, input.sequence);
			ostream.putBytes(sequenceData, 32 / 8);
		}

		void AuxPow::deserializeBtcTxIn(ByteStream &istream, BRTxInput &input) {
			uint8_t hashData[256 / 8];
			istream.getBytes(hashData, 256 / 8);
			UInt256Get(&input.txHash, hashData);

			uint8_t indexData[32 / 8];
			istream.getBytes(indexData, 32 / 8);
			input.index = UInt32GetLE(indexData);

			uint8_t signatureScriptLengthDataHead;
			istream.getBytes(&signatureScriptLengthDataHead, 8 / 8);
			input.sigLen = (size_t)ReadVarInt(istream, signatureScriptLengthDataHead);

			if (input.sigLen != 0) {
				input.signature = (uint8_t *)malloc(input.sigLen * sizeof(uint8_t));
				istream.getBytes(input.signature, input.sigLen);
			}

			uint8_t sequenceData[32 / 8];
			istream.getBytes(sequenceData, 32 / 8);
			input.sequence = UInt32GetLE(sequenceData);
		}

		void AuxPow::serializeBtcTxOut(ByteStream &ostream, const BRTxOutput &output) const {
			uint8_t amountData[64 / 8];
			UInt64SetLE(amountData, output.amount);
			ostream.putBytes(amountData, 64 / 8);

			uint8_t pkScriptLengthData[64 / 8];
			UInt64SetLE(pkScriptLengthData, uint64_t(output.scriptLen));
			ostream.putBytes(pkScriptLengthData, 64 / 8);

			ostream.putBytes(output.script, output.scriptLen);
		}

		void AuxPow::deserializeBtcTxOut(ByteStream &istream, BRTxOutput &output) {
			uint8_t amountData[64 / 8];
			istream.getBytes(amountData, 64 / 8);
			output.amount = UInt64GetLE(amountData);

			uint8_t pkScriptLengthDataHead;
			istream.getBytes(&pkScriptLengthDataHead, 8 / 8);
			output.scriptLen = (size_t)ReadVarInt(istream, pkScriptLengthDataHead);

			if (output.scriptLen != 0) {
				output.script = (uint8_t *)malloc(output.scriptLen * sizeof(uint8_t));
				istream.getBytes(output.script, output.scriptLen);
			}
		}

		void AuxPow::serializeBtcBlockHeader(ByteStream &ostream) const {
			uint8_t versionData[32 / 8];
			UInt32SetLE(versionData, _parBlockHeader->version);
			ostream.putBytes(versionData, 32 / 8);

			uint8_t prevBlockData[256 / 8];
			UInt256Set(prevBlockData, _parBlockHeader->prevBlock);
			ostream.putBytes(prevBlockData, 256 / 8);

			uint8_t merkleRootData[256 / 8];
			UInt256Set(merkleRootData, _parBlockHeader->merkleRoot);
			ostream.putBytes(merkleRootData, 256 / 8);

			uint8_t timeStampData[32 / 8];
			UInt32SetLE(timeStampData, _parBlockHeader->timestamp);
			ostream.putBytes(timeStampData, 32 / 8);

			uint8_t bitsData[32 / 8];
			UInt32SetLE(bitsData, _parBlockHeader->target);
			ostream.putBytes(bitsData, 32 / 8);

			uint8_t nonceData[32 / 8];
			UInt32SetLE(nonceData, _parBlockHeader->nonce);
			ostream.putBytes(nonceData, 32 / 8);
		}

		void AuxPow::deserializeBtcBlockHeader(ByteStream &istream) {
			uint8_t versionData[32 / 8];
			istream.getBytes(versionData, 32 / 8);
			_parBlockHeader->version = UInt32GetLE(versionData);

			uint8_t prevBlockData[256 / 8];
			istream.getBytes(prevBlockData, 256 / 8);
			UInt256Get(&_parBlockHeader->prevBlock, prevBlockData);

			uint8_t merkleRootData[256 / 8];
			istream.getBytes(merkleRootData, 256 / 8);
			UInt256Get(&_parBlockHeader->merkleRoot, merkleRootData);

			uint8_t timeStampData[32 / 8];
			istream.getBytes(timeStampData, 32 / 8);
			_parBlockHeader->timestamp = UInt32GetLE(timeStampData);

			uint8_t bitsData[32 / 8];
			istream.getBytes(bitsData, 32 / 8);
			_parBlockHeader->target = UInt32GetLE(bitsData);

			uint8_t nonceData[32 / 8];
			istream.getBytes(nonceData, 32 / 8);
			_parBlockHeader->nonce = UInt32GetLE(nonceData);
		}
	}
}