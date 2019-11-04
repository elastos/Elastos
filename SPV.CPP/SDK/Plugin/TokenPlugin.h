// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_TOKENPLUGIN_H__
#define __ELASTOS_SDK_TOKENPLUGIN_H__

#include <Plugin/Interface/IPlugin.h>
#include <Plugin/Block/SidechainMerkleBlock.h>
#include <Plugin/Registry.h>

#include <fruit/fruit.h>

namespace Elastos {
	namespace ElaWallet {

		class TokenPlugin : public IPlugin{
		public:
			INJECT(TokenPlugin(ISidechainMerkleBlockFactory *merkleBlockFactory)) :
								_merkleBlockFactory(merkleBlockFactory) {
			}

			virtual MerkleBlockPtr CreateBlock();

		private:
			ISidechainMerkleBlockFactory *_merkleBlockFactory;
		};

		fruit::Component<> getTokenPluginComponent();

	}
}

#endif //__ELASTOS_SDK_TOKENPLUGIN_H__
