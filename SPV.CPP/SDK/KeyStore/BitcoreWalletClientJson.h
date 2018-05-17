// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_BITCOREWALLETCLIENTJSON_H__
#define __ELASTOS_SDK_BITCOREWALLETCLIENTJSON_H__

#include <string>

namespace Elastos {
	namespace SDK {

		class BitcoreWalletClientJson {
		public:
			BitcoreWalletClientJson();

			virtual ~BitcoreWalletClientJson();

		private:
			std::string _coin;
			std::string _network;
			std::string _xPrivKey;
			std::string _xPubKey;
			//todo complete me
		};

	}
}

#endif //__ELASTOS_SDK_BITCOREWALLETCLIENTJSON_H__
