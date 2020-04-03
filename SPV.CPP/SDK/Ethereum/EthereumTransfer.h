/*
 * EthereumTransaction
 *
 * Created by Ed Gamble <ed@breadwallet.com> on 3/20/18.
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

#ifndef __ELASTOS_SPVSDK_ETHEREUMTRANSFER_H__
#define __ELASTOS_SPVSDK_ETHEREUMTRANSFER_H__

#include "ReferenceWithDefaultUnit.h"

#include <ethereum/ewm/BREthereumTransfer.h>
#include <boost/shared_ptr.hpp>

namespace Elastos {
	namespace ElaWallet {

		class EthereumTransfer : public ReferenceWithDefaultUnit {
		public:
			EthereumTransfer(EthereumEWM *ewm, BREthereumTransfer transfer, EthereumAmount::Unit unit);

			~EthereumTransfer();

		public:
			BREthereumTransfer getRaw() const;

		public:
			bool isConfirmed() const;

			bool isSubmitted() const;

			bool isErrored() const;

			std::string getSourceAddress() const;

			std::string getTargetAddress() const;

			std::string getIdentifier() const;

			std::string getOriginationTransactionHash() const;

			// Amount
			std::string getAmount() const;

			std::string getAmount(EthereumAmount::Unit unit) const;

			std::string getFee() const;

			std::string getFee(EthereumAmount::Unit unit) const;

			std::string getGasPrice() const;

			std::string getGasPrice(EthereumAmount::Unit unit) const;

			uint64_t getGasLimit() const;

			uint64_t getGasUsed() const;

			// Nonce
			uint64_t getNonce() const;

			// Block Number, Timestamp
			uint64_t getBlockNumber() const;

			uint64_t getBlockTimestamp() const;

			uint64_t getBlockConfirmations() const;

			std::string getErrorDescription() const;

		};

		typedef boost::shared_ptr<EthereumTransfer> EthereumTransferPtr;
		typedef boost::weak_ptr<EthereumTransfer> EthereumTransferWeakPtr;

	}
}

#endif
