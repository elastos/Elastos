// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <stdio.h>
#include <string>
#include <iostream>

#include <Interface/MasterWalletManager.h>
#include <Interface/IMasterWallet.h>
#include <Interface/ISubWallet.h>
#include <Interface/ISubWalletCallback.h>
#include <Interface/IMainchainSubWallet.h>
#include <Interface/IIDChainSubWallet.h>
#include <Interface/ISidechainSubWallet.h>
#include <Interface/ITokenchainSubWallet.h>
#include <spdlog/logger.h>
#include <spdlog/spdlog.h>

using namespace Elastos::ElaWallet;

static std::string memo = "";
static std::string separator = "===========================================================";
static const std::string gMasterWalletID = "WalletID";
static const std::string gMainchainSubWalletID = "ELA";
static const std::string gIDchainSubWalletID = "IDChain";
static const std::string gTokenchainSubWalletID = "TokenChain";
static const std::string rootPath = "Data";
static const std::string payPasswd = "s12345678";
static uint64_t feePerKB = 10000;

static std::shared_ptr<spdlog::logger> logger = spdlog::stdout_color_mt("sample");;
static bool IDChainSyncSucceed = false;
static bool ELASyncSucceed = false;
static bool TokenSyncSucceed = false;

MasterWalletManager *manager = nullptr;

class SubWalletCallback : public ISubWalletCallback {
public:
	~SubWalletCallback() {}
	SubWalletCallback(const std::string &walletID) : _walletID(walletID) {}

	virtual void OnTransactionStatusChanged(
		const std::string &txid,const std::string &status,
		const nlohmann::json &desc,uint32_t confirms) {
	}

	virtual void OnBlockSyncStarted() {
	}

	virtual void OnBlockSyncProgress(uint32_t currentBlockHeight, uint32_t estimatedHeight, time_t lastBlockTime) {
		if (currentBlockHeight >= estimatedHeight) {
			if (_walletID.find(gMainchainSubWalletID) != std::string::npos) {

				ELASyncSucceed = true;
			} else if (_walletID.find(gIDchainSubWalletID) != std::string::npos) {
				IDChainSyncSucceed = true;
			} else if (_walletID.find(gTokenchainSubWalletID) != std::string::npos) {
				TokenSyncSucceed = true;
			}
		}
	}

	virtual void OnBlockSyncStopped() {
	}

	virtual void OnBalanceChanged(const std::string &asset, const std::string &balance) {
	}

	virtual void OnTxPublished(const std::string &hash, const nlohmann::json &result) {
	}

	virtual void OnAssetRegistered(const std::string &asset, const nlohmann::json &info) {
	}

	virtual void OnConnectStatusChanged(const std::string &status) {
	}

private:
	std::string _walletID;

};

static IMasterWallet *NewWalletWithMnemonic() {
	const std::string phrasePassword = "";
	const std::string mnemonic = manager->GenerateMnemonic("english");
	logger->debug("mnemonic -> {}", mnemonic);
	return manager->CreateMasterWallet(gMasterWalletID, mnemonic, phrasePassword, payPasswd, false);
}

static IMasterWallet *ImportWalletWithMnemonic() {
	const std::string phrasePassword = "";
	const std::string mnemonic =
		"闲 齿 兰 丹 请 毛 训 胁 浇 摄 县 诉";
	logger->debug("mnemonic -> {}", mnemonic);
	return manager->ImportWalletWithMnemonic(gMasterWalletID, mnemonic, phrasePassword, payPasswd, false);
}

static IMasterWallet *ImportWebWalletMultiSignKeystore() {
	const std::string backupPasswd = "11111111";
	nlohmann::json keystore = R"(
		{"iv":"hm0lwrO1A8F7rel/0B0rWQ==","v":1,"iter":10000,"ks":128,"ts":64,"mode":"ccm","adata":"","cipher":"aes","salt":"dDXsCNh45Yw=","ct":"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"}
	)"_json;
	return manager->ImportWalletWithKeystore(gMasterWalletID, keystore, backupPasswd, payPasswd);
}

static IMasterWallet *ImportWalletWithKeystore() {
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

static IMasterWallet *ImportReadonlyWallet() {
	nlohmann::json readonlyJson = R"(
		{"Data":"AAABAAAAAQAAAAAAAAAFQklQNDQhA3CneiV6qB9GYphl6488qcsFL8/YdOhkjPvqH78HGwKAIQPZFsIHL9j7VyJOl0fg8eNqLBF2ic7fOeATLzy0+O5nPU4EiLIeA8Lfuo+AAAAAzxzvoN4uRUku2jeAJVYFthp1yAqwWT+wmZmd8ADTec4CZxQL2ehZLEskJ6WAh562nmyurHH1HicJS9Fk27R5nj9OBIiyHgFHjmpvgAAALZ3aydgHewoRsqEstxlmyYMTTDdKbe4NGOpzyKkSUM4xA+BPNFgR5PjYNXJ5vEFLmyAVc2aPX6+sC8s6SUSmddXGAqDXPFoAAAAAA0VMQcBwN1sAAAAAB0lEQ2hhaW4="}
	)"_json;

	return manager->ImportReadonlyWallet(gMasterWalletID, readonlyJson);
}

static IMasterWallet *NewReadOnlyMultiSignWallet() {
	nlohmann::json coSigners = R"(
		["xpub6CLgvYFxzqHDJCWyGDCRQzc5cwCFp4HJ6QuVJsAZqURxmW9QKWQ7hVKzZEaHgCQWCq1aNtqmE4yQ63Yh7frXWUW3LfLuJWBtDtsndGyxAQg",
		 "xpub6CWEYpNZ3qLG1z2dxuaNGz9QQX58wor9ax8AiKBvRytdWfEifXXio1BgaVcT4t7ouP34mnabcvpJLp9rPJPjPx2m6izpHmjHkZAHAHZDyrc"]
		)"_json;
	return manager->CreateMultiSignMasterWallet(gMasterWalletID, coSigners, 2, true);
}

static IMasterWallet *NewMultiSignWalletWithMnemonic() {
	const std::string mnemonic =
		"abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
	const std::string passphrase = "";
	nlohmann::json coSigners = R"(
		["xpub6CWEYpNZ3qLG1z2dxuaNGz9QQX58wor9ax8AiKBvRytdWfEifXXio1BgaVcT4t7ouP34mnabcvpJLp9rPJPjPx2m6izpHmjHkZAHAHZDyrc",
		 "xpub6CLgvYFxzqHDJCWyGDCRQzc5cwCFp4HJ6QuVJsAZqURxmW9QKWQ7hVKzZEaHgCQWCq1aNtqmE4yQ63Yh7frXWUW3LfLuJWBtDtsndGyxAQg"]
		)"_json;
	return manager->CreateMultiSignMasterWallet(gMasterWalletID, mnemonic, passphrase, payPasswd, coSigners, (uint32_t)3, false);
}

// deprecated
static IMasterWallet *NewMultiSignWalletWithPrvKey() {
	nlohmann::json coSigners = R"(
		["xpub6CWEYpNZ3qLG1z2dxuaNGz9QQX58wor9ax8AiKBvRytdWfEifXXio1BgaVcT4t7ouP34mnabcvpJLp9rPJPjPx2m6izpHmjHkZAHAHZDyrc",
		 "xpub6CLgvYFxzqHDJCWyGDCRQzc5cwCFp4HJ6QuVJsAZqURxmW9QKWQ7hVKzZEaHgCQWCq1aNtqmE4yQ63Yh7frXWUW3LfLuJWBtDtsndGyxAQg"]
		)"_json;
	std::string privKey = "xprv9s21ZrQH143K3GJpoapnV8SFfukcVBSfeCficPSGfubmSFDxo1kuHnLisriDvSnRRuL2Qrg5ggqHKNVpxR86QEC8w35uxmGoggxtQTPvfUu";
	return manager->CreateMultiSignMasterWallet(gMasterWalletID, privKey, payPasswd, coSigners, (uint32_t)3, false);
}

static ISubWallet *GetSubWallet(const std::string &masterWalletID, const std::string &subWalletID) {
	IMasterWallet *masterWallet = manager->GetMasterWallet(masterWalletID);
	if (nullptr == masterWallet) {
		return nullptr;
	}

	std::vector<ISubWallet *> subWallets = masterWallet->GetAllSubWallets();
	for (size_t i = 0; i < subWallets.size(); ++i) {
		if (subWallets[i]->GetChainID() == subWalletID) {
			return subWallets[i];
		}
	}

	return nullptr;
}

static void PublishTransaction(ISubWallet *subWallet, const nlohmann::json &tx) {
	nlohmann::json signedTx = subWallet->SignTransaction(tx, payPasswd);
	subWallet->PublishTransaction(signedTx);
}

static void Consolidate(const std::string &masterWalletID, const std::string &subWalletID,
						const std::string &assetID = "a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0") {
	nlohmann::json tx;
	ISubWallet *subWallet = GetSubWallet(masterWalletID, subWalletID);
	if (!subWallet)
		return;

	if (subWalletID == gTokenchainSubWalletID) {
		ITokenchainSubWallet *tokenchainSubWallet = dynamic_cast<ITokenchainSubWallet *>(subWallet);
		tx = tokenchainSubWallet->CreateConsolidateTransaction(assetID, "memo combine utxo");
	} else {
		tx = subWallet->CreateConsolidateTransaction("memo combine utxo");
	}

	PublishTransaction(subWallet, tx);
}

static void Transafer(const std::string &masterWalletID, const std::string &subWalletID,
					  const std::string &from, const std::string &to, uint64_t amount,
					  const std::string &assetID = "a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0") {
	nlohmann::json tx;
	ISubWallet *subWallet = GetSubWallet(masterWalletID, subWalletID);
	if (!subWallet)
		return;

	if (subWalletID == gTokenchainSubWalletID) {
		ITokenchainSubWallet *tokenSubWallet = dynamic_cast<ITokenchainSubWallet *>(subWallet);
		tx = tokenSubWallet->CreateTransaction(from, to, std::to_string(amount), assetID, memo);
	} else {
		tx = subWallet->CreateTransaction(from, to, std::to_string(amount), memo);
	}

	PublishTransaction(subWallet, tx);
}

static void RegisterCr(const std::string &masterWalletID, const std::string &subWalletID) {
	ISubWallet *subWallet = GetSubWallet(masterWalletID, subWalletID);
	if (!subWallet)
		return;

	IMainchainSubWallet *mainchainSubWallet = dynamic_cast<IMainchainSubWallet *>(subWallet);
	if (mainchainSubWallet == nullptr) {
		logger->error("[{}:{}] is not instance of IMainchainSubWallet", masterWalletID, subWalletID);
		return;
	}

	std::string pubKey = mainchainSubWallet->GetCROwnerPublicKey();
	std::string nickName = "black";
	std::string url = "test.com";
	uint64_t location = 86;

	nlohmann::json payload = mainchainSubWallet->GenerateCRInfoPayload(pubKey, nickName, url, location, payPasswd);

	nlohmann::json tx = mainchainSubWallet->CreateRegisterCRTransaction("", payload, std::to_string(500000000000 + 10000),
																		memo);

	PublishTransaction(mainchainSubWallet, tx);
}

static void UpdateCR(const std::string &masterWalletID, const std::string &subWalletID) {
	ISubWallet *subWallet = GetSubWallet(masterWalletID, subWalletID);
	if (!subWallet)
		return;

	IMainchainSubWallet *mainchainSubWallet = dynamic_cast<IMainchainSubWallet *>(subWallet);
	if (mainchainSubWallet == nullptr) {
		logger->error("[{}:{}] is not instance of IMainchainSubWallet", masterWalletID, subWalletID);
		return;
	}

	std::string pubKey = mainchainSubWallet->GetCROwnerPublicKey();
	std::string nickName = "heropan";
	std::string url = "heropan.com";
	uint64_t location = 86;

	nlohmann::json payload = mainchainSubWallet->GenerateCRInfoPayload(pubKey, nickName, url, location, payPasswd);

	nlohmann::json tx = mainchainSubWallet->CreateUpdateCRTransaction("", payload, memo);

	PublishTransaction(mainchainSubWallet, tx);
}

static void UnregisterCR(const std::string &masterWalletID, const std::string &subWalletID) {
	ISubWallet *subWallet = GetSubWallet(masterWalletID, subWalletID);
	if (!subWallet)
		return;

	IMainchainSubWallet *mainchainSubWallet = dynamic_cast<IMainchainSubWallet *>(subWallet);
	if (mainchainSubWallet == nullptr) {
		logger->error("[{}:{}] is not instance of IMainchainSubWallet", masterWalletID, subWalletID);
		return;
	}

	std::string pubKey = mainchainSubWallet->GetCROwnerPublicKey();

	nlohmann::json payload = mainchainSubWallet->GenerateUnregisterCRPayload(pubKey, payPasswd);

	nlohmann::json tx = mainchainSubWallet->CreateUnregisterCRTransaction("", payload, memo);

	PublishTransaction(mainchainSubWallet, tx);
}

static void RetrieveCRTransaction(const std::string &masterWalletID, const std::string &subWalletID) {
	ISubWallet *subWallet = GetSubWallet(masterWalletID, subWalletID);
	if (!subWallet)
		return;

	IMainchainSubWallet *mainchainSubWallet = dynamic_cast<IMainchainSubWallet *>(subWallet);
	if (mainchainSubWallet == nullptr) {
		logger->error("[{}:{}] is not instance of IMainchainSubWallet", masterWalletID, subWalletID);
		return;
	}

	nlohmann::json tx = mainchainSubWallet->CreateRetrieveCRDepositTransaction (std::to_string(500000000000 - 10000), memo);

	PublishTransaction(mainchainSubWallet, tx);
}

static void Vote(const std::string &masterWalletID, const std::string &subWalletID,
				 uint64_t stake, const nlohmann::json &publicKeys) {
	ISubWallet *subWallet = GetSubWallet(masterWalletID, subWalletID);
	if (!subWallet)
		return;

	IMainchainSubWallet *mainchainSubWallet = dynamic_cast<IMainchainSubWallet *>(subWallet);
	if (mainchainSubWallet == nullptr) {
		logger->error("[{}:{}] is not instance of IMainchainSubWallet", masterWalletID, subWalletID);
		return;
	}

	nlohmann::json tx = mainchainSubWallet->CreateVoteProducerTransaction("", std::to_string(stake), publicKeys, memo);
	logger->debug("tx = {}", tx.dump());

	PublishTransaction(mainchainSubWallet, tx);
}

static void VoteCR(const std::string &masterWalletID, const std::string &subWalletID, const nlohmann::json &votes) {
	ISubWallet *subWallet = GetSubWallet(masterWalletID, subWalletID);
	if (!subWallet)
		return;

	IMainchainSubWallet *mainchainSubWallet = dynamic_cast<IMainchainSubWallet *>(subWallet);
	if (mainchainSubWallet == nullptr) {
		logger->error("[{}:{}] is not instance of IMainchainSubWallet", masterWalletID, subWalletID);
		return;
	}

	nlohmann::json tx = mainchainSubWallet->CreateVoteCRTransaction("", votes, memo);

	PublishTransaction(mainchainSubWallet, tx);
}

static void RegisterProducer(const std::string &masterWalletID, const std::string &subWalletID) {
	ISubWallet *subWallet = GetSubWallet(masterWalletID, subWalletID);
	if (!subWallet)
		return;

	IMainchainSubWallet *mainchainSubWallet = dynamic_cast<IMainchainSubWallet *>(subWallet);
	if (mainchainSubWallet == nullptr) {
		logger->error("[{}:{}] is not instance of IMainchainSubWallet", masterWalletID, subWalletID);
		return;
	}

	std::string pubKey = mainchainSubWallet->GetOwnerPublicKey();
	std::string nodePubKey = "0296e28b9bced49e175de2d2ae0e6a03724da9d00241213c988eeb65583a14f0c9";
	std::string nickName = "heropan";
	std::string url = "heropan.com";
	std::string ipAddress = "127.0.0.1:8080";
	uint64_t location = 86;

	nlohmann::json payload = mainchainSubWallet->GenerateProducerPayload(pubKey, nodePubKey, nickName, url, ipAddress,
																		 location, payPasswd);

	nlohmann::json tx = mainchainSubWallet->CreateRegisterProducerTransaction("", payload, std::to_string(500000000000 + 10000),
																			  memo);

	PublishTransaction(mainchainSubWallet, tx);
}

static void UpdateProducer(const std::string &masterWalletID, const std::string &subWalletID) {
	ISubWallet *subWallet = GetSubWallet(masterWalletID, subWalletID);
	if (!subWallet)
		return;

	IMainchainSubWallet *mainchainSubWallet = dynamic_cast<IMainchainSubWallet *>(subWallet);
	if (mainchainSubWallet == nullptr) {
		logger->error("[{}:{}] is not instance of IMainchainSubWallet", masterWalletID, subWalletID);
		return;
	}

	std::string pubKey = mainchainSubWallet->GetOwnerPublicKey();
	std::string nodePubkey = "02b8a9c5ce14bb02c1752597f3711bb290c4a15f6db49c5fe7332e337dab0a10dd";
	std::string nickName = "heropan";
	std::string url = "heropan.com";
	std::string ipAddress = "110.110.110.110";
	uint64_t location = 86;

	nlohmann::json payload = mainchainSubWallet->GenerateProducerPayload(pubKey, nodePubkey, nickName, url, ipAddress,
																		 location, payPasswd);

	nlohmann::json tx = mainchainSubWallet->CreateUpdateProducerTransaction("", payload, memo);

	PublishTransaction(mainchainSubWallet, tx);
}

static void CancelProducer(const std::string &masterWalletID, const std::string &subWalletID) {
	ISubWallet *subWallet = GetSubWallet(masterWalletID, subWalletID);
	if (!subWallet)
		return;

	IMainchainSubWallet *mainchainSubWallet = dynamic_cast<IMainchainSubWallet *>(subWallet);
	if (mainchainSubWallet == nullptr) {
		logger->error("[{}:{}] is not instance of IMainchainSubWallet", masterWalletID, subWalletID);
		return;
	}

	std::string pubKey = mainchainSubWallet->GetOwnerPublicKey();

	nlohmann::json payload = mainchainSubWallet->GenerateCancelProducerPayload(pubKey, payPasswd);

	nlohmann::json tx = mainchainSubWallet->CreateCancelProducerTransaction("", payload, memo);

	PublishTransaction(mainchainSubWallet, tx);
}

static void GetRegisteredProducerInfo(const std::string &masterWalletID, const std::string &subWalletID) {
	ISubWallet *subWallet = GetSubWallet(masterWalletID, subWalletID);
	if (!subWallet)
		return;

	IMainchainSubWallet *mainchainSubWallet = dynamic_cast<IMainchainSubWallet *>(subWallet);
	if (mainchainSubWallet == nullptr) {
		logger->error("[{}:{}] is not instance of IMainchainSubWallet", masterWalletID, subWalletID);
		return;
	}

	nlohmann::json info = mainchainSubWallet->GetRegisteredProducerInfo();
	logger->debug("registered producer info = {}", info.dump());
}

static void GetRegisteredCrInfo(const std::string &masterWalletID, const std::string &subWalletID) {
	ISubWallet *subWallet = GetSubWallet(masterWalletID, subWalletID);
	if (!subWallet)
		return;

	IMainchainSubWallet *mainchainSubWallet = dynamic_cast<IMainchainSubWallet *>(subWallet);
	if (mainchainSubWallet == nullptr) {
		logger->error("[{}:{}] is not instance of IMainchainSubWallet", masterWalletID, subWalletID);
		return;
	}

	mainchainSubWallet->GetRegisteredCRInfo();
}

static void CreateCRProposalTransaction(const std::string &masterWalletID, const std::string &subWalletID) {
	ISubWallet *subWallet = GetSubWallet(masterWalletID, subWalletID);
	if (!subWallet)
		return;

	IMainchainSubWallet *mainchainSubWallet = dynamic_cast<IMainchainSubWallet *>(subWallet);
	if (mainchainSubWallet == nullptr) {
		logger->error("[{}:{}] is not instance of IMainchainSubWallet", masterWalletID, subWalletID);
		return;
	}

	subWallet = GetSubWallet(masterWalletID, gIDchainSubWalletID);
	IIDChainSubWallet *iidChainSubWallet = dynamic_cast<IIDChainSubWallet *>(subWallet);

	if (iidChainSubWallet == nullptr) {
		logger->error("[{}:{}] is not instance of IIDChainSubWallet", masterWalletID, subWalletID);
		return;
	}

	uint8_t type = 0;
	nlohmann::json pubKeyJson = iidChainSubWallet->GetAllPublicKeys(0, 1);
	std::string pubkey = pubKeyJson["PublicKeys"][0].get<std::string>();
	nlohmann::json allDID = iidChainSubWallet->GetAllDID(0, 1);
	std::string crSponsorDID = allDID["DID"][0].get<std::string>();
	std::string draftHash = "a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0";
	nlohmann::json budgets = R"(["324","266","234"])"_json;
	std::string receiptAddress = "Ed8ZSxSB98roeyuRZwwekrnRqcgnfiUDeQ";

	std::string sponsorDID = iidChainSubWallet->GetPublicKeyDID(pubkey);

	std::string digest = mainchainSubWallet->SponsorProposalDigest(type, pubkey, crSponsorDID, draftHash, budgets,
	                                                               receiptAddress);

	std::string sponsorSignature = iidChainSubWallet->SignDigest(sponsorDID, digest, payPasswd);

	digest = mainchainSubWallet->CRSponsorProposalDigest(type, pubkey, crSponsorDID, draftHash, budgets, receiptAddress,
	                                                     sponsorSignature);

	std::string crSignature = iidChainSubWallet->SignDigest(crSponsorDID, digest,  payPasswd);

	nlohmann::json tx = mainchainSubWallet->CreateCRCProposalTransaction(type, pubkey, crSponsorDID, draftHash, budgets,
	                                                                     receiptAddress, sponsorSignature, crSignature,
	                                                                     memo);

	PublishTransaction(mainchainSubWallet, tx);

}

static void CreateVoteCRProposalTransaction(const std::string &masterWalletID, const std::string &subWalletID) {
	ISubWallet *subWallet = GetSubWallet(masterWalletID, subWalletID);
	if (!subWallet)
		return;

	IMainchainSubWallet *mainchainSubWallet = dynamic_cast<IMainchainSubWallet *>(subWallet);
	if (mainchainSubWallet == nullptr) {
		logger->error("[{}:{}] is not instance of IMainchainSubWallet", masterWalletID, subWalletID);
		return;
	}

	nlohmann::json votes = nlohmann::json::parse(
			"{\"109780cf45c7a6178ad674ac647545b47b10c2c3e3b0020266d0707e5ca8af7c\":\"50\",\"92990788d66bf558052d112f5498111747b3e28c55984d43fed8c8822ad9f1a7\":\"40\"}");

	nlohmann::json tx = mainchainSubWallet->CreateVoteCRCProposalTransaction("", votes, memo);

	PublishTransaction(mainchainSubWallet, tx);
}

static void GetVotedList(const std::string &masterWalletID, const std::string &subWalletID) {
	ISubWallet *subWallet = GetSubWallet(masterWalletID, subWalletID);
	if (!subWallet)
		return;

	IMainchainSubWallet *mainchainSubWallet = dynamic_cast<IMainchainSubWallet *>(subWallet);

	mainchainSubWallet->GetVotedProducerList();
	mainchainSubWallet->GetVotedCRList();
}

static void RetrieveDeposit(const std::string &masterWalletID, const std::string &subWalletID) {
	ISubWallet *subWallet = GetSubWallet(masterWalletID, subWalletID);
	if (!subWallet)
		return;

	IMainchainSubWallet *mainchainSubWallet = dynamic_cast<IMainchainSubWallet *>(subWallet);
	if (mainchainSubWallet == nullptr) {
		logger->error("[{}:{}] is not instance of IMainchainSubWallet", masterWalletID, subWalletID);
		return;
	}

	nlohmann::json tx = mainchainSubWallet->CreateRetrieveDepositTransaction(std::to_string(500000000000), memo);

	PublishTransaction(mainchainSubWallet, tx);
}

static void Deposit(const std::string &fromMasterWalletID, const std::string &fromSubWalletID,
					const std::string &toMasterWalletID, const std::string &toSubWalletID,
					const std::string &from, const std::string &sidechainAddress, uint64_t amount) {
	ISubWallet *fromSubWallet = GetSubWallet(fromMasterWalletID, fromSubWalletID);
	if (fromSubWallet == nullptr) {
		logger->error("[{}:{}] subWallet not found", fromMasterWalletID, fromSubWalletID);
		return ;
	}

	IMainchainSubWallet *mainchainSubWallet = dynamic_cast<IMainchainSubWallet *>(fromSubWallet);
	if (mainchainSubWallet == nullptr) {
		logger->error("[{}:{}] is not instance of IMainchainSubWallet!", fromMasterWalletID, fromSubWalletID);
		return ;
	}

	ISubWallet *toSubWallet = GetSubWallet(toMasterWalletID, toSubWalletID);
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

	nlohmann::json tx = mainchainSubWallet->CreateDepositTransaction(from, lockedAddress, std::to_string(amount), sidechainAddress, memo);

	logger->debug("[{}:{}] deposit {} to {}", fromMasterWalletID, fromSubWalletID, amount, sidechainAddress);

	PublishTransaction(fromSubWallet, tx);
}

static void Withdraw(const std::string &fromMasterWalletID, const std::string &fromSubWalletID,
					const std::string &from, const std::string &mainchainAddress, uint64_t amount) {
	ISubWallet *fromSubWallet = GetSubWallet(fromMasterWalletID, fromSubWalletID);
	if (fromSubWallet == nullptr) {
		logger->error("[{}:{}] subWallet not found", fromMasterWalletID, fromSubWalletID);
		return ;
	}

	ISidechainSubWallet *sidechainSubWallet = dynamic_cast<ISidechainSubWallet *>(fromSubWallet);
	if (sidechainSubWallet == nullptr) {
		logger->error("[{}:{}] is not instance of ISidechainSubWallet", fromMasterWalletID, fromSubWalletID);
		return ;
	}

	nlohmann::json tx = sidechainSubWallet->CreateWithdrawTransaction(from, std::to_string(amount), mainchainAddress, memo);

	logger->debug("[{}:{}] withdraw {} to {}", fromMasterWalletID, fromSubWalletID, amount, mainchainAddress);

	PublishTransaction(sidechainSubWallet, tx);
}

static nlohmann::json GetDIDPayloadJson() {
//	nlohmann::json inputJson = R"({"id":"iZFrhZLetd6i6qPu2MsYvE2aKrgw7Af4Ww","operation":"create","publicKey":[{"id": "#primary","publicKey":"03d25d582c485856520c501b2e2f92934eda0232ded70cad9e51cf13968cac22cc"}],
//"credentialSubject":{"id":"iZFrhZLetd6i6qPu2MsYvE2aKrgw7Af4Ww","didName":"myName","name":"H60CZ","nickname":"jHo8AB","alipay":"alipay@223.com","avatar":"img.jpg","birthday":"2019-10-12","descript":"this is simple descript","email":"test@test.com","facebook":"facebook","gender":"male","googleAccount":"google@google.com","homePage":"homePage","microsoftPassport":"MicrosoftPassport","nation":"china","phone":"+8613032454523","twitter":"twitter","wechat":"wechat2333","weibo":"test@sina.com"},
//"expires":"2024-02-10T17:00:00Z"})"_json;
	nlohmann::json inputJson = R"({"id":"iZFrhZLetd6i6qPu2MsYvE2aKrgw7Af4Ww","operation":"create","publicKey":[{"id": "#primary","publicKey":"03d25d582c485856520c501b2e2f92934eda0232ded70cad9e51cf13968cac22cc"}],
"expires":1575104460})"_json;
	nlohmann::json payload;
	ISubWallet *subWallet = GetSubWallet(gMasterWalletID, gIDchainSubWalletID);
	IIDChainSubWallet *idChainSubWallet = dynamic_cast<IIDChainSubWallet *>(subWallet);
	if (idChainSubWallet == nullptr) {
		logger->error("[{}:{}] is not instance of IIdChainSubWallet", gMasterWalletID, gIDchainSubWalletID);
		return payload;
	}
	payload = idChainSubWallet->GenerateDIDInfoPayload(inputJson, payPasswd);

	return payload;
}

static void CreateIDTransaction(const std::string &masterWalletID, const std::string &DIDSubWalletID) {
	IMasterWallet *masterWalelt = manager->GetMasterWallet(masterWalletID);
	if (masterWalelt == nullptr) {
		logger->error("[{}] master wallet not found", masterWalletID);
		return ;
	}

	ISubWallet *subWallet = GetSubWallet(masterWalletID, DIDSubWalletID);
	IIDChainSubWallet *DIDSubWallet = dynamic_cast<IIDChainSubWallet *>(subWallet);
	if (DIDSubWallet == nullptr) {
		logger->error("[{}:{}] is not instance of IIdChainSubWallet", masterWalletID, DIDSubWalletID);
		return ;
	}

	nlohmann::json didPayloadJSON = GetDIDPayloadJson();

	nlohmann::json tx = DIDSubWallet->CreateIDTransaction(didPayloadJSON, memo);

	logger->debug("[{}:{}] create id tx", masterWalletID, DIDSubWalletID);

	PublishTransaction(subWallet, tx);
}

static void CloseWallet(const std::string &masterWalletID, const std::string &subWalletID) {
	IMasterWallet *masterWalelt = manager->GetMasterWallet(masterWalletID);
	ISubWallet *subWallet = GetSubWallet(masterWalletID, subWalletID);

	masterWalelt->DestroyWallet(subWallet);
}

static void OpenWallet(const std::string &masterWalletID, const std::string &subWalletID) {
	IMasterWallet *masterWalelt = manager->GetMasterWallet(masterWalletID);

	masterWalelt->CreateSubWallet(subWalletID);
}

static void TestIDSubWallet() {
	ISubWallet *subWallet = GetSubWallet(gMasterWalletID, gIDchainSubWalletID);

	nlohmann::json j = subWallet->GetAllPublicKeys(0, 10);
	std::vector<std::string> pubkeys = j["PublicKeys"];
	IIDChainSubWallet *iidChainSubWallet = dynamic_cast<IIDChainSubWallet *>(subWallet);
	for (size_t i = 0; i <  pubkeys.size(); ++i) {
		iidChainSubWallet->GetPublicKeyDID(pubkeys[i]);
	}

	iidChainSubWallet->GetAllDID(0, 500);

	iidChainSubWallet->GetResolveDIDInfo(0, 10, "");
}

static void InitWallets() {
	std::vector<IMasterWallet *> masterWallets = manager->GetAllMasterWallets();
	if (masterWallets.size() == 0) {
		IMasterWallet *masterWallet = nullptr;
//		masterWallet = ImportWebWalletMultiSignKeystore();
//		masterWallet = ImportWalletWithKeystore();
//		masterWallet = ImportReadonlyWallet();
		if (masterWallet == nullptr) {
			masterWallet = ImportWalletWithMnemonic();
//			masterWallet = NewWalletWithMnemonic();
//			masterWallet = NewReadOnlyMultiSignWallet();
//			masterWallet = NewMultiSignWalletWithMnemonic();
//			masterWallet = NewMultiSignWalletWithPrvKey();

			masterWallet->CreateSubWallet(gMainchainSubWalletID);
			masterWallet->CreateSubWallet(gIDchainSubWalletID);
//			masterWallet->CreateSubWallet(gTokenchainSubWalletID);
		}
		masterWallets.push_back(masterWallet);
	}

	for (size_t i = 0; i < masterWallets.size(); ++i) {
		masterWallets[i]->ExportReadonlyWallet();
		masterWallets[i]->GetPubKeyInfo();
//		logger->debug("{} xprv -> {}", masterWallets[i]->GetID(), manager->ExportxPrivateKey(masterWallets[i], payPasswd));
		masterWallets[i]->ExportMasterPublicKey();
		std::vector<ISubWallet *> subWallets = masterWallets[i]->GetAllSubWallets();
		for (size_t j = 0; j < subWallets.size(); ++j) {
			std::string walletID = masterWallets[i]->GetID() + ":" + subWallets[j]->GetChainID();
			subWallets[j]->AddCallback(new SubWalletCallback(walletID));
			subWallets[j]->GetBasicInfo();
			subWallets[j]->GetAllAddress(0, 500);
			if (subWallets[j]->GetChainID() == gIDchainSubWalletID) {
				TestIDSubWallet();
			}
		}
	}
}

static void DestroyWallets() {
	std::vector<IMasterWallet *>masterWallets = manager->GetAllMasterWallets();
	if (!masterWallets.empty()) {
		manager->DestroyWallet(masterWallets[0]->GetID());
	}
}

static void GetAllTxSummary(const std::string &masterWalletID, const std::string &subWalletID) {
	ISubWallet *subWallet = GetSubWallet(masterWalletID, subWalletID);
	if (!subWallet)
		return;

	nlohmann::json txSummary = subWallet->GetAllTransaction(0, 300, "");

#if 0
	nlohmann::json txns = txSummary["Transactions"];
	for (nlohmann::json::iterator it = txns.begin(); it != txns.end(); ++it) {
		nlohmann::json tx = subWallet->GetAllTransaction(0, 40, (*it)["TxHash"]);
		logger->debug("tx = {}", tx.dump());
	}
	nlohmann::json cbSummary = subWallet->GetAllCoinBaseTransaction(0, 10000, "");
	logger->debug("[{}:{}] all coinbase tx -> {}", masterWalletID, subWalletID, cbSummary.dump());

	nlohmann::json cbs = cbSummary["Transactions"];
	for (nlohmann::json::iterator it = cbs.begin(); it != cbs.end(); ++it) {
		nlohmann::json cb = subWallet->GetAllCoinBaseTransaction(0, 10000, (*it)["TxHash"]);
		logger->debug("cb = {}", cb.dump());
	}
#endif
	nlohmann::json utxoSummary = subWallet->GetAllUTXOs(0, 1000, "");
}

static std::string GetBalance(const std::string &masterWalletID, const std::string &subWalletID,
					   const std::string &assetID = "a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0") {
	ISubWallet *subWallet = GetSubWallet(masterWalletID, subWalletID);
	if (!subWallet)
		return "0";

	std::string amount;

	if (subWalletID == gTokenchainSubWalletID) {
		ITokenchainSubWallet *tokenSubWallet = dynamic_cast<ITokenchainSubWallet *>(subWallet);
		amount = tokenSubWallet->GetBalance(assetID);
		tokenSubWallet->GetBalanceInfo(assetID);
	} else {
		amount = subWallet->GetBalance();
		subWallet->GetBalanceInfo();
	}

	return amount;
}

static void SyncStart(const std::string &masterWalletID, const std::string &subWalletID) {
	ISubWallet *subWallet = GetSubWallet(masterWalletID, subWalletID);
	if (!subWallet)
		return;

	subWallet->SyncStart();
}

static void SyncStop(const std::string &masterWalletID, const std::string &subWalletID) {
	ISubWallet *subWallet = GetSubWallet(masterWalletID, subWalletID);
	if (!subWallet)
		return;

	subWallet->SyncStop();
}

static void GetAllAssets(const std::string &masterWalletID, const std::string &subWalletID) {
	ISubWallet *subWallet = GetSubWallet(masterWalletID, subWalletID);
	if (!subWallet)
		return;

	ITokenchainSubWallet *tokenSubWallet = dynamic_cast<ITokenchainSubWallet *>(subWallet);
	logger->debug("{}:{} supported assets -> {}", masterWalletID, subWalletID, tokenSubWallet->GetAllAssets().dump());
}

static void ELATest() {
	static bool combineUTXODone = true, transferDone = true, depositDone = true;
	static bool voteDone = true, registerProducer = true, updateProducer = true, cancelProducer = true, retrieveDeposit = true;
	static bool registerCR = true, updateCR = true, unregisterCR = true, retrieveCr = true, voteCR = true;
	static bool createCRProposal = true, voteCRProposal = true;

	logger->debug("ELA {}", separator);
	GetAllTxSummary(gMasterWalletID, gMainchainSubWalletID);
	std::string balance = GetBalance(gMasterWalletID, gMainchainSubWalletID);
	GetVotedList(gMasterWalletID, gMainchainSubWalletID);
	GetRegisteredProducerInfo(gMasterWalletID, gMainchainSubWalletID);
	GetRegisteredCrInfo(gMasterWalletID, gMainchainSubWalletID);

	if (!combineUTXODone) {
		Consolidate(gMasterWalletID, gMainchainSubWalletID);
		combineUTXODone = true;
	}

	if (!transferDone) {
		uint64_t amount = strtol(balance.c_str(), nullptr, 10);
		Transafer(gMasterWalletID, gMainchainSubWalletID,
				  "", "ET42VNGXNUeqJ5yP4iGrqja6qhSEdSQmeP", amount - 10000);
		transferDone = true;
	}

	if (!depositDone) {
		Deposit(gMasterWalletID, gMainchainSubWalletID, gMasterWalletID, gIDchainSubWalletID,
				"", "EYMVuGs1FscpgmghSzg243R6PzPiszrgj7", 100000000);
		depositDone = true;
	}

	if (!voteDone) {
		Vote(gMasterWalletID, gMainchainSubWalletID, 5000000000,
			 nlohmann::json::parse("[\"0205a250b3a96ccc776604fafb84b0f8623fdfda6ec8f42c9154aa727bd95edfe2\",\"03d55285f06683c9e5c6b5892a688affd046940c7161571611ea3a98330f72459f\",\"032459feb52daadef9d6336386bd962c4ca28077c54bda35c83b9430fc9ce7e049\",\"02b6d98b9e8f484e4ea83d5278099be59f945951bb6dc464b792ba0895eab1a774\",\"02f545384070dbee5e259502bd2c22382082c64505ff9df6bb36e3ba44f0607b7e\",\"0368044f3b3582000597d40c9293ea894237a88b2cd55f79a18193399937d22664\",\"022145c89fb500c02ce6b8ba9a51f608cd2c7d1dc99b43f11bdf8589161aa7d690\",\"030d3d2b2c49b7f4f4156f8807471b70a31d33a8ec8b5db7b644e6ae9286d73cb7\",\"024b527700491895b79fc5bfde8a60395307c5416a075503e6ac7d1df61c971c78\",\"02a85be1f6244b40b8778b626bde33e1d666b3b5863f195487e72dc0e2a6af33a1\",\"03b5a108c6e2f2327433b8157c1bfb0c6974b6fa708c2039576952889b9cace3d8\",\"03c0b8a45b51bd3cbb996663681ea79ef58d2925596eaeac3968d157514faffe90\",\"0279d982cda37fa7edc1906ec2f4b3d8da5af2c15723e14f368f3684bb4a1e0889\",\"02a5b07a250fce6147ab6e54febc1f04495f7218ba19fbb4928604e7704838afe4\",\"038659a36232f36f52fbfc67a2f606922c037ec8a53757a04e4e7623943a05fc03\"]"));
		voteDone = true;
	}

	if (!registerProducer) {
		RegisterProducer(gMasterWalletID, gMainchainSubWalletID);
		registerProducer = true;
	}

	if (!updateProducer) {
		UpdateProducer(gMasterWalletID, gMainchainSubWalletID);
		updateProducer = true;
	}

	if (!cancelProducer) {
		CancelProducer(gMasterWalletID, gMainchainSubWalletID);
		cancelProducer = true;
	}

	if (!retrieveDeposit) {
		RetrieveDeposit(gMasterWalletID, gMainchainSubWalletID);
		retrieveDeposit = true;
	}

	if (!registerCR) {
		RegisterCr(gMasterWalletID, gMainchainSubWalletID);
		registerCR = true;
	}

	if (!updateCR) {
		UpdateCR(gMasterWalletID, gMainchainSubWalletID);
		updateCR = true;
	}

	if (!unregisterCR) {
		UnregisterCR(gMasterWalletID, gMainchainSubWalletID);
		unregisterCR = true;
	}

	if (!retrieveCr) {
		RetrieveCRTransaction(gMasterWalletID, gMainchainSubWalletID);
		retrieveCr = true;
	}

	if (!voteCR) {
		VoteCR(gMasterWalletID, gMainchainSubWalletID,
		     nlohmann::json::parse("{\"0205a250b3a96ccc776604fafb84b0f8623fdfda6ec8f42c9154aa727bd95edfe2\":23,\"03d55285f06683c9e5c6b5892a688affd046940c7161571611ea3a98330f72459f\":34}"));
		voteCR = true;
	}

	if (!createCRProposal) {
		CreateCRProposalTransaction(gMasterWalletID, gMainchainSubWalletID);
		createCRProposal = true;
	}

	if (!voteCRProposal) {
		CreateVoteCRProposalTransaction(gMasterWalletID, gMainchainSubWalletID);
		voteCRProposal = true;
	}

	logger->debug("ELA {}", separator);
}

static void RegisterAsset() {
	ISubWallet *subWallet = GetSubWallet(gMasterWalletID, gTokenchainSubWalletID);

	ITokenchainSubWallet *tokenSubWallet = dynamic_cast<ITokenchainSubWallet *>(subWallet);
	nlohmann::json tx = tokenSubWallet->CreateRegisterAssetTransaction("Poon",
				"Description: test spv interface",
				"EPbdmxUVBzfNrVdqJzZEySyWGYeuKAeKqv",
				std::to_string(1000000000000000), 10, memo);
	logger->debug("tx after created = {}", tx.dump());

	PublishTransaction(subWallet, tx);
}

static void TokenTest() {
	static bool transferDone = true, withdrawDone = true, registerDone = true;

	logger->debug("token {}", separator);
	GetAllTxSummary(gMasterWalletID, gTokenchainSubWalletID);
	GetBalance(gMasterWalletID, gTokenchainSubWalletID);
	GetAllAssets(gMasterWalletID, gTokenchainSubWalletID);

	if (!registerDone) {
		RegisterAsset();
		registerDone = true;
	}

	if (!transferDone) {
		Transafer(gMasterWalletID, gTokenchainSubWalletID,
				  "", "EYMVuGs1FscpgmghSzg243R6PzPiszrgj7", 100000000, "c1e10619c17d9091fc2d0b0d741e68396e751e4a3c603d3efb7a04c0038b1238");
		transferDone = true;
	}
	if (!withdrawDone) {
		Withdraw(gMasterWalletID, gTokenchainSubWalletID,
				 "", "EYMVuGs1FscpgmghSzg243R6PzPiszrgj7", 100000000);
		withdrawDone = true;
	}
	logger->debug("token {}", separator);
}

static void DIDTest() {
	static bool transferDone = true, withdrawDone = true, registerID = true;

	logger->debug("DID {}", separator);
	GetAllTxSummary(gMasterWalletID, gIDchainSubWalletID);
	GetBalance(gMasterWalletID, gIDchainSubWalletID);

	if (!transferDone) {
		Transafer(gMasterWalletID, gIDchainSubWalletID,
				  "", "EYMVuGs1FscpgmghSzg243R6PzPiszrgj7", 100000000);
		transferDone = true;
	}

	if (!withdrawDone) {
		Withdraw(gMasterWalletID, gIDchainSubWalletID,
				 "", "EYMVuGs1FscpgmghSzg243R6PzPiszrgj7", 100000000);
		withdrawDone = true;
	}

	if (!registerID) {
		CreateIDTransaction(gMasterWalletID, gIDchainSubWalletID);
		registerID = true;
	}
	logger->debug("DID {}", separator);
}

int main(int argc, char *argv[]) {
	std::string line;
	logger->set_level(spdlog::level::level_enum::debug);
	logger->set_pattern("%m-%d %T.%e %P %t %^%L%$ %n %v");

	manager = new MasterWalletManager(rootPath);
	if (manager == nullptr) {
		logger->error("MasterWalletManager init fail");
		return -1;
	}

	InitWallets();

	while(line != "quit") {
		std::cin >> line;
		logger->info(">>> {} <<<", line);

		if (line == "test" || line == "t") {
			ELATest();
			TokenTest();
			DIDTest();
		} else if (line == "connect" || line == "c") {
			// trigger p2p connect now
			SyncStart(gMasterWalletID, gMainchainSubWalletID);
			SyncStart(gMasterWalletID, gIDchainSubWalletID);
		} else if (line == "stop" || line == "s") {
			SyncStop(gMasterWalletID, gMainchainSubWalletID);
			SyncStop(gMasterWalletID, gIDchainSubWalletID);
		} else if (line == "close") {
			CloseWallet(gMasterWalletID, gMainchainSubWalletID);
			CloseWallet(gMasterWalletID, gIDchainSubWalletID);
			//CloseWallet(gMasterWalletID, gTokenchainSubWalletID);
		} else if (line == "open") {
			OpenWallet(gMasterWalletID, gMainchainSubWalletID);
			OpenWallet(gMasterWalletID, gIDchainSubWalletID);
			//OpenWallet(gMasterWalletID, gTokenchainSubWalletID);
		} else if (line == "quit" || line == "q") {
			break;
		}
	}

	DestroyWallets();

	return 0;
}

