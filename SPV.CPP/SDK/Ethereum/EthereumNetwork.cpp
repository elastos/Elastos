/*
 * EthereumNetwork
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

#include "EthereumNetwork.h"
#include <string>

namespace Elastos {
	namespace ElaWallet {

		EthereumNetwork::EthereumNetwork(BREthereumNetwork network) :
			_network(network) {
		}

		EthereumNetwork::EthereumNetwork(const std::string &netType) {
			if (netType == "MainNet") {
				_network = ethereumMainnet;
			} else if (netType == "TestNet") {
				_network = ethereumTestnet;
			} else if (netType == "RegTest") {
				_network = ethereumRinkeby;
			} else if (netType == "PrvNet") {
				_network = ethereumTestnet;
			} else {
				_network = NULL;
			}
		}

		EthereumNetwork::~EthereumNetwork() {

		}

		BREthereumNetwork EthereumNetwork::getRaw() const {
			return _network;
		}

	}
}
