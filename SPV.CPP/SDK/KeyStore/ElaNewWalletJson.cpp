// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "ElaNewWalletJson.h"

namespace Elastos {
	namespace SDK {

		ElaNewWalletJson::ElaNewWalletJson() :
				_id(""),
				_idInfo(""),
				_earliestPeerTime(0) {

		}

		ElaNewWalletJson::~ElaNewWalletJson() {

		}

		const std::string &ElaNewWalletJson::getID() const {
			return _id;
		}

		void ElaNewWalletJson::setID(const std::string &id) {
			_id = id;
		}

		const std::string &ElaNewWalletJson::getIDInfo() const {
			return _idInfo;
		}

		void ElaNewWalletJson::setIDInfo(const std::string &value) {
			_idInfo = value;
		}

		uint32_t ElaNewWalletJson::getEarliestPeerTime() const {
			return _earliestPeerTime;
		}

		void ElaNewWalletJson::setEaliestPeerTime(uint32_t time) {
			_earliestPeerTime = time;
		}

		void to_json(nlohmann::json &j, const ElaNewWalletJson &p) {

		}

		void from_json(const nlohmann::json &j, ElaNewWalletJson &p) {

		}
	}
}