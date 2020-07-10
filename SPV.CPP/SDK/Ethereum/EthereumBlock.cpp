/*
 * EthereumBlock
 *
 * Created by Ed Gamble <ed@breadwallet.com> on 4/24/18.
 * Copyright (c) 2018 Breadwinner AG.  All right reserved.
 * Copyright (c) 2020 Elastos Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "EthereumBlock.h"

namespace Elastos {
	namespace ElaWallet {

		EthereumBlock::EthereumBlock(EthereumEWM *ewm, BREthereumBlock block) :
			Reference(ewm, block) {

		}

		EthereumBlock::~EthereumBlock() {

		}

		uint256 EthereumBlock::getHash() const {
			BREthereumHash hash = blockGetHash ((BREthereumBlock) _identifier);
			uint256 h;
			memcpy(h.begin(), hash.bytes, h.size());
			return h;
		}

		uint64_t EthereumBlock::getBlockNumber() const {
			return blockGetNumber ((BREthereumBlock) _identifier);
		}

		uint64_t EthereumBlock::getTimestamp() const {
			return blockGetTimestamp ((BREthereumBlock) _identifier);
		}

		uint256 EthereumBlock::getDifficulty() const {
			UInt256 diff = blockGetDifficulty ((BREthereumBlock) _identifier);
			uint256 d;
			memcpy(d.begin(), diff.u8, d.size());
			return d;
		}

	}
}