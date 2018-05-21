// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_MASTERWALLET_H__
#define __ELASTOS_SDK_MASTERWALLET_H__

#include <map>
#include <boost/shared_ptr.hpp>

#include "Interface/IMasterWallet.h"

namespace Elastos {
	namespace SDK {

		class Key;
		class MasterPubKey;

		class MasterWallet : public IMasterWallet {
		public:
			virtual ~MasterWallet();

			virtual ISubWallet *CreateSubWallet(
					const std::string &chainID,
					int coinTypeIndex,
					const std::string &payPassword,
					bool singleAddress);

			virtual ISubWallet *RecoverSubWallet(
					const std::string &chainID,
					int coinTypeIndex,
					const std::string &payPassword,
					bool singleAddress,
					int limitGap);

			virtual void DestroyWallet(ISubWallet *wallet);

			virtual std::string GetPublicKey();

			const std::string &GetName() const;

			bool Initialized() const;

		protected:
			friend class WalletFactory;

			typedef boost::shared_ptr<MasterPubKey> MasterPubKeyPtr;

			typedef boost::shared_ptr<Key> KeyPtr;

			typedef std::map<std::string, ISubWallet *> WalletMap;

			MasterWallet();

			MasterWallet(const std::string &name,
						 const std::string &backupPassword,
						 const std::string &payPassword);

			bool importFromKeyStore(const std::string &keystorePath,
									const std::string &backupPassword,
									const std::string &payPassword);

			bool importFromMnemonic(const std::string &mnemonic,
									const std::string &phrasePassword,
									const std::string &payPassword);

			bool exportKeyStore(const std::string &backupPassword,
								const std::string &keystorePath);

			bool exportMnemonic(const std::string &phrasePassword, std::string &mnemonic);

		protected:
			std::string _name;

			bool _initialized;
			WalletMap _createdWallets;

			MasterPubKeyPtr _masterPubKey;
			KeyPtr _key;
		};

	}
}

#endif //__ELASTOS_SDK_MASTERWALLET_H__
