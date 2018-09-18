// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Registry.h"

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

		void Registry::AddMerkleBlockProto(IMerkleBlock *merkleBlock) {
			_merkleBlocks[merkleBlock->getBlockType()] = MerkleBlockPtr(merkleBlock);
		}

		void Registry::RemoveMerkleBlockProto(IMerkleBlock *merkleBlock) {
			_merkleBlocks.erase(merkleBlock->getBlockType());
		}

		IMerkleBlock *Registry::CloneMerkleBlock(const std::string &blockType, const BRMerkleBlock *block, bool manageRaw) {
			if(_merkleBlocks.find(blockType) == _merkleBlocks.end())
				return nullptr;

			return _merkleBlocks[blockType]->Clone(block, manageRaw);
		}

		IMerkleBlock *Registry::CreateMerkleBlock(const std::string &blockType, bool manageRaw) {
			if(_merkleBlocks.find(blockType) == _merkleBlocks.end())
				return nullptr;

			return _merkleBlocks[blockType]->CreateMerkleBlock(manageRaw);
		}

	}
}
