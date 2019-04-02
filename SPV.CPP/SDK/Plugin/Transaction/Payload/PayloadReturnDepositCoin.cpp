// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PayloadReturnDepositCoin.h"
#include <SDK/Common/Log.h>

namespace Elastos {
	namespace ElaWallet {
		PayloadReturnDepositCoin::PayloadReturnDepositCoin() {
		}

		PayloadReturnDepositCoin::PayloadReturnDepositCoin(const PayloadReturnDepositCoin &payload) {
			operator=(payload);
		}

		PayloadReturnDepositCoin::~PayloadReturnDepositCoin() {
		}

		void PayloadReturnDepositCoin::Serialize(ByteStream &ostream, uint8_t version) const {

		}

		bool PayloadReturnDepositCoin::Deserialize(const ByteStream &istream, uint8_t version) {
			return true;
		}

		nlohmann::json PayloadReturnDepositCoin::ToJson(uint8_t version) const {
			return nlohmann::json();
		}

		void PayloadReturnDepositCoin::FromJson(const nlohmann::json &j, uint8_t version) {

		}

		IPayload &PayloadReturnDepositCoin::operator=(const IPayload &payload) {
			try {
				const PayloadReturnDepositCoin &payloadReturnDepositCoin = dynamic_cast<const PayloadReturnDepositCoin &>(payload);
				operator=(payloadReturnDepositCoin);
			} catch (const std::bad_cast &e) {
				Log::error("payload is not instance of PayloadReturnDepositCoin");
			}

			return *this;
		}

		PayloadReturnDepositCoin &PayloadReturnDepositCoin::operator=(const PayloadReturnDepositCoin &payload) {
			return *this;
		}

	}
}