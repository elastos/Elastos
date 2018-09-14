// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

#include "Registry.h"
#include "Block/MerkleBlock.h"
#include "Block/SidechainMerkleBlock.h"

namespace Elastos {
	namespace ElaWallet {

		Registry::Registry() {

		}

		Registry *Registry::Instance(bool erase) {
			static std::shared_ptr<Registry> instance(new Registry);
			if (erase) {
				instance.reset();
				instance = nullptr;
			}
			return instance.get();
		}

		IMerkleBlock *
		Registry::CreateMerkleBlock(const std::string &blockType, bool manageRaw, const BRMerkleBlock *block) {

			typedef fruit::Injector<IMerkleBlock> MerkleBlockInj;
			typedef boost::shared_ptr<MerkleBlockInj> MerkleBlockInjPtr;
			 MerkleBlockInjPtr injector;
			if (blockType == "ELA") {
				if (block == nullptr)
					injector = boost::make_shared<MerkleBlockInj>(GetMerkleBlockComponent, manageRaw);
				else
					injector = boost::make_shared<MerkleBlockInj>(
							GetMerkleBlockComponentWithParams, (ELAMerkleBlock *)block, manageRaw);

			} else if (blockType == "SideStandard") {
				if (block == nullptr)
					injector = boost::make_shared<MerkleBlockInj>(GetSidechainMerkleBlockComponent, manageRaw);
				else
					injector = boost::make_shared<MerkleBlockInj>(
							GetSidechainMerkleBlockComponentWithParams, (IdMerkleBlock *)block, manageRaw);
			}

			return injector->get<IMerkleBlock *>();
		}

		fruit::Component<bool> GetManageRawComponent(bool manage) {
			return fruit::createComponent().bindInstance(manage);
		}

	}
}