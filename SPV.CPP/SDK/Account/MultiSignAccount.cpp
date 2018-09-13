// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <set>

#include <SDK/ELACoreExt/ErrorCode.h>
#include <SDK/Common/Utils.h>
#include <boost/bind.hpp>
#include <SDK/Wrapper/ByteStream.h>
#include "MultiSignAccount.h"

namespace Elastos {
	namespace ElaWallet {

		MultiSignAccount::MultiSignAccount(IAccount *me, const std::vector<std::string> &coSigners,
										   uint32_t requiredSignCount) :
				_me(me),
				_requiredSignCount(requiredSignCount),
				_coSigners(coSigners) {
		}

		Key MultiSignAccount::DeriveKey(const std::string &payPassword) {
			checkSigners();
			return _me->DeriveKey(payPassword);
		}

		UInt512 MultiSignAccount::DeriveSeed(const std::string &payPassword) {
			checkSigners();
			return _me->DeriveSeed(payPassword);
		}

		void MultiSignAccount::ChangePassword(const std::string &oldPassword, const std::string &newPassword) {
			if (_me != nullptr)
				_me->ChangePassword(oldPassword, newPassword);
		}

		nlohmann::json MultiSignAccount::ToJson() const {
			nlohmann::json j;
			to_json(j, *this);
			return j;
		}

		void MultiSignAccount::FromJson(const nlohmann::json &j) {
			from_json(j, *this);
		}

		const CMBlock &MultiSignAccount::GetEncryptedKey() const {
			checkSigners();
			return _me->GetEncryptedKey();
		}

		const CMBlock &MultiSignAccount::GetEncryptedMnemonic() const {
			checkSigners();
			return _me->GetEncryptedMnemonic();
		}

		const CMBlock &MultiSignAccount::GetEncryptedPhrasePassword() const {
			checkSigners();
			return _me->GetEncryptedPhrasePassword();
		}

		const std::string &MultiSignAccount::GetPublicKey() const {
			checkSigners();
			return _me->GetPublicKey();
		}

		const MasterPubKey &MultiSignAccount::GetIDMasterPubKey() const {
			checkSigners();
			return _me->GetIDMasterPubKey();
		}

		bool MultiSignAccount::Compare(const std::string &a, const std::string &b) const {
			CMBlock cbA = Utils::decodeHex(a);
			CMBlock cbB = Utils::decodeHex(b);
			return memcmp(cbA, cbB, cbA.GetSize()) >= 0;
		}

		void MultiSignAccount::checkSigners() const {
			if (_me == nullptr)
				ErrorCode::StandardLogicError(ErrorCode::WrongAccountType,
											  "Readonly account do not support this operation.");
		}

		std::string MultiSignAccount::GetAddress() {
			if (_address.empty()) {
				// redeem script -> program hash
				UInt168 programHash = Utils::codeToProgramHash(GenerateRedeemScript());

				// program hash -> address
				_address = Utils::UInt168ToAddress(programHash);
			}
			return _address;
		}

		CMBlock MultiSignAccount::GenerateRedeemScript() const {
			std::set<std::string> uniqueSigners(_coSigners.begin(), _coSigners.end());
			if (_me != nullptr)
				uniqueSigners.insert(_me->GetPublicKey());

			if (uniqueSigners.size() < _requiredSignCount)
				ErrorCode::StandardLogicError(ErrorCode::MultiSignError, "Required sign count greater than signers.");

			if (uniqueSigners.size() > sizeof(uint8_t) - OP_1)
				ErrorCode::StandardLogicError(ErrorCode::MultiSignError, "Signers should less than 205.");

			std::vector<std::string> sortedSigners(uniqueSigners.begin(), uniqueSigners.end());

			std::sort(sortedSigners.begin(), sortedSigners.end(),
					  boost::bind(&MultiSignAccount::Compare, this, _1, _2));

			ByteStream stream;
			stream.writeUint8(uint8_t(OP_1 + _requiredSignCount - 1));
			for (size_t i = 0; i < sortedSigners.size(); i++) {
				CMBlock pubKey = Utils::decodeHex(sortedSigners[i]);
				stream.writeUint8(uint8_t(pubKey.GetSize()));
				stream.writeBytes(pubKey, pubKey.GetSize());
			}

			stream.writeUint8(uint8_t(OP_1 + sortedSigners.size() - 1));
			stream.writeUint8(ELA_MULTISIG);

			return stream.getBuffer();
		}

		void to_json(nlohmann::json &j, const MultiSignAccount &p) {
			j["Account"] = p._me->ToJson();
			j["CoSigners"] = p._coSigners;
			j["RequiredSignCount"] = p._requiredSignCount;
		}

		void from_json(const nlohmann::json &j, MultiSignAccount &p) {
			p._me->FromJson(j["Account"]);
			p._coSigners = j["CoSigners"].get<std::vector<std::string>>();
			p._requiredSignCount = j["RequiredSignCount"].get<uint32_t>();
		}

		nlohmann::json MultiSignAccount::GetBasicInfo() const {
			nlohmann::json j;
			j["Type"] = "Multi-Sign";
			nlohmann::json details;
			std::vector<std::string> signers = _coSigners;
			if(_me != nullptr)
				signers.push_back(_me->GetPublicKey());
			details["Signers"] = signers;
			details["RequiredSignCount"] = _requiredSignCount;
			j["Details"] = details;
			return j;
		}

	}
}
