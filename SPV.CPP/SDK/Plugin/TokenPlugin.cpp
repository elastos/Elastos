// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "TokenPlugin.h"

namespace Elastos {
	namespace ElaWallet {

		MerkleBlockPtr TokenPlugin::CreateBlock() {
			return _merkleBlockFactory->createBlock();
		}

		fruit::Component<> getTokenPluginComponent() {
			return fruit::createComponent()
					.addMultibinding<IPlugin, TokenPlugin>()
					.install(getSidechainMerkleBlockFactoryComponent);
		}

		REGISTER_MERKLEBLOCKPLUGIN(TokenChain, getTokenPluginComponent);
	}
}
