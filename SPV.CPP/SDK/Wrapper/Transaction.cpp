// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PreCompiled.h"

#include <cstring>
#include <BRTransaction.h>

#include "Transaction.h"
#include "Payload/PayloadCoinBase.h"
#include "ELABRTransaction.h"
#include "BRCrypto.h"

namespace Elastos {
	namespace SDK {

		Transaction::Transaction() :
				_isRegistered(false),
				_payload(nullptr) {

			_transaction = (BRTransaction *) ELABRTransactionNew();
			setPayloadByTransactionType();
			convertFrom(_transaction);
		}

		Transaction::Transaction(BRTransaction *transaction) :
				_transaction(transaction),
				_isRegistered(false),
				_payload(nullptr) {
			setPayloadByTransactionType();
			convertFrom(transaction);
		}

		Transaction::Transaction(const ByteData &buffer) :
				_isRegistered(false),
				_payload(nullptr) {
			_transaction = BRTransactionParse(buffer.data, buffer.length);
			assert (nullptr != _transaction);
			setPayloadByTransactionType();
			convertFrom(_transaction);
		}

		Transaction::Transaction(const ByteData &buffer, uint32_t blockHeight, uint32_t timeStamp) :
				_isRegistered(false),
				_payload(nullptr) {

			_transaction = BRTransactionParse(buffer.data, buffer.length);
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

		UInt256 Transaction::getHash() const {
			UInt256 zero = UINT256_ZERO;
			if (UInt256Eq(&_transaction->txHash, &zero)) {
				ByteStream ostream;
				serializeUnsigned(ostream);
				UInt256 hash = UINT256_ZERO;
				BRSHA256_2(&hash, ostream.getBuf(), ostream.position());
				UInt256Set(&_transaction->txHash, hash);
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
			if (_type == CoinBase) {
				_payload = boost::shared_ptr<PayloadCoinBase>(new PayloadCoinBase());

			} else if (_type == RegisterAsset) {

			} else if (_type == TransferAsset) {

			} else if (_type == Record) {

			} else if (_type == Deploy) {

			} else if (_type == SideMining) {

			} else if (_type == IssueToken) {

			} else if (_type == WithdrawAsset) {

			} else if (_type == TransferCrossChainAsset) {

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

		void Transaction::transactionOutputCopy(BRTxOutput *target, const BRTxOutput *source) const {
			assert (target != nullptr);
			assert (source != nullptr);
			*target = *source;

			target->script = nullptr;
			BRTxOutputSetScript(target, source->script, source->scriptLen);
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

			if (_outputs.empty()) {
				for (int i = 0; i < _transaction->outCount; i++) {
					BRTxOutput *output = new BRTxOutput;
					transactionOutputCopy(output, &_transaction->outputs[i]);
					_outputs.push_back(TransactionOutputPtr(new TransactionOutput(output)));
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

		ByteData Transaction::serialize() {

			ByteData result;
			result.length = BRTransactionSerialize(_transaction, nullptr, 0);
			result.data = new uint8_t[result.length];
			memset(result.data, 0, result.length);
			BRTransactionSerialize(_transaction, result.data, result.length);
			return result;
		}

		void Transaction::addInput(const TransactionInput &input) {

			BRTransactionAddInput(_transaction, input.getHash(), input.getIndex(), input.getAmount(),
			                      input.getScript().data, input.getScript().length,
			                      input.getSignature().data, input.getSignature().length,
			                      input.getSequence());
		}

		void Transaction::addOutput(const TransactionOutput &output) {

			BRTransactionAddOutput(_transaction, output.getAmount(),
			                       output.getScript().data, output.getScript().length);
		}

		void Transaction::shuffleOutputs() {

			BRTransactionShuffleOutputs(_transaction);
		}

		size_t Transaction::getSize() {

			return BRTransactionSize(_transaction);
		}

		uint64_t Transaction::getStandardFee() {

			return BRTransactionStandardFee(_transaction);
		}

		bool Transaction::isSigned() {

			return BRTransactionIsSigned(_transaction) != 0;
		}

		void Transaction::sign(const WrapperList<Key, BRKey> &keys, int forkId) {

			BRTransactionSign(_transaction, forkId, keys.getRawArray().data(), keys.size());
		}

		void Transaction::sign(const Key &key, int forkId) {

			WrapperList<Key, BRKey> keys(1);
			keys.push_back(key);
			sign(keys, forkId);
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

		void Transaction::Serialize(ByteStream &ostream) const {
			serializeUnsigned(ostream);

			ostream.putVarUint(_programs.size());
			for (size_t i = 0; i < _programs.size(); i++) {
				_programs[i]->Serialize(ostream);
			}
		}

		void Transaction::serializeUnsigned(ByteStream &ostream) const{
			ostream.put(_type);

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
			_inputs.resize(inputLength);
			for (size_t i = 0; i < inputLength; i++) {
				_inputs[i] = TransactionInputPtr(new TransactionInput);
				_inputs[i]->Deserialize(istream);
			}

			uint64_t outputLength = istream.getVarUint();
			_outputs.resize(outputLength);
			for (size_t i = 0; i < outputLength; i++) {
				_outputs[i] = TransactionOutputPtr(new TransactionOutput);
				_outputs[i]->Deserialize(istream);
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
			ssize_t len = inputs.size();
			TransactionInput *input = nullptr;
			for (ssize_t i = 0; i < len; ++i) {
				input = inputs[i].get();
				BRTransactionAddInput(&transaction->raw, input->getHash(), input->getIndex(), input->getAmount(),
				                      input->getScript().data, input->getScript().length, input->getSignature().data,
				                      input->getSignature().length, input->getSequence());
			}

			SharedWrapperList<TransactionOutput, BRTxOutput *> outputs = getOutputs();
			len = outputs.size();
			TransactionOutput *output = nullptr;
			uint8_t *scriptPubkey = nullptr;

			for (ssize_t i = 0; i < len; i++) {
				output = outputs[i].get();
				BRTransactionAddOutput(&transaction->raw, output->getAmount(), output->getScript().data,
				                       output->getScript().length);
			}

			transaction->type = _type;
			transaction->payloadVersion = _payloadVersion;
			ByteStream byteStream;
			_payload->Serialize(byteStream);
			transaction->payloadData.length = byteStream.length();
			if (transaction->payloadData.length > 0) {
				transaction->payloadData.data = byteStream.getBuf();
			}

			transaction->attributeData.clear();
			len = _attributes.size();
			for (ssize_t i = 0; i < len; i++) {
				AttributePtr attr = _attributes[i];
				byteStream.reSet();
				attr->Serialize(byteStream);
				transaction->attributeData.push_back(ByteData(byteStream.getBuf(), (size_t)byteStream.length()));
			}

			transaction->programData.clear();
			len = _programs.size();
			for (ssize_t i = 0; i < len; i++) {

				ProgramPtr programPtr = _programs[i];
				byteStream.reSet();
				programPtr->Serialize(byteStream);
				ByteData programBytes(byteStream.getBuf(), (size_t)byteStream.length());
				transaction->programData.push_back(programBytes);
			}

			return (BRTransaction *) transaction;
		}

		void Transaction::convertFrom(const BRTransaction *raw) {
			assert(raw != nullptr);
			ELABRTransaction *elabrTransaction = ELABRTransactioCopy((ELABRTransaction *)raw);

			_inputs.clear();
			getInputs();

			_outputs.clear();
			getOutputs();

			_type = (Type) elabrTransaction->type;
			_payloadVersion = elabrTransaction->payloadVersion;

			setPayloadByTransactionType();
			if (elabrTransaction->payloadData.data && elabrTransaction->payloadData.length > 0) {
				uint8_t *data = new uint8_t[elabrTransaction->payloadData.length];
				memcpy(data, elabrTransaction->payloadData.data, elabrTransaction->payloadData.length);
				ByteStream byteStream1(data, elabrTransaction->payloadData.length);
				_payload->Deserialize(byteStream1);
			}

			_attributes.clear();
			ssize_t len = elabrTransaction->attributeData.size();
			if (len > 0) {
				for (ssize_t i = 0; i < len; ++i) {
					ByteData byteData = elabrTransaction->attributeData[i];
					uint8_t *data = new uint8_t[byteData.length];
					memcpy(data, byteData.data, byteData.length);
					ByteStream byteStream1(data, byteData.length);
					AttributePtr attr(new Attribute());
					attr->Deserialize(byteStream1);
					_attributes.push_back(attr);
				}
			}

			_programs.clear();
			len = elabrTransaction->programData.size();
			if(len > 0) {
				for (ssize_t i = 0; i < len; i++) {
					ByteData byteData = elabrTransaction->programData[i];
					uint8_t *data = new uint8_t[byteData.length];
					memcpy(data, byteData.data, byteData.length);
					ByteStream byteStream(data, byteData.length);
					ProgramPtr programPtr(new Program);
					programPtr->Deserialize(byteStream);
					_programs.push_back(programPtr);
				}
			}

			ELABRTransactionFree(elabrTransaction);
		}
	}
}
