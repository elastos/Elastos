// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <boost/scoped_ptr.hpp>

#include "catch.hpp"

#include "Transaction/Transaction.h"
#include "Utils.h"
#include "Log.h"

#include "MasterWalletManager.h"
#include "MasterWallet.h"
#include "Address.h"
#include "SidechainSubWallet.h"

using namespace Elastos::ElaWallet;

TEST_CASE("test sideChain Create Transaction", "") {
	std::string mnemonic = "clarify polar wrong ethics someone cabbage wing doctor brain add say room";
	std::string phrasePassword = "";
	std::string payPassword = "payPassword";
	std::string chainId = "IdChain";
	boost::scoped_ptr<MasterWalletManager> walletFactory(new MasterWalletManager("Data"));
	IMasterWallet *masterWallet = walletFactory->ImportWalletWithMnemonic(chainId, mnemonic, phrasePassword, payPassword);
	ISubWallet *subWallet = masterWallet->CreateSubWallet(chainId, payPassword, false);

	SidechainSubWallet *sidechainSubWallet = dynamic_cast<SidechainSubWallet *>(subWallet);

	nlohmann::json j = subWallet->GetAllAddress(0, 3);
	std::vector<std::string> addresses = j["Addresses"].get<std::vector<std::string>>();
	std::cout << "address=" << j << std::endl;

	uint64_t value = subWallet->GetBalance();
	std::cout<< "balance=" << value << std::endl;

	std::string fromAddress = addresses[0];

	std::vector<std::string> accounts;
		accounts.push_back("EKBXRYk6wTz7eVeKEy1YFuDMVovzGvWu3b");
	std::vector<uint64_t> amounts;
	std::vector<uint64_t> indexs;
	indexs.push_back(0);
	amounts.push_back(10000);

	nlohmann::json mainChainAccount(accounts);
	nlohmann::json mainChainIndex(indexs);
	nlohmann::json mainChainAmount(amounts);

	bool isSend = false;
	while (1) {
		sleep(5);

		if (value > 10000 && !isSend) {

//			nlohmann::json txJson = sidechainSubWallet->CreateWithdrawTransaction("", "1111111111111111111114oLvT2", 20000, mainChainAccount,
//			                                              mainChainAmount, mainChainIndex, 10002, "memo", "remark");
//
//			nlohmann::json result = sidechainSubWallet->SendRawTransaction(txJson, 10002, payPassword);
//			std::cout<< "result" << result << std::endl;

			isSend = true;
		}

		value = subWallet->GetBalance();
		std::cout<< "balance22=" << value << std::endl;
	}

	masterWallet->DestroyWallet(subWallet);
	walletFactory->DestroyWallet(masterWallet->GetId());
}
