// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/make_shared.hpp>
#include <cstring>
#include <BRTransaction.h>
#include <SDK/Common/Log.h>
#include <Core/BRTransaction.h>
#include <SDK/Common/ParamChecker.h>

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
#include "Utils.h"
#include "BRAddress.h"
#include "SDK/TransactionHub/TransactionHub.h"

#define STANDARD_FEE_PER_KB 10000
#define DEFAULT_PAYLOAD_TYPE  TransferAsset
#define TX_VERSION           0x00000001
#define TX_LOCKTIME          0x00000000

namespace Elastos {
	namespace ElaWallet {

		Transaction::Transaction() :
				_version(TX_VERSION),
				_lockTime(TX_LOCKTIME),
				_blockHeight(TX_UNCONFIRMED),
				_payloadVersion(0),
				_fee(0),
				_assetTableID(UINT32_MAX),
				_payload(nullptr),
				_type(DEFAULT_PAYLOAD_TYPE),
				_isRegistered(false) {
		}

		Transaction::Transaction(const Transaction &tx) {
			operator=(tx);
		}

		Transaction &Transaction::operator=(const Transaction &orig) {
			_isRegistered = orig._isRegistered;
			_assetTableID = orig._assetTableID;

			_type = orig._type;
			_payloadVersion = orig._payloadVersion;
			_fee = orig._fee;

			initPayloadFromType(orig._type);

			ByteStream stream;
			orig._payload->Serialize(stream);
			stream.setPosition(0);
			_payload->Deserialize(stream);

			_inputs.clear();
			for (size_t i = 0; i < orig._inputs.size(); ++i) {
				_inputs.push_back(orig._inputs[i]);
			}

			_outputs.clear();
			for (size_t i = 0; i < orig._outputs.size(); ++i) {
				_outputs.push_back(orig._outputs[i]);
			}

			_attributes.clear();
			for (size_t i = 0; i < orig._attributes.size(); ++i) {
				_attributes.push_back(orig._attributes[i]);
			}

			_programs.clear();
			for (size_t i = 0; i < _programs.size(); ++i) {
				_programs.push_back(orig._programs[i]);
			}

			_remark = orig._remark;
			return *this;
		}

		Transaction::~Transaction() {
		}

		bool Transaction::isRegistered() const {
			return _isRegistered;
		}

		bool &Transaction::isRegistered() {
			return _isRegistered;
		}

		void Transaction::resetHash() {
			UInt256Set(&_txHash, UINT256_ZERO);
		}

		const UInt256 &Transaction::getHash() const {
			if (UInt256IsZero(&_txHash)) {
				ByteStream ostream;
				serializeUnsigned(ostream);
				CMBlock buff = ostream.getBuffer();
				BRSHA256_2(&_txHash, buff, buff.GetSize());
			}
			return _txHash;
		}

		uint32_t Transaction::getVersion() const {
			return _version;
		}

		void Transaction::setVersion(uint32_t v) {
			_version = v;
		}

		void Transaction::setTransactionType(Type t) {
			if (_type != t) {
				_type = t;
				initPayloadFromType(_type);
			}
		}

		Transaction::Type Transaction::getTransactionType() const {
			return _type;
		}

		void Transaction::reinit() {
			Cleanup();
			_type = DEFAULT_PAYLOAD_TYPE;
			initPayloadFromType(_type);

			_version = TX_VERSION;
			_lockTime = TX_LOCKTIME;
			_blockHeight = TX_UNCONFIRMED;
			_payloadVersion = 0;
			_fee = 0;
		}

		const std::vector<TransactionOutput> &Transaction::getOutputs() const {
			return _outputs;
		}

		std::vector<TransactionOutput> &Transaction::getOutputs() {
			return _outputs;
		}

		const std::vector<TransactionInput> &Transaction::getInputs() const {
			return _inputs;
		}

		std::vector<TransactionInput>& Transaction::getInputs() {
			return _inputs;
		}

		std::vector<std::string> Transaction::getOutputAddresses() {

			const std::vector<TransactionOutput> &_outputs = getOutputs();
			ssize_t len = _outputs.size();
			std::vector<std::string> addresses(len);
			for (int i = 0; i < len; i++)
				addresses[i] = _outputs[i].getAddress();

			return addresses;
		}

		uint32_t Transaction::getLockTime() {

			return _lockTime;
		}

		void Transaction::setLockTime(uint32_t t) {

			_lockTime = t;
		}

		uint32_t Transaction::getBlockHeight() {
			return _blockHeight;
		}

		void Transaction::setBlockHeight(uint32_t height) {
			_blockHeight = height;
		}

		uint32_t Transaction::getTimestamp() {
			return _timestamp;
		}

		void Transaction::setTimestamp(uint32_t t) {
			_timestamp = t;
		}

		void Transaction::addOutput(const TransactionOutput &o) {
			_outputs.push_back(o);
		}

		void Transaction::removeChangeOutput() {
			if (_outputs.size() > 1) {
				_outputs.erase(_outputs.begin() + 1);
			}
		}

		void Transaction::addInput(const TransactionInput &input) {
			_inputs.push_back(input);
		}

		size_t Transaction::getSize() {
			size_t size;
			size = 8 + BRVarIntSize(_inputs.size()) + BRVarIntSize(_outputs.size());

			for (size_t i = 0; _inputs.size(); i++) {
				size += _inputs[i].getSize();
			}

			for (size_t i = 0; _outputs.size(); i++) {
				size += _outputs[i].GetSize();
			}

			return size;
		}

		bool Transaction::isSigned() const {
			if (_type == Type::TransferAsset) {
				if (_programs.size() <= 0) {
					return false;
				}
				for (size_t i = 0; i < _programs.size(); ++i) {
					if (!_programs[i].isValid(this)) {
						return false;
					}
				}
			} else if (_type == Type::IssueToken) {
				// TODO verify merkle proof
				return true;
			} else if (_type == Type::CoinBase) {
				return true;
			}

			return true;
		}

		bool Transaction::sign(const WrapperList<Key, BRKey> &keys, const boost::shared_ptr<TransactionHub> &wallet) {
			return transactionSign(keys, wallet);
		}

		bool Transaction::transactionSign(const WrapperList<Key, BRKey> keys, const boost::shared_ptr<TransactionHub> &wallet) {
			size_t i, j, keysCount = keys.size();
			Address addrs[keysCount], address;

			ParamChecker::checkCondition(keysCount <= 0, Error::Transaction,
										 "Transaction sign key not found");
			LOG_DEBUG("tx sign with {} keys", keysCount);

			for (i = 0; i < keysCount; i++) {
				addrs[i] = Address::None;
				std::string tempAddr = keys[i].address();
				if (!tempAddr.empty()) {
					addrs[i] = tempAddr;
				}
			}

			for (i = 0; i < _inputs.size(); i++) {
				const TransactionPtr &tx = wallet->transactionForHash(_inputs[i].getTransctionHash());
				address = tx->getOutputs()[_inputs[i].getIndex()].getAddress();

				j = 0;
				while (j < keysCount && !addrs[j].IsEqual(address)) j++;
				if (j >= keysCount) continue;
				int signType = address.getSignType();
				std::string redeemScript = keys[j].keyToRedeemScript(signType);
				CMBlock code = Utils::decodeHex(redeemScript);
				if (_type == Type::RegisterIdentification ||
					i >= _programs.size()) {

					Program newProgram;
					newProgram.setCode(code);
					_programs.push_back(newProgram);

				}

				CMBlock signData = keys[j].compactSign(GetShaData());
				_programs[i].setParameter(signData);
			}

			return isSigned();
		}

		UInt256 Transaction::getReverseHash() {

			return UInt256Reverse(&_txHash);
		}

		uint64_t Transaction::getMinOutputAmount() {

			return TX_MIN_OUTPUT_AMOUNT;
		}

		const IPayload *Transaction::getPayload() const {
			return _payload.get();
		}

		IPayload *Transaction::getPayload() {
			return _payload.get();
		}

		void Transaction::addAttribute(const Attribute &attribute) {
			_attributes.push_back(attribute);
		}

		const std::vector<Attribute> &Transaction::getAttributes() const {
			return _attributes;
		}

		void Transaction::addProgram(const Program &program) {
			_programs.push_back(program);
		}

		const std::vector<Program> &Transaction::getPrograms() const {
			return _programs;
		}

		std::vector<Program> &Transaction::getPrograms() {
			return _programs;
		}

		void Transaction::clearPrograms() {
			_programs.clear();
		}

		void Transaction::removeDuplicatePrograms() {
			std::set<std::string> programSet;

			for (std::vector<Program>::iterator iter = _programs.begin(); iter != _programs.end();) {
				std::string key = Utils::encodeHex(iter->getCode());
				if (programSet.find(key) == programSet.end()) {
					programSet.insert(key);
					++iter;
				} else {
					iter = _programs.erase(iter);
				}
			}
		}

		const std::string Transaction::getRemark() const {
			return _remark;
		}

		void Transaction::setRemark(const std::string &remark) {
			_remark = remark;
		}

		void Transaction::Serialize(ByteStream &ostream) const {
			serializeUnsigned(ostream);

			ostream.writeVarUint(_programs.size());
			for (size_t i = 0; i < _programs.size(); i++) {
				_programs[i].Serialize(ostream);
			}
		}

		void Transaction::serializeUnsigned(ByteStream &ostream) const {
			ostream.writeBytes(&_type, 1);

			ostream.writeBytes(&_payloadVersion, 1);

			ParamChecker::checkCondition(_payload == nullptr, Error::Transaction,
										 "payload should not be null");

			_payload->Serialize(ostream);

			ostream.writeVarUint(_attributes.size());
			for (size_t i = 0; i < _attributes.size(); i++) {
				_attributes[i].Serialize(ostream);
			}

			ostream.writeVarUint(_inputs.size());
			for (size_t i = 0; i < _inputs.size(); i++) {
				_inputs[i].Serialize(ostream);
			}

			const std::vector<TransactionOutput> &_outputs = getOutputs();
			ostream.writeVarUint(_outputs.size());
			for (size_t i = 0; i < _outputs.size(); i++) {
				_outputs[i].Serialize(ostream);
			}

			ostream.writeUint32(_lockTime);
		}

		bool Transaction::Deserialize(ByteStream &istream) {
			reinit();

			if (!istream.readBytes(&_type, 1))
				return false;
			if (!istream.readBytes(&_payloadVersion, 1))
				return false;

			initPayloadFromType(_type);

			if (_payload == nullptr) {
				Log::error("new _payload with _type={} when deserialize error", _type);
				return false;
			}
			if (!_payload->Deserialize(istream))
				return false;

			uint64_t attributeLength = 0;
			if (!istream.readVarUint(attributeLength))
				return false;

			for (size_t i = 0; i < attributeLength; i++) {
				Attribute attribute;
				if (!attribute.Deserialize(istream)) {
					Log::error("deserialize tx attribute[{}] error", i);
					return false;
				}
				_attributes.push_back(attribute);
			}

			uint64_t inCount = 0;
			if (!istream.readVarUint(inCount)) {
				Log::error("deserialize tx inCount error");
				return false;
			}

			for (size_t i = 0; i < inCount; i++) {
				TransactionInput input;
				if (!input.Deserialize(istream)) {
					Log::error("deserialize tx input [{}] error", i);
					return false;
				}
				_inputs.push_back(input);
			}

			uint64_t outputLength = 0;
			if (!istream.readVarUint(outputLength)) {
				Log::error("deserialize tx output len error");
				return false;
			}

			for (size_t i = 0; i < outputLength; i++) {
				TransactionOutput output;
				if (!output.Deserialize(istream)) {
					Log::error("deserialize tx output[{}] error", i);
					return false;
				}
				_outputs.push_back(output);
			}

			if (!istream.readUint32(_lockTime)) {
				Log::error("deserialize tx lock time error");
				return false;
			}

			uint64_t programLength = 0;
			if (!istream.readVarUint(programLength)) {
				Log::error("deserialize tx program length error");
				return false;
			}

			for (size_t i = 0; i < programLength; i++) {
				Program program;
				if (!program.Deserialize(istream)) {
					Log::error("deserialize program[{}] error", i);
					return false;
				}
				_programs.push_back(program);
			}

			ByteStream ostream;
			serializeUnsigned(ostream);
			CMBlock buff = ostream.getBuffer();
			BRSHA256_2(&_txHash, buff, buff.GetSize());

			return true;
		}

		nlohmann::json Transaction::toJson() const {
			nlohmann::json jsonData;

			jsonData["IsRegistered"] = _isRegistered;

			jsonData["TxHash"] = Utils::UInt256ToString(getHash(), true);
			jsonData["Version"] = _version;
			jsonData["LockTime"] = _lockTime;
			jsonData["BlockHeight"] = _blockHeight;
			jsonData["Timestamp"] = _timestamp;

			std::vector<nlohmann::json> inputsJson(_inputs.size());
			for (size_t i = 0; i < _inputs.size(); ++i) {
				inputsJson[i] = _inputs[i].toJson();
			}
			jsonData["Inputs"] = inputsJson;

			jsonData["Type"] = (uint8_t) _type;
			jsonData["PayloadVersion"] = _payloadVersion;
			jsonData["PayLoad"] = _payload->toJson();

			std::vector<nlohmann::json> attributesJson(_attributes.size());
			for (size_t i = 0; i < _attributes.size(); ++i) {
				attributesJson[i] = _attributes[i].toJson();
			}
			jsonData["Attributes"] = attributesJson;

			std::vector<nlohmann::json> programsJson(_programs.size());
			for (size_t i = 0; i < _programs.size(); ++i) {
				programsJson[i] = _programs[i].toJson();
			}
			jsonData["Programs"] = programsJson;

			const std::vector<TransactionOutput> &txOutputs = getOutputs();
			std::vector<nlohmann::json> _outputs(txOutputs.size());
			for (size_t i = 0; i < txOutputs.size(); ++i) {
				_outputs[i] = txOutputs[i].toJson();
			}
			jsonData["Outputs"] = _outputs;

			jsonData["Fee"] = _fee;

			jsonData["_remark"] = _remark;

			return jsonData;
		}

		void Transaction::fromJson(const nlohmann::json &jsonData) {
			reinit();

			_isRegistered = jsonData["IsRegistered"];

			_txHash = Utils::UInt256FromString(jsonData["TxHash"].get<std::string>(), true);
			_version = jsonData["Version"].get<uint32_t>();
			_lockTime = jsonData["LockTime"].get<uint32_t>();
			_blockHeight = jsonData["BlockHeight"].get<uint32_t>();
			_timestamp = jsonData["Timestamp"].get<uint32_t>();

			std::vector<nlohmann::json> inputJsons = jsonData["Inputs"];
			for (size_t i = 0; i < inputJsons.size(); ++i) {
				_inputs.push_back(TransactionInput());
				_inputs[i].fromJson(inputJsons[i]);
			}

			_type = Type(jsonData["Type"].get<uint8_t>());
			_payloadVersion = jsonData["PayloadVersion"];
			initPayloadFromType(_type);

			if (_payload == nullptr) {
				Log::error("_payload is nullptr when convert from json");
			} else {
				_payload->fromJson(jsonData["PayLoad"]);
			}

			std::vector<nlohmann::json> attributesJson = jsonData["Attributes"];
			for (size_t i = 0; i < _attributes.size(); ++i) {
				Attribute attribute;
				attribute.fromJson(attributesJson[i]);
				_attributes.push_back(attribute);
			}

			std::vector<nlohmann::json> programsJson = jsonData["Programs"];
			for (size_t i = 0; i < _programs.size(); ++i) {
				Program program;
				program.fromJson(programsJson[i]);
				_programs.push_back(program);
			}

			std::vector<nlohmann::json> outputsJson = jsonData["Outputs"];
			for (size_t i = 0; i < _outputs.size(); ++i) {
				TransactionOutput output;
				output.fromJson(outputsJson[i]);
				_outputs.push_back(output);
			}

			_fee = jsonData["Fee"].get<uint64_t>();

			_remark = jsonData["_remark"].get<std::string>();
		}

		uint64_t Transaction::calculateFee(uint64_t feePerKb) {
			return ((getSize() + 999) / 1000) * feePerKb;
		}

		uint64_t Transaction::getTxFee(const boost::shared_ptr<TransactionHub> &wallet) {
			uint64_t fee = 0, inputAmount = 0, outputAmount = 0;

			for (size_t i = 0; i < _inputs.size(); ++i) {
				const TransactionPtr &tx = wallet->transactionForHash(_inputs[i].getTransctionHash());
				if (tx == nullptr) continue;
				inputAmount += tx->getOutputs()[_inputs[i].getIndex()].getAmount();
			}

			for (size_t i = 0; i < _outputs.size(); ++i) {
				outputAmount += _outputs[i].getAmount();
			}

			if (inputAmount >= outputAmount)
				fee = inputAmount - outputAmount;

			return fee;
		}

		void
		Transaction::generateExtraTransactionInfo(nlohmann::json &rawTxJson, const boost::shared_ptr<Wallet> &wallet,
												  uint32_t lastBlockHeight) {

			std::string remark = wallet->GetRemark(Utils::UInt256ToString(getHash()));
			setRemark(remark);

			nlohmann::json summary;
			summary["TxHash"] = Utils::UInt256ToString(getHash(), true);
			summary["Status"] = getStatus(lastBlockHeight);
			summary["ConfirmStatus"] = getConfirmInfo(lastBlockHeight);
			summary["Remark"] = getRemark();
			summary["Fee"] = getTxFee(wallet);
			summary["Timestamp"] = getTimestamp();
			nlohmann::json jOut;
			nlohmann::json jIn;

			if (_inputs.size() > 0 && wallet->containsTransaction(wallet->transactionForHash(_inputs[0].getTransctionHash()))) {
				std::string toAddress = "";

				if (wallet->containsAddress(_outputs[0].getAddress())) {
					// transfer to my other address of wallet
					jOut["Amount"] = _outputs[0].getAmount();
					jOut["ToAddress"] = _outputs[0].getAddress();
					summary["Outcoming"] = jOut;

					jIn["Amount"] = _outputs[0].getAmount();
					jIn["ToAddress"] = _outputs[0].getAddress();
					summary["Incoming"] = jIn;
				} else {
					jOut["Amount"] = _outputs[0].getAmount();
					jOut["ToAddress"] = _outputs[0].getAddress();
					summary["Outcoming"] = jOut;

					jIn["Amount"] = 0;
					jIn["ToAddress"] = "";
					summary["Incoming"] = jIn;
				}
			} else {
				uint64_t inputAmount = 0;
				std::string toAddress = "";

				for (size_t i = 0; i < _outputs.size(); ++i) {
					if (wallet->containsAddress(_outputs[i].getAddress())) {
						inputAmount += _outputs[i].getAmount();
						toAddress = _outputs[i].getAddress();
					}
				}

				jOut["Amount"] = 0;
				jOut["ToAddress"] = "";
				summary["Outcoming"] = jOut;

				jIn["Amount"] = inputAmount;
				jIn["ToAddress"] = toAddress;
				summary["Incoming"] = jIn;
			}

			rawTxJson["Summary"] = summary;
		}

		std::string Transaction::getConfirmInfo(uint32_t lastBlockHeight) {
			if (getBlockHeight() == TX_UNCONFIRMED)
				return std::to_string(0);

			uint32_t confirmCount = lastBlockHeight >= getBlockHeight() ? lastBlockHeight - getBlockHeight() + 1 : 0;
			return confirmCount <= 6 ? std::to_string(confirmCount) : "6+";
		}

		std::string Transaction::getStatus(uint32_t lastBlockHeight) {
			if (getBlockHeight() == TX_UNCONFIRMED)
				return "Unconfirmed";

			uint32_t confirmCount = lastBlockHeight >= getBlockHeight() ? lastBlockHeight - getBlockHeight() + 1 : 0;
			return confirmCount <= 6 ? "Pending" : "Confirmed";
		}

		CMBlock Transaction::GetShaData() const {
			ByteStream ostream;
			serializeUnsigned(ostream);
			CMBlock data = ostream.getBuffer();
			CMBlock shaData(sizeof(UInt256));
			BRSHA256(shaData, data, data.GetSize());
			return shaData;
		}

		void Transaction::initPayloadFromType(Type type) {
			if (type == CoinBase) {
				_payload = PayloadPtr(new PayloadCoinBase());
			} else if (type == RegisterAsset) {
				_payload = PayloadPtr(new PayloadRegisterAsset());
			} else if (type == TransferAsset) {
				_payload = PayloadPtr(new PayloadTransferAsset());
			} else if (type == Record) {
				_payload = PayloadPtr(new PayloadRecord());
			} else if (type == Deploy) {
				//todo add deploy _payload
				//_payload = boost::shared_ptr<PayloadDeploy>(new PayloadDeploy());
			} else if (type == SideMining) {
				_payload = PayloadPtr(new PayloadSideMining());
			} else if (type == IssueToken) {
				_payload = PayloadPtr(new PayloadIssueToken());
			} else if (type == WithdrawAsset) {
				_payload = PayloadPtr(new PayloadWithDrawAsset());
			} else if (type == TransferCrossChainAsset) {
				_payload = PayloadPtr(new PayloadTransferCrossChainAsset());
			} else if (type == RegisterIdentification) {
				_payload = PayloadPtr(new PayloadRegisterIdentification());
			}
		}

		void Transaction::Cleanup() {
			_inputs.clear();
			_outputs.clear();
			_attributes.clear();
			_programs.clear();
			_payload.reset();
		}

		uint8_t Transaction::getPayloadVersion() const {
			return _payloadVersion;
		}

		void Transaction::setPayloadVersion(uint8_t _version) {
			_payloadVersion = _version;
		}

		uint64_t Transaction::getFee() const {
			return _fee;
		}

		void Transaction::setFee(uint64_t f) {
			_fee = f;
		}

		bool Transaction::IsEqual(const Transaction *tx) const {
			return (tx == this || UInt256Eq(&_txHash, &tx->getHash()));
		}

		UInt256 Transaction::GetAssetID() const {
			UInt256 result = UINT256_ZERO;
			if (!_outputs.empty())
				result = _outputs.begin()->getAssetId();
			return result;
		}

		uint32_t Transaction::GetAssetTableID() const {
			return _assetTableID;
		}

		void Transaction::SetAssetTableID(uint32_t assetTableID) {
			_assetTableID = assetTableID;
		}

	}
}
