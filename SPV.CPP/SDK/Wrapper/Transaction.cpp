// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <BRTransaction.h>
#include "Transaction.h"

#include "BRTransaction.h"

namespace Elastos {
	namespace SDK {

		Transaction::Transaction() :
				_isRegistered(false) {

			_transaction = BRTransactionNew();
		}

		Transaction::Transaction(BRTransaction *transaction) :
				_transaction(transaction),
				_isRegistered(false) {
		}

		Transaction::Transaction(const ByteData &buffer) :
				_isRegistered(false) {

			_transaction = BRTransactionParse(buffer.data, buffer.length);
			assert (nullptr != _transaction);
		}

		Transaction::Transaction(const ByteData &buffer, uint32_t blockHeight, uint32_t timeStamp) :
				_isRegistered(false) {

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

		SharedWrapperList<TransactionInput, BRTxInput *> Transaction::getInputs() {

			size_t inputCount = _transaction->inCount;

			SharedWrapperList<TransactionInput, BRTxInput *> inputs;
			for (int i = 0; i < inputCount; i++) {

				BRTxInput *input = new BRTxInput;
				transactionInputCopy(input, &_transaction->inputs[i]);
				TransactionInputPtr inputPtr = TransactionInputPtr(new TransactionInput(input));
				inputs.push_back(inputPtr);
			}

			return inputs;
		}

		void Transaction::transactionInputCopy(BRTxInput *target, const BRTxInput *source) {
			assert (target != nullptr);
			assert (source != nullptr);
			*target = *source;

			target->script = nullptr;
			BRTxInputSetScript(target, source->script, source->scriptLen);

			target->signature = nullptr;
			BRTxInputSetSignature(target, source->signature, source->sigLen);
		}

		void Transaction::transactionOutputCopy(BRTxOutput *target, const BRTxOutput *source) {
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

		SharedWrapperList<TransactionOutput, BRTxOutput *> Transaction::getOutputs() {

			size_t outputCount = _transaction->outCount;

			SharedWrapperList<TransactionOutput, BRTxOutput *> outputs;
			for (int i = 0; i < outputCount; i++) {

				BRTxOutput *output = new BRTxOutput;
				transactionOutputCopy(output, &_transaction->outputs[i]);
				outputs.push_back(TransactionOutputPtr(new TransactionOutput(output)));
			}

			return outputs;
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

	}
}
