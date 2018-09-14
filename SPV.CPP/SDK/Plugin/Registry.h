// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_REGISTRY_H__
#define __ELASTOS_SDK_REGISTRY_H__

#include <map>
#include <memory>
#include <fruit/fruit.h>
#include <boost/noncopyable.hpp>

#include "Interface/IMerkleBlock.h"

namespace Elastos {
	namespace ElaWallet {

	class Registry : public boost::noncopyable {
		public:
			static Registry *Instance(bool erase = false);

			IMerkleBlock *CreateMerkleBlock(const std::string &blockType, bool manageRaw, const BRMerkleBlock *block = nullptr);

		private:
			Registry();

		};

		fruit::Component<bool> GetManageRawComponent(bool manage);
	}

}

#endif //__ELASTOS_SDK_REGISTRY_H__
