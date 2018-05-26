// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <BRTransaction.h>
#include <stdlib.h>

#include "BRMerkleBlock.h"
#include "BRAddress.h"
#include "Utils.h"
#include "AuxPow.h"

namespace Elastos {
	namespace SDK {

		uint64_t ReadVarInt(ByteStream &istream, uint8_t h) {
			if (h < 0xFD) {
				return (uint64_t) h;
			} else if (h == 0xfd) {
				uint8_t txInCountData[16 / 8];
				istream.getBytes(txInCountData, 16 / 8);
				return (uint64_t) UInt16GetLE(txInCountData);
			} else if (h == 0xfe) {
				uint8_t txInCountData[32 / 8];
				istream.getBytes(txInCountData, 32 / 8);
				return (uint64_t) UInt32GetLE(txInCountData);
			} else if (h == 0xff) {
				uint8_t txInCountData[64 / 8];
				istream.getBytes(txInCountData, 64 / 8);
				return (uint64_t) UInt64GetLE(txInCountData);
			}

			return 0;
		}

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

			uint64_t parCoinBaseMerkleCount = istream.getVarUint();
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
			input.sigLen = (size_t) ReadVarInt(istream, signatureScriptLengthDataHead);

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

		UInt256 AuxPow::getParBlockHeaderHash() const {
			ByteStream stream;
			serializeBtcBlockHeader(stream);
			UInt256 hash = UINT256_ZERO;
			BRSHA256_2(&hash, stream.getBuf(), stream.position());
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
			jsonData["auxMerkleBranch"] = auxMerkleBranch;

			len = _parCoinBaseMerkle.size();
			std::vector<std::string> parCoinBaseMerkle(len);
			for (size_t i = 0; i < len; ++i) {
				parCoinBaseMerkle[i] = Utils::UInt256ToString(_parCoinBaseMerkle[i]);
			}
			jsonData["parCoinBaseMerkle"] = parCoinBaseMerkle;

			jsonData["auxMerkleIndex"] = _auxMerkleIndex;

			nlohmann::json transaction = transactionToJson();
			jsonData["transaction"] = transaction;

			jsonData["parMerkleIndex"] = _parMerkleIndex;

			jsonData["parBlockHeader"] = merkleBlockToJson();

			jsonData["parentHash"] = Utils::UInt256ToString(_parentHash);

			return jsonData;
		}

		nlohmann::json AuxPow::transactionToJson() {
			nlohmann::json jsonData;

			jsonData["txHash"] = Utils::UInt256ToString(_btcTransaction->txHash);

			jsonData["version"] = _btcTransaction->version;

			jsonData["inCount"] = _btcTransaction->inCount;
			std::vector<nlohmann::json> inputs(_btcTransaction->inCount);
			for (size_t i = 0; i < _btcTransaction->inCount; ++i) {
				nlohmann::json inputsJson = txInputsToJson(i);
				inputs[i] = inputsJson;
			}

			jsonData["inputs"] = inputs;

			jsonData["outCount"] = _btcTransaction->outCount;
			std::vector<nlohmann::json> outputs(_btcTransaction->outCount);
			for (size_t i = 0; i < _btcTransaction->outCount; ++i) {
				nlohmann::json outputsJson = txOutputsToJson(i);
				outputs[i] = outputsJson;
			}
			jsonData["outputs"] = outputs;

			jsonData["lockTime"] = _btcTransaction->lockTime;

			jsonData["blockHeight"] = _btcTransaction->blockHeight;

			jsonData["timestamp"] = _btcTransaction->timestamp;

			return jsonData;
		}

		nlohmann::json AuxPow::txInputsToJson(size_t index) {
			nlohmann::json jsonData;
			BRTxInput input = _btcTransaction->inputs[index];
			jsonData["txHash"] = Utils::UInt256ToString(input.txHash);
			jsonData["index"] = input.index;

			std::string addr = input.address;
			jsonData["address"] = addr;

			jsonData["amount"] = input.amount;

			jsonData["scriptLen"] = input.scriptLen;
			char *script = new char[input.scriptLen];
			memcpy(script, input.script, input.scriptLen);
			std::string scriptStr(script, input.scriptLen);
			jsonData["script"] = scriptStr;

			jsonData["sigLen"] = input.sigLen;
			char *signature = new char[input.sigLen];
			memcpy(signature, input.signature, input.sigLen);
			std::string signatureStr(signature, input.sigLen);
			jsonData["signature"] = signatureStr;

			jsonData["sequence"] = input.sequence;

			return jsonData;
		}

		nlohmann::json AuxPow::txOutputsToJson(size_t index) {
			nlohmann::json jsonData;
			BRTxOutput output = _btcTransaction->outputs[index];

			jsonData["address"] = output.address;

			jsonData["amount"] = output.amount;

			jsonData["scriptLen"] = output.scriptLen;

			char *script = new char[output.scriptLen];
			memcpy(script, output.script, output.scriptLen);
			std::string scriptStr(script, output.scriptLen);
			jsonData["script"] = scriptStr;

			return jsonData;
		}

		nlohmann::json AuxPow::merkleBlockToJson() {
			nlohmann::json jsonData;

			jsonData["blockHash"] = Utils::UInt256ToString(_parBlockHeader->blockHash);

			jsonData["version"] = _parBlockHeader->version;

			jsonData["prevBlock"] = Utils::UInt256ToString(_parBlockHeader->prevBlock);

			jsonData["merkleRoot"] = Utils::UInt256ToString(_parBlockHeader->merkleRoot);

			jsonData["timestamp"] = _parBlockHeader->timestamp;

			jsonData["target"] = _parBlockHeader->target;

			jsonData["nonce"] = _parBlockHeader->nonce;

			jsonData["totalTx"] = _parBlockHeader->totalTx;

			jsonData["hashesCount"] = _parBlockHeader->hashesCount;
			std::vector<std::string> hashes(_parBlockHeader->hashesCount);
			for (size_t i = 0; i < _parBlockHeader->hashesCount; ++i) {
				hashes[i] = Utils::UInt256ToString(_parBlockHeader->hashes[i]);
			}
			jsonData["hashes"] = hashes;

			jsonData["flagsLen"] = _parBlockHeader->flagsLen;

			char *flags = new char[_parBlockHeader->flagsLen];
			memcpy(flags, _parBlockHeader->flags, _parBlockHeader->flagsLen);
			std::string flagsStr(flags, _parBlockHeader->flagsLen);
			jsonData["flags"] = flagsStr;

			jsonData["height"] = _parBlockHeader->height;

			return jsonData;
		}

		void AuxPow::fromJson(nlohmann::json jsonData) {
			std::vector<std::string> auxMerkleBranch = jsonData["auxMerkleBranch"];
			_auxMerkleBranch.resize(auxMerkleBranch.size());
			for (size_t i = 0; i < _auxMerkleBranch.size(); ++i) {
				_auxMerkleBranch[i] = Utils::UInt256FromString(auxMerkleBranch[i]);
			}

			std::vector<std::string> parCoinBaseMerkle = jsonData["parCoinBaseMerkle"];
			_parCoinBaseMerkle.resize(parCoinBaseMerkle.size());
			for (size_t i = 0; i < parCoinBaseMerkle.size(); ++i) {
				_parCoinBaseMerkle[i] = Utils::UInt256FromString(parCoinBaseMerkle[i]);
			}

			_auxMerkleIndex = jsonData["auxMerkleIndex"].get<uint32_t>();

			transactionFromJson(jsonData["transaction"]);

			_parMerkleIndex = jsonData["parMerkleIndex"].get<uint32_t>();

			merkleBlockFromJson(jsonData["parBlockHeader"]);

			std::string parentHash = jsonData["parentHash"].get<std::string>();
			_parentHash = Utils::UInt256FromString(parentHash);
		}

		void AuxPow::transactionFromJson(nlohmann::json jsonData) const {

			std::string txHash = jsonData["txHash"].get<std::string>();
			_btcTransaction->txHash = Utils::UInt256FromString(txHash);

			_btcTransaction->version = jsonData["version"].get<uint32_t>();

			_btcTransaction->inCount = jsonData["inCount"].get<size_t>();

			txInputsFromJson(jsonData["inputs"]);

			_btcTransaction->outCount = jsonData["outCount"].get<size_t>();

			txOutputsFromJson(jsonData["outputs"]);

			_btcTransaction->lockTime = jsonData["lockTime"].get<uint32_t>();

			_btcTransaction->blockHeight = jsonData["blockHeight"].get<uint32_t>();

			_btcTransaction->timestamp = jsonData["timestamp"].get<uint32_t>();
		}

		void AuxPow::merkleBlockFromJson(nlohmann::json jsonData) const {

			std::string blockHash = jsonData["blockHash"].get<std::string>();
			_parBlockHeader->blockHash = Utils::UInt256FromString(blockHash);

			_parBlockHeader->version = jsonData["version"].get<uint32_t>();

			std::string prevBlock = jsonData["prevBlock"].get<std::string>();
			_parBlockHeader->prevBlock = Utils::UInt256FromString(prevBlock);

			std::string merkleRoot = jsonData["merkleRoot"].get<std::string>();
			_parBlockHeader->merkleRoot = Utils::UInt256FromString(merkleRoot);

			_parBlockHeader->timestamp = jsonData["timestamp"].get<uint32_t>();

			_parBlockHeader->target = jsonData["target"].get<uint32_t>();

			_parBlockHeader->nonce = jsonData["nonce"].get<uint32_t>();

			_parBlockHeader->totalTx = jsonData["totalTx"].get<uint32_t>();

			_parBlockHeader->hashesCount = jsonData["hashesCount"].get<size_t>();

			std::vector<std::string> hashs = jsonData["hashes"];
			UInt256 hashes[_parBlockHeader->hashesCount];
			for (size_t i = 0; i < _parBlockHeader->hashesCount; ++i) {
				hashes[i] = Utils::UInt256FromString(hashs[i]);
			}

			_parBlockHeader->flagsLen = jsonData["flagsLen"].get<size_t>();

			std::string flags = jsonData["flags"].get<std::string>();

			BRMerkleBlockSetTxHashes(_parBlockHeader, hashes, _parBlockHeader->hashesCount,
			                         (const uint8_t *) flags.data(), _parBlockHeader->flagsLen);

			_parBlockHeader->height = jsonData["height"].get<uint32_t>();

		}

		void AuxPow::txInputsFromJson(std::vector<nlohmann::json> inputs) const {

			size_t len = inputs.size();
			for (size_t i = 0; i < len; ++i) {
				nlohmann::json jsonData = inputs[i];

				UInt256 hash = Utils::UInt256FromString(jsonData["txHash"]);
				uint32_t index = jsonData["index"].get<uint32_t>();
				uint64_t amount = jsonData["amount"].get<uint64_t>();
				size_t scriptLen = jsonData["scriptLen"].get<size_t>();
				std::string scriptStr = jsonData["script"].get<std::string>();
				size_t sigLen = jsonData["sigLen"].get<size_t>();
				std::string signature = jsonData["signature"].get<std::string>();
				uint32_t sequence = jsonData["sequence"].get<uint32_t>();

				BRTransactionAddInput(_btcTransaction, hash, index, amount,
				                      (uint8_t *) scriptStr.data(), scriptLen,
				                      (uint8_t *) signature.data(), sigLen,
				                      sequence);
				std::string address = jsonData["address"].get<std::string>();
				memset(_btcTransaction->inputs[i].address, 0, sizeof(_btcTransaction->inputs[i]));
				memcpy(_btcTransaction->inputs[i].address, address.c_str(), address.size());

			}
		}

		void AuxPow::txOutputsFromJson(std::vector<nlohmann::json> outputs) const {
			size_t len = outputs.size();
			for (size_t i = 0; i < len; ++i) {
				nlohmann::json jsonData = outputs[i];

				uint64_t amount = jsonData["amount"].get<uint64_t>();
				uint8_t *script = nullptr;
				size_t scriptLen = jsonData["scriptLen"].get<size_t>();
				if (scriptLen > 0) {
					std::string scriptStr = jsonData["script"].get<std::string>();
					script = (uint8_t *) scriptStr.data();
				}

				BRTransactionAddOutput(_btcTransaction, amount, script, scriptLen);

				std::string address = jsonData["address"];
				memset(_btcTransaction->outputs[i].address, 0, sizeof(_btcTransaction->outputs[i].address));
				memcpy(_btcTransaction->outputs[i].address, address.data(), address.size());
			}
		}

	}
}
