// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <BRPeerMessages.h>
#include <BRBloomFilter.h>
#include "BRBloomFilter.h"

#include "BloomFilter.h"
#include "BloomFilterMessage.h"
#include "ByteStream.h"

namespace Elastos {
	namespace SDK {

		namespace {

			BRBloomFilter *BRBloomFilterCopy(BRBloomFilter *filter) {
				BRBloomFilter *result = (BRBloomFilter *)calloc(1, sizeof(*filter));
				result->length = filter->length;
				result->tweak = filter->tweak;
				result->hashFuncs = filter->hashFuncs;
				result->flags = filter->flags;
				result->elemCount = filter->elemCount;
				result->filter = (uint8_t *)malloc(filter->length);
				memcpy(result->filter, filter->filter, filter->length);
				return result;
			}
		}

		int BloomFilterMessage::Accept(BRPeer *peer, const uint8_t *msg, size_t msgLen) {
			return 0;
		}

		void BloomFilterMessage::Send(BRPeer *peer, void *serializable) {

			BRBloomFilter *filter = static_cast<BRBloomFilter *>(serializable);
			BloomFilter wrappedFilter = BloomFilter(BRBloomFilterCopy(filter));
			ByteStream byteStream;
			wrappedFilter.Serialize(byteStream);
			((BRPeerContext *)peer)->sentFilter = 1;
			((BRPeerContext *)peer)->sentMempool = 0;
			BRPeerSendMessage(peer, byteStream.getBuf(), byteStream.length(), MSG_FILTERLOAD);
		}
	}
}