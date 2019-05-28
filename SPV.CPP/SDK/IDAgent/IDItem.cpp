// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "IDItem.h"

using namespace nlohmann;

namespace Elastos {
	namespace ElaWallet {

		json &operator<<(json &j, const IDItem &p) {
			to_json(j, p);

			return j;
		}

		const json &operator>>(const json &j, IDItem &p) {
			from_json(j, p);

			return j;
		}

		void to_json(json &j, const IDItem &p) {
			j["Purpose"] = p.Purpose;
			j["Index"] = p.Index;
			j["PublicKey"] = p.PublicKey.getHex();
		}

		void from_json(const json &j, IDItem &p) {
			p.Purpose = j["Purpose"].get<uint32_t>();
			p.Index = j["Index"].get<uint32_t>();
			p.PublicKey.setHex(j["PublicKey"].get<std::string>());
		}

	}
}