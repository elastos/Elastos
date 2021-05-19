// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "AuxPow.h"

#include <Common/Utils.h>
#include <Common/Log.h>
#include <Common/hash.h>

#include <stdlib.h>

namespace Elastos {
	namespace ElaWallet {

		AuxPow::AuxPow() {
			_parCoinBaseTx = BRTransactionNew();
			_parBlockHeader = BRMerkleBlockNew();
			_auxMerkleIndex = 0;
			_parMerkleIndex = 0;
			_parentHash = 0;
		}

		AuxPow::~AuxPow() {
			if (_parCoinBaseTx != nullptr)
				BRTransactionFree(_parCoinBaseTx);
			if (_parBlockHeader)
				BRMerkleBlockFree(_parBlockHeader);
		}

		void AuxPow::Serialize(ByteStream &ostream) const {
			SerializeBtcTransaction(ostream, _parCoinBaseTx);

			ostream.WriteBytes(_parentHash);

			ostream.WriteVarUint(uint64_t(_parCoinBaseMerkle.size()));
			for (uint64_t i = 0; i < _parCoinBaseMerkle.size(); ++i) {
				ostream.WriteBytes(_parCoinBaseMerkle[i]);
			}

			ostream.WriteUint32(_parMerkleIndex);

			ostream.WriteVarUint(uint64_t(_auxMerkleBranch.size()));
			for (uint64_t i = 0; i < _auxMerkleBranch.size(); ++i) {
				ostream.WriteBytes(_auxMerkleBranch[i]);
			}

			ostream.WriteUint32(_auxMerkleIndex);

			return SerializeBtcBlockHeader(ostream, _parBlockHeader);
		}

		bool AuxPow::Deserialize(const ByteStream &istream) {
			if (!DeserializeBtcTransaction(istream, _parCoinBaseTx)) {
				Log::error("deserialize AuxPow btc tx error");
				return false;
			}

			if (!istream.ReadBytes(_parentHash)) {
				Log::error("deserialize AuxPow parentHash error");
				return false;
			}

			uint64_t parCoinBaseMerkleCount = 0;
			if (!istream.ReadVarUint(parCoinBaseMerkleCount)) {
				Log::error("deserialize AuxPow parCoinBaseMerkle size error");
				return false;
			}

			_parCoinBaseMerkle.resize(parCoinBaseMerkleCount);
			for (uint64_t i = 0; i < parCoinBaseMerkleCount; ++i) {
				if (!istream.ReadBytes(_parCoinBaseMerkle[i])) {
					Log::error("deserialize AuxPow parCoinBaseMerkle[{}] error", i);
					return false;
				}
			}

			if (!istream.ReadUint32(_parMerkleIndex)) {
				Log::error("deserialize AuxPow parMerkleIndex error");
				return false;
			}

			uint64_t auxMerkleBranchCount = 0;
			if (!istream.ReadVarUint(auxMerkleBranchCount)) {
				Log::error("deserialize AuxPow auxMerkleBranchCount error");
				return false;
			}

			_auxMerkleBranch.resize(auxMerkleBranchCount);
			for (uint64_t i = 0; i < auxMerkleBranchCount; ++i) {
				if (!istream.ReadBytes(_auxMerkleBranch[i])) {
					Log::error("deserialize AuxPow auxMerkleBranch error");
					return false;
				}
			}

			if (!istream.ReadUint32(_auxMerkleIndex)) {
				Log::error("deserialize AuxPow auxMerkleIndex error");
				return false;
			}

			if (!DeserializeBtcBlockHeader(istream, _parBlockHeader)) {
				Log::error("deserialize AuxPow btc block header error");
				return false;
			}

			return true;
		}

		void AuxPow::SerializeBtcTransaction(ByteStream &ostream, const BRTransaction *tx) const {
			ostream.WriteUint32(tx->version);

			ostream.WriteVarUint(uint64_t(tx->inCount));
			for (uint64_t i = 0; i < tx->inCount; ++i) {
				SerializeBtcTxIn(ostream, &tx->inputs[i]);
			}

			ostream.WriteVarUint(uint64_t(tx->outCount));
			for (uint64_t i = 0; i < tx->outCount; ++i) {
				SerializeBtcTxOut(ostream, &tx->outputs[i]);
			}

			ostream.WriteUint32(tx->lockTime);
		}

		bool AuxPow::DeserializeBtcTransaction(const ByteStream &istream, BRTransaction *tx) {
			if (!istream.ReadUint32(tx->version)) {
				Log::error("deserialize version error");
				return false;
			}

			uint64_t inCount = 0;
			if (!istream.ReadVarUint(inCount)) {
				Log::error("deserialize inCount error");
				return false;
			}
			for (uint64_t i = 0; i < inCount; ++i) {
				if (!DeserializeBtcTxIn(istream, tx)) {
					Log::error("deserialize input[{}] error", i);
					return false;
				}
			}

			uint64_t outCount = 0;
			if (!istream.ReadVarUint(outCount)) {
				Log::error("deserialize outCount error");
				return false;
			}
			for (uint64_t i = 0; i < outCount; ++i) {
				if (!DeserializeBtcTxOut(istream, tx)) {
					Log::error("deserialize output[{}] error", i);
					return false;
				}
			}

			if (!istream.ReadUint32(tx->lockTime)) {
				Log::error("deserialize lockTime error");
				return false;
			}

			return true;
		}

		void AuxPow::SerializeBtcTxIn(ByteStream &ostream, const BRTxInput *in) const {
			ostream.WriteBytes(in->txHash.u8, sizeof(in->txHash));
			ostream.WriteUint32(in->index);
			ostream.WriteVarBytes(in->signature, in->sigLen);
			ostream.WriteUint32(in->sequence);
		}

		bool AuxPow::DeserializeBtcTxIn(const ByteStream &istream, BRTransaction *tx) {
			UInt256 txHash;
			if (!istream.ReadBytes(txHash.u8, sizeof(txHash))) {
				Log::error("deserialize txHash error");
				return false;
			}

			uint32_t index = 0;
			if (!istream.ReadUint32(index)) {
				Log::error("deserialize index error");
				return false;
			}

			bytes_t signature;
			if (!istream.ReadVarBytes(signature)) {
				Log::error("deserialize signature error");
				return false;
			}

			uint32_t sequence = 0;
			if (!istream.ReadUint32(sequence)) {
				Log::error("deserialize sequence error");
				return false;
			}

			BRTransactionAddInput(tx, txHash, index, 0, nullptr, 0,
								  signature.size() > 0 ? &signature[0] : nullptr, signature.size(), nullptr, 0, sequence);
			return true;
		}

		void AuxPow::SerializeBtcTxOut(ByteStream &ostream, const BRTxOutput *out) const {
			ostream.WriteUint64(out->amount);
			ostream.WriteVarBytes(out->script, out->scriptLen);
		}

		bool AuxPow::DeserializeBtcTxOut(const ByteStream &istream, BRTransaction *tx) {
			uint64_t amount = 0;
			if (!istream.ReadUint64(amount)) {
				Log::error("deserialize amount error");
				return false;
			}

			bytes_t script;
			if (!istream.ReadVarBytes(script)) {
				Log::error("deserialize script error");
				return false;
			}

			BRTransactionAddOutput(tx, amount, script.size() > 0 ? &script[0] : nullptr, script.size());
			return true;
		}

		void AuxPow::SerializeBtcBlockHeader(ByteStream &ostream, const BRMerkleBlock *b) const {
			ostream.WriteUint32(b->version);
			ostream.WriteBytes(b->prevBlock.u8, sizeof(b->prevBlock));
			ostream.WriteBytes(b->merkleRoot.u8, sizeof(b->merkleRoot));
			ostream.WriteUint32(b->timestamp);
			ostream.WriteUint32(b->target);
			ostream.WriteUint32(b->nonce);
		}

		bool AuxPow::DeserializeBtcBlockHeader(const ByteStream &istream, BRMerkleBlock *b) {
			if (!istream.ReadUint32(b->version)) {
				Log::error("deserialize version error");
				return false;
			}

			if (!istream.ReadBytes(b->prevBlock.u8, sizeof(b->prevBlock))) {
				Log::error("deserialize prevBlock error");
				return false;
			}

			if (!istream.ReadBytes(b->merkleRoot.u8, sizeof(b->merkleRoot))) {
				Log::error("deserialize merkleRoot error");
				return false;
			}

			if (!istream.ReadUint32(b->timestamp)) {
				Log::error("deserialize timestamp error");
				return false;
			}

			if (!istream.ReadUint32(b->target)) {
				Log::error("deserialize target error");
				return false;
			}

			if (!istream.ReadUint32(b->nonce)) {
				Log::error("deserialize nonce error");
				return false;
			}
			return true;
		}

		uint256 AuxPow::GetParBlockHeaderHash() const {
			ByteStream stream;
			SerializeBtcBlockHeader(stream, _parBlockHeader);
			return uint256(sha256_2(stream.GetBytes()));
		}

		AuxPow::AuxPow(const AuxPow &auxPow) {
			operator=(auxPow);
		}

		void AuxPow::SetBTCTransaction(BRTransaction *transaction) {
			if (_parCoinBaseTx != nullptr)
				BRTransactionFree(_parCoinBaseTx);
			_parCoinBaseTx = transaction;
		}

		void AuxPow::SetParBlockHeader(BRMerkleBlock *block) {
			if (_parBlockHeader != nullptr)
				BRMerkleBlockFree(_parBlockHeader);
			_parBlockHeader = block;
		}

		AuxPow &AuxPow::operator=(const AuxPow &auxPow) {
			_auxMerkleBranch = auxPow._auxMerkleBranch;
			_parCoinBaseMerkle = auxPow._parCoinBaseMerkle;
			_auxMerkleIndex = auxPow._auxMerkleIndex;
			SetBTCTransaction(BRTransactionCopy(auxPow._parCoinBaseTx));
			_parMerkleIndex = auxPow._parMerkleIndex;
			SetParBlockHeader(BRMerkleBlockCopy(auxPow._parBlockHeader));
			_parentHash = auxPow._parentHash;
			return *this;
		}

		BRTransaction *AuxPow::GetBTCTransaction() const {
			return _parCoinBaseTx;
		}

		BRMerkleBlock *AuxPow::GetParBlockHeader() const {
			return _parBlockHeader;
		}

		void AuxPow::SetAuxMerkleBranch(const std::vector<uint256> &hashes) {
			_auxMerkleBranch = hashes;
		}

		void AuxPow::SetCoinBaseMerkle(const std::vector<uint256> &hashes) {
			_parCoinBaseMerkle = hashes;
		}

		void AuxPow::SetAuxMerkleIndex(uint32_t index) {
			_auxMerkleIndex = index;
		}

		void AuxPow::SetParMerkleIndex(uint32_t index) {
			_parMerkleIndex = index;
		}

		void AuxPow::SetParentHash(const uint256 &hash) {
			_parentHash = hash;
		}

		uint32_t AuxPow::GetAuxMerkleIndex() const {
			return _auxMerkleIndex;
		}

		uint32_t AuxPow::GetParMerkleIndex() const {
			return _parMerkleIndex;
		}

		const uint256 &AuxPow::GetParentHash() const {
			return _parentHash;
		}

		const std::vector<uint256> &AuxPow::GetAuxMerkleBranch() const {
			return _auxMerkleBranch;
		}

		const std::vector<uint256> &AuxPow::GetParCoinBaseMerkle() const {
			return _parCoinBaseMerkle;
		}

	}
}
