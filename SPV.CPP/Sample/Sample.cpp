// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <stdio.h>
#include <iostream>
#include <sqlite3.h>

#include <Interface/MasterWalletManager.h>
#include <Interface/IMasterWallet.h>
#include <Interface/ISubWallet.h>
#include <Interface/ISubWalletCallback.h>
#include <Interface/IMainchainSubWallet.h>
#include <Interface/IIdChainSubWallet.h>
#include <Interface/ISidechainSubWallet.h>
#include <Interface/IIdAgent.h>
#include <spdlog/logger.h>
#include <spdlog/spdlog.h>

using namespace Elastos::ElaWallet;

static const std::string gMasterWalletID = "WalletID";
static const std::string gMainchainSubWalletID = "ELA";
static const std::string gSidechainSubWalletID = "IdChain";
static const std::string rootPath = "Data";
static const std::string payPasswd = "s12345678";
static uint64_t feePerKB = 10000;

static std::shared_ptr<spdlog::logger> logger = spdlog::stdout_color_mt("sample");;
static bool syncSucceed = false;

class SubWalletCallback : public ISubWalletCallback {
public:
	~SubWalletCallback() {}
	SubWalletCallback() {}

	virtual void OnTransactionStatusChanged(
		const std::string &txid,const std::string &status,
		const nlohmann::json &desc,uint32_t confirms) {
		logger->debug("OnTransactionStatusChanged ----> txid = {}, confirms = {}", txid, confirms);
//		logger->debug("OnTransactionStatusChanged ----> desc = {}", desc.dump());
	}

	virtual void OnBlockSyncStarted() {
		logger->debug("OnBlockSyncStarted");
	}

	virtual void OnBlockSyncProgress(uint32_t currentBlockHeight, uint32_t estimatedHeight) {
		logger->debug("OnBlockSyncProgress ----> [ {} / {} ]", currentBlockHeight, estimatedHeight);
		if (currentBlockHeight >= estimatedHeight) {
			syncSucceed = true;
		}
	}

	virtual void OnBlockSyncStopped() {
		logger->debug("OnBlockSyncStopped");
	}

	virtual void OnBalanceChanged(uint64_t balance) {
		logger->debug("OnBalanceChanged ----> {}", balance);
	}

	virtual void OnTxPublished(const std::string &hash, const nlohmann::json &result) {
		logger->debug("OnTxPublished ----> hash = {}, result = {}", hash, result.dump());
	}

	virtual void OnTxDeleted(const std::string &hash, bool notifyUser, bool recommendRescan) {
		logger->debug("OnTxDeleted ----> hash = {}, notifyUser = {}, recommendRescan", recommendRescan);
	}

};

static IMasterWallet *NewWalletWithMnemonic(MasterWalletManager *manager) {
	const std::string phrasePassword = "";
	const std::string mnemonic = manager->GenerateMnemonic("english");
	logger->debug("mnemonic -> {}", mnemonic);
	return manager->CreateMasterWallet(gMasterWalletID, mnemonic, phrasePassword, payPasswd, false);
}

static IMasterWallet *ImportWalletWithMnemonic(MasterWalletManager *manager) {
	const std::string phrasePassword = "";
	const std::string mnemonic =
		"abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
	logger->debug("mnemonic -> {}", mnemonic);
	return manager->ImportWalletWithMnemonic(gMasterWalletID, mnemonic, phrasePassword, payPasswd, false);
}

static IMasterWallet *ImportWalletWithKeystore(MasterWalletManager *manager) {
	const std::string backupPasswd = "heropoon";
	nlohmann::json keystore = nlohmann::json::parse(
		"{\"iv\":\"kha3Vctm00fA1hjOdP0YQA==\",\"v\":1,\"iter\":10000,\"ks\":128,\"ts\":64,\"mode\":\"ccm\","
		"\"adata\":\"\",\"cipher\":\"aes\",\"salt\":\"J4aFQZSWvFo=\",\"ct\":\"mDVCy7fnV3RnehbDlafq4ALab43Zk"
		"Vkzm5x2sHVjQscmQt4jWXUQQJP/A/+SYixFvj/fqZ4u+alT5/x08OAcWqnr/ft19ItfMB+bBd7vT1T93DygjRj9nRKMSyPu9rI"
		"arCU1fRPbxmNKF7dE7E2njtPcEZ+FAUKorj0P5AGvzWQM8QIlVbxz6TQTu120Ar1ukzlHc+xXTA8pw6ZlJcZQ6i7khI/hnVQzk"
		"uXr7R7y09yvU4LIDob/KmzsP7hSMha3hVUFLtvnTl5ElHlll7nywERMNYVt2khNSDfPACsFyaVlPHopKGsdp66IjJBemTVna6d"
		"H/n0HEIGFzsQ/dr//vsk9/BUoq7eLMart0StzaNm4R43q/3h0f4+SwqG0hk2aBGCG0QEgSor/S02ts50P80pbuGOQ7zARfmxkk"
		"Epm5z4iT0krPyJEsccitDtYBHmUEaSnT1zffkchtJsaZJFFx/9s5UpLkD2JrtYKvAM/txuBmacPFyxzUIdCoAH1n2z1rBvdd2h"
		"UPOXqds9IvHIEhx0aXR7fjRivS1JBpjqFHRKOEkGjkb+lN+HGtjooQxdOcmN1lVR5/oHETKqelupSosdNOiw2z2Ytpd5ovzX14"
		"PyGqnY8um4d/CzG7q5h8KDsGlrxPPvVEtl+WuAQ6zO/4YxLP+llukuY4b+vyT+v6c40qP5+acnkjb3zJ301EAo/PMIMf8IX2Ns"
		"sPVcd+XuRIospbvKMpTh05VjoXFr6rNu+/zho527C8JDe4tG4Fjs1yWOwQVM1Qk2ear2Rnwbf/lnYmU2TcO7zd69iRkiY7rhuL"
		"UTerbCjqVgZUTixri0HvsU0g8y5YZ5Thor7ghr/vV/YOywlgTAxknnr6/mtrVE1YXRP071lDsQ0734FK1L6oUfzoMBVlecUV8D"
		"UMQO6U4A34Y9THqHbhMBem05US7D0AQ8HtRuhg7c28LUOugxd6LTmbo4RNnre0bMCGNI6ZGhtBHXn1lMWWLe1bFy6tj9Uo6oVE"
		"/vg7pykEA/zJ3BhE4TJv8J3HIsAUEgVe5f7StDieODk8Y4uYAp2XLTi8QhiDt4dvaLHVkLz1Vjn3GQ15882bI6/H3/ezUTfxgT"
		"rdfGm8T7hFrYQx7QibYQRYgs+9UWWbHVKbqA5T0DA87yUFIcVgVv+CrubxrIr61vF7nu3EpLKCstqcKTM3LKjPxJlCKqCJAPDS"
		"gvpwjCWmI6fftZQ1SlCgIWAT3yGOy+FBaHr25+ZhT2iKh6UTUD3Yxr7mzo1/wzQGgpMqCLSKCmJUuHDzjdzjAu13RL9i+YA4Pk"
		"0FjkGhRLjhg/XT5trsLA2Hl8ZvUtZK0BXPPUYCXlFjLB6By1CkH4CPQJU69yV+MWO7NsJ59fQy5wl+5NS4Ks7XyNyAPacK3BkH"
		"uFlulGKCJyeQ9mLy9qKaXsdgUwThvVfgnRSl5qFoUvdZIHCgPESVRudP3lptlrTX1+ryuSLztNj8DE4zXUJRAgmua/QwVXrlBN"
		"6Y+I6tctjrSXONWtNVCp5HYmeaGbDo1w0oUnOQkpgP/4RjEfvDa5InRjJ6NxhuhOXfqWiYUNiHL9AXJPlulSnda/nOylOneJqx"
		"CFP4/dFF/0fXBon+pVf4HMc0M7KrRxwGq2fvk2OhIU/lo6ouZQCIfQuV0JM/PHEm/PK0YMbyQx+FvGiZzBtTiVMQyKUOxAgNDJ"
		"MyJCWvhBm/+ktzIQgtpvBq9NI1pkF+GFd2AgIoArhA4DidpLBM477HXAntj+PGeSc4FvS6gk=\"}");
	return manager->ImportWalletWithKeystore(gMasterWalletID, keystore, backupPasswd, payPasswd);
}

static IMasterWallet *NewReadOnlyMultiSignWallet(MasterWalletManager *manager) {
	nlohmann::json coSigners = nlohmann::json::parse(
		"[\"03FC8B9408A7C5AE6F8109BA97CE5429C1CD1F09C0655E4EC05FC0649754E4FB6C\","
		" \"02848A8F1880408C4186ED31768331BC9296E1B0C3EC7AE6F11E9069B16013A9C5\","
		" \"02775B47CCB0808BA70EA16800385DBA2737FDA090BB0EBAE948DD16FF658CA74D\","
		" \"03E5B45B44BB1E2406C55B7DD84B727FAD608BA7B7C11A9C5FFBFEE60E427BD1DA\"]");
	return manager->CreateMultiSignMasterWallet(gMasterWalletID, coSigners, 3);
}

static IMasterWallet *NewMultiSignWalletWithMnemonic(MasterWalletManager *manager) {
	const std::string mnemonic =
		"abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
	const std::string phrasePassword = "";
	nlohmann::json coSigners = nlohmann::json::parse(
		"[\"02848A8F1880408C4186ED31768331BC9296E1B0C3EC7AE6F11E9069B16013A9C5\","
		" \"02775B47CCB0808BA70EA16800385DBA2737FDA090BB0EBAE948DD16FF658CA74D\","
		" \"03E5B45B44BB1E2406C55B7DD84B727FAD608BA7B7C11A9C5FFBFEE60E427BD1DA\"]");

	return manager->CreateMultiSignMasterWallet(gMasterWalletID, mnemonic, phrasePassword, payPasswd, coSigners, 3);
}

static IMasterWallet *NewMultiSignWalletWithPrvKey(MasterWalletManager *manager) {
	nlohmann::json coSigners = nlohmann::json::parse(
		"[\"02848A8F1880408C4186ED31768331BC9296E1B0C3EC7AE6F11E9069B16013A9C5\","
		" \"02775B47CCB0808BA70EA16800385DBA2737FDA090BB0EBAE948DD16FF658CA74D\","
		" \"03E5B45B44BB1E2406C55B7DD84B727FAD608BA7B7C11A9C5FFBFEE60E427BD1DA\"]");
	std::string privKey = "C1BD9550387E49F2A2CB012C2B794DD2E4C4B3ABC614A0C485D848C2A9136A29";
	return manager->CreateMultiSignMasterWallet(gMasterWalletID, privKey, payPasswd, coSigners, 3);
}

static ISubWallet *GetSubWallet(MasterWalletManager *manager,
						 const std::string &masterWalletID, const std::string &subWalletID) {
	IMasterWallet *masterWallet = manager->GetWallet(masterWalletID);
	if (nullptr == masterWallet) {
		return nullptr;
	}

	std::vector<ISubWallet *> subWallets = masterWallet->GetAllSubWallets();
	for (size_t i = 0; i < subWallets.size(); ++i) {
		if (subWallets[i]->GetChainId() == subWalletID) {
			return subWallets[i];
		}
	}

	return nullptr;
}

static nlohmann::json CalculateFeeAndSign(ISubWallet *subWallet, const nlohmann::json &tx) {
	uint64_t fee = subWallet->CalculateTransactionFee(tx, feePerKB);
	nlohmann::json tmpTx = subWallet->UpdateTransactionFee(tx, fee);

	return subWallet->SignTransaction(tmpTx, payPasswd);
}

static void PublishTransaction(ISubWallet *subWallet, const nlohmann::json &tx) {
	nlohmann::json signedTx = CalculateFeeAndSign(subWallet, tx);

	nlohmann::json result = subWallet->PublishTransaction(signedTx);
	logger->debug("published tx result -> {}", result.dump());
}

static void Transafer(MasterWalletManager *manager,
			   const std::string &masterWalletID, const std::string &subWalletID,
			   const std::string &from, const std::string &to, uint64_t amount) {

	ISubWallet *subWallet = GetSubWallet(manager, masterWalletID, subWalletID);

	nlohmann::json tx = subWallet->CreateTransaction(from, to, amount, "memo", "remark");

	PublishTransaction(subWallet, tx);
}

static void Deposit(MasterWalletManager *manager,
					const std::string &fromMasterWalletID, const std::string &fromSubWalletID,
					const std::string &toMasterWalletID, const std::string &toSubWalletID,
					const std::string &from, const std::string &sidechainAddress, uint64_t amount) {
	ISubWallet *fromSubWallet = GetSubWallet(manager, fromMasterWalletID, fromSubWalletID);
	if (fromSubWallet == nullptr) {
		logger->error("[{}:{}] subWallet not found", fromMasterWalletID, fromSubWalletID);
		return ;
	}

	IMainchainSubWallet *mainchainSubWallet = dynamic_cast<IMainchainSubWallet *>(fromSubWallet);
	if (mainchainSubWallet == nullptr) {
		logger->error("[{}:{}] is not instance of IMainchainSubWallet!", fromMasterWalletID, fromSubWalletID);
		return ;
	}

	ISubWallet *toSubWallet = GetSubWallet(manager, toMasterWalletID, toSubWalletID);
	if (toSubWallet == nullptr) {
		logger->error("[{}:{}] subWallet not found", toMasterWalletID, toSubWalletID);
		return;
	}

	ISidechainSubWallet *sidechainSubWallet = dynamic_cast<ISidechainSubWallet *>(toSubWallet);
	if (sidechainSubWallet == nullptr) {
		logger->error("[{}:{}] is not instance of ISidechainSubWallet", toMasterWalletID, toSubWalletID);
		return ;
	}

	std::string lockedAddress = sidechainSubWallet->GetGenesisAddress();

	nlohmann::json sidechainAccounts;
	sidechainAccounts.push_back(sidechainAddress);

	nlohmann::json sidechainAmounts;
	sidechainAmounts.push_back(amount);

	nlohmann::json sidechainIndices;
	sidechainIndices.push_back(0);

	nlohmann::json tx = mainchainSubWallet->CreateDepositTransaction(
		from, lockedAddress, amount, sidechainAccounts, sidechainAmounts,
		sidechainIndices, "memo", "remark");

	logger->debug("[{}:{}] deposit {} to {}", fromMasterWalletID, fromSubWalletID, amount, sidechainAddress);

	PublishTransaction(fromSubWallet, tx);
}

static void Withdraw(MasterWalletManager *manager,
					const std::string &fromMasterWalletID, const std::string &fromSubWalletID,
					const std::string &from, const std::string &mainchainAddress, uint64_t amount) {
	ISubWallet *fromSubWallet = GetSubWallet(manager, fromMasterWalletID, fromSubWalletID);
	if (fromSubWallet == nullptr) {
		logger->error("[{}:{}] subWallet not found", fromMasterWalletID, fromSubWalletID);
		return ;
	}

	ISidechainSubWallet *sidechainSubWallet = dynamic_cast<ISidechainSubWallet *>(fromSubWallet);
	if (sidechainSubWallet == nullptr) {
		logger->error("[{}:{}] is not instance of ISidechainSubWallet", fromMasterWalletID, fromSubWalletID);
		return ;
	}

	nlohmann::json mainchainAccounts;
	mainchainAccounts.push_back(mainchainAddress);

	nlohmann::json mainchainAmounts;
	mainchainAmounts.push_back(amount);

	nlohmann::json mainchainIndexs;
	mainchainIndexs.push_back(0);

	nlohmann::json tx = sidechainSubWallet->CreateWithdrawTransaction(
		from, amount, mainchainAccounts, mainchainAmounts, mainchainIndexs, "memo", "remark");

	logger->debug("[{}:{}] withdraw {} to {}", fromMasterWalletID, fromSubWalletID, amount, mainchainAddress);

	PublishTransaction(sidechainSubWallet, tx);
}

static void RegisterID(MasterWalletManager *manager,
					   const std::string &masterWalletID, const std::string &DIDSubWalletID) {
	IMasterWallet *masterWalelt = manager->GetWallet(masterWalletID);
	if (masterWalelt == nullptr) {
		logger->error("[{}] master wallet not found", masterWalletID);
		return ;
	}

	IIdAgent *IDAgent = dynamic_cast<IIdAgent *>(masterWalelt);
	if (IDAgent == nullptr) {
		logger->error("[{}] is not instance of IIdAgent", masterWalletID);
		return ;
	}

	ISubWallet *subWallet = GetSubWallet(manager, masterWalletID, DIDSubWalletID);
	IIdChainSubWallet *DIDSubWallet = dynamic_cast<IIdChainSubWallet *>(subWallet);
	if (DIDSubWallet == nullptr) {
		logger->error("[{}:{}] is not instance of IIdChainSubWallet", masterWalletID, DIDSubWalletID);
		return ;
	}

	std::string id = IDAgent->DeriveIdAndKeyForPurpose(1, 0);

	nlohmann::json payload = nlohmann::json::parse(
		"{\"Id\":\"ij8rfb6A4Ri7c5CRE1nDVdVCUMuUxkk2c6\",\"Contents\":[{\"Path\":\"kyc/person/identityCard\","
		"\"Values\": [ { \"Proof\": \"\\\"signature\\\":\\\"30450220499a5de3f84e7e919c26b6a8543fd24129634c65"
		"ee4d38fe2e3386ec8a5dae57022100b7679de8d181a454e2def8f55de423e9e15bebcde5c58e871d20aa0d91162ff6\\\","
		"\\\"notary\\\":\\\"COOIX\\\"\", \"DataHash\": \"bd117820c4cf30b0ad9ce68fe92b0117ca41ac2b6a49235fabd"
		"793fc3a9413c0\"}]}]}");
	payload["Id"] = id;

	payload["Sign"] = IDAgent->Sign(id, payload.dump(), payPasswd);;
	nlohmann::json program = IDAgent->GenerateProgram(id, payload.dump(), payPasswd);

	nlohmann::json tx = DIDSubWallet->CreateIdTransaction("", payload, program, "memo", "remark");

	logger->debug("[{}:{}] register id", masterWalletID, DIDSubWalletID);

	PublishTransaction(subWallet, tx);
}

static void InitWallets(MasterWalletManager *manager) {
	std::vector<IMasterWallet *> masterWallets = manager->GetAllMasterWallets();
	if (masterWallets.size() == 0) {
		IMasterWallet *masterWallet = nullptr;
//		masterWallet = ImportWalletWithMnemonic(manager);
		masterWallet = ImportWalletWithKeystore(manager);
//		masterWallet = NewWalletWithMnemonic(manager);
//		masterWallet = NewReadOnlyMultiSignWallet(manager);
//		masterWallet = NewMultiSignWalletWithMnemonic(manager);
//		masterWallet = NewMultiSignWalletWithPrvKey(manager);
		masterWallets.push_back(masterWallet);

		masterWallet->CreateSubWallet(gMainchainSubWalletID);
		masterWallet->CreateSubWallet(gSidechainSubWalletID);
	}

	for (size_t i = 0; i < masterWallets.size(); ++i) {
		std::vector<ISubWallet *> subWallets = masterWallets[i]->GetAllSubWallets();
		for (size_t j = 0; j < subWallets.size(); ++j) {
			subWallets[j]->AddCallback(new SubWalletCallback());
			logger->debug("[{}:{}] all addresses -> {}",
						  masterWallets[i]->GetId(), subWallets[j]->GetChainId(),
						  subWallets[j]->GetAllAddress(0, 500).dump());

		}
	}
}

int main(int argc, char *argv[]) {

	bool transferDone = true, depositDone = true, withdrawDone = true, registerID = true;

	logger->set_level(spdlog::level::level_enum::debug);
	logger->set_pattern("%m-%d %T.%e %P %t %^%L%$ %n: %v");

	MasterWalletManager *manager = new MasterWalletManager(rootPath);
	if (manager == nullptr) {
		logger->error("MasterWalletManager init fail");
		return -1;
	}

	InitWallets(manager);


	while(1) {
		if (syncSucceed) {
			if (!transferDone) {
				Transafer(manager, gMasterWalletID, gMainchainSubWalletID,
						  "", "EYMVuGs1FscpgmghSzg243R6PzPiszrgj7", 100000000);
				transferDone = true;
			}

			if (!depositDone) {
				Deposit(manager, gMasterWalletID, gMainchainSubWalletID, gMasterWalletID, gSidechainSubWalletID,
						"", "EYMVuGs1FscpgmghSzg243R6PzPiszrgj7", 100000000);
				depositDone = true;
			}

			if (!withdrawDone) {
				Withdraw(manager, gMasterWalletID, gSidechainSubWalletID,
						 "", "EYMVuGs1FscpgmghSzg243R6PzPiszrgj7", 100000000);
				withdrawDone = true;
			}

			if (!registerID) {
				RegisterID(manager, gMasterWalletID, gSidechainSubWalletID);
				registerID = true;
			}

			sleep(10);
		}
	}

	return 0;
}

