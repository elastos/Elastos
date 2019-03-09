// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "SubAccountBase.h"

#include <SDK/Common/Utils.h>
#include <SDK/Common/ErrorChecker.h>
#include <SDK/Common/Log.h>

namespace Elastos {
	namespace ElaWallet {

		SubAccountBase::SubAccountBase(IAccount *account) :
			_coinIndex(0),
			_parentAccount(account) {

		}

		IAccount *SubAccountBase::GetParent() const {
			return _parentAccount;
		}

		void SubAccountBase::InitAccount(const std::vector<TransactionPtr> &transactions, Lockable *lock) {

		}

		void SubAccountBase::AddUsedAddrs(const TransactionPtr &tx) {

		}

		void SubAccountBase::ClearUsedAddresses() {

		}

		CMBlock SubAccountBase::GetMultiSignPublicKey() const {
			return _parentAccount->GetMultiSignPublicKey();
		}

		Key SubAccountBase::DeriveMultiSignKey(const std::string &payPassword) {
			return _parentAccount->DeriveMultiSignKey(payPassword);
		}

		CMBlock SubAccountBase::GetVotePublicKey() const {
			return _votePublicKey;
		}

		bool SubAccountBase::IsDepositAddress(const Address &address) const {
			if (GetVotePublicKey().GetSize() == 0) {
				return false;
			}

			if (!_depositAddress.Valid()) {
				return false;
			}

			return _depositAddress == address;
		}

		void SubAccountBase::SignTransaction(const TransactionPtr &tx, const std::string &payPasswd) {
			std::string addr;
			Key key;
			ByteStream stream;

			ErrorChecker::CheckParam(tx->IsSigned(), Error::AlreadySigned, "Transaction signed");
			ErrorChecker::CheckParam(tx->GetPrograms().empty(), Error::InvalidTransaction,
									 "Invalid transaction program");

			UInt256 md = tx->GetShaData();

			std::vector<Program> &programs = tx->GetPrograms();
			for (size_t i = 0; i < programs.size(); ++i) {
				std::vector<CMBlock> publicKeys = programs[i].DecodePublicKey();
				ErrorChecker::CheckLogic(publicKeys.empty(), Error::InvalidRedeemScript, "Invalid redeem script");

				bool found = false;
				for (size_t k = 0; k < publicKeys.size(); ++k) {
					found = FindKey(key, publicKeys[k], payPasswd);
					if (found)
						break;
				}
				ErrorChecker::CheckLogic(!found, Error::PrivateKeyNotFound, "Private key not found");

				stream.SetPosition(0);
				if (programs[i].GetParameter().GetSize() > 0) {
					ByteStream verifyStream(programs[i].GetParameter());
					CMBlock signature;
					while (verifyStream.ReadVarBytes(signature)) {
						ErrorChecker::CheckLogic(key.Verify(md, signature), Error::AlreadySigned, "Already signed");
					}
					stream.WriteBytes(programs[i].GetParameter());
				}

				CMBlock signedData = key.Sign(md);
				stream.WriteVarBytes(signedData);
				programs[i].SetParameter(stream.GetBuffer());
			}
		}

		bool SubAccountBase::FindKey(Key &key, const CMBlock &pubKey, const std::string &payPasswd) {
			if (GetVotePublicKey() == pubKey) {
				key = DeriveVoteKey(payPasswd);
				return true;
			}

			if (GetMultiSignPublicKey() == pubKey) {
				key = DeriveMultiSignKey(payPasswd);
				return true;
			}

			return false;
		}

	}
}
