// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <Core/BRBloomFilter.h>
#include "BRBloomFilter.h"
#include "BRInt.h"
#include "BRAddress.h"

#include "BloomFilter.h"

namespace Elastos {
	namespace SDK {

		BloomFilter::BloomFilter(double falsePositiveRate, size_t elemCount, uint32_t tweak, uint8_t flags) {
			_bloomFilter = BRBloomFilterNew(falsePositiveRate, elemCount, tweak, flags);
		}

		BloomFilter::BloomFilter(BRBloomFilter *filter) :
			_bloomFilter(filter) {
		}

		BloomFilter::~BloomFilter() {
			if (_bloomFilter != nullptr) {
				BRBloomFilterFree(_bloomFilter);
			}
		}

		std::string BloomFilter::toString() const {
			//todo complete me
			return "";
		}

		BRBloomFilter *BloomFilter::getRaw() const {
			return _bloomFilter;
		}

		void BloomFilter::Serialize(ByteStream &ostream) const {
			size_t len = BRVarIntSize(_bloomFilter->length);
			uint8_t lengthData[len];
			BRVarIntSet(lengthData, len, _bloomFilter->length);
			ostream.putBytes(lengthData, len);

			ostream.putBytes(_bloomFilter->filter, _bloomFilter->length);

			uint8_t hashFunctionsData[32 / 8];
			UInt32SetLE(hashFunctionsData, _bloomFilter->hashFuncs);
			ostream.putBytes(hashFunctionsData, 32 / 8);

			uint8_t tweakData[32 / 8];
			UInt32SetLE(tweakData, _bloomFilter->tweak);
			ostream.putBytes(tweakData, 32 / 8);
		}

		//todo add max size check of BLOOM_MAX_FILTER_LENGTH
		void BloomFilter::Deserialize(ByteStream &istream) {
			uint8_t lengthData[64 / 8];
			istream.getBytes(lengthData, 64 / 8);
			_bloomFilter->length = UInt64GetLE(lengthData);

			_bloomFilter->filter = (uint8_t *)malloc(_bloomFilter->length);
			istream.getBytes(_bloomFilter->filter, _bloomFilter->length);

			uint8_t hashFunctionsData[32 / 8];
			istream.getBytes(hashFunctionsData, 32 / 8);
			_bloomFilter->hashFuncs = UInt32GetLE(hashFunctionsData);

			uint8_t tweakData[32 / 8];
			istream.getBytes(tweakData, 32 / 8);
			_bloomFilter->tweak = UInt32GetLE(tweakData);
		}

		nlohmann::json BloomFilter::toJson() {
			nlohmann::json jsonData;
			jsonData["length"] = _bloomFilter->length;

			std::vector<uint8_t> filters(_bloomFilter->length);
			for (int i = 0; i < _bloomFilter->length; ++i) {
				filters[i] = _bloomFilter->filter[i];
			}
			jsonData["filter"] = filters;

			jsonData["hashFuncs"] = _bloomFilter->hashFuncs;
			jsonData["tweak"] = _bloomFilter->tweak;

			return jsonData;
		}

		void BloomFilter::fromJson(nlohmann::json jsonData) {
			_bloomFilter->length = jsonData["length"].get<uint32_t>();

			_bloomFilter->filter = (uint8_t *)malloc(_bloomFilter->length);
			std::vector<uint8_t> filters = jsonData["filter"];
			assert(_bloomFilter->length == filters.size());
			for (int i = 0; i < _bloomFilter->length; ++i) {
				_bloomFilter->filter[i] = filters[i];
			}

			_bloomFilter->hashFuncs = jsonData["hashFuncs"].get<uint32_t>();
			_bloomFilter->tweak = jsonData["tweak"].get<uint32_t>();
		}
	}
}