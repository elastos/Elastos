// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_SUBWALLETCALLBACK_H__
#define __ELASTOS_SDK_SUBWALLETCALLBACK_H__

#include <Interface/ISubWalletCallback.h>

#include <nlohmann/json.hpp>

namespace Elastos {
	namespace ElaWallet {

		class SubWalletCallback : public ISubWalletCallback {
		public:
			enum TransactionStatus {
				Added = 0,
				Deleted,
				Updated,
				Unknown
			};

		public:
			virtual ~SubWalletCallback();

			virtual void OnTransactionStatusChanged(
					const std::string &txid,
					const std::string &status,
					const nlohmann::json &desc,
					uint32_t confirms);

		public:
			static TransactionStatus convertToStatus(const std::string &status);

			static std::string convertToString(TransactionStatus status);
		};

	}
}

#endif //__ELASTOS_SDK_SUBWALLETCALLBACK_H__
