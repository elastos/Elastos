// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_REGISTRY_H__
#define __ELASTOS_SDK_REGISTRY_H__

#include <map>
#include <memory>

#include "Interface/IMerkleBlock.h"

namespace Elastos {
	namespace ElaWallet {

		class Registry {
		public:
			static Registry *Instance(bool erase = false);

			IMerkleBlock *CloneMerkleBlock(const std::string &blockType, bool manageRaw);

			IMerkleBlock *CreateMerkleBlock(const std::string &blockType, bool manageRaw);

			IMerkleBlock *CreateMerkleBlock(const std::string &blockType, BRMerkleBlock *block, bool manageRaw);

			void AddMerkleBlockProto(IMerkleBlock *merkleBlock);

			void RemoveMerkleBlockProto(IMerkleBlock *merkleBlock);

		private:

			Registry();

			typedef std::map<std::string, MerkleBlockPtr> MerkleBlockMap;
			MerkleBlockMap _merkleBlocks;
		};

		template<class T>
		class RegisterMerkleBlockProxy {
		public:
			RegisterMerkleBlockProxy() {
				if (Registry::Instance()) {
					_block = new T;
					Registry::Instance()->AddMerkleBlockProto(_block);
				}
			}

			~RegisterMerkleBlockProxy() {
				if (Registry::Instance()) {
					Registry::Instance()->RemoveMerkleBlockProto(_block);
				}
			}

			T *get() { return _block; }

		private:
			T *_block;
		};

#define REGISTER_MERKLEBLOCKPLUGIN(classname) \
    static Elastos::ElaWallet::RegisterMerkleBlockProxy<classname> g_proxy_##classname;

	}
}

#endif //__ELASTOS_SDK_REGISTRY_H__
