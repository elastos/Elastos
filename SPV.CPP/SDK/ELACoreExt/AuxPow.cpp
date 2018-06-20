// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <BRTransaction.h>
#include <stdlib.h>
#include <Core/BRTransaction.h>
#include <Core/BRMerkleBlock.h>

#include "BRMerkleBlock.h"
#include "BRAddress.h"
#include "Utils.h"
#include "AuxPow.h"

namespace Elastos {
	namespace ElaWallet {

		AuxPow::AuxPow() {
			_btcTransaction = BRTransactionNew();
			_parBlockHeader = BRMerkleBlockNew();
			_auxMerkleIndex = 0;
			_parMerkleIndex = 0;
			_parentHash = UINT256_ZERO;
		}

		AuxPow::~AuxPow() {
			if (_btcTransaction != nullptr)
				BRTransactionFree(_btcTransaction);
			if (_parBlockHeader)
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

			ostream.putVarUint(_auxMerkleBranch.size());
			uint8_t auxMerkleBranchData[256 / 8];
			for (uint64_t i = 0; i < _auxMerkleBranch.size(); ++i) {
				UInt256Set(auxMerkleBranchData, _auxMerkleBranch[i]);
				ostream.putBytes(auxMerkleBranchData, 256 / 8);
			}

			uint8_t parMerkleIndexData[32 / 8];
			UInt32SetLE(parMerkleIndexData, _parMerkleIndex);
			ostream.putBytes(parMerkleIndexData, 32 / 8);

			ostream.putVarUint(_parCoinBaseMerkle.size());
			uint8_t parCoinBaseMerkleData[256 / 8];
			for (uint64_t i = 0; i < _parCoinBaseMerkle.size(); ++i) {
				UInt256Set(parCoinBaseMerkleData, _parCoinBaseMerkle[i]);
				ostream.putBytes(parCoinBaseMerkleData, 256 / 8);
			}

			serializeBtcBlockHeader(ostream);
		}

		bool AuxPow::Deserialize(ByteStream &istream) {
			deserializeBtcTransaction(istream);

			uint8_t parentHashData[256 / 8];
			istream.getBytes(parentHashData, 256 / 8);
			UInt256Get(&_parentHash, parentHashData);

			uint8_t auxMerkleIndexData[32 / 8];
			istream.getBytes(auxMerkleIndexData, 32 / 8);
			_auxMerkleIndex = UInt32GetLE(auxMerkleIndexData);

			uint64_t auxMerkleBranchCount = istream.getVarUint();

			_auxMerkleBranch.resize(auxMerkleBranchCount);
			uint8_t auxMerkleBranchData[256 / 8];
			for (uint64_t i = 0; i < auxMerkleBranchCount; ++i) {
				istream.getBytes(auxMerkleBranchData, 256 / 8);
				UInt256Get(&_auxMerkleBranch[i], auxMerkleBranchData);
			}

			uint8_t parMerkleIndexData[32 / 8];
			istream.getBytes(parMerkleIndexData, 32 / 8);
			_parMerkleIndex = UInt32GetLE(parMerkleIndexData);

			uint64_t parCoinBaseMerkleCount = istream.getVarUint();
			_parCoinBaseMerkle.resize(parCoinBaseMerkleCount);
			uint8_t parCoinBaseMerkleData[256 / 8];
			for (uint64_t i = 0; i < parCoinBaseMerkleCount; ++i) {
				istream.getBytes(parCoinBaseMerkleData, 256 / 8);
				UInt256Get(&_parCoinBaseMerkle[i], parCoinBaseMerkleData);
			}

			deserializeBtcBlockHeader(istream);

			return true;
		}

		void AuxPow::serializeBtcTransaction(ByteStream &ostream) const {
			uint8_t versionData[32 / 8];
			UInt32SetLE(versionData, _btcTransaction->version);
			ostream.putBytes(versionData, 32 / 8);

			ostream.putVarUint(uint64_t(_btcTransaction->inCount));
			for (uint64_t i = 0; i < _btcTransaction->inCount; ++i) {
				serializeBtcTxIn(ostream, _btcTransaction->inputs[i]);
			}

			ostream.putVarUint(uint64_t(_btcTransaction->outCount));
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

			_btcTransaction->inCount = istream.getVarUint();
			for (uint64_t i = 0; i < _btcTransaction->inCount; ++i) {
				deserializeBtcTxIn(istream, _btcTransaction->inputs[i]);
			}

			_btcTransaction->outCount = istream.getVarUint();
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

			ostream.putVarUint(input.sigLen);
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

			input.sigLen = (size_t) istream.getVarUint();

			if (input.sigLen != 0) {
				uint8_t signature[input.sigLen];
				istream.getBytes(signature, input.sigLen);
				BRTxInputSetSignature(&input, signature, input.sigLen);
			}

			uint8_t sequenceData[32 / 8];
			istream.getBytes(sequenceData, 32 / 8);
			input.sequence = UInt32GetLE(sequenceData);
		}

		void AuxPow::serializeBtcTxOut(ByteStream &ostream, const BRTxOutput &output) const {
			uint8_t amountData[64 / 8];
			UInt64SetLE(amountData, output.amount);
			ostream.putBytes(amountData, 64 / 8);

			ostream.putVarUint(output.scriptLen);
			ostream.putBytes(output.script, output.scriptLen);
		}

		void AuxPow::deserializeBtcTxOut(ByteStream &istream, BRTxOutput &output) {
			uint8_t amountData[64 / 8];
			istream.getBytes(amountData, 64 / 8);
			output.amount = UInt64GetLE(amountData);

			output.scriptLen = istream.getVarUint();
			if (output.scriptLen != 0) {
				output.script = (uint8_t *) malloc(output.scriptLen * sizeof(uint8_t));
				istream.getBytes(output.script, output.scriptLen);
			} else BRTxOutputSetScript(&output, nullptr, 0);
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

		UInt256 AuxPow::getParBlockHeaderHash() const {
			ByteStream stream;
			serializeBtcBlockHeader(stream);
			UInt256 hash = UINT256_ZERO;
			CMBlock buf = stream.getBuffer();
			BRSHA256_2(&hash, buf, buf.GetSize());
			return hash;
		}

		AuxPow::AuxPow(const AuxPow &auxPow) {
			_auxMerkleBranch = auxPow._auxMerkleBranch;
			_parCoinBaseMerkle = auxPow._parCoinBaseMerkle;
			_auxMerkleIndex = auxPow._auxMerkleIndex;
			_btcTransaction = BRTransactionCopy(auxPow._btcTransaction);
			_parMerkleIndex = auxPow._parMerkleIndex;
			_parBlockHeader = BRMerkleBlockCopy(auxPow._parBlockHeader);
			UInt256Set(&_parentHash, auxPow._parentHash);
		}

		void AuxPow::setBTCTransaction(BRTransaction *transaction) {
			if (_btcTransaction != nullptr)
				BRTransactionFree(_btcTransaction);
			_btcTransaction = transaction;
		}

		void AuxPow::setParBlockHeader(BRMerkleBlock *block) {
			if (_parBlockHeader != nullptr)
				BRMerkleBlockFree(_parBlockHeader);
			_parBlockHeader = block;
		}

		AuxPow &AuxPow::operator=(const AuxPow &auxPow) {
			_auxMerkleBranch = auxPow._auxMerkleBranch;
			_parCoinBaseMerkle = auxPow._parCoinBaseMerkle;
			_auxMerkleIndex = auxPow._auxMerkleIndex;
			setBTCTransaction(BRTransactionCopy(auxPow._btcTransaction));
			_parMerkleIndex = auxPow._parMerkleIndex;
			setParBlockHeader(BRMerkleBlockCopy(auxPow._parBlockHeader));
			UInt256Set(&_parentHash, auxPow._parentHash);
			return *this;
		}

		BRTransaction *AuxPow::getBTCTransaction() const {
			return _btcTransaction;
		}

		BRMerkleBlock *AuxPow::getParBlockHeader() const {
			return _parBlockHeader;
		}

		nlohmann::json AuxPow::toJson() {
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

			jsonData["TxHash"] = Utils::UInt256ToString(_btcTransaction->txHash);
			jsonData["Version"] = _btcTransaction->version;

			std::vector<nlohmann::json> inputs(_btcTransaction->inCount);
			for (size_t i = 0; i < _btcTransaction->inCount; ++i) {
				inputs[i] = txInputsToJson(_btcTransaction->inputs[i]);;
			}
			jsonData["Inputs"] = inputs;

			std::vector<nlohmann::json> outputs(_btcTransaction->outCount);
			for (size_t i = 0; i < _btcTransaction->outCount; ++i) {
				outputs[i] = txOutputsToJson(_btcTransaction->outputs[i]);
			}
			jsonData["Outputs"] = outputs;

			jsonData["LockTime"] = _btcTransaction->lockTime;
			jsonData["BlockHeight"] = _btcTransaction->blockHeight;
			jsonData["Timestamp"] = _btcTransaction->timestamp;

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
			_btcTransaction->txHash = Utils::UInt256FromString(jsonData["TxHash"].get<std::string>());
			_btcTransaction->version = jsonData["Version"].get<uint32_t>();

			std::vector<nlohmann::json> inputs = jsonData["Inputs"];
			for (size_t i = 0; i < inputs.size(); ++i) {
				txInputsFromJson(inputs[i]);
			}

			std::vector<nlohmann::json> outputs = jsonData["Outputs"];
			for (size_t i = 0; i < outputs.size(); ++i) {
				txOutputsFromJson(outputs[i]);
			}

			_btcTransaction->lockTime = jsonData["LockTime"].get<uint32_t>();
			_btcTransaction->blockHeight = jsonData["BlockHeight"].get<uint32_t>();
			_btcTransaction->timestamp = jsonData["Timestamp"].get<uint32_t>();
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

			BRTransactionAddInput(_btcTransaction, hash, index, amount,
								  script, script.GetSize(),
								  signature, signature.GetSize(),
								  sequence);
		}

		void AuxPow::txOutputsFromJson(const nlohmann::json &output) {
			uint64_t amount = output["Amount"].get<uint64_t>();
			CMBlock script = Utils::decodeHex(output["Script"].get<std::string>());
			BRTransactionAddOutput(_btcTransaction, amount, script, script.GetSize());
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

		const std::vector<UInt256> &AuxPow::getAuxMerkleBranch() {
			return _auxMerkleBranch;
		}

		const std::vector<UInt256> &AuxPow::getParCoinBaseMerkle() {
			return _parCoinBaseMerkle;
		}

	}
}
