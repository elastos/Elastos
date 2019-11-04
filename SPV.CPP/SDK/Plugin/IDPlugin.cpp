// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "IDPlugin.h"

#include <Plugin/Block/SidechainMerkleBlock.h>

namespace Elastos {
	namespace ElaWallet {

		MerkleBlockPtr IDPlugin::CreateBlock() {
			return _merkleBlockFactory->createBlock();
		}

		fruit::Component<> getIDPluginComponent() {
			return fruit::createComponent()
					.addMultibinding<IPlugin, IDPlugin>()
					.install(getSidechainMerkleBlockFactoryComponent);
		}

		REGISTER_MERKLEBLOCKPLUGIN(IDChain, getIDPluginComponent);

	}
}
