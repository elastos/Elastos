// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_IIDCHAINSUBWALLET_H__
#define __ELASTOS_SDK_IIDCHAINSUBWALLET_H__

#include "ISidechainSubWallet.h"

namespace Elastos {
	namespace ElaWallet {

		class IIdChainSubWallet : public virtual ISidechainSubWallet {
		public:

			/**
			 * Virtual destructor.
			 */
			virtual ~IIdChainSubWallet() noexcept {}

			/**
			 * Create a id transaction and return the content of transaction in json format, this is a special transaction to register id related information on id chain.
			 * @param fromAddress specify which address we want to spend, or just input empty string to let wallet choose UTXOs automatically.
			 * @param payloadJson is payload for register id related information in json format, the content of payload should have Id, Path, DataHash, Proof, and Sign.
			 * @param programJson is program data in json format, which contains redeem script of id (Code) and sign of the \p payloadJson with id related private key (Parameter).
			 * @param memo input memo attribute for describing.
			 * @param remark is used to record message of local wallet.
			 * @return If success return the content of transaction in json format.
			 */
			virtual nlohmann::json CreateIdTransaction(
					const std::string &fromAddress,
					const nlohmann::json &payloadJson,
					const nlohmann::json &programJson,
					const std::string &memo,
					const std::string &remark) = 0;
		};

	}
}

#endif //__ELASTOS_SDK_IIDCHAINSUBWALLET_H__
