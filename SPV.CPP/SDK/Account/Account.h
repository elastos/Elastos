// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_ACCOUNT_H__
#define __ELASTOS_SDK_ACCOUNT_H__

#include <SDK/WalletCore/BIPs/Mnemonic.h>
#include <SDK/Common/Mstream.h>
#include <SDK/WalletCore/BIPs/Address.h>
#include <SDK/SpvService/LocalStore.h>

#include <nlohmann/json.hpp>

namespace Elastos {
	namespace ElaWallet {

		class Account {
		public:
			enum SignType {
				Standard,
				MultiSign,
			};
		public:
			Account(const LocalStorePtr &store, const std::string &rootpath);

			bytes_t RequestPubKey() const;

			Key RequestPrivKey(const std::string &payPassword) const;

			HDKeychain RootKey(const std::string &payPassword) const;

			HDKeychain MasterPubKey() const;

			bytes_t OwnerPubKey() const;

			const Address &GetAddress() const;

			void ChangePassword(const std::string &oldPassword, const std::string &newPassword);

			nlohmann::json GetBasicInfo() const;

			SignType GetSignType() const;

			bool ReadOnly() const;

			bool SingleAddress() const;

			bool Equal(const Account &account) const;

		private:
			LocalStorePtr _localstore;
			Address _address;
			std::string _rootpath;
		};

		typedef boost::shared_ptr<Account> AccountPtr;

	}
}

#endif //__ELASTOS_SDK_ACCOUNT_H__
