// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_BLOOMFILTER_H__
#define __ELASTOS_SDK_BLOOMFILTER_H__

#include <Common/ByteStream.h>
#include <nlohmann/json.hpp>
#include <boost/shared_ptr.hpp>

#define BLOOM_DEFAULT_FALSEPOSITIVE_RATE 0.0005 // use 0.00005 for less data, 0.001 for good anonymity
#define BLOOM_REDUCED_FALSEPOSITIVE_RATE 0.00005
#define BLOOM_UPDATE_NONE                0
#define BLOOM_UPDATE_ALL                 1
#define BLOOM_UPDATE_P2PUBKEY_ONLY       2
#define BLOOM_MAX_FILTER_LENGTH          36000 // this allows for 10,000 elements with a <0.0001% false positive rate

namespace Elastos {
	namespace ElaWallet {

		class BloomFilter {
		public:
			BloomFilter(double falsePositiveRate, size_t elemCount, uint32_t tweak, uint8_t flags);

			~BloomFilter();

			virtual void Serialize(ByteStream &ostream) const;

			virtual bool Deserialize(const ByteStream &istream);

			virtual nlohmann::json ToJson() const;

			virtual void FromJson(const nlohmann::json &jsonData);

			void InsertData(const bytes_t &data);

			bool ContainsData(const bytes_t &data);

		private:
			inline uint32_t ROTL32(uint32_t x, int8_t r) {
				return (x << r) | (x >> (32 - r));
			}

			uint32_t MurmurHash3(uint32_t seed, const uchar_vector& data);

			uint32_t CalculateHash(const bytes_t &data, uint32_t hashNum);

		private:
			bytes_t _filter;
			uint32_t _hashFuncs;
			size_t _elemCount;
			uint32_t _tweak;
			uint8_t _flags;
		};

		typedef boost::shared_ptr<BloomFilter> BloomFilterPtr;

	}
}

#endif //__ELASTOS_SDK_BLOOMFILTER_H__
