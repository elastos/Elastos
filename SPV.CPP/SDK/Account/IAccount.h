// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_IACCOUNT_H__
#define __ELASTOS_SDK_IACCOUNT_H__

#include <boost/shared_ptr.hpp>
#include <nlohmann/json.hpp>

#include "BRInt.h"
#include "Key.h"
#include "MasterPubKey.h"

namespace Elastos {
	namespace ElaWallet {

		class IAccount {
		public:
			virtual ~IAccount() {}

			virtual bool initFromPhrase(const std::string &phrase,
										const std::string &phrasePassword,
										const std::string &payPassword) = 0;

			virtual void ChangePassword(const std::string &oldPassword, const std::string &newPassword) = 0;

			virtual UInt512 deriveSeed(const std::string &payPassword) = 0;

			virtual Key deriveKey(const std::string &payPassword) = 0;

			virtual void resetMnemonic(const std::string &language) = 0;

			virtual nlohmann::json ToJson() const = 0;

			virtual void FromJson(const nlohmann::json &j) = 0;

		public: //properties

			virtual const CMBlock &GetEncrpytedKey() const = 0;

			virtual void SetEncryptedKey(const CMBlock &data) = 0;

			virtual const CMBlock &GetEncryptedMnemonic() const = 0;

			virtual void SetEncryptedMnemonic(const CMBlock &data) = 0;

			virtual const CMBlock &GetEncrptedPhrasePassword() const = 0;

			virtual void SetEncryptedPhrasePassword(const CMBlock &data) = 0;

			virtual const std::string &GetPublicKey() const = 0;

			virtual void SetPublicKey(const std::string &pubKey) = 0;

			virtual const std::string &GetLanguage() const = 0;

			virtual void SetLanguage(const std::string &language) = 0;

			virtual const MasterPubKey &GetIDMasterPubKey() const = 0;

			virtual void SetIDMasterPubKey(const MasterPubKey &masterPubKey) = 0;

		};

		typedef boost::shared_ptr<IAccount> AccountPtr;
	}
}

#endif //__ELASTOS_SDK_IACCOUNT_H__
