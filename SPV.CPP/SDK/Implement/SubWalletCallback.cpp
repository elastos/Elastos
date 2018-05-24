// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "SubWalletCallback.h"

namespace Elastos {
	namespace SDK {

		SubWalletCallback::~SubWalletCallback() {

		}

		void SubWalletCallback::OnTransactionStatusChanged(const std::string &txid,
														   const std::string &status,
														   const nlohmann::json &desc,
														   uint32_t confirms) {
		}

		SubWalletCallback::TransactionStatus SubWalletCallback::convertToStatus(const std::string &status) {
			if (status == "Added")
				return Added;
			else if (status == "Deleted")
				return Deleted;
			else if (status == "Updated")
				return Updated;
			return Unknown;
		}

		std::string SubWalletCallback::convertToString(SubWalletCallback::TransactionStatus status) {
			switch (status) {
				case Added:
					return "Added";
				case Deleted:
					return "Deleted";
				case Updated:
					return "Updated";
				default:
					return "Unknown";
			}
		}

	}
}