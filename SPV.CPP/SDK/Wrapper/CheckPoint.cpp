// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <sstream>
#include <Core/BRChainParams.h>

#include "Utils.h"
#include "CheckPoint.h"

namespace Elastos {
	namespace ElaWallet {

		CheckPoint::CheckPoint() {

		}

		CheckPoint::CheckPoint(const BRCheckPoint &checkPoint) :
				_checkPoint(checkPoint) {
		}

		std::string CheckPoint::toString() const {
			std::stringstream ss;
			ss << this;
			return ss.str();
		}

		const BRCheckPoint *CheckPoint::getRaw() const {
			return &_checkPoint;
		}

		nlohmann::json &operator<<(nlohmann::json &j, const CheckPoint &p) {
			to_json(j, p);

			return j;
		}

		const nlohmann::json &operator>>(const nlohmann::json &j, CheckPoint &p) {
			from_json(j, p);

			return j;
		}

		void to_json(nlohmann::json &j, const CheckPoint &p) {
			j["Height"] = p._checkPoint.height;
			j["Hash"] = Utils::UInt256ToString(p._checkPoint.hash);
			j["Timestamp"] = p._checkPoint.timestamp;
			j["Target"] = p._checkPoint.target;
		}

		void from_json(const nlohmann::json &j, CheckPoint &p) {
			p._checkPoint.height = j["Height"].get<uint32_t>();
			p._checkPoint.hash = Utils::UInt256FromString(j["Hash"].get<std::string>());
			p._checkPoint.timestamp = j["Timestamp"].get<uint32_t>();
			p._checkPoint.target = j["Target"].get<uint32_t>();
		}
	}
}