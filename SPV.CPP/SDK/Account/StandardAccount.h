// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_STANDARDACCOUNT_H__
#define __ELASTOS_SDK_STANDARDACCOUNT_H__

#include <SDK/Account/IAccount.h>
#include <SDK/KeyStore/Mnemonic.h>
#include <SDK/Common/CMemBlock.h>
#include <SDK/Crypto/MasterPubKey.h>
#include <SDK/Common/Mstream.h>

#include <nlohmann/json.hpp>

namespace Elastos {
	namespace ElaWallet {

		class StandardAccount : public IAccount {
		public:
			StandardAccount(const std::string &rootPath,
							const std::string &phrase,
							const std::string &phrasePassword,
							const std::string &payPassword);

			virtual nlohmann::json GetBasicInfo() const;

			virtual Key DeriveMultiSignKey(const std::string &payPassword);

			virtual UInt512 DeriveSeed(const std::string &payPassword);

			virtual void ChangePassword(const std::string &oldPassword, const std::string &newPassword);

			virtual std::string GetType() const;

			virtual nlohmann::json ToJson() const;

			virtual void FromJson(const nlohmann::json &j);

			virtual bool IsReadOnly() const;

			virtual bool IsEqual(const IAccount &account) const;

		public: //properties

			virtual const std::string &GetEncryptedMnemonic() const;

			virtual const std::string &GetEncryptedPhrasePassword() const;

			virtual CMBlock GetMultiSignPublicKey() const;

			virtual const MasterPubKey &GetIDMasterPubKey() const;

			virtual std::string GetAddress() const;

			const std::string &GetLanguage() const;

			MasterPubKey DeriveIDMasterPubKey(const std::string &payPasswd);

		private:
			friend class AccountFactory;

			StandardAccount(const std::string &rootPath);

			JSON_SM_LS(StandardAccount);

			JSON_SM_RS(StandardAccount);

			TO_JSON(StandardAccount);

			FROM_JSON(StandardAccount);

		private:
			std::string _encryptedMnemonic;
			std::string _encryptedPhrasePass;
			CMBlock _multiSignPublicKey;
			MasterPubKey _masterIDPubKey;
			std::string _language;

			boost::shared_ptr<Mnemonic> _mnemonic;
			std::string _rootPath;
		};

	}
}

#endif //__ELASTOS_SDK_STANDARDACCOUNT_H_
