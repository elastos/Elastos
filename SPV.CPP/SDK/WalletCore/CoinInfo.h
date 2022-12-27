// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_ELACOINPATH_H__
#define __ELASTOS_SDK_ELACOINPATH_H__

#include <Common/uint256.h>

#include <nlohmann/json.hpp>

#include <string>
#include <cstdint>

namespace Elastos {
	namespace ElaWallet {

		class CoinInfo {
		public:
			CoinInfo();

			const std::string &GetChainID() const;

			void SetChainID(const std::string &id);

			nlohmann::json ToJson() const;

			void FromJson(const nlohmann::json &j);

		private:
			std::string _chainID;
		};

		typedef boost::shared_ptr<CoinInfo> CoinInfoPtr;

	}
}

#endif //__ELASTOS_SDK_ELACOINPATH_H__
