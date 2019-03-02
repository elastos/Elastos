// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "SubAccountBase.h"

#include <SDK/Common/Utils.h>
#include <SDK/Common/ParamChecker.h>
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

			if (!_depositAddress.IsValid()) {
				return false;
			}

			return _depositAddress == address;
		}

		void SubAccountBase::SignTransaction(const TransactionPtr &tx, const std::string &payPasswd) {
			std::string addr;
			Key key;
			ByteStream stream;

			ParamChecker::checkParam(tx->IsSigned(), Error::AlreadySigned, "Transaction signed");
			ParamChecker::checkParam(tx->getPrograms().empty(), Error::InvalidTransaction, "Invalid transaction program");

			UInt256 md = tx->GetShaData();

			std::vector<Program> &programs = tx->getPrograms();
			for (size_t i = 0; i < programs.size(); ++i) {
				std::vector<CMBlock> publicKeys = programs[i].DecodePublicKey();
				ParamChecker::checkLogic(publicKeys.empty(), Error::InvalidRedeemScript, "Invalid redeem script");

				bool found = false;
				for (size_t k = 0; k < publicKeys.size(); ++k) {
					found = FindKey(key, publicKeys[k], payPasswd);
					if (found)
						break;
				}
				ParamChecker::checkLogic(!found, Error::PrivateKeyNotFound, "Private key not found");

				stream.setPosition(0);
				if (programs[i].getParameter().GetSize() > 0) {
					ByteStream verifyStream(programs[i].getParameter());
					CMBlock signature;
					while (verifyStream.readVarBytes(signature)) {
						ParamChecker::checkLogic(key.Verify(md, signature), Error::AlreadySigned, "Already signed");
					}
					stream.writeBytes(programs[i].getParameter());
				}

				CMBlock signedData = key.Sign(md);
				stream.writeVarBytes(signedData);
				programs[i].setParameter(stream.getBuffer());
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
