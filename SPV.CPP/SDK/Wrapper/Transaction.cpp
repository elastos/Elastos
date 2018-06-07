// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PreCompiled.h"

#include <cstring>
#include <BRTransaction.h>

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
#include "ELABRTransaction.h"
#include "BRCrypto.h"
#include "ELABRTxOutput.h"
#include "Utils.h"
#include "BRAddress.h"

namespace Elastos {
	namespace SDK {

		Transaction::Transaction() :
				_isRegistered(false),
				_payload(nullptr) {

			_transaction = (BRTransaction *) ELABRTransactionNew();
			convertFrom(_transaction);
		}

		Transaction::Transaction(BRTransaction *transaction) :
				_transaction(transaction),
				_isRegistered(false),
				_payload(nullptr) {
			convertFrom(transaction);
		}

		Transaction::Transaction(const CMBlock &buffer) :
				_isRegistered(false),
				_payload(nullptr) {
			_transaction = BRTransactionParse(buffer, buffer.GetSize());
			assert (nullptr != _transaction);
			convertFrom(_transaction);
		}

		Transaction::Transaction(const CMBlock &buffer, uint32_t blockHeight, uint32_t timeStamp) :
				_isRegistered(false),
				_payload(nullptr) {

			_transaction = BRTransactionParse(buffer, buffer.GetSize());
			assert (nullptr != _transaction);
			setPayloadByTransactionType();
			convertFrom(_transaction);

			_transaction->blockHeight = blockHeight;
			_transaction->timestamp = timeStamp;
		}

		Transaction::~Transaction() {
			if (_transaction != nullptr)
				ELABRTransactionFree((ELABRTransaction *) _transaction);
		}

		std::string Transaction::toString() const {
			//todo complete me
			return "";
		}

		BRTransaction *Transaction::getRaw() const {
			return _transaction;
		}

		bool Transaction::isRegistered() const {
			return _isRegistered;
		}

		bool &Transaction::isRegistered() {
			return _isRegistered;
		}

		void Transaction::resetHash() {
			UInt256Set(&_transaction->txHash, UINT256_ZERO);
		}

		UInt256 Transaction::getHash() const {
			UInt256 zero = UINT256_ZERO;
			if (UInt256Eq(&_transaction->txHash, &zero)) {
				ByteStream ostream;
				serializeUnsigned(ostream);
				uint8_t *buff = ostream.getBuf();
				UInt256 hash = UINT256_ZERO;
				BRSHA256_2(&hash, buff, ostream.position());
				UInt256Set(&_transaction->txHash, hash);
				delete buff;
			}
			return _transaction->txHash;
		}

		uint32_t Transaction::getVersion() const {
			return _transaction->version;
		}

		void Transaction::setTransactionType(Transaction::Type type) {
			if (_type != type) {
				_type = type;
				setPayloadByTransactionType();
			}
		}

		Transaction::Type Transaction::getTransactionType() const {
			return _type;
		}

		void Transaction::setPayloadByTransactionType() {
			//todo initializing payload other than just creating
			if (_type == CoinBase) {
				_payload = boost::shared_ptr<PayloadCoinBase>(new PayloadCoinBase());

			} else if (_type == RegisterAsset) {
				_payload = boost::shared_ptr<PayloadRegisterAsset>(new PayloadRegisterAsset());

			} else if (_type == TransferAsset) {
				_payload = boost::shared_ptr<PayloadTransferAsset>(new PayloadTransferAsset());

			} else if (_type == Record) {
				_payload = boost::shared_ptr<PayloadRecord>(new PayloadRecord());
			} else if (_type == Deploy) {
				//todo add deploy payload
				//_payload = boost::shared_ptr<

			} else if (_type == SideMining) {
				_payload = boost::shared_ptr<PayloadSideMining>(new PayloadSideMining());

			} else if (_type == IssueToken) {
				_payload = boost::shared_ptr<PayloadIssueToken>(new PayloadIssueToken());

			} else if (_type == WithdrawAsset) {
				_payload = boost::shared_ptr<PayloadWithDrawAsset>(new PayloadWithDrawAsset());

			} else if (_type == TransferCrossChainAsset) {
				_payload = boost::shared_ptr<PayloadTransferCrossChainAsset>(new PayloadTransferCrossChainAsset());
			} else if (_type == RegisterIdentification) {
				_payload = boost::shared_ptr<PayloadRegisterIdentification>(new PayloadRegisterIdentification());
			}
		}

		const SharedWrapperList<TransactionInput, BRTxInput *> &Transaction::getInputs() const {

			if (_inputs.empty()) {
				for (int i = 0; i < _transaction->inCount; i++) {
					BRTxInput *input = new BRTxInput;
					transactionInputCopy(input, &_transaction->inputs[i]);
					TransactionInputPtr inputPtr = TransactionInputPtr(new TransactionInput(input));
					_inputs.push_back(inputPtr);
				}
			}

			return _inputs;
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

		void Transaction::transactionOutputCopy(ELABRTxOutput *target, const ELABRTxOutput *source) const {
			assert (target != nullptr);
			assert (source != nullptr);
			*target = *source;
			target->raw.script = nullptr;
			if (source->raw.scriptLen > 0) {
				BRTxOutputSetScript((BRTxOutput *)target, source->raw.script, source->raw.scriptLen);
			}
		}

		std::vector<std::string> Transaction::getInputAddresses() {

			SharedWrapperList<TransactionInput, BRTxInput *> inputs = getInputs();
			ssize_t len = inputs.size();
			std::vector<std::string> addresses(len);
			for (int i = 0; i < len; i++)
				addresses[i] = inputs[i]->getAddress();

			return addresses;
		}

		const SharedWrapperList<TransactionOutput, BRTxOutput *> &Transaction::getOutputs() const {

			ELABRTransaction *elabrTransaction = (ELABRTransaction *) _transaction;
			if (_outputs.empty()) {
				for (int i = 0; i < _transaction->outCount; i++) {
					ELABRTxOutput *output = new ELABRTxOutput;
					transactionOutputCopy(output, &elabrTransaction->outputs[i]);
					_outputs.push_back(TransactionOutputPtr(new TransactionOutput((BRTxOutput *) output)));
				}
			}

			return _outputs;
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

			return _transaction->lockTime;
		}

		void Transaction::setLockTime(uint32_t lockTime) {

			_transaction->lockTime = lockTime;
		}

		uint32_t Transaction::getBlockHeight() {

			return _transaction->blockHeight;
		}

		uint32_t Transaction::getTimestamp() {

			return _transaction->timestamp;
		}

		void Transaction::setTimestamp(uint32_t timestamp) {

			_transaction->timestamp = timestamp;
		}

		void Transaction::addInput(const TransactionInput &input) {

			BRTransactionAddInput(_transaction, input.getHash(), input.getIndex(), input.getAmount(),
								  input.getScript(), input.getScript().GetSize(),
								  input.getSignature(), input.getSignature().GetSize(),
								  input.getSequence());

			BRTxInput *brTxInput = new BRTxInput;
			transactionInputCopy(brTxInput, &_transaction->inputs[_transaction->inCount - 1]);
			TransactionInputPtr inputPtr = TransactionInputPtr(new TransactionInput(brTxInput));
			_inputs.push_back(inputPtr);

			ProgramPtr programPtr(new Program());
			if (input.getScript() && input.getScript().GetSize() > 0) {
				CMBlock code(input.getScript().GetSize());
				memcpy(code, input.getScript(), input.getScript().GetSize());
				programPtr->setCode(code);
			}

			if (input.getSignature() && input.getSignature().GetSize() > 0) {
				CMBlock parameter(input.getSignature().GetSize());
				memcpy(parameter, input.getSignature(), input.getSignature().GetSize());
				programPtr->setParameter(parameter);
			}
			addProgram(programPtr);

		}

		void Transaction::addOutput(const TransactionOutput &output) {
			ELABRTransaction *elabrTransaction = (ELABRTransaction *) _transaction;

			ELABRTransactionAddOutput(elabrTransaction, output.getAmount(), output.getScript(),
			                          output.getScript().GetSize());

			const UInt256 assetID = output.getAssetId();
			elabrTransaction->outputAssetIDList.push_back(assetID);
			elabrTransaction->outputLockList.push_back(output.getOutputLock());
			const UInt168 programHash = output.getProgramHash();
			elabrTransaction->outputProgramHashList.push_back(programHash);

			ELABRTxOutput *brTxOutput = new ELABRTxOutput();
			size_t index = _transaction->outCount - 1;
			transactionOutputCopy(brTxOutput, &elabrTransaction->outputs[index]);

			UInt256Set(&brTxOutput->assetId, assetID);
			brTxOutput->outputLock = output.getOutputLock();
			UInt168Set(&brTxOutput->programHash, programHash);

			_outputs.push_back(TransactionOutputPtr(new TransactionOutput((BRTxOutput *) brTxOutput)));

		}

		size_t Transaction::getSize() {

			return ELABRTransactionSize((ELABRTransaction *)_transaction);
		}

		uint64_t Transaction::getStandardFee() {

			return BRTransactionStandardFee(_transaction);
		}

		bool Transaction::isSigned() {
			size_t len = _programs.size();
			if (len <= 0) {
				return false;
			}
			for (size_t i = 0; i < len; ++i) {
				if (!_programs[i]->isValid()) {
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
			size_t size = _transaction->inCount;
			for (i = 0; i < size; i++) {
				BRTxInput *input = &_transaction->inputs[i];
				if (i >= _programs.size())
				{
					CMBlock code(input->scriptLen);
					memcpy(code, input->script, input->scriptLen);
					ProgramPtr programPtr(new Program());
					programPtr->setCode(code);
					_programs.push_back(programPtr);
				}
				ProgramPtr program = _programs[i];
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
			}

			return isSigned();
		}

		bool Transaction::isStandard() {

			return BRTransactionIsStandard(_transaction) != 0;
		}

		UInt256 Transaction::getReverseHash() {

			return UInt256Reverse(&_transaction->txHash);
		}

		uint64_t Transaction::getMinOutputAmount() {

			return TX_MIN_OUTPUT_AMOUNT;
		}

		const PayloadPtr &Transaction::getPayload() const {
			return _payload;
		}

		const std::vector<AttributePtr> Transaction::getAttributes() const {
			return _attributes;
		}

		void Transaction::addProgram(const ProgramPtr &program) {
			_programs.push_back(program);
		}

		const std::vector<ProgramPtr> Transaction::getPrograms() const {
			return _programs;
		}

		void Transaction::Serialize(ByteStream &ostream) const {
			serializeUnsigned(ostream);

			ostream.putVarUint(_programs.size());
			for (size_t i = 0; i < _programs.size(); i++) {
				_programs[i]->Serialize(ostream);
			}
		}

		void Transaction::serializeUnsigned(ByteStream &ostream) const {
			ostream.put((uint8_t) _type);

			ostream.put(_payloadVersion);

			assert(_payload != nullptr);
			_payload->Serialize(ostream);

			ostream.putVarUint(_attributes.size());
			for (size_t i = 0; i < _attributes.size(); i++) {
				_attributes[i]->Serialize(ostream);
			}

			SharedWrapperList<TransactionInput, BRTxInput *> inputs = getInputs();
			ostream.putVarUint(inputs.size());
			for (size_t i = 0; i < inputs.size(); i++) {
				inputs[i]->Serialize(ostream);
			}

			SharedWrapperList<TransactionOutput, BRTxOutput *> outputs = getOutputs();
			ostream.putVarUint(outputs.size());
			for (size_t i = 0; i < outputs.size(); i++) {
				outputs[i]->Serialize(ostream);
			}

			uint8_t lockTimeData[32 / 8];
			memset(lockTimeData, 0, sizeof(lockTimeData));
			UInt32SetLE(lockTimeData, _transaction->lockTime);
			ostream.putBytes(lockTimeData, sizeof(lockTimeData));
		}

		void Transaction::Deserialize(ByteStream &istream) {
			_type = Type(istream.get());
			setPayloadByTransactionType();

			_payloadVersion = istream.get();

			_payload->Deserialize(istream);

			uint64_t attributeLength = istream.getVarUint();
			_attributes.resize(attributeLength);
			for (size_t i = 0; i < attributeLength; i++) {
				_attributes[i] = AttributePtr(new Attribute);
				_attributes[i]->Deserialize(istream);
			}

			uint64_t inputLength = istream.getVarUint();
			TransactionInputPtr transactionInputPtr;
			for (size_t i = 0; i < inputLength; i++) {
				transactionInputPtr = TransactionInputPtr(new TransactionInput);
				transactionInputPtr->Deserialize(istream);
				addInput(*transactionInputPtr.get());
			}

			uint64_t outputLength = istream.getVarUint();
			TransactionOutputPtr transactionOutputPtr;
			for (size_t i = 0; i < outputLength; i++) {
				transactionOutputPtr = TransactionOutputPtr(new TransactionOutput);
				transactionOutputPtr->Deserialize(istream);
				addOutput(*transactionOutputPtr.get());
			}

			uint8_t lockTimeData[32 / 8];
			istream.getBytes(lockTimeData, sizeof(lockTimeData));
			_transaction->lockTime = UInt32GetLE(lockTimeData);

			uint64_t programLength = istream.getVarUint();
			_programs.resize(programLength);
			for (size_t i = 0; i < programLength; i++) {
				_programs[i] = ProgramPtr(new Program());
				_programs[i]->Deserialize(istream);
			}
		}

		BRTransaction *Transaction::convertToRaw() const {
			ELABRTransaction *transaction = ELABRTransactionNew();
			transaction->raw.txHash = getHash();
			transaction->raw.version = _transaction->version;
			transaction->raw.blockHeight = _transaction->blockHeight;
			transaction->raw.inCount = _transaction->inCount;
			transaction->raw.lockTime = _transaction->lockTime;
			transaction->raw.timestamp = _transaction->timestamp;
			transaction->raw.outCount = _transaction->outCount;

			SharedWrapperList<TransactionInput, BRTxInput *> inputs = getInputs();
			size_t len = inputs.size();
			transaction->raw.inCount = len;
			TransactionInput *input = nullptr;
			for (ssize_t i = 0; i < len; ++i) {
				input = inputs[i].get();
				BRTransactionAddInput(&transaction->raw, input->getHash(), input->getIndex(), input->getAmount(),
									  input->getScript(), input->getScript().GetSize(), input->getSignature(),
									  input->getSignature().GetSize(), input->getSequence());
			}

			SharedWrapperList<TransactionOutput, BRTxOutput *> outputs = getOutputs();
			transaction->raw.outCount = len = outputs.size();
			TransactionOutput *output = nullptr;
			uint8_t *scriptPubkey = nullptr;

			for (ssize_t i = 0; i < len; i++) {
				output = outputs[i].get();
				ELABRTransactionAddOutput(transaction, output->getAmount(), output->getScript(),
				                          output->getScript().GetSize());
				const UInt256 assetID = output->getAssetId();
				transaction->outputAssetIDList.push_back(assetID);
				transaction->outputLockList.push_back(output->getOutputLock());
				const UInt168 programHash = output->getProgramHash();
				transaction->outputProgramHashList.push_back(programHash);
			}

			transaction->type = _type;
			transaction->payloadVersion = _payloadVersion;
			ByteStream byteStream;
			_payload->Serialize(byteStream);
			transaction->payloadData.Resize(byteStream.length());
			if (transaction->payloadData.GetSize() > 0) {
				uint8_t *tmp = byteStream.getBuf();
				memcpy(transaction->payloadData, tmp, byteStream.length());
				delete[]tmp;
			}

			transaction->attributeData.clear();
			len = _attributes.size();
			for (ssize_t i = 0; i < len; i++) {
				AttributePtr attr = _attributes[i];
				byteStream.reSet();
				attr->Serialize(byteStream);
				CMBlock pd(byteStream.length());
				uint8_t *tmp = byteStream.getBuf();
				memcpy(pd, tmp, pd.GetSize());
				delete[]tmp;
				transaction->attributeData.push_back(pd);
			}

			transaction->programData.clear();
			len = _programs.size();
			for (ssize_t i = 0; i < len; i++) {

				ProgramPtr programPtr = _programs[i];
				byteStream.reSet();
				programPtr->Serialize(byteStream);
				CMBlock pd(byteStream.length());
				uint8_t *tmp = byteStream.getBuf();
				memcpy(pd, tmp, pd.GetSize());
				delete[]tmp;
				transaction->programData.push_back(pd);
			}

			return (BRTransaction *) transaction;
		}

		void Transaction::convertFrom(const BRTransaction *raw) {
			assert(raw != nullptr);
			ELABRTransaction *elabrTransaction = ELABRTransactioCopy((ELABRTransaction *) raw);

			//ELABRTransaction *elabrTransaction = ELABRTransactionNew();

			//memcpy(elabrTransaction, raw, sizeof(*elabrTransaction));
			//*elabrTransaction = *((ELABRTransaction*)raw);

			_inputs.clear();
			getInputs();

			_outputs.clear();
			getOutputs();

			_type = (Type) elabrTransaction->type;
			_payloadVersion = elabrTransaction->payloadVersion;

			setPayloadByTransactionType();
			if (elabrTransaction->payloadData && elabrTransaction->payloadData.GetSize() > 0) {
				uint8_t *data = new uint8_t[elabrTransaction->payloadData.GetSize()];
				memcpy(data, elabrTransaction->payloadData, elabrTransaction->payloadData.GetSize());
				ByteStream byteStream1(data, elabrTransaction->payloadData.GetSize());
				_payload->Deserialize(byteStream1);
			}

			_attributes.clear();
			ssize_t len = elabrTransaction->attributeData.size();
			if (len > 0) {
				for (ssize_t i = 0; i < len; ++i) {
					CMBlock byteData = elabrTransaction->attributeData[i];
					uint8_t *data = new uint8_t[byteData.GetSize()];
					memcpy(data, byteData, byteData.GetSize());
					ByteStream byteStream1(data, byteData.GetSize());
					AttributePtr attr(new Attribute());
					attr->Deserialize(byteStream1);
					_attributes.push_back(attr);
				}
			}

			_programs.clear();
			len = elabrTransaction->programData.size();
			if (len > 0) {
				for (ssize_t i = 0; i < len; i++) {
					CMBlock byteData = elabrTransaction->programData[i];
					uint8_t *data = new uint8_t[byteData.GetSize()];
					memcpy(data, byteData, byteData.GetSize());
					ByteStream byteStream(data, byteData.GetSize());
					ProgramPtr programPtr(new Program);
					programPtr->Deserialize(byteStream);
					_programs.push_back(programPtr);
				}
			}

			ELABRTransactionFree(elabrTransaction);
		}

		nlohmann::json Transaction::toJson() {
			nlohmann::json jsonData;

			jsonData["isRegistered"] = _isRegistered;

			jsonData["transaction"] = rawTransactionToJson();

			jsonData["type"] = _type;

			jsonData["payloadVersion"] = _payloadVersion;

			jsonData["payLoad"] = _payload->toJson();

			std::vector<nlohmann::json> attributes(_attributes.size());
			for (size_t i = 0; i < attributes.size(); ++i) {
				attributes[i] = _attributes[i]->toJson();
			}
			jsonData["attributes"] = attributes;

			std::vector<nlohmann::json> programs(_programs.size());
			for (size_t i = 0; i < _programs.size(); ++i) {
				programs[i] = _programs[i]->toJson();
			}
			jsonData["programs"] = programs;

			SharedWrapperList<TransactionInput, BRTxInput *> txInputs = getInputs();
			std::vector<nlohmann::json> inputs(txInputs.size());
			for (size_t i = 0; i < txInputs.size(); ++i) {
				nlohmann::json inputJson = txInputs[i]->toJson();
				inputs[i] = inputJson;
			}
			jsonData["txInputs"] = inputs;

			SharedWrapperList<TransactionOutput, BRTxOutput *> txOutputs = getOutputs();
			std::vector<nlohmann::json> outputs(txOutputs.size());
			for (size_t i = 0; i < txOutputs.size(); ++i) {
				outputs[i] = txOutputs[i]->toJson();
			}
			jsonData["txOutputs"] = outputs;

			return jsonData;
		}

		nlohmann::json Transaction::rawTransactionToJson() {
			nlohmann::json jsonData;

			jsonData["txHash"] = Utils::UInt256ToString(_transaction->txHash);

			jsonData["version"] = _transaction->version;

			jsonData["inCount"] = _transaction->inCount;

			jsonData["outCount"] = _transaction->outCount;

			jsonData["lockTime"] = _transaction->lockTime;

			jsonData["blockHeight"] = _transaction->blockHeight;

			jsonData["timestamp"] = _transaction->timestamp;

			return jsonData;
		}

		void Transaction::rawTransactionFromJson(nlohmann::json jsonData) {
			std::string txHash = jsonData["txHash"];

			_transaction->txHash = Utils::UInt256FromString(txHash);

			_transaction->version = jsonData["version"].get<uint32_t>();

			_transaction->inCount = jsonData["inCount"].get<size_t>();

			_transaction->outCount = jsonData["outCount"].get<size_t>();

			_transaction->lockTime = jsonData["lockTime"].get<uint32_t>();

			_transaction->blockHeight = jsonData["blockHeight"].get<uint32_t>();

			_transaction->timestamp = jsonData["timestamp"].get<uint32_t>();

		}

		void Transaction::fromJson(nlohmann::json jsonData) {
			_isRegistered = jsonData["isRegistered"];

			rawTransactionFromJson(jsonData["transaction"]);

			_type = jsonData["type"].get<Type>();

			setPayloadByTransactionType();

			_payloadVersion = jsonData["payloadVersion"];

			nlohmann::json payLoad = jsonData["payLoad"];
			_payload->fromJson(payLoad);

			std::vector<nlohmann::json> attributes = jsonData["attributes"];
			_attributes.resize(attributes.size());
			for (size_t i = 0; i < attributes.size(); ++i) {
				_attributes[i] = AttributePtr(new Attribute);
				_attributes[i]->fromJson(attributes[i]);
			}

			std::vector<nlohmann::json> programs = jsonData["programs"];
			_programs.resize(programs.size());
			for (size_t i = 0; i < _programs.size(); ++i) {
				_programs[i] = ProgramPtr(new Program());
				_programs[i]->fromJson(programs[i]);
			}

			std::vector<nlohmann::json> inputs = jsonData["txInputs"];
			TransactionInputPtr transactionInputPtr;
			for (size_t i = 0; i < inputs.size(); ++i) {
				transactionInputPtr = TransactionInputPtr(new TransactionInput);
				transactionInputPtr->fromJson(inputs[i]);
				addInput(*transactionInputPtr.get());
			}

			std::vector<nlohmann::json> outputs = jsonData["txOutputs"];
			TransactionOutputPtr transactionOutputPtr;
			for (size_t i = 0; i < outputs.size(); ++i) {
				transactionOutputPtr = TransactionOutputPtr(new TransactionOutput);
				transactionOutputPtr->fromJson(outputs[i]);
				addOutput(*transactionOutputPtr.get());
			}
		}
	}
}
