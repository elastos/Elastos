// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_SINGLEADDRESSWALLET_H__
#define __ELASTOS_SDK_SINGLEADDRESSWALLET_H__

#include "Wallet.h"

namespace Elastos {
	namespace SDK {

		class SingleAddressWallet : public Wallet {
		public:
#ifdef TEMPORARY_HD_STRATEGY
			SingleAddressWallet(const SharedWrapperList<Transaction, BRTransaction *> &transactions,
								const MasterPrivKey &masterPrivKey,
								const std::string &payPassword,
								const boost::shared_ptr<Listener> &listener);
#else
			SingleAddressWallet(const SharedWrapperList<Transaction, BRTransaction *> &transactions,
								const MasterPubKeyPtr &masterPubKey,
								const boost::shared_ptr<Listener> &listener);
#endif

			virtual ~SingleAddressWallet();

		private:
#ifdef TEMPORARY_HD_STRATEGY
			ELAWallet *createSingleWallet(BRTransaction *transactions[], size_t txCount,
										  const MasterPrivKey &masterPrivKey, const std::string &payPassword);
#else
			ELAWallet *createSingleWallet(BRTransaction *transactions[], size_t txCount, const BRMasterPubKey &mpk);
#endif
		};

	}
}

#endif //__ELASTOS_SDK_SINGLEADDRESSWALLET_H__
