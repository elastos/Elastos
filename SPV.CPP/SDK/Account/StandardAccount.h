// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_STANDARDACCOUNT_H__
#define __ELASTOS_SDK_STANDARDACCOUNT_H__

#include <nlohmann/json.hpp>

#include "SDK/KeyStore/Mnemonic.h"
#include "SDK/Common/CMemBlock.h"
#include "SDK/Wrapper/MasterPubKey.h"
#include "SDK/Common/Mstream.h"

namespace Elastos {
	namespace ElaWallet {

		class StandardAccount {
		public:
			StandardAccount(const std::string &rootPath);

			bool initFromPhrase(const std::string &phrase,
								const std::string &phrasePassword,
								const std::string &payPassword);

			void resetMnemonic(const std::string &language);

			Key deriveKey(const std::string &payPassword);

			UInt512 deriveSeed(const std::string &payPassword);

			virtual void ChangePassword(const std::string &oldPassword, const std::string &newPassword);

		public: //properties

			const CMBlock &GetEncrpytedKey() const;

			void SetEncryptedKey(const CMBlock &data);

			const CMBlock &GetEncryptedMnemonic() const;

			void SetEncryptedMnemonic(const CMBlock &data);

			const CMBlock &GetEncrptedPhrasePassword() const;

			void SetEncryptedPhrasePassword(const CMBlock &data);

			const std::string &GetPublicKey() const;

			void SetPublicKey(const std::string &pubKey);

			const std::string &GetLanguage() const;

			void SetLanguage(const std::string &language);

			const MasterPubKey &GetIDMasterPubKey() const;

			void SetIDMasterPubKey(const MasterPubKey &masterPubKey);

		private:
			JSON_SM_LS(StandardAccount);

			JSON_SM_RS(StandardAccount);

			TO_JSON(StandardAccount);

			FROM_JSON(StandardAccount);

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
