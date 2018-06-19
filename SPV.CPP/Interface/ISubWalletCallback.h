// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_ISUBWALLETCALLBACK_H__
#define __ELASTOS_SDK_ISUBWALLETCALLBACK_H__

#include <string>

#include <nlohmann/json.hpp>

namespace Elastos {
	namespace ElaWallet {

		class ISubWalletCallback {
		public:
			virtual ~ISubWalletCallback() noexcept {}

			/**
			 * Call back method fired when status of a transaction changed.
			 * @param txid indicate hash of the transaction.
			 * @param status can be "Added", "Deleted" or "Updated".
			 * @param desc is an detail description of transaction status.
			 * @param confirms is confirm count util this callback fired.
			 */
			virtual void OnTransactionStatusChanged(
					const std::string &txid,
					const std::string &status,
					const nlohmann::json &desc,
					uint32_t confirms) = 0;
		};

	}
}

#endif //__ELASTOS_SDK_ISUBWALLETCALLBACK_H__
