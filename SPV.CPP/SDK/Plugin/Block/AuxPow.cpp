// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <BRTransaction.h>
#include <stdlib.h>
#include <Core/BRTransaction.h>
#include <Core/BRMerkleBlock.h>
#include <SDK/Common/Log.h>

#include "BRMerkleBlock.h"
#include "BRAddress.h"
#include "Utils.h"
#include "AuxPow.h"

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
			serializeBtcTransaction(ostream, _parCoinBaseTx);

			ostream.writeBytes(_parentHash.u8, sizeof(UInt256));

			ostream.writeVarUint(uint64_t(_parCoinBaseMerkle.size()));
			for (uint64_t i = 0; i < _parCoinBaseMerkle.size(); ++i) {
				ostream.writeBytes(_parCoinBaseMerkle[i].u8, sizeof(UInt256));
			}

			ostream.writeUint32(_parMerkleIndex);

			ostream.writeVarUint(uint64_t(_auxMerkleBranch.size()));
			for (uint64_t i = 0; i < _auxMerkleBranch.size(); ++i) {
				ostream.writeBytes(_auxMerkleBranch[i].u8, sizeof(UInt256));
			}

			ostream.writeUint32(_auxMerkleIndex);

			serializeBtcBlockHeader(ostream, _parBlockHeader);
		}

		bool AuxPow::Deserialize(ByteStream &istream) {
			if (!deserializeBtcTransaction(istream, _parCoinBaseTx)) {
				Log::getLogger()->error("deserialize AuxPow btc tx error");
				return false;
			}

			if (!istream.readBytes(_parentHash.u8, sizeof(UInt256))) {
				Log::getLogger()->error("deserialize AuxPow parentHash error");
				return false;
			}

			uint64_t parCoinBaseMerkleCount = 0;
			if (!istream.readVarUint(parCoinBaseMerkleCount)) {
				Log::getLogger()->error("deserialize AuxPow parCoinBaseMerkle size error");
				return false;
			}

			_parCoinBaseMerkle.resize(parCoinBaseMerkleCount);
			for (uint64_t i = 0; i < parCoinBaseMerkleCount; ++i) {
				if (!istream.readBytes(_parCoinBaseMerkle[i].u8, sizeof(UInt256))) {
					Log::getLogger()->error("deserialize AuxPow parCoinBaseMerkle[{}] error", i);
					return false;
				}
			}

			if (!istream.readUint32(_parMerkleIndex)) {
				Log::getLogger()->error("deserialize AuxPow parMerkleIndex error");
				return false;
			}

			uint64_t auxMerkleBranchCount = 0;
			if (!istream.readVarUint(auxMerkleBranchCount)) {
				Log::getLogger()->error("deserialize AuxPow auxMerkleBranchCount error");
				return false;
			}

			_auxMerkleBranch.resize(auxMerkleBranchCount);
			for (uint64_t i = 0; i < auxMerkleBranchCount; ++i) {
				if (!istream.readBytes(_auxMerkleBranch[i].u8, sizeof(UInt256))) {
					Log::getLogger()->error("deserialize AuxPow auxMerkleBranch error");
					return false;
				}
			}

			if (!istream.readUint32(_auxMerkleIndex)) {
				Log::getLogger()->error("deserialize AuxPow auxMerkleIndex error");
				return false;
			}

			if (!deserializeBtcBlockHeader(istream, _parBlockHeader)) {
				Log::getLogger()->error("deserialize AuxPow btc block header error");
				return false;
			}

			return true;
		}

		void AuxPow::serializeBtcTransaction(ByteStream &ostream, const BRTransaction *tx) const {
			ostream.writeUint32(tx->version);

			ostream.writeVarUint(uint64_t(tx->inCount));
			for (uint64_t i = 0; i < tx->inCount; ++i) {
				serializeBtcTxIn(ostream, &tx->inputs[i]);
			}

			ostream.writeVarUint(uint64_t(tx->outCount));
			for (uint64_t i = 0; i < tx->outCount; ++i) {
				serializeBtcTxOut(ostream, &tx->outputs[i]);
			}

			ostream.writeUint32(tx->lockTime);
		}

		bool AuxPow::deserializeBtcTransaction(ByteStream &istream, BRTransaction *tx) {
			if (!istream.readUint32(tx->version)) {
				Log::getLogger()->error("deserialize version error");
				return false;
			}

			uint64_t inCount = 0;
			if (!istream.readVarUint(inCount)) {
				Log::getLogger()->error("deserialize inCount error");
				return false;
			}
			for (uint64_t i = 0; i < inCount; ++i) {
				if (!deserializeBtcTxIn(istream, tx)) {
					Log::getLogger()->error("deserialize input[{}] error", i);
					return false;
				}
			}

			uint64_t outCount = 0;
			if (!istream.readVarUint(outCount)) {
				Log::getLogger()->error("deserialize outCount error");
				return false;
			}
			for (uint64_t i = 0; i < outCount; ++i) {
				if (!deserializeBtcTxOut(istream, tx)) {
					Log::getLogger()->error("deserialize output[{}] error", i);
					return false;
				}
			}

			if (!istream.readUint32(tx->lockTime)) {
				Log::getLogger()->error("deserialize lockTime error");
				return false;
			}

			return true;
		}

		void AuxPow::serializeBtcTxIn(ByteStream &ostream, const BRTxInput *in) const {
			ostream.writeBytes(in->txHash.u8, sizeof(UInt256));
			ostream.writeUint32(in->index);
			ostream.writeVarBytes(in->signature, in->sigLen);
			ostream.writeUint32(in->sequence);
		}

		bool AuxPow::deserializeBtcTxIn(ByteStream &istream, BRTransaction *tx) {
			UInt256 txHash;
			if (!istream.readBytes(txHash.u8, sizeof(UInt256))) {
				Log::getLogger()->error("deserialize txHash error");
				return false;
			}

			uint32_t index = 0;
			if (!istream.readUint32(index)) {
				Log::getLogger()->error("deserialize index error");
				return false;
			}

			CMBlock signature;
			if (!istream.readVarBytes(signature)) {
				Log::getLogger()->error("deserialize signature error");
				return false;
			}

			uint32_t sequence = 0;
			if (!istream.readUint32(sequence)) {
				Log::getLogger()->error("deserialize sequence error");
				return false;
			}

			BRTransactionAddInput(tx, txHash, index, 0, nullptr, 0,
								  signature.GetSize() > 0 ? (uint8_t *)signature : nullptr, signature.GetSize(), sequence);
			return true;
		}

		void AuxPow::serializeBtcTxOut(ByteStream &ostream, const BRTxOutput *out) const {
			ostream.writeUint64(out->amount);
			ostream.writeVarBytes(out->script, out->scriptLen);
		}

		bool AuxPow::deserializeBtcTxOut(ByteStream &istream, BRTransaction *tx) {
			uint64_t amount = 0;
			if (!istream.readUint64(amount)) {
				Log::getLogger()->error("deserialize amount error");
				return false;
			}

			CMBlock script;
			if (!istream.readVarBytes(script)) {
				Log::getLogger()->error("deserialize script error");
				return false;
			}

			BRTransactionAddOutput(tx, amount, script.GetSize() > 0 ? (uint8_t *)script : nullptr, script.GetSize());
			return true;
		}

		void AuxPow::serializeBtcBlockHeader(ByteStream &ostream, const BRMerkleBlock *b) const {
			ostream.writeUint32(b->version);
			ostream.writeBytes(b->prevBlock.u8, sizeof(UInt256));
			ostream.writeBytes(b->merkleRoot.u8, sizeof(UInt256));
			ostream.writeUint32(b->timestamp);
			ostream.writeUint32(b->target);
			ostream.writeUint32(b->nonce);
		}

		bool AuxPow::deserializeBtcBlockHeader(ByteStream &istream, BRMerkleBlock *b) {
			if (!istream.readUint32(b->version)) {
				Log::getLogger()->error("deserialize version error");
				return false;
			}

			if (!istream.readBytes(b->prevBlock.u8, sizeof(UInt256))) {
				Log::getLogger()->error("deserialize prevBlock error");
				return false;
			}

			if (!istream.readBytes(b->merkleRoot.u8, sizeof(UInt256))) {
				Log::getLogger()->error("deserialize merkleRoot error");
				return false;
			}

			if (!istream.readUint32(b->timestamp)) {
				Log::getLogger()->error("deserialize timestamp error");
				return false;
			}

			if (!istream.readUint32(b->target)) {
				Log::getLogger()->error("deserialize target error");
				return false;
			}

			if (!istream.readUint32(b->nonce)) {
				Log::getLogger()->error("deserialize nonce error");
				return false;
			}
			return true;
		}

		UInt256 AuxPow::getParBlockHeaderHash() const {
			ByteStream stream;
			serializeBtcBlockHeader(stream, _parBlockHeader);
			UInt256 hash = UINT256_ZERO;
			CMBlock buf = stream.getBuffer();
			BRSHA256_2(&hash, buf, buf.GetSize());
			return hash;
		}

		AuxPow::AuxPow(const AuxPow &auxPow) {
			operator=(auxPow);
		}

		void AuxPow::setBTCTransaction(BRTransaction *transaction) {
			if (_parCoinBaseTx != nullptr)
				BRTransactionFree(_parCoinBaseTx);
			_parCoinBaseTx = transaction;
		}

		void AuxPow::setParBlockHeader(BRMerkleBlock *block) {
			if (_parBlockHeader != nullptr)
				BRMerkleBlockFree(nullptr, _parBlockHeader);
			_parBlockHeader = block;
		}

		AuxPow &AuxPow::operator=(const AuxPow &auxPow) {
			_auxMerkleBranch = auxPow._auxMerkleBranch;
			_parCoinBaseMerkle = auxPow._parCoinBaseMerkle;
			_auxMerkleIndex = auxPow._auxMerkleIndex;
			setBTCTransaction(BRTransactionCopy(auxPow._parCoinBaseTx));
			_parMerkleIndex = auxPow._parMerkleIndex;
			setParBlockHeader(BRMerkleBlockCopy(auxPow._parBlockHeader));
			UInt256Set(&_parentHash, auxPow._parentHash);
			return *this;
		}

		BRTransaction *AuxPow::getBTCTransaction() const {
			return _parCoinBaseTx;
		}

		BRMerkleBlock *AuxPow::getParBlockHeader() const {
			return _parBlockHeader;
		}

		nlohmann::json AuxPow::toJson() const {
			nlohmann::json jsonData;

			size_t len = _auxMerkleBranch.size();
			std::vector<std::string> auxMerkleBranch(len);
			for (size_t i = 0; i < len; ++i) {
				auxMerkleBranch[i] = Utils::UInt256ToString(_auxMerkleBranch[i]);
			}
			jsonData["AuxMerkleBranch"] = auxMerkleBranch;

			len = _parCoinBaseMerkle.size();
			std::vector<std::string> parCoinBaseMerkle(len);
			for (size_t i = 0; i < len; ++i) {
				parCoinBaseMerkle[i] = Utils::UInt256ToString(_parCoinBaseMerkle[i]);
			}
			jsonData["ParCoinBaseMerkle"] = parCoinBaseMerkle;

			jsonData["AuxMerkleIndex"] = _auxMerkleIndex;
			jsonData["Transaction"] = transactionToJson();
			jsonData["ParMerkleIndex"] = _parMerkleIndex;
			jsonData["ParBlockHeader"] = merkleBlockToJson();
			jsonData["ParentHash"] = Utils::UInt256ToString(_parentHash);

			return jsonData;
		}

		nlohmann::json AuxPow::transactionToJson() const {
			nlohmann::json jsonData;

			jsonData["TxHash"] = Utils::UInt256ToString(_parCoinBaseTx->txHash);
			jsonData["Version"] = _parCoinBaseTx->version;

			std::vector<nlohmann::json> inputs(_parCoinBaseTx->inCount);
			for (size_t i = 0; i < _parCoinBaseTx->inCount; ++i) {
				inputs[i] = txInputsToJson(_parCoinBaseTx->inputs[i]);;
			}
			jsonData["Inputs"] = inputs;

			std::vector<nlohmann::json> outputs(_parCoinBaseTx->outCount);
			for (size_t i = 0; i < _parCoinBaseTx->outCount; ++i) {
				outputs[i] = txOutputsToJson(_parCoinBaseTx->outputs[i]);
			}
			jsonData["Outputs"] = outputs;

			jsonData["LockTime"] = _parCoinBaseTx->lockTime;
			jsonData["BlockHeight"] = _parCoinBaseTx->blockHeight;
			jsonData["Timestamp"] = _parCoinBaseTx->timestamp;

			return jsonData;
		}

		nlohmann::json AuxPow::txInputsToJson(const BRTxInput &input) const {
			nlohmann::json jsonData;

			jsonData["TxHash"] = Utils::UInt256ToString(input.txHash);
			jsonData["Index"] = input.index;
			jsonData["Address"] = std::string(input.address);
			jsonData["Amount"] = input.amount;
			jsonData["Script"] = Utils::encodeHex(input.script, input.scriptLen);
			jsonData["Signature"] = Utils::encodeHex(input.signature, input.sigLen);
			jsonData["Sequence"] = input.sequence;

			return jsonData;
		}

		nlohmann::json AuxPow::txOutputsToJson(const BRTxOutput &output) const {
			nlohmann::json jsonData;

			jsonData["Address"] = std::string(output.address);
			jsonData["Amount"] = output.amount;
			jsonData["Script"] = Utils::encodeHex(output.script, output.scriptLen);

			return jsonData;
		}

		nlohmann::json AuxPow::merkleBlockToJson() const {
			nlohmann::json jsonData;

			jsonData["BlockHash"] = Utils::UInt256ToString(_parBlockHeader->blockHash);
			jsonData["Version"] = _parBlockHeader->version;
			jsonData["PrevBlock"] = Utils::UInt256ToString(_parBlockHeader->prevBlock);
			jsonData["MerkleRoot"] = Utils::UInt256ToString(_parBlockHeader->merkleRoot);
			jsonData["Timestamp"] = _parBlockHeader->timestamp;
			jsonData["Target"] = _parBlockHeader->target;
			jsonData["Nonce"] = _parBlockHeader->nonce;
			jsonData["TotalTx"] = _parBlockHeader->totalTx;

			std::vector<std::string> hashes(_parBlockHeader->hashesCount);
			for (size_t i = 0; i < _parBlockHeader->hashesCount; ++i) {
				hashes[i] = Utils::UInt256ToString(_parBlockHeader->hashes[i]);
			}
			jsonData["Hashes"] = hashes;

			jsonData["Flags"] = Utils::encodeHex(_parBlockHeader->flags, _parBlockHeader->flagsLen);;
			jsonData["Height"] = _parBlockHeader->height;

			return jsonData;
		}

		void AuxPow::fromJson(const nlohmann::json &jsonData) {
			std::vector<std::string> auxMerkleBranch = jsonData["AuxMerkleBranch"];
			_auxMerkleBranch.resize(auxMerkleBranch.size());
			for (size_t i = 0; i < _auxMerkleBranch.size(); ++i) {
				_auxMerkleBranch[i] = Utils::UInt256FromString(auxMerkleBranch[i]);
			}

			std::vector<std::string> parCoinBaseMerkle = jsonData["ParCoinBaseMerkle"];
			_parCoinBaseMerkle.resize(parCoinBaseMerkle.size());
			for (size_t i = 0; i < parCoinBaseMerkle.size(); ++i) {
				_parCoinBaseMerkle[i] = Utils::UInt256FromString(parCoinBaseMerkle[i]);
			}

			_auxMerkleIndex = jsonData["AuxMerkleIndex"].get<uint32_t>();
			transactionFromJson(jsonData["Transaction"]);
			_parMerkleIndex = jsonData["ParMerkleIndex"].get<uint32_t>();
			merkleBlockFromJson(jsonData["ParBlockHeader"]);
			_parentHash = Utils::UInt256FromString(jsonData["ParentHash"].get<std::string>());
		}

		void AuxPow::transactionFromJson(const nlohmann::json &jsonData) {
			_parCoinBaseTx->txHash = Utils::UInt256FromString(jsonData["TxHash"].get<std::string>());
			_parCoinBaseTx->version = jsonData["Version"].get<uint32_t>();

			std::vector<nlohmann::json> inputs = jsonData["Inputs"];
			for (size_t i = 0; i < inputs.size(); ++i) {
				txInputsFromJson(inputs[i]);
			}

			std::vector<nlohmann::json> outputs = jsonData["Outputs"];
			for (size_t i = 0; i < outputs.size(); ++i) {
				txOutputsFromJson(outputs[i]);
			}

			_parCoinBaseTx->lockTime = jsonData["LockTime"].get<uint32_t>();
			_parCoinBaseTx->blockHeight = jsonData["BlockHeight"].get<uint32_t>();
			_parCoinBaseTx->timestamp = jsonData["Timestamp"].get<uint32_t>();
		}

		void AuxPow::merkleBlockFromJson(nlohmann::json jsonData) {

			std::string blockHash = jsonData["BlockHash"].get<std::string>();
			_parBlockHeader->blockHash = Utils::UInt256FromString(blockHash);
			_parBlockHeader->version = jsonData["Version"].get<uint32_t>();
			std::string prevBlock = jsonData["PrevBlock"].get<std::string>();
			_parBlockHeader->prevBlock = Utils::UInt256FromString(prevBlock);
			std::string merkleRoot = jsonData["MerkleRoot"].get<std::string>();
			_parBlockHeader->merkleRoot = Utils::UInt256FromString(merkleRoot);
			_parBlockHeader->timestamp = jsonData["Timestamp"].get<uint32_t>();
			_parBlockHeader->target = jsonData["Target"].get<uint32_t>();
			_parBlockHeader->nonce = jsonData["Nonce"].get<uint32_t>();
			_parBlockHeader->totalTx = jsonData["TotalTx"].get<uint32_t>();

			std::vector<std::string> hashArray = jsonData["Hashes"];
			_parBlockHeader->hashesCount = hashArray.size();
			UInt256 hashes[_parBlockHeader->hashesCount];
			for (size_t i = 0; i < _parBlockHeader->hashesCount; ++i) {
				hashes[i] = Utils::UInt256FromString(hashArray[i]);
			}

			CMBlock flags = Utils::decodeHex(jsonData["Flags"].get<std::string>());
			_parBlockHeader->flagsLen = flags.GetSize();

			BRMerkleBlockSetTxHashes(_parBlockHeader, hashes, _parBlockHeader->hashesCount,
									 flags, _parBlockHeader->flagsLen);

			_parBlockHeader->height = jsonData["Height"].get<uint32_t>();
		}

		void AuxPow::txInputsFromJson(const nlohmann::json &input) {
			UInt256 hash = Utils::UInt256FromString(input["TxHash"]);
			uint32_t index = input["Index"].get<uint32_t>();
			uint64_t amount = input["Amount"].get<uint64_t>();
			CMBlock script = Utils::decodeHex(input["Script"].get<std::string>());
			CMBlock signature = Utils::decodeHex(input["Signature"].get<std::string>());
			uint32_t sequence = input["Sequence"].get<uint32_t>();

			BRTransactionAddInput(_parCoinBaseTx, hash, index, amount,
								  script, script.GetSize(),
								  signature, signature.GetSize(),
								  sequence);
		}

		void AuxPow::txOutputsFromJson(const nlohmann::json &output) {
			uint64_t amount = output["Amount"].get<uint64_t>();
			CMBlock script = Utils::decodeHex(output["Script"].get<std::string>());
			BRTransactionAddOutput(_parCoinBaseTx, amount, script, script.GetSize());
		}

		void AuxPow::setAuxMerkleBranch(const std::vector<UInt256> &hashes) {
			_auxMerkleBranch = hashes;
		}

		void AuxPow::setCoinBaseMerkle(const std::vector<UInt256> &hashes) {
			_parCoinBaseMerkle = hashes;
		}

		void AuxPow::setAuxMerkleIndex(uint32_t index) {
			_auxMerkleIndex = index;
		}

		void AuxPow::setParMerkleIndex(uint32_t index) {
			_parMerkleIndex = index;
		}

		void AuxPow::setParentHash(const UInt256 &hash) {
			_parentHash = hash;
		}

		uint32_t AuxPow::getAuxMerkleIndex() const {
			return _auxMerkleIndex;
		}

		uint32_t AuxPow::getParMerkleIndex() const {
			return _parMerkleIndex;
		}

		const UInt256 &AuxPow::getParentHash() const {
			return _parentHash;
		}

		const std::vector<UInt256> &AuxPow::getAuxMerkleBranch() const {
			return _auxMerkleBranch;
		}

		const std::vector<UInt256> &AuxPow::getParCoinBaseMerkle() const {
			return _parCoinBaseMerkle;
		}

	}
}
