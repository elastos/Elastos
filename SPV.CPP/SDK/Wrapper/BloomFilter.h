// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_BLOOMFILTER_H__
#define __ELASTOS_SDK_BLOOMFILTER_H__

#include <cmake-build-debug/ThirdParty/boost/install/include/boost/exception/detail/shared_ptr.hpp>
#include "BRBloomFilter.h"

#include "Wrapper.h"
#include "SDK/Plugin/Interface/ELAMessageSerializable.h"

namespace Elastos {
	namespace ElaWallet {

		//fixme [refactor] remove BRBloomFilter from BloomFilter class
		class BloomFilter :
			Wrapper<BRBloomFilter>,
			ELAMessageSerializable {
		public:
			BloomFilter(double falsePositiveRate, size_t elemCount, uint32_t tweak, uint8_t flags);

			BloomFilter(BRBloomFilter *filter);

			~BloomFilter();

			virtual std::string toString() const;

			virtual BRBloomFilter *getRaw() const;

			virtual void Serialize(ByteStream &ostream) const;

			virtual bool Deserialize(ByteStream &istream);

			virtual nlohmann::json toJson() const;

			virtual void fromJson(const nlohmann::json &jsonData);

		private:
			BRBloomFilter *_bloomFilter;
		};

		typedef boost::shared_ptr<BloomFilter> BloomFilterPtr;

	}
}

#endif //__ELASTOS_SDK_BLOOMFILTER_H__
