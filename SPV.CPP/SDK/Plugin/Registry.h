// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_REGISTRY_H__
#define __ELASTOS_SDK_REGISTRY_H__

#include <map>
#include <memory>

#include "Interface/IMerkleBlock.h"
#include "Interface/IHDPath.h"

namespace Elastos {
	namespace ElaWallet {

		class Registry {
		public:
			static Registry *Instance(bool erase = false);

			IMerkleBlock *CloneMerkleBlock(const std::string &blockType, const BRMerkleBlock *block, bool manageRaw);

			IMerkleBlock *CreateMerkleBlock(const std::string &blockType, bool manageRaw);

			void AddMerkleBlockProto(IMerkleBlock *merkleBlock);

			void RemoveMerkleBlockProto(IMerkleBlock *merkleBlock);

			IHDPath *CreateHDPath(const std::string &pathType);

			void AddHDPathProto(IHDPath *hdPath);

			void RemoveHDPathProto(IHDPath *hdPath);

		private:

			Registry();

			typedef std::map<std::string, MerkleBlockPtr> MerkleBlockMap;
			MerkleBlockMap _merkleBlocks;

			typedef std::map<std::string, HDPathPtr> HDPathMap;
			HDPathMap _hdPaths;
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

		template <class T>
		class RegisterHDPathProxy {
		public:
			RegisterHDPathProxy() {
				if (Registry::Instance()) {
					_hdPath = new T;
					Registry::Instance()->AddHDPathProto(_hdPath);
				}
			}

			~RegisterHDPathProxy() {
				if (Registry::Instance()) {
					Registry::Instance()->RemoveHDPathProto(_hdPath);
				}
			}

			T *get() { return _hdPath;}

		private:
			T *_hdPath;
		};

#define REGISTER_MERKLEBLOCKPLUGIN(classname) \
    static Elastos::ElaWallet::RegisterMerkleBlockProxy<classname> g_proxy_##classname;

	}

#define REGISTER_HDPATHPLUGIN(classname) \
	static Elastos::ElaWallet::RegisterHDPathProxy<classname> g_proxy_##classname;
}

#endif //__ELASTOS_SDK_REGISTRY_H__
