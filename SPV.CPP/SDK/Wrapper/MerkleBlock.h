// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_MERKLEBLOCK_H__
#define __ELASTOS_SDK_MERKLEBLOCK_H__

#include <boost/shared_ptr.hpp>
#include <nlohmann/json.hpp>

#include "Wrapper.h"
#include "c_util.h"
#include "ELAMessageSerializable.h"
#include "ELACoreExt/AuxPow.h"
#include "ELACoreExt/ELAMerkleBlock.h"

namespace Elastos {
	namespace SDK {

		class MerkleBlock :
			public Wrapper<BRMerkleBlock>,
			public ELAMessageSerializable {

		public:
			MerkleBlock();

			MerkleBlock(ELAMerkleBlock *merkleBlock);

			~MerkleBlock();

			virtual std::string toString() const;

			virtual BRMerkleBlock *getRaw() const;

			virtual void Serialize(ByteStream &ostream) const;

			virtual void Deserialize(ByteStream &istream);

			UInt256 getBlockHash() const;

			uint32_t getVersion() const;

			UInt256 getPrevBlockHash() const;

			UInt256 getRootBlockHash() const;

			uint32_t getTimestamp() const;

			uint32_t getTarget() const;

			uint32_t getNonce() const;

			uint32_t getTransactionCount() const;

			uint32_t getHeight() const;

			void setHeight(uint32_t height);

			const AuxPow &getAuxPow() const;

			bool isValid(uint32_t currentTime) const;

			bool containsTransactionHash(UInt256 hash) const;

		private:

			void serializeNoAux(ByteStream &ostream) const;

			UInt256 MerkleBlockRootR(size_t *hashIdx, size_t *flagIdx, int depth) const;

		private:
			ELAMerkleBlock *_merkleBlock;
		};

		typedef boost::shared_ptr<MerkleBlock> MerkleBlockPtr;

		//support for json converting
		//read "Arbitrary types conversions" section in readme of
		//	https://github.com/nlohmann/json for more details
		void to_json(nlohmann::json &j, const MerkleBlock &p);

		void from_json(const nlohmann::json &j, MerkleBlock &p);

	}
}

#endif //__ELASTOS_SDK_MERKLEBLOCK_H__
