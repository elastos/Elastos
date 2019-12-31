// Created by Aaron Voisine on 9/7/15.
// Copyright (c) 2015 breadwallet LLC
//
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.
//

#ifndef __ELASTOS_SDK_BIP39_H__
#define __ELASTOS_SDK_BIP39_H__

#include <Common/typedefs.h>
#include <Common/uint256.h>

namespace Elastos {
	namespace ElaWallet {

#define BIP39_WORDLIST_COUNT 2048

		class BIP39 {
		public:
			static uint512 DeriveSeed(const std::string &mnemonic, const std::string &passphrase = "");

			static std::string Encode(const std::vector<std::string> &dictionary, const bytes_t &entropy);

			static bytes_t Decode(const std::vector<std::string> &dictionary, const std::string &mnemonic);

		private:
			static uint512 PBKDF2(const bytes_t &pw, const bytes_t &salt, unsigned int rounds);
		};

	}
}

#endif //__ELASTOS_SDK_BIP39_H__
