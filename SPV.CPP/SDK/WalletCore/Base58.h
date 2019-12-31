// Created by Aaron Voisine on 9/15/15.
// Copyright (c) 2015 breadwallet LLC
// Copyright (c) 2017-2019 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_BASE58_H__
#define __ELASTOS_SDK_BASE58_H__

#include <Common/typedefs.h>
#include <string>

namespace Elastos {
	namespace ElaWallet {
		class Base58 {
		public:
			// unsecure versions, suitable for public keys
			static unsigned int countLeading0s(const bytes_t& data);

			static unsigned int countLeading0s(const std::string& numeral, char zeroSymbol);

			static std::string CheckEncode(const bytes_t &payload, uint8_t version);

			static std::string CheckEncode(const bytes_t &payload, const bytes_t &version = bytes_t());

			static bool CheckDecode(const std::string &base58check, bytes_t &payload, unsigned int &version);

			static bool CheckDecode(const std::string &base58check, bytes_t &payload);

			static std::string Encode(const bytes_t &payload);

			static bytes_t Decode(const std::string &base58);

			static bool Valid(const std::string &base58check);

		};
	}
}


#endif //SPVSDK_BASE58_H
