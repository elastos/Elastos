// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PayloadDefault.h"
#include <SDK/Common/Utils.h>
#include <SDK/Common/Log.h>
#include <SDK/Common/ParamChecker.h>

namespace Elastos {
	namespace ElaWallet {
		PayloadDefault::PayloadDefault() {
		}

		PayloadDefault::PayloadDefault(const PayloadDefault &payload){
			operator=(payload);
		}

		PayloadDefault::~PayloadDefault() {

		}

		void PayloadDefault::Serialize(ByteStream &ostream) const {
		}

		bool PayloadDefault::Deserialize(ByteStream &istream) {
			return true;
		}

		nlohmann::json PayloadDefault::toJson() const {
			return nlohmann::json();
		}

		void PayloadDefault::fromJson(const nlohmann::json &j) {
		}

		IOutputPayload &PayloadDefault::operator=(const IOutputPayload &payload) {
			try {
				const PayloadDefault &payloadDefault = dynamic_cast<const PayloadDefault &>(payload);
				operator=(payloadDefault);
			} catch (const std::bad_cast &e) {
				Log::error("payload is not instance of PayloadDefault");
			}

			return *this;
		}

		PayloadDefault& PayloadDefault::operator=(const PayloadDefault &payload) {
			return *this;
		}

	}
}
