// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_IMERKLEBLOCK_H__
#define __ELASTOS_SDK_IMERKLEBLOCK_H__

#include <boost/shared_ptr.hpp>

#include "BRInt.h"
#include "ELAMessageSerializable.h"

namespace Elastos {
	namespace ElaWallet {

		class IMerkleBlock : public ELAMessageSerializable {
		public:
			virtual ~IMerkleBlock() {}

			virtual uint32_t getHeight() const = 0;

			virtual void setHeight(uint32_t height) = 0;

			virtual uint32_t getTimestamp() const = 0;

			virtual void setTimestamp(uint32_t timestamp) = 0;

			virtual uint32_t getTarget() const = 0;

			virtual void setTarget(uint32_t target) = 0;

			virtual const UInt256 &getPrevBlockHash() const = 0;

			virtual void setPrevBlockHash(const UInt256 &hash) = 0;

			virtual const UInt256 &getRootBlockHash() const = 0;

			virtual void setRootBlockHash(const UInt256 &hash) = 0;

			virtual uint32_t getNonce() const = 0;

			virtual void setNonce(uint32_t nonce) = 0;

			virtual uint32_t getTransactionCount() const = 0;

			virtual void setTransactionCount(uint32_t count) = 0;

			virtual const UInt256 &getHash() const = 0;

			virtual void setHash(const UInt256 &hash) = 0;

			virtual bool isValid(uint32_t currentTime) const = 0;

			virtual std::string getBlockType() const = 0;

			virtual std::vector<UInt256> MerkleBlockTxHashes() const = 0;
		};

		typedef boost::shared_ptr<IMerkleBlock> MerkleBlockPtr;

	}
}

#endif //__ELASTOS_SDK_IMERKLEBLOCK_H__
