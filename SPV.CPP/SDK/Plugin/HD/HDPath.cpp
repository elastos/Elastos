// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Core/BRBIP32Sequence.h"

#include "HDPath.h"
#include "SDK/Plugin/Registry.h"

namespace Elastos {
	namespace ElaWallet {

		HDPath::HDPath() {

		}

		Key HDPath::CalculateSubWalletMasterKey(const UInt512 &seed, int coinIndex, UInt256 &chainCode) {
			Key wrapperKey;
			wrapperKey.deriveKeyAndChain(chainCode, &seed, sizeof(seed), 5, 1 | BIP32_HARD, 0, 44, coinIndex, 0);
			return wrapperKey;
		}

		std::string HDPath::GetHDPathType() const {
			return "Normal";
		}

		IHDPath *HDPath::CreateHDPath() const {
			return new HDPath();
		}

		REGISTER_HDPATHPLUGIN(HDPath);
	}
}