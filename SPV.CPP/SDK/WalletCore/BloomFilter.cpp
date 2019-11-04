// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "BloomFilter.h"

#include <Common/Log.h>

#include <cfloat>

#define BLOOM_MAX_HASH_FUNCS 50

namespace Elastos {
	namespace ElaWallet {

		BloomFilter::BloomFilter(double falsePositiveRate, size_t elemCount, uint32_t tweak, uint8_t flags) :
				_flags(flags),
				_tweak(tweak) {

			size_t length = (falsePositiveRate < DBL_EPSILON) ? BLOOM_MAX_FILTER_LENGTH :
							(-1.0 / (M_LN2 * M_LN2)) * elemCount * log(falsePositiveRate) / 8.0;
			if (length > BLOOM_MAX_FILTER_LENGTH) length = BLOOM_MAX_FILTER_LENGTH;
			if (length < 1) length = 1;

			_filter = bytes_t(length, 0);
			_hashFuncs = uint32_t(((length * 8.0) / elemCount) * M_LN2);
			if (_hashFuncs > BLOOM_MAX_HASH_FUNCS) _hashFuncs = BLOOM_MAX_HASH_FUNCS;
		}

		BloomFilter::~BloomFilter() {
		}

		void BloomFilter::Serialize(ByteStream &ostream) const {
			ostream.WriteVarBytes(_filter);
			ostream.WriteUint32(_hashFuncs);
			ostream.WriteUint32(_tweak);
			//ostream.WriteByte(_flags);
		}

		//todo add max size check of BLOOM_MAX_FILTER_LENGTH
		bool BloomFilter::Deserialize(const ByteStream &istream) {
			if (!istream.ReadVarBytes(_filter)) {
				Log::error("Bloom filter deserialize filter fail");
				return false;
			}

			if (!istream.ReadUint32(_hashFuncs)) {
				Log::error("Bloom filter deserialize hash funcs fail");
				return false;
			}

			if (!istream.ReadUint32(_tweak)) {
				Log::error("Bloom filter deserialize tweak fail");
				return false;
			}

			if (!istream.ReadByte(_flags)) {
				Log::error("Bloom filter deserialize flags fail");
				return false;
			}

			return true;
		}

		nlohmann::json BloomFilter::ToJson() const {
			nlohmann::json jsonData;
			jsonData["filter"] = _filter.getBase64();
			jsonData["hashFuncs"] = _hashFuncs;
			jsonData["tweak"] = _tweak;

			return jsonData;
		}

		void BloomFilter::FromJson(const nlohmann::json &jsonData) {
			_filter.setBase64(jsonData["filter"].get<std::string>());
			_hashFuncs = jsonData["hashFuncs"].get<uint32_t>();
			_tweak = jsonData["tweak"].get<uint32_t>();
		}

		void BloomFilter::InsertData(const bytes_t &data) {
			size_t i, idx;

			for (i = 0; i < _hashFuncs; i++) {
				idx = CalculateHash(data, i);
				_filter[idx >> 3] |= (1 << (7 & idx));
			}

			if (!data.empty()) _elemCount++;
		}

		uint32_t BloomFilter::MurmurHash3(uint32_t seed, const uchar_vector& data) {
			// The following is MurmurHash3 (x86_32), see http://code.google.com/p/smhasher/source/browse/trunk/MurmurHash3.cpp
			uint32_t h1 = seed;
			const uint32_t c1 = 0xcc9e2d51;
			const uint32_t c2 = 0x1b873593;

			const int nblocks = data.size() / 4;

			//----------
			// body
			const uint32_t * blocks = (const uint32_t *)(&data[0] + nblocks*4);

			for(int i = -nblocks; i; i++)
			{
				uint32_t k1 = blocks[i];

				k1 *= c1;
				k1 = ROTL32(k1,15);
				k1 *= c2;

				h1 ^= k1;
				h1 = ROTL32(h1,13);
				h1 = h1*5+0xe6546b64;
			}

			//----------
			// tail
			const uint8_t * tail = (const uint8_t*)(&data[0] + nblocks*4);

			uint32_t k1 = 0;

			switch(data.size() & 3)
			{
				case 3: k1 ^= tail[2] << 16;
				case 2: k1 ^= tail[1] << 8;
				case 1: k1 ^= tail[0];
					k1 *= c1; k1 = ROTL32(k1,15); k1 *= c2; h1 ^= k1;
			};

			//----------
			// finalization
			h1 ^= data.size();
			h1 ^= h1 >> 16;
			h1 *= 0x85ebca6b;
			h1 ^= h1 >> 13;
			h1 *= 0xc2b2ae35;
			h1 ^= h1 >> 16;

			return h1;
		}

		uint32_t BloomFilter::CalculateHash(const bytes_t &data, uint32_t hashNum) {
			return (uint32_t)(MurmurHash3((uint32_t)(hashNum * 0xfba4c795 + _tweak), data) % (_filter.size() * 8));
		}

		bool BloomFilter::ContainsData(const bytes_t &data) {
			size_t i, idx;

			for (i = 0; i < _hashFuncs; i++) {
				idx = CalculateHash(data, i);
				if (!(_filter[idx >> 3] & (1 << (7 & idx)))) return false;
			}

			return !data.empty();
		}
	}
}
