// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_BLOOMFILTER_H__
#define __ELASTOS_SDK_BLOOMFILTER_H__

#include <boost/shared_ptr.hpp>

#include "CMemBlock.h"
#include "Wrapper.h"
#include "SDK/Plugin/Interface/ELAMessageSerializable.h"

namespace Elastos {
	namespace ElaWallet {

		class BloomFilter :
				ELAMessageSerializable {
		public:
			BloomFilter(double falsePositiveRate, size_t elemCount, uint32_t tweak, uint8_t flags);

			~BloomFilter();

			virtual void Serialize(ByteStream &ostream) const;

			virtual bool Deserialize(ByteStream &istream);

			virtual nlohmann::json toJson() const;

			virtual void fromJson(const nlohmann::json &jsonData);

			void insertData(const CMBlock &data);

			bool ContainsData(const CMBlock &data);

			uint32_t calculateHash(const CMBlock &data, uint32_t hashNum);

		private:
			CMBlock _filter;
			uint32_t _hashFuncs;
			size_t _elemCount;
			uint32_t _tweak;
			uint8_t _flags;
		};

		typedef boost::shared_ptr<BloomFilter> BloomFilterPtr;

	}
}

#endif //__ELASTOS_SDK_BLOOMFILTER_H__
