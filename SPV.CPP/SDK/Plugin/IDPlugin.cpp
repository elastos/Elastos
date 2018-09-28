// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "IDPlugin.h"
#include "Plugin/Block/SidechainMerkleBlock.h"

namespace Elastos {
	namespace ElaWallet {

		MerkleBlockPtr IDPlugin::CreateBlock() {
			return _merkleBlockFactory->createBlock();
		}

		fruit::Component<fruit::Annotated<IDPluginTag, IPlugin>, IDPlugin> getIDPluginComponent() {
			return fruit::createComponent()
					.bind<fruit::Annotated<IDPluginTag, IPlugin>, IDPlugin>()
					.install(getSidechainMerkleBlockFactoryComponent);
		}

	}
}
