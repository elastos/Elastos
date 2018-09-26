// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include "catch.hpp"

#include "Account/MultiSignAccount.h"
#include "Account/SimpleAccount.h"
#include "Account/MultiSignSubAccount.h"
#include "Utils.h"
#include "WalletTool.h"
#include "Log.h"

using namespace Elastos::ElaWallet;

TEST_CASE("SignTransaction test", "[SignTransaction]") {

	TransactionPtr transaction(new Transaction);
	nlohmann::json content = "{\"Attributes\":[{\"Data\":\"353634383333303934\",\"Usage\":0}],\"BlockHeight\":2147483647,\"Fee\":10000,\"Inputs\":[{\"Address\":\"8Gqrkk876Kc1HUjeG9evyFsc91RGYWyQj4\",\"Amount\":200000000,\"Index\":0,\"Script\":\"76a914134a742f7782c295d3ea18cb59cd0101b21b1a2f88ac\",\"Sequence\":4294967295,\"Signature\":\"\",\"TxHash\":\"e77c3bea963d124311076d4737372cbb23aef8d63d5eadaad578455d481cc025\"}],\"IsRegistered\":false,\"LockTime\":0,\"Outputs\":[{\"Address\":\"Ed8ZSxSB98roeyuRZwwekrnRqcgnfiUDeQ\",\"Amount\":10000000,\"AssetId\":\"b037db964a231458d2d6ffd5ea18944c4f90e63d547c5d3b9874df66a4ead0a3\",\"OutputLock\":0,\"ProgramHash\":\"21db215de2758b7d743f66e4c66cfcc35dc54ccbcb\",\"Script\":\"76a914db215de2758b7d743f66e4c66cfcc35dc54ccbcb88ac\",\"ScriptLen\":25,\"SignType\":172},{\"Address\":\"8Gqrkk876Kc1HUjeG9evyFsc91RGYWyQj4\",\"Amount\":189990000,\"AssetId\":\"b037db964a231458d2d6ffd5ea18944c4f90e63d547c5d3b9874df66a4ead0a3\",\"OutputLock\":0,\"ProgramHash\":\"12134a742f7782c295d3ea18cb59cd0101b21b1a2f\",\"Script\":\"76a914134a742f7782c295d3ea18cb59cd0101b21b1a2f88ac\",\"ScriptLen\":25,\"SignType\":174}],\"PayLoad\":null,\"PayloadVersion\":0,\"Programs\":[],\"Remark\":\"\",\"Timestamp\":0,\"TxHash\":\"80a0eb3c6bbce2c21d542c7ce9d248fe013fc1c757addd7fcee04b14098d5fa7\",\"Type\":2,\"Version\":1}"_json;
	transaction->fromJson(content);

	CMBlock key;
	std::string payPassword = "payPassword";
	std::vector<std::string> coSigners;

	coSigners.clear();
	coSigners.push_back("03b73a64f50c142c1f08710e04b928553508c3028e045dfdfdc5489434df13275e");
	MultiSignAccount *account = new MultiSignAccount(
			new SimpleAccount("2c7c9180792e49a624b02ac2adff2f994ecc28044ee9889d6054159189da03a5", payPassword),
			coSigners, 2);
	MultiSignSubAccount *subAccount = new MultiSignSubAccount(account);

	coSigners.clear();
	coSigners.push_back("02f925e82f4482a9aa853a35203ab8965439c9db6aee8ef1783d2e1a491c28a482");
	MultiSignAccount *account2 = new MultiSignAccount(
			new SimpleAccount("6e7910da9c066524273be2b493616ef0d4a848a0696829141a90458a9cf160af", payPassword),
			coSigners, 2);
	MultiSignSubAccount *subAccount2 = new MultiSignSubAccount(account2);


	subAccount2->SignTransaction(transaction, nullptr, payPassword);
	std::vector<std::string> signers = subAccount2->GetTransactionSignedSigners(transaction);
	REQUIRE(signers.size() == 1);
	REQUIRE(signers[0] == "03b73a64f50c142c1f08710e04b928553508c3028e045dfdfdc5489434df13275e");

	subAccount->SignTransaction(transaction, nullptr, payPassword);
	signers = subAccount2->GetTransactionSignedSigners(transaction);
	REQUIRE(signers.size() == 2);

	REQUIRE(transaction->getPrograms()[0]->isValid(transaction));
}
