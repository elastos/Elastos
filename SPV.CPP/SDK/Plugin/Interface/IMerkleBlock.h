// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_IMERKLEBLOCK_H__
#define __ELASTOS_SDK_IMERKLEBLOCK_H__

#include <boost/shared_ptr.hpp>

#include "BRMerkleBlock.h"

#include "IClonable.h"
#include "ELAMessageSerializable.h"

namespace Elastos {
	namespace ElaWallet {

#ifdef MERKLE_BLOCK_PLUGIN
		class IMerkleBlock : public ELAMessageSerializable, public IClonable<IMerkleBlock> {
		public:
			virtual ~IMerkleBlock() {}

			virtual BRMerkleBlock *getRawBlock() const = 0;

			virtual void deleteRawBlock() = 0;

			virtual IMerkleBlock *CreateFromRaw(BRMerkleBlock *block, bool manageRaw) = 0;

			virtual void initFromRaw(BRMerkleBlock *block, bool manageRaw) = 0;

			virtual UInt256 getBlockHash() const = 0;

			virtual uint32_t getHeight() const = 0;

			virtual void setHeight(uint32_t height) = 0;

			virtual bool isValid(uint32_t currentTime) const = 0;

			virtual std::string getBlockType() const = 0;
		};

		typedef boost::shared_ptr<IMerkleBlock> MerkleBlockPtr;
#endif

	}
}

#endif //__ELASTOS_SDK_IMERKLEBLOCK_H__
