// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PreCompiled.h"

#include <cstring>
#include <BRTransaction.h>
#include <SDK/Common/Log.h>
#include <Core/BRTransaction.h>

#include "Transaction.h"
#include "Payload/PayloadCoinBase.h"
#include "Payload/PayloadIssueToken.h"
#include "Payload/PayloadWithDrawAsset.h"
#include "Payload/PayloadRecord.h"
#include "Payload/PayloadRegisterAsset.h"
#include "Payload/PayloadSideMining.h"
#include "Payload/PayloadTransferCrossChainAsset.h"
#include "Payload/PayloadTransferAsset.h"
#include "Payload/PayloadRegisterIdentification.h"
#include "BRCrypto.h"
#include "ELATxOutput.h"
#include "Utils.h"
#include "BRAddress.h"

namespace Elastos {
	namespace SDK {

		Transaction::Transaction() :
			_isRegistered(false) {
			_transaction = ELATransactionNew();
		}

		Transaction::Transaction(const ELATransaction *tx) :
				_isRegistered(false) {
			_transaction = ELATransactioCopy(tx);
		}

		Transaction::Transaction(const ELATransaction &tx) :
				_isRegistered(false) {
			_transaction = ELATransactioCopy(&tx);

		}

		Transaction::Transaction(const CMBlock &buffer) :
				_isRegistered(false) {
			_transaction = ELATransactionNew();

			ByteStream stream(buffer, buffer.GetSize(), false);
			this->Deserialize(stream);
		}

		Transaction::Transaction(const CMBlock &buffer, uint32_t blockHeight, uint32_t timeStamp) :
				_isRegistered(false) {

			_transaction = ELATransactionNew();

			ByteStream stream(buffer, buffer.GetSize(), false);
			this->Deserialize(stream);

			_transaction->raw.blockHeight = blockHeight;
			_transaction->raw.timestamp = timeStamp;
		}

		Transaction::~Transaction() {
			if (_transaction != nullptr)
				ELATransactionFree(_transaction);
		}

		std::string Transaction::toString() const {
			//todo complete me
			return "";
		}

		BRTransaction *Transaction::getRaw() const {
			return &_transaction->raw;
		}

		bool Transaction::isRegistered() const {
			return _isRegistered;
		}

		bool &Transaction::isRegistered() {
			return _isRegistered;
		}

		void Transaction::resetHash() {
			UInt256Set(&_transaction->raw.txHash, UINT256_ZERO);
		}

		UInt256 Transaction::getHash() const {
			UInt256 zero = UINT256_ZERO;
			if (UInt256Eq(&_transaction->raw.txHash, &zero)) {
				ByteStream ostream;
				serializeUnsigned(ostream);
				uint8_t *buff = ostream.getBuf();
				UInt256 hash = UINT256_ZERO;
				BRSHA256_2(&hash, buff, ostream.position());
				UInt256Set(&_transaction->raw.txHash, hash);
				delete buff;
			}
			return _transaction->raw.txHash;
		}

		uint32_t Transaction::getVersion() const {
			return _transaction->raw.version;
		}

		void Transaction::setTransactionType(ELATransaction::Type type) {
			if (_transaction->type != type) {
				_transaction->type = type;
				_transaction->payload = newPayload(type);
			}
		}

		ELATransaction::Type Transaction::getTransactionType() const {
			return _transaction->type;
		}

		PayloadPtr Transaction::newPayload(ELATransaction::Type type) {
			//todo initializing payload other than just creating
			return ELAPayloadNew(type);
		}

#if 0
		const SharedWrapperList<TransactionInput, BRTxInput *> &Transaction::getInputs() const {
			SharedWrapperList<TransactionInput, BRTxInput *> inputs;
			for (size_t i = 0; i < _transaction->raw.inCount; ++i) {
				inputs.push_back(TransactionInputPtr(new TransactionInput(&_transaction->raw.inputs[i])));
			}
			return inputs;
		}

		void Transaction::transactionInputCopy(BRTxInput *target, const BRTxInput *source) const {
			assert (target != nullptr);
			assert (source != nullptr);
			*target = *source;

			target->script = nullptr;
			BRTxInputSetScript(target, source->script, source->scriptLen);

			target->signature = nullptr;
			BRTxInputSetSignature(target, source->signature, source->sigLen);
		}

		void Transaction::transactionOutputCopy(ELATxOutput *target, const ELATxOutput *source) const {
			assert (target != nullptr);
			assert (source != nullptr);
			*target = *source;
			target->raw.script = nullptr;
			if (source->raw.scriptLen > 0) {
				BRTxOutputSetScript((BRTxOutput *)target, source->raw.script, source->raw.scriptLen);
			}
		}
#endif

		std::vector<std::string> Transaction::getInputAddresses() {

			std::vector<std::string> addresses(_transaction->raw.inCount);
			for (int i = 0; i < _transaction->raw.inCount; i++)
				addresses[i] = _transaction->raw.inputs[i].address;

			return addresses;
		}

		const SharedWrapperList<TransactionOutput, BRTxOutput *> &Transaction::getOutputs() const {
			return _transaction->outputs;
		}

		std::vector<std::string> Transaction::getOutputAddresses() {

			SharedWrapperList<TransactionOutput, BRTxOutput *> outputs = getOutputs();
			ssize_t len = outputs.size();
			std::vector<std::string> addresses(len);
			for (int i = 0; i < len; i++)
				addresses[i] = outputs[i]->getAddress();

			return addresses;
		}

		uint32_t Transaction::getLockTime() {

			return _transaction->raw.lockTime;
		}

		void Transaction::setLockTime(uint32_t lockTime) {

			_transaction->raw.lockTime = lockTime;
		}

		uint32_t Transaction::getBlockHeight() {

			return _transaction->raw.blockHeight;
		}

		uint32_t Transaction::getTimestamp() {

			return _transaction->raw.timestamp;
		}

		void Transaction::setTimestamp(uint32_t timestamp) {

			_transaction->raw.timestamp = timestamp;
		}

		void Transaction::addInput(const UInt256 &hash, uint32_t index, uint64_t amount,
								   const CMBlock script, const CMBlock signature, uint32_t sequence) {
			BRTransactionAddInput(&_transaction->raw, hash, index, amount,
								  script, script.GetSize(), signature, signature.GetSize(),
								  sequence);

			ProgramPtr programPtr(new Program());
			programPtr->setCode(script);
			programPtr->setParameter(signature);
			addProgram(programPtr);
		}

		void Transaction::addOutput(TransactionOutput *output) {
			_transaction->outputs.push_back(TransactionOutputPtr(output));
		}

		// shuffles order of tx outputs
		void Transaction::shuffleOutputs() {
			ELATransactionShuffleOutputs(_transaction);
		}

		size_t Transaction::getSize() {
			return ELATransactionSize(_transaction);
		}

		uint64_t Transaction::getStandardFee() {
			return BRTransactionStandardFee(&_transaction->raw);
		}

		bool Transaction::isSigned() {
			size_t len = _transaction->programs.size();
			if (len <= 0) {
				return false;
			}
			for (size_t i = 0; i < len; ++i) {
				if (!_transaction->programs[i]->isValid()) {
					return false;
				}
			}
			return true;
		}

		bool Transaction::sign(const WrapperList<Key, BRKey> &keys, int forkId) {
			return transactionSign(forkId, keys.getRawArray().data(), keys.size());
		}

		bool Transaction::sign(const Key &key, int forkId) {

			WrapperList<Key, BRKey> keys(1);
			keys.push_back(key);
			return sign(keys, forkId);
		}

		bool Transaction::transactionSign(int forkId, BRKey keys[], size_t keysCount) {
			const int SIGHASH_ALL = 0x01; // default, sign all of the outputs
			BRAddress addrs[keysCount], address;
			size_t i, j;

			assert(keys != NULL || keysCount == 0);

			for (i = 0; i < keysCount; i++) {
				if (! BRKeyAddress(&keys[i], addrs[i].s, sizeof(addrs[i]))) addrs[i] = BR_ADDRESS_NONE;
			}

			for (i = 0; i < _transaction->raw.inCount; i++) {
				BRTxInput *input = &_transaction->raw.inputs[i];
				if (i >= _transaction->programs.size()) {
					ProgramPtr programPtr(new Program());
					CMBlock script(_transaction->raw.inCount);
					memcpy(script, input->script, input->scriptLen);
					programPtr->setCode(script);
					_transaction->programs.push_back(programPtr);
				}
				ProgramPtr program = _transaction->programs[i];
				if (! BRAddressFromScriptPubKey(address.s, sizeof(address), program->getCode(),
				                                program->getCode().GetSize())) continue;
				j = 0;
				while (j < keysCount && ! BRAddressEq(&addrs[j], &address)) j++;
				if (j >= keysCount) continue;

				const uint8_t *elems[BRScriptElements(NULL, 0, program->getCode(), program->getCode().GetSize())];
				size_t elemsCount = BRScriptElements(elems, sizeof(elems)/sizeof(*elems), program->getCode(),
				                                     program->getCode().GetSize());
				uint8_t pubKey[BRKeyPubKey(&keys[j], NULL, 0)];
				size_t pkLen = BRKeyPubKey(&keys[j], pubKey, sizeof(pubKey));
				uint8_t sig[73], script[1 + sizeof(sig) + 1 + sizeof(pubKey)];
				size_t sigLen, scriptLen;
				UInt256 md = UINT256_ZERO;

				ByteStream ostream;
				serializeUnsigned(ostream);
				uint8_t *data = ostream.getBuf();
				size_t dataLen = ostream.position();

				if (elemsCount >= 2 && *elems[elemsCount - 2] == OP_EQUALVERIFY) { // pay-to-pubkey-hash
					BRSHA256_2(&md, data, dataLen);
					sigLen = BRKeySign(&keys[j], sig, sizeof(sig) - 1, md);
					sig[sigLen++] = forkId | SIGHASH_ALL;
					scriptLen = BRScriptPushData(script, sizeof(script), sig, sigLen);
					scriptLen += BRScriptPushData(&script[scriptLen], sizeof(script) - scriptLen, pubKey, pkLen);
					BRTxInputSetSignature(input, script, scriptLen);
					CMBlock parameter(scriptLen);
					memcpy(parameter, script, scriptLen);
					program->setParameter(parameter);
				}
				else { // pay-to-pubkey
					BRSHA256_2(&md, data, dataLen);
					sigLen = BRKeySign(&keys[j], sig, sizeof(sig) - 1, md);
					sig[sigLen++] = forkId | SIGHASH_ALL;
					scriptLen = BRScriptPushData(script, sizeof(script), sig, sigLen);
					BRTxInputSetSignature(input, script, scriptLen);
					CMBlock parameter(scriptLen);
					memcpy(parameter, script, scriptLen);
					program->setParameter(parameter);
				}
				delete data;
			}

			return isSigned();
		}

		bool Transaction::isStandard() {
			return BRTransactionIsStandard(&_transaction->raw) != 0;
		}

		UInt256 Transaction::getReverseHash() {

			return UInt256Reverse(&_transaction->raw.txHash);
		}

		uint64_t Transaction::getMinOutputAmount() {

			return TX_MIN_OUTPUT_AMOUNT;
		}

		const PayloadPtr &Transaction::getPayload() const {
			return _transaction->payload;
		}

		const std::vector<AttributePtr> &Transaction::getAttributes() const {
			return _transaction->attributes;
		}

		void Transaction::addProgram(const ProgramPtr &program) {
			_transaction->programs.push_back(program);
		}

		const std::vector<ProgramPtr> &Transaction::getPrograms() const {
			return _transaction->programs;
		}

		void Transaction::Serialize(ByteStream &ostream) const {
			serializeUnsigned(ostream);

			ostream.putVarUint(_transaction->programs.size());
			for (size_t i = 0; i < _transaction->programs.size(); i++) {
				_transaction->programs[i]->Serialize(ostream);
			}
		}

		void Transaction::serializeUnsigned(ByteStream &ostream) const {
			ostream.put((uint8_t) _transaction->type);

			ostream.put(_transaction->payloadVersion);

			assert(_transaction->payload != nullptr);
			_transaction->payload->Serialize(ostream);

			ostream.putVarUint(_transaction->attributes.size());
			for (size_t i = 0; i < _transaction->attributes.size(); i++) {
				_transaction->attributes[i]->Serialize(ostream);
			}

			ostream.putVarUint(_transaction->raw.inCount);
			for (size_t i = 0; i < _transaction->raw.inCount; i++) {
				uint8_t transactionHashData[256 / 8];
				UInt256Set(transactionHashData, _transaction->raw.inputs[i].txHash);
				ostream.putBytes(transactionHashData, 256 / 8);

				uint8_t indexData[16 / 8];
				UInt16SetLE(indexData, uint16_t(_transaction->raw.inputs[i].index));
				ostream.putBytes(indexData, 16 / 8);

				uint8_t sequenceData[32 / 8];
				UInt32SetLE(sequenceData, _transaction->raw.inputs[i].sequence);
				ostream.putBytes(sequenceData, 32 / 8);
			}

			SharedWrapperList<TransactionOutput, BRTxOutput *> outputs = getOutputs();
			ostream.putVarUint(outputs.size());
			for (size_t i = 0; i < outputs.size(); i++) {
				outputs[i]->Serialize(ostream);
			}

			uint8_t lockTimeData[32 / 8];
			UInt32SetLE(lockTimeData, _transaction->raw.lockTime);
			ostream.putBytes(lockTimeData, sizeof(lockTimeData));
		}

		bool Transaction::Deserialize(ByteStream &istream) {
			_transaction->type = ELATransaction::Type(istream.get());

			_transaction->payloadVersion = istream.get();

			_transaction->payload = newPayload(_transaction->type);
			if (_transaction->payload == nullptr) {
				Log::getLogger()->error("new payload when deserialize error");
				return false;
			}
			_transaction->payload->Deserialize(istream);

			uint64_t attributeLength = istream.getVarUint();
			_transaction->attributes.resize(attributeLength);

			for (size_t i = 0; i < attributeLength; i++) {
				_transaction->attributes[i] = AttributePtr(new Attribute);
				_transaction->attributes[i]->Deserialize(istream);
			}

			size_t inCount = istream.getVarUint();
			for (size_t i = 0; i < inCount; i++) {
				uint8_t transactionHashData[256 / 8];
				istream.getBytes(transactionHashData, 256 / 8);
				UInt256 txHash;
				UInt256Get(&txHash, transactionHashData);

				uint8_t indexData[16 / 8];
				istream.getBytes(indexData, 16 / 8);
				uint16_t index = UInt16GetLE(indexData);

				uint8_t sequenceData[32 / 8];
				istream.getBytes(sequenceData, 32 / 8);
				uint32_t sequence = UInt32GetLE(sequenceData);

				BRTransactionAddInput(&_transaction->raw, txHash, index, 0, nullptr, 0, nullptr, 0, sequence);
			}

			uint64_t outputLength = istream.getVarUint();
			_transaction->outputs.resize(outputLength);
			for (size_t i = 0; i < outputLength; i++) {
				_transaction->outputs[i] = TransactionOutputPtr(new TransactionOutput());
				_transaction->outputs[i]->Deserialize(istream);
			}

			uint8_t lockTimeData[32 / 8];
			istream.getBytes(lockTimeData, sizeof(lockTimeData));
			_transaction->raw.lockTime = UInt32GetLE(lockTimeData);

			uint64_t programLength = istream.getVarUint();
			_transaction->programs.resize(programLength);
			for (size_t i = 0; i < programLength; i++) {
				_transaction->programs[i] = ProgramPtr(new Program());
				_transaction->programs[i]->Deserialize(istream);
			}

			return true;
		}

		nlohmann::json Transaction::rawTransactionToJson() {
			nlohmann::json jsonData;
			BRTransaction *raw = &_transaction->raw;

			jsonData["TxHash"]      = Utils::UInt256ToString(raw->txHash);
			jsonData["Version"]     = raw->version;
			jsonData["LockTime"]    = raw->lockTime;
			jsonData["BlockHeight"] = raw->blockHeight;
			jsonData["Timestamp"]   = raw->timestamp;
//			jsonData["InputCount"]  = raw->inCount;

			std::vector<nlohmann::json> inputs(raw->inCount);
			for (size_t i = 0; i < raw->inCount; ++i) {
				BRTxInput *input = &raw->inputs[i];
				nlohmann::json jsonData;

				jsonData["TxHash"] = Utils::UInt256ToString(input->txHash);
				jsonData["Index"] = input->index;
				jsonData["Address"] = std::string(input->address);
				jsonData["Amount"] = input->amount;
//				jsonData["ScriptLen"] = input->scriptLen;
				jsonData["Script"] = Utils::encodeHex(input->script, input->scriptLen);
//				jsonData["SigLen"] = input->sigLen;
				jsonData["Signature"] = Utils::encodeHex(input->signature, input->sigLen);
				jsonData["Sequence"] = input->sequence;

				inputs[i] = jsonData;
			}
			jsonData["Inputs"] = inputs;

			return jsonData;
		}

		nlohmann::json Transaction::toJson() {
			nlohmann::json jsonData;

			jsonData["IsRegistered"] = _isRegistered;
			jsonData["Raw"] = rawTransactionToJson();
			jsonData["Type"] = (uint8_t)_transaction->type;
			jsonData["PayloadVersion"] = _transaction->payloadVersion;
			jsonData["PayLoad"] = _transaction->payload->toJson();

			std::vector<nlohmann::json> attributes(_transaction->attributes.size());
			for (size_t i = 0; i < attributes.size(); ++i) {
				attributes[i] = _transaction->attributes[i]->toJson();
			}
			jsonData["Attributes"] = attributes;

			std::vector<nlohmann::json> programs(_transaction->programs.size());
			for (size_t i = 0; i < programs.size(); ++i) {
				programs[i] = _transaction->programs[i]->toJson();
			}
			jsonData["Programs"] = programs;

			SharedWrapperList<TransactionOutput, BRTxOutput *> txOutputs = getOutputs();
			std::vector<nlohmann::json> outputs(txOutputs.size());
			for (size_t i = 0; i < txOutputs.size(); ++i) {
				outputs[i] = txOutputs[i]->toJson();
			}
			jsonData["Outputs"] = outputs;

			return jsonData;
		}

		void Transaction::rawTransactionFromJson(nlohmann::json jsonData) {
			BRTransaction *raw = &_transaction->raw;

			raw->txHash      = Utils::UInt256FromString(jsonData["TxHash"].get<std::string>());
			raw->version     = jsonData["Version"].get<uint32_t>();
			raw->lockTime    = jsonData["LockTime"].get<uint32_t>();
			raw->blockHeight = jsonData["BlockHeight"].get<uint32_t>();
			raw->timestamp   = jsonData["Timestamp"].get<uint32_t>();

			std::vector<nlohmann::json> inputs = jsonData["Inputs"];
			raw->inCount     = inputs.size();

			for (size_t i = 0; i < raw->inCount; ++i) {
				nlohmann::json jsonData = inputs[i];

				UInt256 txHash = Utils::UInt256FromString(jsonData["TxHash"].get<std::string>());
				uint32_t index = jsonData["Index"].get<uint32_t>();
				std::string address = jsonData["Address"].get<std::string>();
				uint64_t amount = jsonData["Amount"].get<uint64_t>();

				std::string scriptString = jsonData["Script"].get<std::string>();
				size_t scriptLen = scriptString.length() / 2;
				uint8_t *script = new uint8_t[scriptLen];
				Utils::decodeHex(script, scriptLen, scriptString.c_str(), scriptString.length());

				std::string signatureString = jsonData["Signature"].get<std::string>();
				size_t sigLen = signatureString.length() / 2;
				uint8_t *signature =  new uint8_t[sigLen];
				Utils::decodeHex(signature, sigLen, signatureString.c_str(), signatureString.length());

				uint32_t sequence = jsonData["Sequence"].get<uint32_t>();

				BRTransactionAddInput(raw, txHash, index, amount, script, scriptLen, signature, sigLen, sequence);
				delete[] script;
				delete[] signature;
			}
		}

		void Transaction::fromJson(nlohmann::json jsonData) {
			_isRegistered = jsonData["IsRegistered"];
			rawTransactionFromJson(jsonData["Raw"]);
			_transaction->type = ELATransaction::Type(jsonData["Type"].get<uint8_t>());
			_transaction->payloadVersion = jsonData["PayloadVersion"];

			_transaction->payload = newPayload(_transaction->type);
			if (_transaction->payload == nullptr) {
				Log::getLogger()->error("payload is nullptr when convert from json");
			} else {
				_transaction->payload->fromJson(jsonData["PayLoad"]);
			}

			std::vector<nlohmann::json> attributes = jsonData["Attributes"];
			_transaction->attributes.resize(attributes.size());
			for (size_t i = 0; i < _transaction->attributes.size(); ++i) {
				_transaction->attributes[i] = AttributePtr(new Attribute);
				_transaction->attributes[i]->fromJson(attributes[i]);
			}

			std::vector<nlohmann::json> programs = jsonData["Programs"];
			_transaction->programs.resize(programs.size());
			for (size_t i = 0; i < _transaction->programs.size(); ++i) {
				_transaction->programs[i] = ProgramPtr(new Program());
				_transaction->programs[i]->fromJson(programs[i]);
			}

			std::vector<nlohmann::json> outputs = jsonData["Outputs"];
			_transaction->outputs.resize(outputs.size());
			for (size_t i = 0; i < outputs.size(); ++i) {
				_transaction->outputs[i] = TransactionOutputPtr(new TransactionOutput());
				_transaction->outputs[i]->fromJson(outputs[i]);
			}
		}
	}
}
