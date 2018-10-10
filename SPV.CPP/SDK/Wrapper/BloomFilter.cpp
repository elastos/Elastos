// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <Core/BRBloomFilter.h>
#include <SDK/Common/Log.h>
#include "BRBloomFilter.h"
#include "BRInt.h"
#include "BRAddress.h"

#include "BloomFilter.h"

namespace Elastos {
	namespace ElaWallet {

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
			ostream.writeVarBytes(_bloomFilter->filter, _bloomFilter->length);
			ostream.writeUint32(_bloomFilter->hashFuncs);
			ostream.writeUint32(_bloomFilter->tweak);
		}

		//todo add max size check of BLOOM_MAX_FILTER_LENGTH
		bool BloomFilter::Deserialize(ByteStream &istream) {
			CMBlock filter;

			if (!istream.readVarBytes(filter)) {
				Log::error("Bloom filter deserialize filter fail");
				return false;
			}

			_bloomFilter->length = filter.GetSize();
			_bloomFilter->filter = (uint8_t *)malloc(_bloomFilter->length);
			memcpy(_bloomFilter->filter, filter, _bloomFilter->length);

			if (!istream.readUint32(_bloomFilter->hashFuncs)) {
				Log::error("Bloom filter deserialize hash funcs fail");
				free(_bloomFilter->filter);
				_bloomFilter->filter = NULL;
				_bloomFilter->length = 0;
				return false;
			}

			if (!istream.readUint32(_bloomFilter->tweak)) {
				Log::error("Bloom filter deserialize tweak fail");
				free(_bloomFilter->filter);
				_bloomFilter->filter = NULL;
				_bloomFilter->length = 0;
				return false;
			}

			return true;
		}

		nlohmann::json BloomFilter::toJson() const {
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

		void BloomFilter::fromJson(const nlohmann::json &jsonData) {
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