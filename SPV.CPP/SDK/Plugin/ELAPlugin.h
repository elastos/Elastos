// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_ELAPLUGIN_H__
#define __ELASTOS_SDK_ELAPLUGIN_H__

#include <fruit/fruit.h>

#include "Interface/IPlugin.h"
#include "Block/MerkleBlock.h"

namespace Elastos {
	namespace ElaWallet {

		struct ELAPluginTag {};

		class ELAPlugin : public IPlugin {
		public:
			INJECT(ELAPlugin(IMerkleBlockFactory *merkleBlockFactory)) :
					_merkleBlockFactory(merkleBlockFactory) {
			}

			virtual MerkleBlockPtr CreateBlock();

		private:
			IMerkleBlockFactory *_merkleBlockFactory;
		};

		fruit::Component<fruit::Annotated<ELAPluginTag, IPlugin>, ELAPlugin> getELAPluginComponent();

	}
}


#endif //__ELASTOS_SDK_ELAPLUGIN_H__
