// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_PLUGINTYPES_H__
#define __ELASTOS_SDK_PLUGINTYPES_H__

#include "KeyStore/CoinConfig.h"

namespace Elastos {
	namespace ElaWallet {

		struct PluginTypes {
			PluginTypes() :
				BlockType("") {

			}

			PluginTypes(const CoinConfig &coinConfig) :
				BlockType(coinConfig.BlockType) {

			}

			PluginTypes(const std::string &blockType):
				BlockType(blockType) {

			}

			std::string BlockType;
		};

	}
}

#endif //__ELASTOS_SDK_PLUGINTYPES_H__
