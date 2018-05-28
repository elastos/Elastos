// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_MASTERWALLET_H__
#define __ELASTOS_SDK_MASTERWALLET_H__

#include <map>
#include <boost/shared_ptr.hpp>
#include <boost/filesystem.hpp>

#include "Interface/IMasterWallet.h"
#include "KeyStore/KeyStore.h"
#include "Manager/Mnemonic.h"
#include "MasterPubKey.h"

namespace Elastos {
	namespace SDK {

		class ChainParams;
		class SubWallet;

		class MasterWallet : public IMasterWallet {
		public:
			virtual ~MasterWallet();

			virtual ISubWallet *CreateSubWallet(
					const std::string &chainID,
					int coinTypeIndex,
					const std::string &payPassword,
					bool singleAddress,
					uint64_t feePerKb = 0);

			virtual ISubWallet *RecoverSubWallet(
					const std::string &chainID,
					int coinTypeIndex,
					const std::string &payPassword,
					bool singleAddress,
					int limitGap,
					uint64_t feePerKb = 0);

			virtual void DestroyWallet(ISubWallet *wallet);

			virtual std::string GetPublicKey();

			virtual std::string Sign(
					const std::string &message,
					const std::string &payPassword);

			virtual nlohmann::json CheckSign(
					const std::string &publicKey,
					const std::string &message,
					const std::string &signature);

			bool Initialized() const;

		protected:
			friend class WalletFactory;

			friend class SubWallet;

			typedef std::map<std::string, ISubWallet *> WalletMap;

			MasterWallet();

			MasterWallet(const std::string &phrasePassword,
						 const std::string &payPassword);

			bool importFromKeyStore(const std::string &keystorePath,
									const std::string &backupPassword,
									const std::string &payPassword);

			bool importFromMnemonic(const std::string &mnemonic,
									const std::string &phrasePassword,
									const std::string &payPassword);

			bool exportKeyStore(const std::string &backupPassword,
								const std::string &keystorePath);

			bool exportMnemonic(const std::string &payPassword,
								std::string &mnemonic);

			bool initFromEntropy(const UInt128 &entropy,
								 const std::string &phrasePassword,
								 const std::string &payPassword);

			bool initFromPhrase(const std::string &phrase,
								const std::string &phrasePassword,
								const std::string &payPassword);

			Key deriveKey(const std::string &payPassword);

			UInt512 deriveSeed(const std::string &payPassword);

			void initPublicKey(const std::string &payPassword);

			SubWallet *SubWalletFactoryMethod(const CoinInfo &info,
											  const ChainParams &chainParams,
											  const std::string &payPassword,
											  MasterWallet *parent);

		protected:
			bool _initialized;
			WalletMap _createdWallets;

			CMemBlock<unsigned char> _encryptedKey;
			CMemBlock<unsigned char> _encryptedEntropy;
			CMemBlock<unsigned char> _encryptedPhrasePass;

			KeyStore _keyStore;
			boost::filesystem::path _dbRoot;
			Mnemonic _mnemonic;

			std::string _publicKey;
		};

	}
}

#endif //__ELASTOS_SDK_MASTERWALLET_H__
