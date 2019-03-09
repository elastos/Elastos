// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "AuxPow.h"

#include <SDK/Common/Utils.h>
#include <SDK/Common/Log.h>

#include <Core/BRMerkleBlock.h>
#include <Core/BRAddress.h>
#include <Core/BRTransaction.h>
#include <Core/BRTransaction.h>
#include <Core/BRMerkleBlock.h>

#include <stdlib.h>

namespace Elastos {
	namespace ElaWallet {

		AuxPow::AuxPow() {
			_parCoinBaseTx = BRTransactionNew();
			_parBlockHeader = BRMerkleBlockNew(nullptr);
			_auxMerkleIndex = 0;
			_parMerkleIndex = 0;
			_parentHash = UINT256_ZERO;
		}

		AuxPow::~AuxPow() {
			if (_parCoinBaseTx != nullptr)
				BRTransactionFree(_parCoinBaseTx);
			if (_parBlockHeader)
				BRMerkleBlockFree(nullptr, _parBlockHeader);
		}

		void AuxPow::Serialize(ByteStream &ostream) const {
			SerializeBtcTransaction(ostream, _parCoinBaseTx);

			ostream.WriteBytes(_parentHash.u8, sizeof(UInt256));

			ostream.writeVarUint(uint64_t(_parCoinBaseMerkle.size()));
			for (uint64_t i = 0; i < _parCoinBaseMerkle.size(); ++i) {
				ostream.WriteBytes(_parCoinBaseMerkle[i].u8, sizeof(UInt256));
			}

			ostream.WriteUint32(_parMerkleIndex);

			ostream.writeVarUint(uint64_t(_auxMerkleBranch.size()));
			for (uint64_t i = 0; i < _auxMerkleBranch.size(); ++i) {
				ostream.WriteBytes(_auxMerkleBranch[i].u8, sizeof(UInt256));
			}

			ostream.WriteUint32(_auxMerkleIndex);

			SerializeBtcBlockHeader(ostream, _parBlockHeader);
		}

		bool AuxPow::Deserialize(ByteStream &istream) {
			if (!DeserializeBtcTransaction(istream, _parCoinBaseTx)) {
				Log::error("deserialize AuxPow btc tx error");
				return false;
			}

			if (!istream.ReadBytes(_parentHash.u8, sizeof(UInt256))) {
				Log::error("deserialize AuxPow parentHash error");
				return false;
			}

			uint64_t parCoinBaseMerkleCount = 0;
			if (!istream.readVarUint(parCoinBaseMerkleCount)) {
				Log::error("deserialize AuxPow parCoinBaseMerkle size error");
				return false;
			}

			_parCoinBaseMerkle.resize(parCoinBaseMerkleCount);
			for (uint64_t i = 0; i < parCoinBaseMerkleCount; ++i) {
				if (!istream.ReadBytes(_parCoinBaseMerkle[i].u8, sizeof(UInt256))) {
					Log::error("deserialize AuxPow parCoinBaseMerkle[{}] error", i);
					return false;
				}
			}

			if (!istream.ReadUint32(_parMerkleIndex)) {
				Log::error("deserialize AuxPow parMerkleIndex error");
				return false;
			}

			uint64_t auxMerkleBranchCount = 0;
			if (!istream.readVarUint(auxMerkleBranchCount)) {
				Log::error("deserialize AuxPow auxMerkleBranchCount error");
				return false;
			}

			_auxMerkleBranch.resize(auxMerkleBranchCount);
			for (uint64_t i = 0; i < auxMerkleBranchCount; ++i) {
				if (!istream.ReadBytes(_auxMerkleBranch[i].u8, sizeof(UInt256))) {
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

			ostream.writeVarUint(uint64_t(tx->inCount));
			for (uint64_t i = 0; i < tx->inCount; ++i) {
				SerializeBtcTxIn(ostream, &tx->inputs[i]);
			}

			ostream.writeVarUint(uint64_t(tx->outCount));
			for (uint64_t i = 0; i < tx->outCount; ++i) {
				SerializeBtcTxOut(ostream, &tx->outputs[i]);
			}

			ostream.WriteUint32(tx->lockTime);
		}

		bool AuxPow::DeserializeBtcTransaction(ByteStream &istream, BRTransaction *tx) {
			if (!istream.ReadUint32(tx->version)) {
				Log::error("deserialize version error");
				return false;
			}

			uint64_t inCount = 0;
			if (!istream.readVarUint(inCount)) {
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
			if (!istream.readVarUint(outCount)) {
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
			ostream.WriteBytes(in->txHash.u8, sizeof(UInt256));
			ostream.WriteUint32(in->index);
			ostream.WriteVarBytes(in->signature, in->sigLen);
			ostream.WriteUint32(in->sequence);
		}

		bool AuxPow::DeserializeBtcTxIn(ByteStream &istream, BRTransaction *tx) {
			UInt256 txHash;
			if (!istream.ReadBytes(txHash.u8, sizeof(UInt256))) {
				Log::error("deserialize txHash error");
				return false;
			}

			uint32_t index = 0;
			if (!istream.ReadUint32(index)) {
				Log::error("deserialize index error");
				return false;
			}

			CMBlock signature;
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
								  signature.GetSize() > 0 ? (uint8_t *)signature : nullptr, signature.GetSize(), sequence);
			return true;
		}

		void AuxPow::SerializeBtcTxOut(ByteStream &ostream, const BRTxOutput *out) const {
			ostream.WriteUint64(out->amount);
			ostream.WriteVarBytes(out->script, out->scriptLen);
		}

		bool AuxPow::DeserializeBtcTxOut(ByteStream &istream, BRTransaction *tx) {
			uint64_t amount = 0;
			if (!istream.ReadUint64(amount)) {
				Log::error("deserialize amount error");
				return false;
			}

			CMBlock script;
			if (!istream.ReadVarBytes(script)) {
				Log::error("deserialize script error");
				return false;
			}

			BRTransactionAddOutput(tx, amount, script.GetSize() > 0 ? (uint8_t *)script : nullptr, script.GetSize());
			return true;
		}

		void AuxPow::SerializeBtcBlockHeader(ByteStream &ostream, const BRMerkleBlock *b) const {
			ostream.WriteUint32(b->version);
			ostream.WriteBytes(b->prevBlock.u8, sizeof(UInt256));
			ostream.WriteBytes(b->merkleRoot.u8, sizeof(UInt256));
			ostream.WriteUint32(b->timestamp);
			ostream.WriteUint32(b->target);
			ostream.WriteUint32(b->nonce);
		}

		bool AuxPow::DeserializeBtcBlockHeader(ByteStream &istream, BRMerkleBlock *b) {
			if (!istream.ReadUint32(b->version)) {
				Log::error("deserialize version error");
				return false;
			}

			if (!istream.ReadBytes(b->prevBlock.u8, sizeof(UInt256))) {
				Log::error("deserialize prevBlock error");
				return false;
			}

			if (!istream.ReadBytes(b->merkleRoot.u8, sizeof(UInt256))) {
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

		UInt256 AuxPow::GetParBlockHeaderHash() const {
			ByteStream stream;
			SerializeBtcBlockHeader(stream, _parBlockHeader);
			UInt256 hash = UINT256_ZERO;
			CMBlock buf = stream.GetBuffer();
			BRSHA256_2(&hash, buf, buf.GetSize());
			return hash;
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
				BRMerkleBlockFree(nullptr, _parBlockHeader);
			_parBlockHeader = block;
		}

		AuxPow &AuxPow::operator=(const AuxPow &auxPow) {
			_auxMerkleBranch = auxPow._auxMerkleBranch;
			_parCoinBaseMerkle = auxPow._parCoinBaseMerkle;
			_auxMerkleIndex = auxPow._auxMerkleIndex;
			SetBTCTransaction(BRTransactionCopy(auxPow._parCoinBaseTx));
			_parMerkleIndex = auxPow._parMerkleIndex;
			SetParBlockHeader(BRMerkleBlockCopy(auxPow._parBlockHeader));
			UInt256Set(&_parentHash, auxPow._parentHash);
			return *this;
		}

		BRTransaction *AuxPow::GetBTCTransaction() const {
			return _parCoinBaseTx;
		}

		BRMerkleBlock *AuxPow::GetParBlockHeader() const {
			return _parBlockHeader;
		}

		nlohmann::json AuxPow::ToJson() const {
			nlohmann::json jsonData;

			size_t len = _auxMerkleBranch.size();
			std::vector<std::string> auxMerkleBranch(len);
			for (size_t i = 0; i < len; ++i) {
				auxMerkleBranch[i] = Utils::UInt256ToString(_auxMerkleBranch[i], true);
			}
			jsonData["AuxMerkleBranch"] = auxMerkleBranch;

			len = _parCoinBaseMerkle.size();
			std::vector<std::string> parCoinBaseMerkle(len);
			for (size_t i = 0; i < len; ++i) {
				parCoinBaseMerkle[i] = Utils::UInt256ToString(_parCoinBaseMerkle[i], true);
			}
			jsonData["ParCoinBaseMerkle"] = parCoinBaseMerkle;

			jsonData["AuxMerkleIndex"] = _auxMerkleIndex;
			jsonData["Transaction"] = TransactionToJson();
			jsonData["ParMerkleIndex"] = _parMerkleIndex;
			jsonData["ParBlockHeader"] = MerkleBlockToJson();
			jsonData["ParentHash"] = Utils::UInt256ToString(_parentHash, true);

			return jsonData;
		}

		nlohmann::json AuxPow::TransactionToJson() const {
			nlohmann::json jsonData;

			jsonData["TxHash"] = Utils::UInt256ToString(_parCoinBaseTx->txHash, true);
			jsonData["Version"] = _parCoinBaseTx->version;

			std::vector<nlohmann::json> inputs(_parCoinBaseTx->inCount);
			for (size_t i = 0; i < _parCoinBaseTx->inCount; ++i) {
				inputs[i] = TxInputsToJson(_parCoinBaseTx->inputs[i]);;
			}
			jsonData["Inputs"] = inputs;

			std::vector<nlohmann::json> outputs(_parCoinBaseTx->outCount);
			for (size_t i = 0; i < _parCoinBaseTx->outCount; ++i) {
				outputs[i] = TxOutputsToJson(_parCoinBaseTx->outputs[i]);
			}
			jsonData["Outputs"] = outputs;

			jsonData["LockTime"] = _parCoinBaseTx->lockTime;
			jsonData["BlockHeight"] = _parCoinBaseTx->blockHeight;
			jsonData["Timestamp"] = _parCoinBaseTx->timestamp;

			return jsonData;
		}

		nlohmann::json AuxPow::TxInputsToJson(const BRTxInput &input) const {
			nlohmann::json jsonData;

			jsonData["TxHash"] = Utils::UInt256ToString(input.txHash, true);
			jsonData["Index"] = input.index;
			jsonData["Address"] = std::string(input.address);
			jsonData["Amount"] = input.amount;
			jsonData["Script"] = Utils::EncodeHex(input.script, input.scriptLen);
			jsonData["Signature"] = Utils::EncodeHex(input.signature, input.sigLen);
			jsonData["Sequence"] = input.sequence;

			return jsonData;
		}

		nlohmann::json AuxPow::TxOutputsToJson(const BRTxOutput &output) const {
			nlohmann::json jsonData;

			jsonData["Address"] = std::string(output.address);
			jsonData["Amount"] = output.amount;
			jsonData["Script"] = Utils::EncodeHex(output.script, output.scriptLen);

			return jsonData;
		}

		nlohmann::json AuxPow::MerkleBlockToJson() const {
			nlohmann::json jsonData;

			jsonData["BlockHash"] = Utils::UInt256ToString(_parBlockHeader->blockHash, true);
			jsonData["Version"] = _parBlockHeader->version;
			jsonData["PrevBlock"] = Utils::UInt256ToString(_parBlockHeader->prevBlock, true);
			jsonData["MerkleRoot"] = Utils::UInt256ToString(_parBlockHeader->merkleRoot, true);
			jsonData["Timestamp"] = _parBlockHeader->timestamp;
			jsonData["Target"] = _parBlockHeader->target;
			jsonData["Nonce"] = _parBlockHeader->nonce;
			jsonData["TotalTx"] = _parBlockHeader->totalTx;

			std::vector<std::string> hashes(_parBlockHeader->hashesCount);
			for (size_t i = 0; i < _parBlockHeader->hashesCount; ++i) {
				hashes[i] = Utils::UInt256ToString(_parBlockHeader->hashes[i], true);
			}
			jsonData["Hashes"] = hashes;

			jsonData["Flags"] = Utils::EncodeHex(_parBlockHeader->flags, _parBlockHeader->flagsLen);;
			jsonData["Height"] = _parBlockHeader->height;

			return jsonData;
		}

		void AuxPow::FromJson(const nlohmann::json &jsonData) {
			std::vector<std::string> auxMerkleBranch = jsonData["AuxMerkleBranch"];
			_auxMerkleBranch.resize(auxMerkleBranch.size());
			for (size_t i = 0; i < _auxMerkleBranch.size(); ++i) {
				_auxMerkleBranch[i] = Utils::UInt256FromString(auxMerkleBranch[i], true);
			}

			std::vector<std::string> parCoinBaseMerkle = jsonData["ParCoinBaseMerkle"];
			_parCoinBaseMerkle.resize(parCoinBaseMerkle.size());
			for (size_t i = 0; i < parCoinBaseMerkle.size(); ++i) {
				_parCoinBaseMerkle[i] = Utils::UInt256FromString(parCoinBaseMerkle[i], true);
			}

			_auxMerkleIndex = jsonData["AuxMerkleIndex"].get<uint32_t>();
			TransactionFromJson(jsonData["Transaction"]);
			_parMerkleIndex = jsonData["ParMerkleIndex"].get<uint32_t>();
			MerkleBlockFromJson(jsonData["ParBlockHeader"]);
			_parentHash = Utils::UInt256FromString(jsonData["ParentHash"].get<std::string>(), true);
		}

		void AuxPow::TransactionFromJson(const nlohmann::json &jsonData) {
			_parCoinBaseTx->txHash = Utils::UInt256FromString(jsonData["TxHash"].get<std::string>(), true);
			_parCoinBaseTx->version = jsonData["Version"].get<uint32_t>();

			std::vector<nlohmann::json> inputs = jsonData["Inputs"];
			for (size_t i = 0; i < inputs.size(); ++i) {
				TxInputsFromJson(inputs[i]);
			}

			std::vector<nlohmann::json> outputs = jsonData["Outputs"];
			for (size_t i = 0; i < outputs.size(); ++i) {
				TxOutputsFromJson(outputs[i]);
			}

			_parCoinBaseTx->lockTime = jsonData["LockTime"].get<uint32_t>();
			_parCoinBaseTx->blockHeight = jsonData["BlockHeight"].get<uint32_t>();
			_parCoinBaseTx->timestamp = jsonData["Timestamp"].get<uint32_t>();
		}

		void AuxPow::MerkleBlockFromJson(nlohmann::json jsonData) {

			std::string blockHash = jsonData["BlockHash"].get<std::string>();
			_parBlockHeader->blockHash = Utils::UInt256FromString(blockHash, true);
			_parBlockHeader->version = jsonData["Version"].get<uint32_t>();
			std::string prevBlock = jsonData["PrevBlock"].get<std::string>();
			_parBlockHeader->prevBlock = Utils::UInt256FromString(prevBlock, true);
			std::string merkleRoot = jsonData["MerkleRoot"].get<std::string>();
			_parBlockHeader->merkleRoot = Utils::UInt256FromString(merkleRoot, true);
			_parBlockHeader->timestamp = jsonData["Timestamp"].get<uint32_t>();
			_parBlockHeader->target = jsonData["Target"].get<uint32_t>();
			_parBlockHeader->nonce = jsonData["Nonce"].get<uint32_t>();
			_parBlockHeader->totalTx = jsonData["TotalTx"].get<uint32_t>();

			std::vector<std::string> hashArray = jsonData["Hashes"];
			_parBlockHeader->hashesCount = hashArray.size();
			UInt256 hashes[_parBlockHeader->hashesCount];
			for (size_t i = 0; i < _parBlockHeader->hashesCount; ++i) {
				hashes[i] = Utils::UInt256FromString(hashArray[i], true);
			}

			CMBlock flags = Utils::DecodeHex(jsonData["Flags"].get<std::string>());
			_parBlockHeader->flagsLen = flags.GetSize();

			BRMerkleBlockSetTxHashes(_parBlockHeader, hashes, _parBlockHeader->hashesCount,
									 flags, _parBlockHeader->flagsLen);

			_parBlockHeader->height = jsonData["Height"].get<uint32_t>();
		}

		void AuxPow::TxInputsFromJson(const nlohmann::json &input) {
			UInt256 hash = Utils::UInt256FromString(input["TxHash"], true);
			uint32_t index = input["Index"].get<uint32_t>();
			uint64_t amount = input["Amount"].get<uint64_t>();
			CMBlock script = Utils::DecodeHex(input["Script"].get<std::string>());
			CMBlock signature = Utils::DecodeHex(input["Signature"].get<std::string>());
			uint32_t sequence = input["Sequence"].get<uint32_t>();

			BRTransactionAddInput(_parCoinBaseTx, hash, index, amount,
								  script, script.GetSize(),
								  signature, signature.GetSize(),
								  sequence);
		}

		void AuxPow::TxOutputsFromJson(const nlohmann::json &output) {
			uint64_t amount = output["Amount"].get<uint64_t>();
			CMBlock script = Utils::DecodeHex(output["Script"].get<std::string>());
			BRTransactionAddOutput(_parCoinBaseTx, amount, script, script.GetSize());
		}

		void AuxPow::SetAuxMerkleBranch(const std::vector<UInt256> &hashes) {
			_auxMerkleBranch = hashes;
		}

		void AuxPow::SetCoinBaseMerkle(const std::vector<UInt256> &hashes) {
			_parCoinBaseMerkle = hashes;
		}

		void AuxPow::SetAuxMerkleIndex(uint32_t index) {
			_auxMerkleIndex = index;
		}

		void AuxPow::SetParMerkleIndex(uint32_t index) {
			_parMerkleIndex = index;
		}

		void AuxPow::SetParentHash(const UInt256 &hash) {
			_parentHash = hash;
		}

		uint32_t AuxPow::GetAuxMerkleIndex() const {
			return _auxMerkleIndex;
		}

		uint32_t AuxPow::GetParMerkleIndex() const {
			return _parMerkleIndex;
		}

		const UInt256 &AuxPow::GetParentHash() const {
			return _parentHash;
		}

		const std::vector<UInt256> &AuxPow::GetAuxMerkleBranch() const {
			return _auxMerkleBranch;
		}

		const std::vector<UInt256> &AuxPow::GetParCoinBaseMerkle() const {
			return _parCoinBaseMerkle;
		}

	}
}
