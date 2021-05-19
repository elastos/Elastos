// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "ELAPlugin.h"
#include <Plugin/Block/MerkleBlock.h>

namespace Elastos {
	namespace ElaWallet {

		MerkleBlockPtr ELAPlugin::CreateBlock() {
			return _merkleBlockFactory->createBlock();
		}

		fruit::Component<> getELAPluginComponent() {
			return fruit::createComponent()
					.addMultibinding<IPlugin, ELAPlugin>()
					.install(getMerkleBlockFactoryComponent);
		};

		REGISTER_MERKLEBLOCKPLUGIN(ELA, getELAPluginComponent);
	}
}
