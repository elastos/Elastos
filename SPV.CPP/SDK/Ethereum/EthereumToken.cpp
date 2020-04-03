/*
 * EthereumToken
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

#include "EthereumToken.h"

#include <string>

namespace Elastos {
	namespace ElaWallet {

		EthereumToken::EthereumToken(BREthereumToken token) :
			_token(token) {

		}

		EthereumToken::~EthereumToken() {

		}

		std::string EthereumToken::getAddressLowerCase() const {
			std::string address = tokenGetAddress(_token);
			std::transform(address.begin(), address.end(), address.begin(),
						   [](unsigned char c) { return std::tolower(c); });
			return address;
		}

		std::string EthereumToken::getAddress() const {
			return tokenGetAddress(_token);
		}

		std::string EthereumToken::getSymbol() const {
			return tokenGetSymbol(_token);
		}

		std::string EthereumToken::getName() const {
			return tokenGetName(_token);
		}

		std::string EthereumToken::getDescription() const {
			return tokenGetDescription(_token);
		}

		int EthereumToken::getDecimals() {
			return tokenGetDecimals(_token);
		}

		int EthereumToken::hashCode() {
			std::hash<std::string> hash;
			std::string addr = tokenGetAddress(_token);
			return hash(addr);
		}

		std::string EthereumToken::toString() const {
			return std::string("EthereumToken{") + tokenGetSymbol(_token) + +"}";
		}

		BREthereumToken EthereumToken::getRaw() const {
			return _token;
		}

		std::vector<EthereumTokenPtr> EthereumToken::getTokenAll() const {
			std::vector<EthereumTokenPtr> allTokens;
			int count = tokenCount();
			// A uint32_t array on x86 platforms - we *require* a long array
			BREthereumToken *tokens = tokenGetAll();

			for (int i = 0; i < count; ++i) {
				EthereumTokenPtr token(new EthereumToken(tokens[i]));
				allTokens.push_back(token);
			}

			free(tokens);

			return allTokens;
		}

	}
}