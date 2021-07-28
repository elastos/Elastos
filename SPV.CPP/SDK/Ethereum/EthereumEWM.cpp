/*
 * EthereumEWM
 *
 * Created by Ed Gamble <ed@breadwallet.com> on 3/7/18.
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

#include "EthereumEWM.h"

#include <Common/ErrorChecker.h>
#include <ethereum/BREthereum.h>
#include <support/BRBIP39Mnemonic.h>
#include <vector>
#include <ethereum/ewm/BREthereumClient.h>
#include <Common/Log.h>

namespace Elastos {
	namespace ElaWallet {

		EthereumNetworkPtr EthereumEWM::getNetwork() const {
			return _network;
		}

		EthereumAccountPtr EthereumEWM::getAccount() const {
			return _account;
		}

		std::string EthereumEWM::getAddress() const {
			return _account->getPrimaryAddress();
		}

		bytes_t EthereumEWM::getAddressPublicKey() const {
			return _account->getPrimaryAddressPublicKey();
		}

		EthereumWalletPtr EthereumEWM::walletLookupOrCreate(BREthereumWallet wid, const EthereumTokenPtr &token) {
			WalletMap::iterator it = _wallets.find(wid);
			if (it != _wallets.end()) {
				return it->second;
			}

			EthereumTokenPtr t;
			if (nullptr == token) {
				BREthereumToken tokenRef = ewmWalletGetToken(_ewm, (BREthereumWallet) wid);
				if (NULL != tokenRef) {
					t = lookupTokenByReference(tokenRef);
				}
			}

			EthereumWalletPtr wallet;
			if (nullptr != t) {
				wallet = EthereumWalletPtr(new EthereumWallet(this, (BREthereumWallet) wid, _account, _network, t));
			} else {
				wallet = EthereumWalletPtr(new EthereumWallet(this, (BREthereumWallet) wid, _account, _network));
			}

			_wallets[wid] = wallet;

			return wallet;
		}

		std::vector<EthereumWalletPtr> EthereumEWM::getWallets() {
			std::vector<EthereumWalletPtr> wallets;

			for (WalletMap::const_iterator  it = _wallets.cbegin(); it != _wallets.cend(); ++it) {
				EthereumTokenPtr t;
				BREthereumToken tokenRef = ewmWalletGetToken(_ewm, it->first);
				if (NULL != tokenRef)
					t = lookupTokenByReference(tokenRef);

				EthereumWalletPtr wallet;
				if (nullptr != t) {
					wallet = EthereumWalletPtr(new EthereumWallet(this, it->first, _account, _network, t));
				} else {
					wallet = EthereumWalletPtr(new EthereumWallet(this, it->first, _account, _network));
				}

				wallets.push_back(wallet);
			}

			return wallets;
		}

		EthereumWalletPtr EthereumEWM::getWallet() {
			BREthereumWallet wid = ewmGetWallet(_ewm);
			return walletLookupOrCreate(wid, nullptr);
		}

		EthereumWalletPtr EthereumEWM::getWallet(const EthereumTokenPtr &token) {
			BREthereumWallet wid = ewmGetWalletHoldingToken(_ewm, token->getRaw());
			return walletLookupOrCreate(wid, token);
		}

		EthereumWalletPtr EthereumEWM::getWalletByIdentifier(BREthereumWallet wid) {
			return walletLookupOrCreate(wid, nullptr);
		}

		EthereumTransferPtr EthereumEWM::transactionLookupOrCreate(BREthereumTransfer tid) {
			TransferMap::iterator it = _transactions.find(tid);

			if (it != _transactions.end()) {
				return it->second;
			}

			BREthereumToken token = ewmTransferGetToken(_ewm, tid);
			EthereumTransferPtr transfer(new EthereumTransfer(this, tid,
															  (NULL == token ? EthereumAmount::Unit::ETHER_ETHER
																			 : EthereumAmount::Unit::TOKEN_DECIMAL)));
			_transactions[tid] = transfer;
			return transfer;
		}

		void EthereumEWM::transferDelete(const EthereumTransferPtr &transfer) {
		}

		uint64_t EthereumEWM::getBlockHeight() const {
			return ewmGetBlockHeight(_ewm);
		}

		std::vector<EthereumTokenPtr> EthereumEWM::getTokens() const {
			std::vector<EthereumTokenPtr> tokens;

			for (TokenAddressMap::const_iterator it = _tokensByAddress.cbegin(); it != _tokensByAddress.cend(); ++it) {
				tokens.push_back(it->second);
			}

			return tokens;
		}

		EthereumTokenPtr EthereumEWM::lookupTokenByReference(BREthereumToken reference) const {
			if (_tokensByReference.find(reference) == _tokensByReference.end())
				return nullptr;

			return _tokensByReference.at(reference);
		}

		EthereumTokenPtr EthereumEWM::addTokenByReference(BREthereumToken reference) {
			EthereumTokenPtr token(new EthereumToken(reference));
			_tokensByReference[reference] = token;
			std::string address = token->getAddressLowerCase();
			_tokensByAddress[address] = token;
			return token;
		}

		EthereumTokenPtr EthereumEWM::lookupToken(const std::string &address) const {
			std::string addr = address;
			std::transform(addr.begin(), addr.end(), addr.begin(),
						   [](unsigned char c) { return std::tolower(c); });
			if (_tokensByAddress.find(addr) == _tokensByAddress.end())
				return nullptr;

			return _tokensByAddress.at(addr);
		}

		EthereumEWM::EthereumEWM(BRCryptoSyncMode mode, const EthereumNetworkPtr &network,
								 const std::string &storagePath, const std::string &paperKey,
								 const std::vector<std::string> &wordList,
								 uint64_t blockHeight,
								 uint64_t confirmationsUntilFinal) :
			EthereumEWM(createRawEWM(mode, network->getRaw(), storagePath, paperKey, wordList, blockHeight,
									 confirmationsUntilFinal), network) {
		}

		EthereumEWM::EthereumEWM(BRCryptoSyncMode mode, const EthereumNetworkPtr &network,
								 const std::string &storagePath, const bytes_t &publicKey,
								 uint64_t blockHeight,
								 uint64_t confirmationsUntilFinal) :
			EthereumEWM(createRawEWMPublicKey(mode, network->getRaw(), storagePath, publicKey, blockHeight,
											  confirmationsUntilFinal), network) {
		}

        EthereumEWM::~EthereumEWM() {
		}

		EthereumEWM::EthereumEWM(BREthereumEWM ewm, const EthereumNetworkPtr &network) :
			_ewm(ewm),
			_network(network),
			_account(EthereumAccountPtr(new EthereumAccount(this, ewmGetAccount(ewm)))) {
		    Log::info("EthereumEWM, network p = {:x}", (size_t)_network->getRaw());
		}

		BREthereumEWM
		EthereumEWM::createRawEWM(BRCryptoSyncMode mode, BREthereumNetwork network, const std::string &storagePath,
								  BREthereumAccount account,
								  uint64_t blockHeight,
								  uint64_t confirmationsUntilFinal) {

			return ewmCreate(network, account, ETHEREUM_TIMESTAMP_UNKNOWN, mode, storagePath.data(),
							 blockHeight, confirmationsUntilFinal);
		}

		BREthereumEWM EthereumEWM::createRawEWM(BRCryptoSyncMode mode, BREthereumNetwork network,
												const std::string &storagePath, const std::string &paperKey,
												const std::vector<std::string> &wordList,
												uint64_t blockHeight,
												uint64_t confirmationsUntilFinal) {
			int wordsCount = wordList.size();
			assert (BIP39_WORDLIST_COUNT == wordsCount);
			static const char *wordListPtr[BIP39_WORDLIST_COUNT];

			for (int i = 0; i < wordsCount; i++)
				wordListPtr[i] = wordList[i].c_str();

			installSharedWordList((const char **) wordListPtr, BIP39_WORDLIST_COUNT);

			return ewmCreateWithPaperKey((BREthereumNetwork) network, paperKey.data(), ETHEREUM_TIMESTAMP_UNKNOWN,
										 mode, storagePath.data(), blockHeight, confirmationsUntilFinal);
		}

		BREthereumEWM EthereumEWM::createRawEWMPublicKey(BRCryptoSyncMode mode, BREthereumNetwork network,
														 const std::string &storagePath, const bytes_t &pubkey,
														 uint64_t blockHeight,
														 uint64_t confirmationsUntilFinal) {
			assert (65 == pubkey.size());

			BRKey key;

			memset(&key, 0, sizeof(key));
			memcpy(key.pubKey, pubkey.data(), 65);

			// TODO: set to correct time to replace ETHEREUM_TIMESTAMP_UNKNOWN
			return ewmCreateWithPublicKey((BREthereumNetwork) network, key, time(NULL),
										  mode, storagePath.data(), blockHeight, confirmationsUntilFinal);
		}

		bool EthereumEWM::addressIsValid(const std::string &address) {
			return address.empty() || ETHEREUM_BOOLEAN_IS_TRUE(addressValidateString(address.data()));
		}

		void EthereumEWM::ensureValidAddress(const std::string &address) {
			ErrorChecker::CheckCondition(!addressIsValid(address), Error::Code::InvalidEthereumAddress,
										 "Invalid Ethereum Address");
		}

		BREthereumEWM EthereumEWM::getRaw() const {
			return _ewm;
		}

	}
}