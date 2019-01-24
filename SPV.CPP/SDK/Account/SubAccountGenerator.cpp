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
#include <SDK/Common/ParamChecker.h>
#include <SDK/BIPs/BIP32Sequence.h>

#include <Core/BRCrypto.h>

namespace Elastos {
	namespace ElaWallet {

		SubAccountGenerator::SubAccountGenerator() :
				_masterPubKey(nullptr),
				_resultChainCode(UINT256_ZERO) {

		}

		SubAccountGenerator::~SubAccountGenerator() {
			Clean();
		}

		ISubAccount *SubAccountGenerator::Generate() {
			MultiSignAccount *multiSignAccount = dynamic_cast<MultiSignAccount *>(_parentAccount);

			if (multiSignAccount != nullptr) {
				return new MultiSignSubAccount(_parentAccount);
			} else {
				if (_coinInfo.getSingleAddress()) {
					if (_masterPubKey == nullptr)
						return new SingleSubAccount(_parentAccount);
					else
						return new StandardSingleSubAccount(*_masterPubKey, _votePubKey, _parentAccount, _coinInfo.getIndex());
				} else {
					if (_masterPubKey == nullptr)
						return GenerateFromCoinInfo(_parentAccount, _coinInfo);
					else
						return GenerateFromHDPath(_parentAccount, _coinInfo.getIndex());
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

		const CMBlock &SubAccountGenerator::GetResultPublicKey() const {
			return _resultPubKey;
		}

		const UInt256 &SubAccountGenerator::GetResultChainCode() const {
			return _resultChainCode;
		}

		ISubAccount *SubAccountGenerator::GenerateFromCoinInfo(IAccount *account, const CoinInfo &coinInfo) {
			ParamChecker::checkArgumentNotEmpty(coinInfo.getPublicKey(), "Sub account public key");
			ParamChecker::checkArgumentNotEmpty(coinInfo.getChainCode(), "Sub account chain code");

			CMBlock pubKey = Utils::decodeHex(coinInfo.getPublicKey());
			MasterPubKey masterPubKey = MasterPubKey(pubKey, Utils::UInt256FromString(coinInfo.getChainCode()));

			_resultChainCode = Utils::UInt256FromString(coinInfo.getChainCode());
			_resultPubKey = Utils::decodeHex(coinInfo.getPublicKey());

			return new HDSubAccount(masterPubKey, _votePubKey, account, _coinInfo.getIndex());
		}

		ISubAccount *
		SubAccountGenerator::GenerateFromHDPath(IAccount *account, uint32_t coinIndex) {
			return new HDSubAccount(*_masterPubKey, _votePubKey, account, _coinInfo.getIndex());
		}

		void SubAccountGenerator::SetMasterPubKey(const MasterPubKeyPtr &masterPubKey) {
			_masterPubKey = masterPubKey;
			if (_masterPubKey != nullptr) {
				_resultPubKey = _masterPubKey->GetPubKey();
				_resultChainCode = _masterPubKey->GetChainCode();
			}
		}

		void SubAccountGenerator::SetVotePubKey(const CMBlock &pubKey) {
			_votePubKey = pubKey;
		}

		MasterPubKeyPtr SubAccountGenerator::GenerateMasterPubKey(IAccount *account, uint32_t coinIndex,
																  const std::string &payPassword) {
			UInt256 chainCode;
			if (account->GetType() == "MultiSign") {
				MultiSignAccount *multiSignAccount = static_cast<MultiSignAccount *>(account);
				if ("Simple" == multiSignAccount->GetInnerAccount()->GetType()) {
					Key key = account->DeriveKey(payPassword);
					return MasterPubKeyPtr(new MasterPubKey(key.PubKey(), chainCode));
				}
			}

			UInt512 seed = account->DeriveSeed(payPassword);
			Key key = BIP32Sequence::PrivKeyPath(&seed, sizeof(seed), chainCode, 3, 44 | BIP32_HARD,
												 coinIndex | BIP32_HARD, 0 | BIP32_HARD);

			var_clean(&seed);

			return MasterPubKeyPtr(new MasterPubKey(key.PubKey(), chainCode));
		}

		CMBlock SubAccountGenerator::GenerateVotePubKey(IAccount *account, uint32_t coinIndex, const std::string &payPasswd) {
			std::string votePubKey;
			Key key;

			if (account->GetType() == "Standard") {
				UInt512 seed = account->DeriveSeed(payPasswd);
				UInt256 chainCode;
				key = BIP32Sequence::PrivKeyPath(&seed, sizeof(seed), chainCode, 5,
												 44 | BIP32_HARD, coinIndex | BIP32_HARD,
												 BIP32::Account::Vote | BIP32_HARD, BIP32::External, 0);

				var_clean(&seed);
				var_clean(&chainCode);
			}

			return key.PubKey();
		}

	}
}
