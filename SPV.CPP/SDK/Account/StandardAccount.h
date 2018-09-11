// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_STANDARDACCOUNT_H__
#define __ELASTOS_SDK_STANDARDACCOUNT_H__

#include <nlohmann/json.hpp>

#include "IAccount.h"
#include "SDK/KeyStore/Mnemonic.h"
#include "SDK/Common/CMemBlock.h"
#include "SDK/Wrapper/MasterPubKey.h"
#include "SDK/Common/Mstream.h"

namespace Elastos {
	namespace ElaWallet {

		class StandardAccount : public IAccount {
		public:
			StandardAccount(const std::string &rootPath,
							const std::string &phrase,
							const std::string &language,
							const std::string &phrasePassword,
							const std::string &payPassword);

			virtual Key DeriveKey(const std::string &payPassword);

			virtual UInt512 DeriveSeed(const std::string &payPassword);

			virtual void ChangePassword(const std::string &oldPassword, const std::string &newPassword);

			virtual nlohmann::json ToJson() const;

			virtual void FromJson(const nlohmann::json &j);

		public: //properties

			virtual const CMBlock &GetEncryptedKey() const;

			virtual const CMBlock &GetEncryptedMnemonic() const;

			virtual const CMBlock &GetEncryptedPhrasePassword() const;

			virtual const std::string &GetPublicKey() const;

			virtual const MasterPubKey &GetIDMasterPubKey() const;

			const std::string &GetLanguage() const;

			void SetLanguage(const std::string &language);

		private:
			JSON_SM_LS(StandardAccount);

			JSON_SM_RS(StandardAccount);

			TO_JSON(StandardAccount);

			FROM_JSON(StandardAccount);

			void resetMnemonic(const std::string &language);

		private:
			CMBlock _encryptedKey;
			CMBlock _encryptedMnemonic;
			CMBlock _encryptedPhrasePass;
			std::string _publicKey;
			MasterPubKey _masterIDPubKey;
			std::string _language;

			boost::shared_ptr<Mnemonic> _mnemonic;
			std::string _rootPath;
		};

	}
}

#endif //__ELASTOS_SDK_STANDARDACCOUNT_H_
