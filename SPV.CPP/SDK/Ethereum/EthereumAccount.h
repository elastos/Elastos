/*
 * EthereumAccount
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

#ifndef __ELASTOS_SDK_ETHEREUMACCOUNT_H__
#define __ELASTOS_SDK_ETHEREUMACCOUNT_H__

#include "Reference.h"
#include <Common/typedefs.h>
#include <ethereum/ewm/BREthereumAccount.h>

#include <boost/weak_ptr.hpp>

namespace Elastos {
	namespace ElaWallet {

		class EthereumAccount : public Reference {
		public:
			EthereumAccount(EthereumEWM *ewm, BREthereumAccount account);

			~EthereumAccount();

			std::string getPrimaryAddress() const;

			bytes_t getPrimaryAddressPublicKey() const;

			bytes_t getPrimaryAddressPrivateKey(const std::string &paperKey) const;
		};

		typedef boost::shared_ptr<EthereumAccount> EthereumAccountPtr;

	}
}

#endif //__ELASTOS_SDK_ETHEREUMACCOUNT_H__
