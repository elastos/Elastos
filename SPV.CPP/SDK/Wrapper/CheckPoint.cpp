// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "CheckPoint.h"
#include <SDK/Common/Utils.h>

#include <sstream>

namespace Elastos {
	namespace ElaWallet {

		CheckPoint::CheckPoint(uint32_t height, const std::string &hash, time_t timestamp, uint32_t target) :
			_height(height),
			_timestamp(timestamp),
			_target(target) {
			_hash = Utils::UInt256FromString(hash, true);
		}

		CheckPoint::CheckPoint(uint32_t height, const UInt256 &hash, time_t timestamp, uint32_t target) :
			_height(height),
			_hash(hash),
			_timestamp(timestamp),
			_target(target) {
		}

		CheckPoint::~CheckPoint() {
		}

		const uint32_t &CheckPoint::GetHeight() const {
			return _height;
		}

		const UInt256 &CheckPoint::GetHash() const {
			return _hash;
		}

		const time_t &CheckPoint::GetTimestamp() const {
			return _timestamp;
		}

		const uint32_t &CheckPoint::GetTarget() const {
			return _target;
		}

		CheckPoint &CheckPoint::operator=(const CheckPoint &checkpoint) {
			_height = checkpoint._height;
			_hash = checkpoint._hash;
			_timestamp = checkpoint._timestamp;
			_target = checkpoint._target;

			return *this;
		}

	}
}