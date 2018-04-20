// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PreCompiled.h"

#include <cstring>
#include <BRTransaction.h>

#include "BRTransaction.h"
#include "Transaction.h"

namespace Elastos {
	namespace SDK {

		Transaction::Transaction() :
				_isRegistered(false),
				_payload(nullptr) {

			_transaction = BRTransactionNew();
		}

		Transaction::Transaction(BRTransaction *transaction) :
				_transaction(transaction),
				_isRegistered(false),
				_payload(nullptr) {
		}

		Transaction::Transaction(const ByteData &buffer) :
				_isRegistered(false),
				_payload(nullptr) {

			_transaction = BRTransactionParse(buffer.data, buffer.length);
			assert (nullptr != _transaction);
		}

		Transaction::Transaction(const ByteData &buffer, uint32_t blockHeight, uint32_t timeStamp) :
				_isRegistered(false),
				_payload(nullptr) {

			_transaction = BRTransactionParse(buffer.data, buffer.length);
			assert (nullptr != _transaction);

			_transaction->blockHeight = blockHeight;
			_transaction->timestamp = timeStamp;
		}

		Transaction::~Transaction() {
			if (_transaction != nullptr)
				BRTransactionFree(_transaction);
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
			return _transaction->txHash;
		}

		uint32_t Transaction::getVersion() const {
			return _transaction->version;
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

			if(_outputs.empty()) {
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

		void Transaction::Serialize(std::istream &istream) const {
			uint8_t typeData[8 / 8];
			UInt8SetLE(typeData, uint8_t(_type));
			istream >> typeData;

			uint8_t payloadVersionData[8 / 8];
			UInt8SetLE(payloadVersionData, _payloadVersion);
			istream >> payloadVersionData;

			assert(_payload != nullptr);
			_payload->Serialize(istream);

			uint8_t attributeLengthData[64 / 8];
			UInt64SetLE(attributeLengthData, _attributes.size());
			istream >> attributeLengthData;
			for (size_t i = 0; i < _attributes.size(); i++) {
				_attributes[i]->Serialize(istream);
			}

			SharedWrapperList<TransactionInput, BRTxInput *> inputs = getInputs();
			uint8_t inputLengthData[64 / 8];
			UInt64SetLE(inputLengthData, inputs.size());
			istream >> inputLengthData;
			for (size_t i = 0; i < inputs.size(); i++) {
				inputs[i]->Serialize(istream);
			}

			SharedWrapperList<TransactionOutput, BRTxOutput *> outputs = getOutputs();
			uint8_t outputLengthData[64 / 8];
			UInt64SetLE(outputLengthData, outputs.size());
			istream >> outputLengthData;
			for (size_t i = 0; i < outputs.size(); i++) {
				outputs[i]->Serialize(istream);
			}

			uint8_t lockTimeData[32/8];
			UInt32SetLE(lockTimeData, _transaction->lockTime);
			istream >> lockTimeData;

			uint8_t programLengthData[64 / 8];
			UInt64SetLE(programLengthData, _programs.size());
			istream >> programLengthData;
			for (size_t i = 0; i < _programs.size(); i++) {
				_programs[i]->Serialize(istream);
			}
		}

		void Transaction::Deserialize(std::ostream &ostream) {
			uint8_t typeData[8 / 8];
			ostream << typeData;
			_type = Type(UInt8GetLE(typeData));

			uint8_t payloadVersionData[8 / 8];
			ostream << payloadVersionData;
			_payloadVersion = UInt8GetLE(payloadVersionData);

			assert(_payload != nullptr);
			_payload->Deserialize(ostream);

			uint8_t attributeLengthData[64 / 8];
			ostream << attributeLengthData;
			uint64_t attributeLength = UInt64GetLE(attributeLengthData);
			_attributes.resize(attributeLength);
			for (size_t i = 0; i < attributeLength; i++) {
				_attributes[i] = AttributePtr(new Attribute);
				_attributes[i]->Deserialize(ostream);
			}

			uint8_t inputLengthData[64 / 8];
			ostream << inputLengthData;
			uint64_t inputLength = UInt64GetLE(inputLengthData);
			_inputs.resize(inputLength);
			for (size_t i = 0; i < inputLength; i++) {
				_inputs[i] = TransactionInputPtr(new TransactionInput);
				_inputs[i]->Deserialize(ostream);
			}

			uint8_t outputLengthData[64 / 8];
			ostream << outputLengthData;
			uint64_t outputLength = UInt64GetLE(outputLengthData);
			_outputs.resize(outputLength);
			for (size_t i = 0; i < outputLength; i++) {
				_outputs[i] = TransactionOutputPtr(new TransactionOutput);
				_outputs[i]->Deserialize(ostream);
			}

			uint8_t lockTimeData[32/8];
			ostream << lockTimeData;
			_transaction->lockTime = UInt32GetLE(lockTimeData);

			uint8_t programLengthData[64 / 8];
			ostream << programLengthData;
			uint64_t programLength = UInt64GetLE(programLengthData);
			_programs.resize(programLength);
			for (size_t i = 0; i < programLength; i++) {
				_programs[i]->Deserialize(ostream);
			}
		}

	}
}
