// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PayloadCoinBase.h"
#include <SDK/Common/Utils.h>
#include <SDK/Common/Log.h>

#include <Core/BRInt.h>

#include <cstring>

namespace Elastos {
	namespace ElaWallet {
		PayloadCoinBase::PayloadCoinBase() {

		}

		PayloadCoinBase::PayloadCoinBase(const CMBlock &coinBaseData) {
			_coinBaseData = coinBaseData;
		}

		PayloadCoinBase::PayloadCoinBase(const PayloadCoinBase &payload) {
			operator=(payload);
		}

		PayloadCoinBase::~PayloadCoinBase() {
		}

		void PayloadCoinBase::SetCoinBaseData(const CMBlock &coinBaseData) {
			_coinBaseData = coinBaseData;
		}

		const CMBlock &PayloadCoinBase::GetCoinBaseData() const {
			return _coinBaseData;
		}

		void PayloadCoinBase::Serialize(ByteStream &ostream, uint8_t version) const {
			ostream.WriteVarBytes(_coinBaseData);
		}

		bool PayloadCoinBase::Deserialize(ByteStream &istream, uint8_t version) {
			return istream.ReadVarBytes(_coinBaseData);
		}

		nlohmann::json PayloadCoinBase::ToJson(uint8_t version) const {
			nlohmann::json j;
			j["CoinBaseData"] = Utils::EncodeHex(_coinBaseData);
			return j;
		}

		void PayloadCoinBase::FromJson(const nlohmann::json &j, uint8_t version) {
			_coinBaseData = Utils::DecodeHex(j["CoinBaseData"].get<std::string>());

		}

		IPayload &PayloadCoinBase::operator=(const IPayload &payload) {
			try {
				const PayloadCoinBase &payloadCoinBase = dynamic_cast<const PayloadCoinBase &>(payload);
				operator=(payloadCoinBase);
			} catch (const std::bad_cast &e) {
				Log::error("payload is not instance of PayloadCoinBase");
			}

			return *this;
		}

		PayloadCoinBase &PayloadCoinBase::operator=(const PayloadCoinBase &payload) {
			_coinBaseData.Memcpy(payload._coinBaseData);
			return *this;
		}

	}
}