// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "SubAccountGenerator.h"
#include "HDSubAccount.h"
#include "SingleSubAccount.h"
#include "MultiSignAccount.h"
#include "MultiSignSubAccount.h"
#include "StandardSingleSubAccount.h"

#include <SDK/Common/Utils.h>
#include <SDK/Common/Log.h>
#include <SDK/Common/ErrorChecker.h>

#include <Core/BRCrypto.h>

namespace Elastos {
	namespace ElaWallet {

		SubAccountGenerator::SubAccountGenerator() {

		}

		SubAccountGenerator::~SubAccountGenerator() {
			Clean();
		}

		ISubAccount *SubAccountGenerator::Generate() {
			MultiSignAccount *multiSignAccount = dynamic_cast<MultiSignAccount *>(_parentAccount);

			if (multiSignAccount != nullptr) {
				return new MultiSignSubAccount(_parentAccount);
			} else {
				if (_coinInfo.GetSingleAddress()) {
					if (!_masterPubKey.valid())
						return new SingleSubAccount(_parentAccount);
					else
						return new StandardSingleSubAccount(_masterPubKey, _votePubKey, _parentAccount,
															_coinInfo.GetIndex());
				} else {
					if (!_masterPubKey.valid())
						return GenerateFromCoinInfo(_parentAccount, _coinInfo);
					else
						return GenerateFromHDPath(_parentAccount, _coinInfo.GetIndex());
				}
			}
		}

		void SubAccountGenerator::SetCoinInfo(const CoinInfo &coinInfo) {
			_coinInfo = coinInfo;
		}

		void SubAccountGenerator::SetParentAccount(IAccount *account) {
			_parentAccount = account;
		}

		void SubAccountGenerator::Clean() {
		}

		const bytes_t &SubAccountGenerator::GetResultPublicKey() const {
			return _resultPubKey;
		}

		const bytes_t &SubAccountGenerator::GetResultChainCode() const {
			return _resultChainCode;
		}

		ISubAccount *SubAccountGenerator::GenerateFromCoinInfo(IAccount *account, const CoinInfo &coinInfo) {
			ErrorChecker::CheckParamNotEmpty(coinInfo.GetPublicKey(), "Sub account public key");
			ErrorChecker::CheckParamNotEmpty(coinInfo.GetChainCode(), "Sub account chain code");

			_resultChainCode.setHex(coinInfo.GetChainCode());
			_resultPubKey.setHex(coinInfo.GetPublicKey());
			HDKeychain masterPubKey = HDKeychain(_resultPubKey, _resultChainCode);

			return new HDSubAccount(masterPubKey, _votePubKey, account, _coinInfo.GetIndex());
		}

		ISubAccount *
		SubAccountGenerator::GenerateFromHDPath(IAccount *account, uint32_t coinIndex) {
			return new HDSubAccount(_masterPubKey, _votePubKey, account, _coinInfo.GetIndex());
		}

		void SubAccountGenerator::SetMasterPubKey(const HDKeychain &masterPubKey) {
			_masterPubKey = masterPubKey;
			if (_masterPubKey.valid()) {
				_resultPubKey = _masterPubKey.pubkey();
				_resultChainCode = _masterPubKey.chain_code();
			}
		}

		void SubAccountGenerator::SetVotePubKey(const bytes_t &pubKey) {
			_votePubKey = pubKey;
		}

		HDKeychain SubAccountGenerator::GenerateMasterPubKey(IAccount *account, uint32_t coinIndex,
																  const std::string &payPassword) {
			if (account->GetType() == "MultiSign") {
				MultiSignAccount *multiSignAccount = static_cast<MultiSignAccount *>(account);
				if (multiSignAccount->GetInnerAccount() == nullptr)
					return HDKeychain();
				if ("Simple" == multiSignAccount->GetInnerAccount()->GetType()) {
					Key key = account->DeriveMultiSignKey(payPassword);
					return HDKeychain(key.PubKey(), bytes_t());
				}
			}

			HDSeed hdseed(account->DeriveSeed(payPassword).bytes());
			HDKeychain rootKey(hdseed.getExtendedKey(true));
			return rootKey.getChild("44'/0'/0'").getPublic();
		}

		bytes_t SubAccountGenerator::GenerateVotePubKey(IAccount *account, uint32_t coinIndex, const std::string &payPasswd) {
			bytes_t votePubKey;

			if (account->GetType() == "Standard") {
				HDSeed hdseed(account->DeriveSeed(payPasswd).bytes());
				HDKeychain rootKey(hdseed.getExtendedKey(true));
				// account is 1
				votePubKey = rootKey.getChild("44'/0'/1'/0/0").pubkey();
			}

			return votePubKey;
		}

	}
}
