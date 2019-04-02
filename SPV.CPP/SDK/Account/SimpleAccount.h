// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_SIMPLEACCOUNT_H__
#define __ELASTOS_SDK_SIMPLEACCOUNT_H__

#include <SDK/Account/IAccount.h>
#include <SDK/Common/Mstream.h>

namespace Elastos {
	namespace ElaWallet {

		class SimpleAccount : public IAccount {
		public:
			SimpleAccount(const bytes_t &privKey, const std::string &payPassword);

			virtual nlohmann::json GetBasicInfo() const;

			virtual Key DeriveMultiSignKey(const std::string &payPassword);

			virtual Key DeriveVoteKey(const std::string &payPasswd, uint32_t coinIndex, uint32_t account, uint32_t change);

			virtual uint512 DeriveSeed(const std::string &payPassword);

			virtual void ChangePassword(const std::string &oldPassword, const std::string &newPassword);

			virtual std::string GetType() const;

			virtual nlohmann::json ToJson() const;

			virtual void FromJson(const nlohmann::json &j);

			virtual bool IsReadOnly() const;

			virtual bool IsEqual(const IAccount &account) const;

		public: //properties

			const std::string &GetEncryptedKey() const;

			virtual const std::string &GetEncryptedMnemonic() const;

			virtual const std::string &GetEncryptedPhrasePassword() const;

			virtual bytes_t GetMultiSignPublicKey() const;

			virtual bytes_t GetVotePublicKey() const;

			virtual const HDKeychain &GetIDMasterPubKey() const;

			virtual Address GetAddress() const;

		private:
			friend class AccountFactory;

			SimpleAccount();

			JSON_SM_LS(SimpleAccount);

			JSON_SM_RS(SimpleAccount);

			TO_JSON(SimpleAccount);

			FROM_JSON(SimpleAccount);

		private:

			std::string _emptyString;
			bytes_t _publicKey;
			std::string _encryptedKey; // encode with base64
		};

	}
}

#endif //__ELASTOS_SDK_SIMPLEACCOUNT_H__
