// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/scoped_ptr.hpp>

#include "ISubWallet.h"
#include "IMasterWallet.h"
#include "MasterWalletManager.h"

#ifndef _ELATSOS_SAMPLE_TESTCONNECTPEER_H_
#define _ELATSOS_SAMPLE_TESTCONNECTPEER_H_

using namespace Elastos::ElaWallet;

class TestConnectPeer {
public:
	static void runPeerConnectTest_WalletManager();

	static void runPeerConnectTest_WalletFactory();

	static IMasterWallet *importWithKeystore(boost::shared_ptr<MasterWalletManager> walletFactory, const std::string &payPassword);
	static IMasterWallet *importWithMnemonic(boost::shared_ptr<MasterWalletManager> walletFactory, const std::string &payPassword);
	static IMasterWallet *createReadOnlyMultiSignWallet(boost::shared_ptr<MasterWalletManager> walletFactory);
	static IMasterWallet *createMultiSignWalletFromMnemonic(boost::shared_ptr<MasterWalletManager> walletFactory,
															const std::string &payPassword);
	static IMasterWallet *createMultiSignWalletFromPrivKey(boost::shared_ptr<MasterWalletManager> walletFactory,
															const std::string &payPassword);

	static void transfer(ISubWallet *subWallet, const std::string &payPassword, uint64_t amount, const std::string &to);
	static void deposit(ISubWallet *mainchainSubWallet, uint64_t amount, const std::string &sidechainAddress, const std::string &payPassword);
	static void withdraw(ISubWallet *sidechainSubWallet, uint64_t amount, const std::string &mainchainAddress, const std::string &payPassword);
	static void registerId(IMasterWallet *masterWallet, ISubWallet *subWallet, const std::string &payPassword);
};


#endif //_ELATSOS_SAMPLE_TESTCONNECTPEER_H_
