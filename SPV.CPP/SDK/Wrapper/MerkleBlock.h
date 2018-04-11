// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_MERKLEBLOCK_H__
#define __ELASTOS_SDK_MERKLEBLOCK_H__

#include <boost/shared_ptr.hpp>
#include <nlohmann/json.hpp>

#include "BRMerkleBlock.h"

#include "Wrapper.h"
#include "ByteData.h"

namespace Elastos {
	namespace SDK {

		class MerkleBlock :
			public Wrapper<BRMerkleBlock *> {

		public:
			MerkleBlock(BRMerkleBlock *merkleBlock);

			MerkleBlock(ByteData block, int blockHeight);

			~MerkleBlock();

			virtual std::string toString() const;

			virtual BRMerkleBlock *getRaw() const;

			UInt256 getBlockHash() const;

			uint32_t getVersion() const;

			UInt256 getPrevBlockHash() const;

			UInt256 getRootBlockHash() const;

			uint32_t getTimestamp() const;

			uint32_t getTarget() const;

			uint32_t getNonce() const;

			uint32_t getTransactionCount() const;

			uint32_t getHeight() const;

			ByteData serialize() const;

			bool isValid(uint32_t currentTime) const;

			bool containsTransactionHash(UInt256 hash) const;

		private:

			BRMerkleBlock *_merkleBlock;
		};

		typedef boost::shared_ptr<MerkleBlock> MerkleBlockPtr;

		//support for json converting
		//read "Arbitrary types conversions" section in readme of
		//	https://github.com/nlohmann/json for more details
		void to_json(nlohmann::json& j, const MerkleBlock& p);

		void from_json(const nlohmann::json& j, MerkleBlock& p);

	}
}

#endif //__ELASTOS_SDK_MERKLEBLOCK_H__
