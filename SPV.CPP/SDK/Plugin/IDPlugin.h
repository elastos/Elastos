// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_IDPLUGIN_H__
#define __ELASTOS_SDK_IDPLUGIN_H__

#include <Plugin/Interface/IPlugin.h>
#include <Plugin/Registry.h>
#include <Plugin/Block/SidechainMerkleBlock.h>

#include <fruit/fruit.h>

namespace Elastos {
	namespace ElaWallet {

		class IDPlugin : public IPlugin {
		public:
			INJECT(IDPlugin(ISidechainMerkleBlockFactory *merkleBlockFactory)) :
					_merkleBlockFactory(merkleBlockFactory) {
			}

			virtual MerkleBlockPtr CreateBlock();

		private:
			ISidechainMerkleBlockFactory *_merkleBlockFactory;
		};

		fruit::Component<> getIDPluginComponent();

	}
}

#endif //__ELASTOS_SDK_IDPLUGIN_H__
