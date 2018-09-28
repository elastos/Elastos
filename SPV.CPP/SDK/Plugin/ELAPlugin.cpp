// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <SDK/Plugin/Block/MerkleBlock.h>
#include "ELAPlugin.h"

namespace Elastos {
	namespace ElaWallet {

		MerkleBlockPtr ELAPlugin::CreateBlock() {
			return _merkleBlockFactory->createBlock();
		}

		fruit::Component<fruit::Annotated<ELAPluginTag, IPlugin>, ELAPlugin> getELAPluginComponent() {
			return fruit::createComponent()
					.bind<fruit::Annotated<ELAPluginTag, IPlugin>, ELAPlugin>()
					.install(getMerkleBlockFactoryComponent);
		};

	}
}
