/*
 * Copyright (c) 2019 Elastos Foundation
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
#ifndef __ELASTOS_SDK_IDCHAINSUBWALLET_H__
#define __ELASTOS_SDK_IDCHAINSUBWALLET_H__

#include "SidechainSubWallet.h"
#include "IIDChainSubWallet.h"

namespace Elastos {
	namespace ElaWallet {
		class CredentialSubject;
		class VerifiableCredential;
		class DIDInfo;

		class IDChainSubWallet : public SidechainSubWallet, public IIDChainSubWallet {
		public:
			virtual ~IDChainSubWallet();

			virtual nlohmann::json CreateIDTransaction(
					const nlohmann::json &payloadJson,
					const std::string &memo = "");

			virtual nlohmann::json GetAllDID(uint32_t start, uint32_t count) const;

			virtual nlohmann::json GetAllCID(uint32_t start, uint32_t count) const;

			virtual std::string Sign(const std::string &DIDOrCID, const std::string &message, const std::string &payPassword) const;

			virtual std::string SignDigest(const std::string &DIDOrCID, const std::string &digest,
			                               const std::string &payPassword) const;

			virtual bool VerifySignature(const std::string &publicKey, const std::string &message, const std::string &signature);

			virtual std::string GetPublicKeyDID(const std::string &pubkey) const;

			virtual std::string GetPublicKeyCID(const std::string &pubkey) const;

		protected:
			friend class MasterWallet;

			IDChainSubWallet(const CoinInfoPtr &info,
							 const ChainConfigPtr &config,
							 MasterWallet *parent,
							 const std::string &netType);
		};

	}
}

#endif //__ELASTOS_SDK_IDCHAINSUBWALLET_H__
