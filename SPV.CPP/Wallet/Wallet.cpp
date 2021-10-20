/*
 * Copyright (c) 2019 Elastos Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>
#include <limits.h>
#include <getopt.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include <signal.h>
#include <termios.h>

#include <MasterWalletManager.h>
#include <IMasterWallet.h>
#include <ISubWallet.h>
#include <IElastosBaseSubWallet.h>
#include <IMainchainSubWallet.h>
#include <IIDChainSubWallet.h>
#include <IEthSidechainSubWallet.h>
#include "RPCHelper.h"

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <IBTCSubWallet.h>

using namespace Elastos::ElaWallet;

#define ERRNO_APP -255
#define ERRNO_CMD -1

#define SELA_PER_ELA 100000000UL
static const std::string CHAINID_ELA = "ELA";
static const std::string CHAINID_ID = "IDChain";
static std::string walletRoot;
static std::string network = "MainNet";
static nlohmann::json config = R"({
                "BTC": { },
             	"ELA": { },
             	"IDChain": { },
             	"ETHSC": { "ChainID": 20, "NetworkID": 20 },
             	"ETHDID": { "ChainID": 20, "NetworkID": 20 },
             	"ETHHECO": { "ChainID": 128, "NetworkID": 128 }
             })"_json;

static MasterWalletManager *manager = nullptr;
static IMasterWallet *currentWallet = nullptr;

static bool verboseMode = false;
static const char *SplitLine = "------------------------------------------------------------------------------------------------------------------------";

static std::string ETHSC_TESTNET_API_URL = "http://api-testnet.elastos.io:21634/api/1/eth/history";
static std::string ETHSC_MAINNET_API_URL = "http://api.elastos.io:20634/api/1/eth/history";

#define checkCurrentWallet() do { \
	if (!currentWallet) { \
		std::cerr << "No wallet actived" << std::endl; \
		return ERRNO_APP; \
	} \
} while (0);

#define checkParam(request_argc) do { \
	if (argc != request_argc) { \
		invalidCmdError(); \
		return ERRNO_CMD; \
	} \
} while (0)

#define getSubWallet(subWallet, masterWallet, chainID) do { \
	subWallet = dynamic_cast<typeof(subWallet)>(masterWallet->GetSubWallet(chainID)); \
	if (!subWallet) { \
		std::cerr << chainID << " not opened" << std::endl; \
		return ERRNO_APP; \
	} \
} while (0)

static struct termios enableLongInput() {
	static struct termios told;

	if (isatty(STDIN_FILENO)) {
		tcgetattr(STDIN_FILENO, &told);
		struct termios tnew = told;
		tnew.c_lflag &= ~ICANON;
		tcsetattr(STDIN_FILENO, TCSAFLUSH, &tnew);
	}

	return told;
}

static void recoveryTTYSetting(const struct termios *t) {
	if (isatty(STDIN_FILENO)) {
		tcsetattr(STDIN_FILENO, TCSAFLUSH, t);
	}
}

static int createPassword(const std::string &comment, std::string &password, bool canbeEmpty) {
	std::string prompt, p1, p2;

	int tryagain = 3;
	while (tryagain--) {
		prompt = "Enter " + comment + " : ";
		p1 = getpass(prompt.c_str());

		prompt = "Enter same " + comment + " again: ";
		p2 = getpass(prompt.c_str());
		if (p1 == p2) {
			if (!canbeEmpty && p1.empty()) {
				std::cerr << comment << " should not be empty, try again" << std::endl;
			} else {
				break;
			}
		} else {
			std::cerr << comment << " not match, try again" << std::endl;
		}
	}

	if (tryagain <= 0)
		return -1;

	password = p1;
	return 0;
}

static int createPaymentPassword(std::string &password) {
	return createPassword("payment password", password, false);
}

static int createBackupPassword(std::string &password) {
	return createPassword("backup password", password, false);
}

static int createPassphrase(std::string &passphrase) {
	return createPassword("passphrase", passphrase, true);
}

static void signAndShowTx(ISubWallet *subWallet, const nlohmann::json &tx, const std::string &payPasswd = "") {
	std::string password = payPasswd;

	std::cout << "Fee: " << tx["Fee"] << std::endl;
	if (payPasswd.empty())
		password = getpass("Enter payment password: ");

	nlohmann::json signedTx = subWallet->SignTransaction(tx, password);
    std::cout << signedTx.dump() << std::endl;
}

static std::string convertAmount(const std::string &amount) {
	return std::to_string((uint64_t) (std::stod(amount) * SELA_PER_ELA));
}

static std::string convertETHAmount(const std::string &amount) {
	return std::to_string((uint64_t) (std::stod(amount) * 1000000000000000000llU));
}

static void walletInit(void) {
	auto masterWalletIDs = manager->GetAllMasterWalletID();
	if (!currentWallet && !masterWalletIDs.empty()) {
		currentWallet = manager->GetMasterWallet(masterWalletIDs[0]);

		currentWallet->GetAllSubWallets();
	}
}

static void walletCleanup(void) {
	auto masterWalletIDs = manager->GetAllMasterWalletID();
	for (const std::string &masterWalletID : masterWalletIDs) {
		if (manager->WalletLoaded(masterWalletID)) {
			IMasterWallet *masterWallet = manager->GetMasterWallet(masterWalletID);
			masterWallet->GetAllSubWallets();
		}
	}

	manager->FlushData();
	currentWallet = nullptr;
}

static void invalidCmdError() {
	std::cerr << "Invalid command syntax." << std::endl;
}

static void exceptionError(const std::exception &e) {
	std::cerr << "Exception: " << e.what() << std::endl;
}

// create walletName
static int create(int argc, char *argv[]) {
	checkParam(2);

	try {
		std::string walletName = argv[1];
		if (nullptr != manager->GetMasterWallet(walletName)) {
			std::cerr << "Wallet '" << walletName << "' already exist." << std::endl;
			return ERRNO_APP;
		}

		std::cout << "What mnemonic language would you like?" << std::endl;
		std::cout << "English (Default)" << std::endl;
		std::cout << "Chinese" << std::endl;
		std::cout << "Japanese" << std::endl;
		std::cout << "French" << std::endl;
		std::cout << "Spanish" << std::endl;
		std::cout << "Enter language (empty for default): ";

		std::string language;
		std::getline(std::cin, language);
		if (language.empty())
			language = "English";

		int wordCount = 12;
		std::cout << "How many mnemonic word count would you like?" << std::endl;
		std::cout << "12 (Default)" << std::endl;
		std::cout << "15" << std::endl;
		std::cout << "18" << std::endl;
		std::cout << "21" << std::endl;
		std::cout << "24" << std::endl;
		std::cout << "Enter word count (empty for default): ";

		std::string wordCountString;
		std::getline(std::cin, wordCountString);
		if (!wordCountString.empty())
			wordCount = std::stoi(wordCountString);

		const std::string mnemonic = manager->GenerateMnemonic(language, wordCount);

		std::cout << "Please write down the following mnemonic words." << std::endl;
		std::cout << mnemonic << std::endl;
		std::cout << "Then press enter to continue...";
		std::cin.get();

		std::string password, passphrase;

		if (0 > createPassphrase(passphrase)) {
			std::cerr << "Create failed!" << std::endl;
			return ERRNO_APP;
		}

		if (0 > createPaymentPassword(password)) {
			std::cerr << "Create failed!" << std::endl;
			return ERRNO_APP;
		}

		IMasterWallet *masterWallet = manager->CreateMasterWallet(walletName,
																  mnemonic, passphrase, password, false);
		if (!masterWallet) {
			std::cerr << "Create master wallet failed." << std::endl;
			return ERRNO_APP;
		}

		ISubWallet *subWallet = masterWallet->CreateSubWallet(CHAINID_ELA);
		if (!subWallet) {
			std::cerr << "Create main chain wallet failed." << std::endl;
			return ERRNO_APP;
		}

		std::cout << "Wallet create success." << std::endl;

		currentWallet = masterWallet;
	} catch (const std::exception &e) {
		exceptionError(e);
		return ERRNO_APP;
	}

	return 0;
}

// import walletName (m[nemonic] | k[eystore]) [keystoreFilePath]
static int import(int argc, char *argv[]) {
	if (argc != 3 && argc != 4) {
		invalidCmdError();
		return ERRNO_CMD;
	}

	std::string walletName = argv[1];
	std::string importWhat = argv[2];

	try {
		IMasterWallet *masterWallet = manager->GetMasterWallet(walletName);
		if (masterWallet) {
			std::cerr << "Wallet '" << walletName << "' already exist." << std::endl;
			return ERRNO_APP;
		}

		if (importWhat == "mnemonic" || importWhat == "m") {
			std::string mnemonic;
			std::cout << "Enter mnemonic: ";
			std::getline(std::cin, mnemonic);

			std::string password, passphrase;
			if (0 > createPassphrase(passphrase)) {
				std::cerr << "Create failed!" << std::endl;
				return ERRNO_APP;
			}

			if (0 > createPaymentPassword(password)) {
				std::cerr << "Create failed!" << std::endl;
				return ERRNO_APP;
			}

			masterWallet = manager->ImportWalletWithMnemonic(walletName, mnemonic, passphrase, password, false);
			if (!masterWallet) {
				std::cerr << "Import master wallet failed." << std::endl;
				return ERRNO_APP;
			}

			ISubWallet *subWallet = masterWallet->CreateSubWallet(CHAINID_ELA);
			if (!subWallet) {
				std::cerr << "Create main chain wallet failed." << std::endl;
				return ERRNO_APP;
			}
		} else if (importWhat == "keystore" || importWhat == "k") {
			std::string keystore;
			if (argc == 4) {
				std::string filepath = argv[3];
				std::ifstream is(filepath);

				std::string line;
				while (!is.eof()) {
					is >> line;
					keystore += line;
					line.clear();
				}
			} else {
				struct termios told = enableLongInput();
				std::cout << "Enter keystore: ";
				std::getline(std::cin, keystore);
				recoveryTTYSetting(&told);
			}

			std::string backupPassword = getpass("Enter backup password: ");

			std::string password;
			if (0 > createPaymentPassword(password)) {
				std::cerr << "Import wallet failed!" << std::endl;
				return ERRNO_APP;
			}

			masterWallet = manager->ImportWalletWithKeystore(walletName, nlohmann::json::parse(keystore),
															 backupPassword, password);
			if (!masterWallet) {
				std::cerr << "Import wallet failed!" << std::endl;
				return ERRNO_APP;
			}
		} else {
			invalidCmdError();
			return ERRNO_APP;
		}

		currentWallet = masterWallet;

		auto subWallets = masterWallet->GetAllSubWallets();

		std::cout << "Wallet import success." << std::endl;
	} catch (const std::exception &e) {
		exceptionError(e);
		return ERRNO_APP;
	}
	return 0;
}

// remove walletName
static int remove(int argc, char *argv[]) {
	checkParam(2);

	std::string walletName = argv[1];

	try {
		auto masterWallet = manager->GetMasterWallet(walletName);
		if (!masterWallet) {
			std::cerr << walletName << " not found" << std::endl;
			return ERRNO_APP;
		}

		bool needUpdate = false;
		if (masterWallet == currentWallet) {
			needUpdate = true;
		}

		std::string password = getpass("Enter payment password: ");
		if (!masterWallet->VerifyPayPassword(password)) {
			std::cerr << "Wrong password!" << std::endl;
			return ERRNO_APP;
		}

		manager->DestroyWallet(walletName);
		std::cout << "Wallet '" << walletName << "' removed." << std::endl;

		if (needUpdate)  {
			auto masterWalletIDs = manager->GetAllMasterWalletID();
			if (masterWalletIDs.empty()) {
				currentWallet = nullptr;
			} else {
				currentWallet = manager->GetMasterWallet(masterWalletIDs[0]);
			}
		}
	} catch (const std::exception &e) {
		exceptionError(e);
		return ERRNO_APP;
	}
	return 0;
}

// list [masterWalletID]
static int list(int argc, char *argv[]) {
    if (argc == 1) {
        std::vector<std::string> walletIDs = manager->GetAllMasterWalletID();
        for (const std::string &walletName : walletIDs)
            std::cout << walletName << std::endl;
    } else if (argc == 2) {
        std::string walletName = argv[1];
        IMasterWallet *masterWallet = manager->GetMasterWallet(walletName);
        if (masterWallet == nullptr) {
            std::cerr << walletName << " not found" << std::endl;
            return ERRNO_APP;
        }

        std::vector<ISubWallet *> subWallets = masterWallet->GetAllSubWallets();
        for (ISubWallet *subWallet : subWallets)
            std::cout << subWallet->GetChainID() << std::endl;
    } else {
        return ERRNO_APP;
    }

    return 0;
}

// switch walletName
static int _switch(int argc, char *argv[]) {
	checkParam(2);

	const std::string walletName = argv[1];
	try {
		bool loaded = manager->WalletLoaded(walletName);
		auto masterWallet = manager->GetMasterWallet(walletName);
		if (masterWallet == nullptr) {
			std::cerr << walletName << " not found" << std::endl;
			return ERRNO_APP;
		}

		currentWallet = masterWallet;
		if (!loaded) {
			auto subWallets = currentWallet->GetAllSubWallets();
		}
	} catch (const std::exception &e) {
		exceptionError(e);
		return ERRNO_APP;
	}
	return 0;
}

// address chainID index count [internal]
static int address(int argc, char *argv[]) {
	checkCurrentWallet();

	bool internal;
	if (argc == 4) {
	    internal = false;
	} else if (argc == 5 && std::string(argv[4]) == "internal") {
		internal = true;
	} else {
		invalidCmdError();
		return ERRNO_CMD;
	}

    std::string chainID = argv[1];
	uint32_t index = strtol(argv[2], NULL, 0);
	uint32_t count = strtol(argv[3], NULL, 0);

	try {
		ISubWallet *subWallet;
		getSubWallet(subWallet, currentWallet, chainID);

        nlohmann::json jtmp = subWallet->GetAddresses(index, count, internal);
        std::cout << jtmp.dump(4) << std::endl;

        if (chainID == "BTC") {
            IBTCSubWallet *btcSubWallet;
            getSubWallet(btcSubWallet, currentWallet, chainID);
            jtmp = btcSubWallet->GetLegacyAddresses(index, count, internal);
            std::cout << "Legacy addresses: " << std::endl;
            std::cout << jtmp.dump(4) << std::endl;
        }
	} catch (const std::exception &e) {
		exceptionError(e);
		return ERRNO_APP;
	}

	return 0;
}

// open chainID
static int _open(int argc, char *argv[]) {
	checkParam(2);
	checkCurrentWallet();

	std::string chainID = argv[1];
	try {
		currentWallet->CreateSubWallet(chainID);
	} catch (const std::exception &e) {
		exceptionError(e);
		return ERRNO_APP;
	}
	return 0;
}

// close chainID
static int _close(int argc, char *argv[]) {
	checkParam(2);
	checkCurrentWallet();

	std::string chainID = argv[1];
	try {
		currentWallet->DestroyWallet(chainID);
	} catch (const std::exception &e) {
		exceptionError(e);
		return ERRNO_APP;
	}
	return 0;
}

// deposit chainID amount address
static int deposit(int argc, char *argv[]) {
	checkParam(4);
	checkCurrentWallet();

	std::string chainID = argv[1];
	std::string amount = convertAmount(argv[2]);
	std::string sideChainAddress = argv[3];

	try {
		IMainchainSubWallet *subWallet;
		getSubWallet(subWallet, currentWallet, CHAINID_ELA);

		std::vector<std::string> supportChainID = currentWallet->GetSupportedChains();
		auto findChainID = std::find(supportChainID.begin(), supportChainID.end(), chainID);
		if (chainID == CHAINID_ELA || findChainID == supportChainID.end()) {
			std::cerr << "Invalid chainID"  << std::endl;
			return ERRNO_APP;
		}

		nlohmann::json tx = subWallet->CreateDepositTransaction(
			nlohmann::json(), chainID, amount, sideChainAddress, "", "10000", "");

		std::cout << "Top up " << sideChainAddress << ":" << amount << " to " << chainID << std::endl;

		signAndShowTx(subWallet, tx);
	} catch (const std::exception &e) {
		exceptionError(e);
		return ERRNO_APP;
	}
	return 0;
}

// withdraw amount address
static int withdraw(int argc, char *argv[]) {
	checkParam(3);
	checkCurrentWallet();

	std::string amount = convertAmount(argv[1]);
	std::string mainChainAddress = argv[2];

	try {
		std::vector<std::string> supportChainID = currentWallet->GetSupportedChains();
		auto stripChainID = std::find(supportChainID.begin(), supportChainID.end(), CHAINID_ELA);
		if (stripChainID != supportChainID.end())
			supportChainID.erase(stripChainID);

		if (supportChainID.empty()) {
			std::cerr << "No support side chain found!" << std::endl;
			return ERRNO_APP;
		}

		std::string chainID;
		if (supportChainID.size() > 1) {
			std::cout << "Support side chain ID: " << std::endl;
			for (const std::string &id : supportChainID) {
				std::cout << "  " << id << std::endl;
			}

			std::cout << "Which side chain do you want to withdraw from: ";
			std::getline(std::cin, chainID);
		} else {
			chainID = supportChainID[0];
		}

		ISidechainSubWallet *subWallet;
		getSubWallet(subWallet, currentWallet, chainID);

		nlohmann::json tx = subWallet->CreateWithdrawTransaction(
			nlohmann::json(), amount, mainChainAddress, "10000", "");

		std::cout << "Withdraw " << mainChainAddress << ":" << amount << " from " << chainID << std::endl;

		signAndShowTx(subWallet, tx);
	} catch (const std::exception &e) {
		exceptionError(e);
		return ERRNO_APP;
	}
	return 0;
}

// didtx [dodDocFilePath]
static int didtx(int argc, char *argv[]) {
	checkCurrentWallet();

	try {
		IIDChainSubWallet *subWallet;
		getSubWallet(subWallet, currentWallet, CHAINID_ID);

		std::string didDoc;
		nlohmann::json payload;
		if (argc == 2) {
			std::string filepath = argv[1];
			std::ifstream is(filepath);
			if (!is.good()) {
				std::cout << "file '" << filepath << "' do not exist!" << std::endl;
				return ERRNO_APP;
			}
			is >> payload;
		} else {
			struct termios told = enableLongInput();
			std::cout << "Enter id document: ";
			std::cin >> payload;
			recoveryTTYSetting(&told);
		}

		nlohmann::json tx = subWallet->CreateIDTransaction(nlohmann::json(), payload, "", "0");
		signAndShowTx(subWallet, tx);
	} catch (const std::exception &e) {
		exceptionError(e);
		return ERRNO_APP;
	}
	return 0;
}

// didsign DID digest
static int didsign(int argc, char *argv[]) {
	checkParam(3);
	checkCurrentWallet();

	std::string did = argv[1];
	std::string digest = argv[2];
	try {
		IIDChainSubWallet *subWallet;
		getSubWallet(subWallet, currentWallet, CHAINID_ID);

		std::string password = getpass("Enter payment password: ");
		std::string signature = subWallet->SignDigest(did, digest, password);
		std::cout << signature << std::endl;
	} catch (const std::exception &e) {
		exceptionError(e);
		return ERRNO_APP;
	}
	return 0;
}

// did
static int did(int argc, char *argv[]) {
	checkCurrentWallet();

	try {
		IIDChainSubWallet *subWallet;
		getSubWallet(subWallet, currentWallet, CHAINID_ID);

		nlohmann::json didlist = subWallet->GetDID(0, 20);
		std::cout << didlist.dump(4) << std::endl;
	} catch (const std::exception &e) {
		exceptionError(e);
		return ERRNO_APP;
	}

	return 0;
}

// cid
static int _cid(int argc, char *argv[]) {
	checkCurrentWallet();

	try {
		IIDChainSubWallet *subWallet;
		getSubWallet(subWallet, currentWallet, CHAINID_ID);

		nlohmann::json cidlist = subWallet->GetCID(0, 20);
		std::cout << cidlist.dump(4) << std::endl;
	} catch (const std::exception &e) {
		exceptionError(e);
		return ERRNO_APP;
	}

	return 0;
}

// pubkey chainID index count [internal]
static int publickeys(int argc, char *argv[]) {
	checkCurrentWallet();

    bool internal;
    if (argc == 4) {
        internal = false;
    } else if (argc == 5 && std::string(argv[4]) == "internal") {
        internal = true;
    } else {
        invalidCmdError();
        return ERRNO_CMD;
    }

    std::string chainID = argv[1];
    uint32_t index = strtol(argv[2], NULL, 0);
    uint32_t count = strtol(argv[3], NULL, 0);

	try {
		ISubWallet *subWallet;
		getSubWallet(subWallet, currentWallet, chainID);

		nlohmann::json list = subWallet->GetPublicKeys(index, count, internal);
		std::cout << list.dump(4) << std::endl;
	} catch (const std::exception &e) {
		exceptionError(e);
		return ERRNO_APP;
	}

	return 0;
}

// register (cr | dpos)
static int _register(int argc, char *argv[]) {
	checkParam(2);
	checkCurrentWallet();

	std::string registerWhat = argv[1];

	try {
		IMainchainSubWallet *subWallet;
		getSubWallet(subWallet, currentWallet, CHAINID_ELA);

		nlohmann::json tx;
		std::string password;

		if (registerWhat == "cr") {
			IIDChainSubWallet *idSubWallet;
			getSubWallet(idSubWallet, currentWallet, CHAINID_ID);

			std::string crPublicKey, nickName, url, did, cid;
			uint64_t location;

			crPublicKey = idSubWallet->GetPublicKeys(0, 1)["PublicKeys"][0];
			did = idSubWallet->GetPublicKeyDID(crPublicKey);
			cid = idSubWallet->GetPublicKeyCID(crPublicKey);

			std::cout << "DID public key: " << crPublicKey << std::endl;
			std::cout << "DID: " << did << std::endl;
			std::cout << "CID: " << cid << std::endl;

			std::cout << "Enter nick name: ";
			std::getline(std::cin, nickName);

			std::cout << "Enter url: ";
			std::getline(std::cin, url);

			std::cout << "Enter location code (example 86): ";
			std::cin >> location;

			nlohmann::json payload = subWallet->GenerateCRInfoPayload(crPublicKey, did, nickName, url, location);

			std::string digest = payload["Digest"].get<std::string>();

			password = getpass("Enter payment password: ");

			std::string signature = idSubWallet->SignDigest(cid, digest, password);
			payload["Signature"] = signature;

			tx = subWallet->CreateRegisterCRTransaction(nlohmann::json(), payload, convertAmount("5000"), "10000", "memo");
		} else if (registerWhat == "dpos") {
			std::string ownerPubkey = subWallet->GetOwnerPublicKey();
			std::string nickName, nodePubkey, url;
			uint64_t location;

			std::cout << "Owner public key: " << ownerPubkey << std::endl;
			std::cout << "Enter node public key (empty will set to owner public key): ";
			std::getline(std::cin, nodePubkey);
			if (nodePubkey.empty())
				nodePubkey = ownerPubkey;

			std::cout << "Enter nick name: ";
			std::getline(std::cin, nickName);

			std::cout << "Enter url: ";
			std::getline(std::cin, url);

			std::cout << "Enter location code (example 86): ";
			std::cin >> location;

			password = getpass("Enter payment password: ");
			nlohmann::json payload = subWallet->GenerateProducerPayload(ownerPubkey, nodePubkey, nickName, url, "", location, password);
			tx = subWallet->CreateRegisterProducerTransaction(nlohmann::json(), payload, convertAmount("5000"), "10000", "memo");
		} else {
			invalidCmdError();
			return ERRNO_APP;
		}

		signAndShowTx(subWallet, tx, password);
	} catch (const std::exception &e) {
		exceptionError(e);
		return ERRNO_APP;
	}
	return 0;
}

// unregister (cr | dpos)
static int unregister(int argc, char *argv[]) {
	checkParam(2);
	checkCurrentWallet();

	std::string registerWhat = argv[1];

	try {
		IMainchainSubWallet *subWallet;
		getSubWallet(subWallet, currentWallet, CHAINID_ELA);

		nlohmann::json tx;
		std::string password;

		if (registerWhat == "cr") {
			IIDChainSubWallet *idSubWallet;
			getSubWallet(idSubWallet, currentWallet, CHAINID_ID);

			std::string cid = idSubWallet->GetCID(0, 1)["DID"][0];

			nlohmann::json payload = subWallet->GenerateUnregisterCRPayload(cid);
			std::string digest = payload["Digest"].get<std::string>();

			password = getpass("Enter payment password: ");

			std::string signature = idSubWallet->SignDigest(cid, digest, password);
			payload["Signature"] = signature;

			tx = subWallet->CreateUnregisterCRTransaction(nlohmann::json(), payload, "10000", "memo");
		} else if (registerWhat == "dpos") {
			std::string ownerPubkey = subWallet->GetOwnerPublicKey();
			password = getpass("Enter payment password: ");
			nlohmann::json payload = subWallet->GenerateCancelProducerPayload(ownerPubkey, password);
			tx = subWallet->CreateCancelProducerTransaction(nlohmann::json(), payload, "10000", "memo");
		} else {
			invalidCmdError();
			return ERRNO_APP;
		}

		signAndShowTx(subWallet, tx, password);
	} catch (const std::exception &e) {
		exceptionError(e);
		return ERRNO_APP;
	}
	return 0;
}

// basicinfo [chainID]
static int basic_info(int argc, char *argv[]) {
	if (argc != 1 && argc != 2) {
		invalidCmdError();
		return ERRNO_CMD;
	}
	checkCurrentWallet();

	try {
		if (argc == 1) {
			std::cout << currentWallet->GetBasicInfo().dump(4) << std::endl;
		} else {
			std::string chainID = argv[1];
			nlohmann::json info = currentWallet->GetSubWallet(chainID)->GetBasicInfo();
			std::cout << info.dump(4) << std::endl;
		}
	} catch (const std::exception &e) {
		exceptionError(e);
		return ERRNO_APP;
	}
	return 0;
}

// retrieve (cr | dpos)
static int retrieve(int argc, char *argv[]) {
	checkParam(2);
	checkCurrentWallet();

	std::string registerWhat = argv[1];

	try {
		IMainchainSubWallet *subWallet;
		getSubWallet(subWallet, currentWallet, CHAINID_ELA);

		nlohmann::json tx;
		if (registerWhat == "cr") {
			IIDChainSubWallet *idSubWallet;
			getSubWallet(idSubWallet, currentWallet, CHAINID_ID);

			std::string crPublicKey = idSubWallet->GetPublicKeys(0, 1)["PublicKeys"][0];
			std::string cid = idSubWallet->GetPublicKeyCID(crPublicKey);

			std::cout << "DID public key: " << crPublicKey << std::endl;
			std::cout << "CID: " << cid << std::endl;

			std::cout << "Enter retrieve amount: ";
			std::string amount;
			std::cin >> amount;

			tx = subWallet->CreateRetrieveCRDepositTransaction(nlohmann::json(), "500000000000", "10000", "memo");

		} else if (registerWhat == "dpos") {

			std::cout << "Enter retrieve amount: ";
			std::string amount;
			std::cin >> amount;

			tx = subWallet->CreateRetrieveDepositTransaction(nlohmann::json(), "500000000000", "10000", "memo");
		} else {
			invalidCmdError();
			return ERRNO_APP;
		}

		signAndShowTx(subWallet, tx);
	} catch (const std::exception &e) {
		exceptionError(e);
		return ERRNO_APP;
	}
	return 0;
}

// proposal
static int proposal(int argc, char *argv[]) {
	checkParam(1);
	checkCurrentWallet();

	try {
		IMainchainSubWallet *subWallet;
		getSubWallet(subWallet, currentWallet, CHAINID_ELA);

		IIDChainSubWallet *idSubWallet;
		getSubWallet(idSubWallet, currentWallet, CHAINID_ID);

		nlohmann::json payload;

		int type = 0;
		std::cout << "Select proposal type: " << std::endl;
		std::cout << "1. Normal" << std::endl;
		std::cout << "2. ELIP" << std::endl;
		std::cout << "3. Flow ELIP" << std::endl;
		std::cout << "4. Info ELIP" << std::endl;
		std::cout << "5. Mainchain Upgrade Code" << std::endl;
		std::cout << "6. Sidechain Upgrade Code" << std::endl;
		std::cout << "7. Register Sidechain" << std::endl;
		std::cout << "8. Secretary General" << std::endl;
		std::cout << "9. Change Owner" << std::endl;
		std::cout << "10. Close Proposal" << std::endl;
		std::cout << "11. Dapp Consensus" << std::endl;
		std::cin >> type;

		switch (type) {
			case 1: payload["Type"] = 0x0000; break;
			case 2: payload["Type"] = 0x0100; break;
			case 3: payload["Type"] = 0x0101; break;
			case 4: payload["Type"] = 0x0102; break;
			case 5: payload["Type"] = 0x0200; break;
			case 6: payload["Type"] = 0x0300; break;
			case 7: payload["Type"] = 0x0301; break;
			case 8: payload["Type"] = 0x0400; break;
			case 9: payload["Type"] = 0x0401; break;
			case 10: payload["Type"] = 0x0402; break;
			case 11: payload["Type"] = 0x0500; break;
			default: std::cout << "invalid input" << std::endl;
				return ERRNO_APP;
		}

		std::string categoryData;
		std::cout << "Enter category data (limit bytes 4096): ";
		std::cin >> categoryData;
		payload["CategoryData"] = categoryData;

		std::string ownerPubkey;
		std::cout << "Enter owner public key: ";
		std::cin >> ownerPubkey;
		payload["OwnerPublicKey"] = ownerPubkey;

		std::string draftHash;
		std::cout << "Enter proposal draft hash: ";
		std::cin >> draftHash;
		payload["DraftHash"] = draftHash;

		std::string recipient;
		std::cout << "Enter recipient: ";
		std::cin >> recipient;
		payload["Recipient"] = recipient;

		std::cout << "Enter budgets: " << std::endl;

		nlohmann::json budgets;
		std::string continueAdd = "y";
		while (continueAdd == "y" || continueAdd == "yes") {
			nlohmann::json budget;
			std::cout << "Select Budget Type: " << std::endl;
			std::cout << "1. Imprest" << std::endl;
			std::cout << "2. NormalPayment" << std::endl;
			std::cout << "3. FinalPayment" << std::endl;
			std::cin >> type;
			if (type < 1 || type > 3) {
				std::cout << "invalid input" << std::endl;
				return ERRNO_APP;
			}
			budget["Type"] = type - 1;

			uint8_t stage;
			std::cout << "Enter stage [0, 128): ";
			std::cin >> stage;
			if (stage >= 128) {
				std::cout << "invalid input" << std::endl;
				return ERRNO_APP;
			}
			budget["Stage"] = stage;

			std::string amount;
			std::cout << "Enter amount (eg: 1.23 means 1.23 ELA): ";
			std::cin >> amount;
			budget["Amount"] = convertAmount(amount);

			budgets.push_back(budget);
			std::cout << "Budgets preview: " << std::endl << budgets.dump(4) << std::endl;

			do {
				std::cout << "Continue to add budget, [Y]es or [N]o ? ";
				std::cin >> continueAdd;
				transform(continueAdd.begin(), continueAdd.end(), continueAdd.begin(), ::tolower);
			} while (continueAdd != "y" && continueAdd != "n" && continueAdd != "yes" && continueAdd != "no");
		}
		payload["Budgets"] = budgets;

		// Proposal owner sign the payload
		std::string digest = subWallet->ProposalOwnerDigest(payload);

		std::cout << "Sign the digest by proposal owner with did sdk" << std::endl;
		std::cout << "Digest: " << digest << std::endl << "Press enter to continue..." << std::endl;

		std::string signature;
		std::cin >> signature;
		std::cout << "Enter owner signature: ";
		std::cin >> signature;

		payload["Signature"] = signature;

		// CR Committee sign the payload
		std::string did;
		std::cout << "Enter CR committee's did: ";
		std::cin >> did;
		payload["CRCouncilMemberDID"] = did;

		digest = subWallet->ProposalCRCouncilMemberDigest(payload);

		std::cout << "Sign the digest by CR committee with did sdk" << std::endl;
		std::cout << "Digest: " << digest << std::endl << "Press enter to continue..." << std::endl;

		std::cout << "Enter CR committee's signature: ";
		std::cin >> signature;

		payload["CRCouncilMemberSignature"] = signature;

		std::cout << "Payload preview: " << std::endl << payload.dump(4) << std::endl;
		nlohmann::json tx = subWallet->CreateProposalTransaction(nlohmann::json(), payload, "10000", "memo");

		signAndShowTx(subWallet, tx);
	} catch (const std::exception &e) {
		exceptionError(e);
		return ERRNO_APP;
	}
	return 0;
}

// tracking
static int tracking(int argc, char *argv[]) {
	checkParam(1);
	checkCurrentWallet();

	try {
		IMainchainSubWallet *subWallet;
		getSubWallet(subWallet, currentWallet, CHAINID_ELA);

		IIDChainSubWallet *idSubWallet;
		getSubWallet(idSubWallet, currentWallet, CHAINID_ID);

		nlohmann::json payload;

		std::string proposalHash;
		std::cout << "Enter proposal hash: ";
		std::cin >> proposalHash;
		payload["ProposalHash"] = proposalHash;

		std::string docHash;
		std::cout << "Enter document hash: ";
		std::cin >> docHash;
		payload["DocumentHash"] = docHash;

		uint8_t stage;
		std::cout << "Enter stage [0, 128): ";
		std::cin >> stage;
		if (stage >= 128) {
			std::cout << "invalid input" << std::endl;
			return ERRNO_APP;
		}
		payload["Stage"] = stage;

		std::string ownerPubKey;
		std::cout << "Enter proposal owner public key: ";
		std::cin >> ownerPubKey;
		payload["OwnerPublicKey"] = ownerPubKey;

		std::string newOwnerPubKey;
		std::cout << "Enter proposal new owner public key (empty for not change owner): ";
		std::cin >> newOwnerPubKey;
		payload["NewOwnerPublicKey"] = newOwnerPubKey;

		// Proposal owner sign
		std::string digest = subWallet->ProposalTrackingOwnerDigest(payload);
		std::cout << "Sign the digest by proposal owner with did sdk" << std::endl;
		std::cout << "Digest: " << digest << std::endl << "Press enter to continue..." << std::endl;

		std::string signature;
		std::cout << "Enter proposal owner signature: ";
		std::cin >> signature;
		payload["OwnerSignature"] = signature;

		// If change proposal owner, then new owner sign
		if (!newOwnerPubKey.empty()) {
			digest = subWallet->ProposalTrackingNewOwnerDigest(payload);
			std::cout << "Sign the digest by proposal new owner with did sdk" << std::endl;
			std::cout << "Digest: " << digest << std::endl << "Press enter to continue..." << std::endl;

			std::cout << "Enter proposal new owner signature: ";
			std::cin >> signature;
			payload["NewOwnerSignature"] = signature;
		} else {
			payload["NewOwnerSignature"] = "";
		}

		// Secretary sign
		int type;
		std::cout << "Select proposal type: " << std::endl;
		std::cout << "1. Common" << std::endl;
		std::cout << "2. Progress" << std::endl;
		std::cout << "3. Reject" << std::endl;
		std::cout << "4. Terminate" << std::endl;
		std::cout << "5. Change Owner" << std::endl;
		std::cout << "6. Finalize" << std::endl;
		std::cin >> type;
		switch (type) {
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
			case 6: payload["Type"] = type - 1; break;
			default: std::cout << "invalid input" << std::endl;
				return ERRNO_APP;
		}

		std::string opinionHash;
		std::cout << "Enter secretary opinion hash: ";
		std::cin >> opinionHash;
		payload["SecretaryOpinionHash"] = opinionHash;

		digest = subWallet->ProposalTrackingSecretaryDigest(payload);
		std::cout << "Sign the digest by secretary with did sdk" << std::endl;
		std::cout << "Digest: " << digest << std::endl << "Press enter to continue..." << std::endl;

		std::cout << "Enter secretary's signature: ";
		std::cin >> signature;
		payload["SecretarySignature"] = signature;

		std::cout << "Proposal tracking payload preview: " << std::endl << payload.dump(4) << std::endl;

		nlohmann::json tx = subWallet->CreateProposalTrackingTransaction(nlohmann::json(), payload, "10000", "memo");

		signAndShowTx(subWallet, tx);
	} catch (const std::exception &e) {
		exceptionError(e);
		return ERRNO_APP;
	}
	return 0;
}

// loglevel (trace | debug | info ...)
static int loglevel(int argc, char *argv[]) {
	checkParam(2);
	if (manager == nullptr) {
		std::cerr << "wallet manager is null" << std::endl; \
		return ERRNO_APP; \
	}

	std::string level = argv[1];

	try {
		manager->SetLogLevel(level);
	} catch (const std::exception &e) {
		exceptionError(e);
		return ERRNO_APP;
	}
	return 0;
}

// vote (cr | dpos)
static int vote(int argc, char *argv[]) {
	checkParam(2);
	checkCurrentWallet();
	std::string voteType = argv[1];

	try {
		IMainchainSubWallet *subWallet;
		getSubWallet(subWallet, currentWallet, CHAINID_ELA);
		nlohmann::json tx;
        tx = subWallet->CreateVoteTransaction(nlohmann::json(), nlohmann::json(), "10000", "memo");

		signAndShowTx(subWallet, tx);
	} catch (const std::exception &e) {
		exceptionError(e);
		return ERRNO_APP;
	}
	return 0;
}

// export (m[nemonic] | k[eystore] | xpub | xprv)
static int _export(int argc, char *argv[]) {
	checkParam(2);
	checkCurrentWallet();

	std::string exportWhat = argv[1];

	try {
		if (exportWhat == "mnemonic" || exportWhat == "m") {
			std::string password = getpass("Enter payment password: ");
			std::string mnemonic = currentWallet->ExportMnemonic(password);
			std::cout << mnemonic << std::endl;
		} else if (exportWhat == "keystore" || exportWhat == "k") {
            std::string backupPassword;
            if (0 > createBackupPassword(backupPassword)) {
                std::cerr << "Export keystore failed!" << std::endl;
                return ERRNO_APP;
            }

            std::string password = getpass("Enter payment password: ");
            nlohmann::json keystore = currentWallet->ExportKeystore(backupPassword, password);
            std::cout << keystore.dump() << std::endl;
        } else if (exportWhat == "xprv") {
            std::string password = getpass("Enter payment password: ");
		    std::string xprv = currentWallet->ExportPrivateKey(password);
		    std::cout << xprv << std::endl;
        } else if (exportWhat == "xpub") {
		    std::string xpub = currentWallet->ExportMasterPublicKey();
		    std::cout << xpub << std::endl;
		} else {
			invalidCmdError();
			return ERRNO_APP;
		}
	} catch (const std::exception &e) {
		exceptionError(e);
		return ERRNO_APP;
	}
	return 0;
}

// passwd [reset]
static int passwd(int argc, char *argv[]) {
	if (argc > 2) {
		invalidCmdError();
		return ERRNO_CMD;
	}

	checkCurrentWallet();
	try {
		if (argc == 1) {
			std::string oldPassword = getpass("Old Password: ");
			if (!currentWallet->VerifyPayPassword(oldPassword)) {
				std::cerr << "Wrong password" << std::endl;
				return ERRNO_APP;
			}

			std::string newPassword = getpass("New Password: ");
			std::string newPasswordVerify = getpass("Retype New Password: ");
			if (newPassword != newPasswordVerify) {
				std::cerr << "Password not match" << std::endl;
				return ERRNO_APP;
			}

			currentWallet->ChangePassword(oldPassword, newPassword);
		} else if (argc == 2) {
			std::cout << "Enter mnemonic:";
			char line[1024] = {0};
			if (NULL == fgets(line, sizeof(line), stdin)) {
			    std::cerr << "fgets error" << std::endl;
                return ERRNO_APP;
			}
			std::string mnemonic = line;
			mnemonic.erase(mnemonic.find_first_of('\n'));
			std::cout << "mnemonic: " << mnemonic << std::endl;

			std::string paypasswd, passphrase;
			if (0 > createPassphrase(passphrase)) {
				std::cerr << "Create failed!" << std::endl;
				return ERRNO_APP;
			}

			if (0 > createPaymentPassword(paypasswd)) {
				std::cerr << "Create failed!" << std::endl;
				return ERRNO_APP;
			}

			currentWallet->ResetPassword(mnemonic, passphrase, paypasswd);
		}
	} catch (const std::exception &e) {
		exceptionError(e);
		return ERRNO_APP;
	}

	return 0;
}

// verify (passphrase | paypasswd)
static int verify(int argc, char *argv[]) {
	checkParam(2);
	checkCurrentWallet();

	std::string verifyWhat = argv[1];
	try {
		bool result = false;
		if (verifyWhat == "passphrase") {
			std::string passphrase = getpass("Passphrase: ");
			std::string paypasswd = getpass("Payment password: ");
			result = currentWallet->VerifyPassPhrase(passphrase, paypasswd);
		} else if (verifyWhat == "paypasswd") {
			std::string paypasswd = getpass("Payment passwd: ");
			result = currentWallet->VerifyPayPassword(paypasswd);
		} else {
			invalidCmdError();
			return ERRNO_APP;
		}

		std::string resultString = result ? "successful" : "failed";
		std::cout << "Verify " << resultString << std::endl;
	} catch (const std::exception &e) {
		exceptionError(e);
		return ERRNO_APP;
	}
	return 0;
}

// transfer chainID
static int transfer(int argc, char *argv[]) {
	checkParam(2);
	checkCurrentWallet();

	std::string chainID = argv[1];
	std::string addr;
	std::string amount;

	try {
		nlohmann::json tx;
		if (chainID.find("ETH") !=  std::string::npos) {
            IEthSidechainSubWallet *subWallet;
            getSubWallet(subWallet, currentWallet, chainID);

            std::cout << "address: " << std::endl;
            std::getline(std::cin, addr);

            std::cout << "amount: " << std::endl;
            std::getline(std::cin, amount);
            amount = convertETHAmount(amount);

            std::string gasPrice;
            std::cout << "gasPrice: " << std::endl;
            std::getline(std::cin, gasPrice);

            std::string gasLimit;
            std::cout << "gasLimit: " << std::endl;
            std::getline(std::cin, gasLimit);

            std::string data;
            std::cout << "data: " << std::endl;
            std::getline(std::cin, data);

            std::string nonce;
            std::cout << "nonce: " << std::endl;
            std::getline(std::cin, nonce);

            tx = subWallet->CreateTransferGeneric(addr, amount, ETHER_WEI, gasPrice, ETHER_WEI, gasLimit, data,
                                                  std::atoll(nonce.c_str()));
            signAndShowTx(subWallet, tx);
        } else if (chainID == "BTC") {
		} else {
			std::cout << "address: " << std::endl;
			std::getline(std::cin, addr);

			std::cout << "amount: " << std::endl;
			std::getline(std::cin, amount);
			amount = convertAmount(argv[3]);

			IElastosBaseSubWallet *subWallet;
			getSubWallet(subWallet, currentWallet, chainID);

			tx = subWallet->CreateTransaction(nlohmann::json(), nlohmann::json(), "10000", "memo");
			signAndShowTx(subWallet, tx);
		}
	} catch (const std::exception &e) {
		exceptionError(e);
		return ERRNO_APP;
	}
	return 0;
}

// tx chainID
static int _rawtx(int argc, char *argv[]) {
	checkParam(2);
	checkCurrentWallet();

	std::string chainID = argv[1];
	try {
        if (chainID == CHAINID_ELA || chainID == CHAINID_ID) {
            std::cout << "Enter tx to convert: ";
            std::string tx;
            struct termios told = enableLongInput();
            std::getline(std::cin, tx);
            recoveryTTYSetting(&told);
            IElastosBaseSubWallet *subWallet;
            getSubWallet(subWallet, currentWallet, chainID);

            std::string rawtx = subWallet->ConvertToRawTransaction(tx);
            std::cout << rawtx << std::endl;
        } else {
            std::cerr << "Unsupported" << std::endl;
        }
	} catch (const std::exception &e) {
		exceptionError(e);
		return ERRNO_APP;
	}

	return 0;
}

// signtx chainID
static int signtx(int argc, char *argv[]) {
	checkParam(2);
	checkCurrentWallet();

	std::string chainID = argv[1];

	try {
		std::cout << "Enter tx to be signed: ";
		std::string tx;
		struct termios told = enableLongInput();
		std::getline(std::cin, tx);
		recoveryTTYSetting(&told);
		ISubWallet *subWallet;
		getSubWallet(subWallet, currentWallet, chainID);

		std::string password = getpass("Enter payment password: ");
		nlohmann::json signedTx = subWallet->SignTransaction(nlohmann::json::parse(tx), password);
		std::cout << signedTx.dump() << std::endl;
	} catch (const std::exception &e) {
		exceptionError(e);
		return ERRNO_APP;
	}

	return 0;
}

// network [netType] [configJson]
static int _network(int argc, char *argv[]) {

	if (argc == 1) {
		std::cout << "Network: " << network << std::endl;
		std::cout << "Config: " << config.dump(4) << std::endl;
	} else {
		checkParam(3);
		std::string netType = argv[1];
		std::string netConfig = argv[2];

		network = netType;
		config = nlohmann::json::parse(netConfig);

		walletCleanup();
		delete manager;

		try {
			manager = new MasterWalletManager(walletRoot, network, config);
		} catch (const std::exception &e) {
			std::cout << e.what() << std::endl;
			exit(-1);
		}
		walletInit();
	}

	return 0;
}

static int verbose(int argc, char *argv[]) {
	bool v = !verboseMode;

	if (argc > 2) {
		invalidCmdError();
		return ERRNO_CMD;
	}

	if (argc > 1) {
		if (strcmp(argv[1], "on") == 0)
			v = true;
		else if (strcmp(argv[1], "off") == 0)
			v = false;
		else {
			std::cerr << "Unknown verbse options: " << argv[1] << std::endl;
			return ERRNO_CMD;
		}
	}

	std::string opt = v ? "on" : "off";
	std::cout << "Verbose mode: " << opt << std::endl;
	verboseMode = v;
	return 0;
}

static int help(int argc, char *argv[]);

struct command {
	const char *cmd;

	int (*function)(int argc, char *argv[]);

	const char *help;
} commands[] = {
	{"help",       help,           "[command]                                        Display available command list, or usage description for specific command."},
	{"create",     create,         "walletName                                       Create a new wallet with given name."},
	{"import",     import,         "walletName (m[nemonic] | k[eystore]) [filePath]  Import wallet with given name and mnemonic or keystore."},
	{"remove",     remove,         "walletName                                       Remove specified wallet."},
	{"switch",     _switch,        "walletName                                       Switch current wallet."},
	{"list",       list,           "[masterWalletID]                                 List wallets."},
	{"passwd",     passwd,         "                                                 Change password of wallet."},
	{"verify",     verify,         "(passphrase | paypasswd)                         Verify payment passwd or passphrase."},
	{"didtx",      didtx,          "[didDocFilepath]                                 Create DID transaction."},
	{"didsign",    didsign,        "DID digest                                       Sign `digest` with private key of DID."},
	{"did",        did,            "                                                 List did of IDChain"},
	{"cid",        _cid,           "                                                 List cid of IDChain"},
	{"publickeys", publickeys,     "chainID index count [internal]                   List public keys of IDChain"},
	{"open",       _open,          "chainID                                          Open wallet of `chainID`."},
	{"close",      _close,         "chainID                                          Close wallet of `chainID`."},
	{"rawtx",      _rawtx,         "chainID                                          Convert spv tx to rawtx"},
	{"signtx",     signtx,         "chainID                                          Sign tx"},
	{"transfer",   transfer,       "chainID                                          Transfer asset from `chainID`."},
	{"address",    address,        "chainID index count [internal]                   Get the revceive addresses or change addresses of chainID."},
	{"proposal",   proposal,       "                                                 Create proposal tx."},
	{"deposit",    deposit,        "chainID amount address                           Deposit to sidechain from mainchain."},
	{"withdraw",   withdraw,       "amount address                                   Withdraw from sidechain to mainchain."},
	{"export",     _export,        "(m[nemonic] | k[eystore] | xpub | xprv)          Export mnemonic or keystore."},
	{"register",   _register,      "(cr | dpos)                                      Register CR or DPoS with specified wallet."},
	{"unregister", unregister,     "(cr | dpos)                                      Unregister CR or DPoS with specified wallet."},
	{"basicinfo",  basic_info,     "[chainID]                                        Get basic info of master/sub Wallet"},
	{"retrieve",   retrieve,       "(cr | dpos)                                      Retrieve Pledge with specified wallet."},
	{"vote",       vote,           "(cr | dpos)                                      CR/DPoS vote."},
	{"network",    _network,       "[netType]                                        Show current net type or set net type: 'MainNet', 'TestNet', 'RegTest', 'PrvNet'"},
	{"verbose",    verbose,        "(on | off)                                       Set verbose mode."},
	{"tracking",   tracking,       "                                                 Create proposal tracking tx."},
	{"loglevel",   loglevel,       "(trace | debug | info | warning | error | critical | off"},
	{"exit", NULL,               "                                                 Quit wallet."},
	{"quit", NULL,               "                                                 Quit wallet."},
	{NULL,   NULL, NULL}
};

static int help(int argc, char *argv[]) {
	struct command *p;

	std::cout << std::endl << "Usage: Command [Arguments]..." << std::endl << std::endl;
	std::cout << "Command list:" << std::endl;

	if (argc == 1) {
		for (p = commands; p->cmd; p++) {
			printf("  %-15s %s\n", p->cmd, p->help);
		}

		printf("\n");
	} else {
		for (p = commands; p->cmd; p++) {
			if (strcmp(argv[1], p->cmd) == 0) {
				printf("  %-15s %s\n\n", p->cmd, p->help);
				return 0;
			}
		}

		std::cerr << "Unknown command: " << argv[1] << std::endl;
	}

	return 0;
}

int run_cmd(int argc, char *argv[]) {
	struct command *p;

	for (p = commands; p->cmd; p++) {
		if (strcmp(argv[0], p->cmd) == 0) {
			if (-1 == p->function(argc, argv)) {
				char *help_argv[2];
				help_argv[0] = (char *)commands[0].cmd;
				help_argv[1] = (char *)p->cmd;
				help(2, help_argv);
			}
			return 0;
		}
	}

	std::cerr << "Unknown command: " << argv[0] << std::endl;
	return -1;
}

static int parseCmdLine(char *cmdLine, char *cmdArgs[]) {
	char *p;
	int arg = 0;
	int count = 0;

	for (p = cmdLine; *p != 0; p++) {
		if (isspace(*p)) {
			*p = 0;
			arg = 0;
		} else {
			if (arg == 0) {
				cmdArgs[count] = p;
				count++;
			}

			arg = 1;
		}
	}

	return count;
}

static int mkdirInternal(const char *path, mode_t mode) {
	struct stat st;
	int rc = 0;

	if (stat(path, &st) != 0) {
		/* Directory does not exist. EEXIST for race condition */
		if (mkdir(path, mode) != 0 && errno != EEXIST)
			rc = -1;
	} else if (!S_ISDIR(st.st_mode)) {
		errno = ENOTDIR;
		rc = -1;
	}

	return rc;
}

static int mkdirs(const char *path, mode_t mode) {
	int rc = 0;
	char *pp;
	char *sp;
	char copypath[PATH_MAX];

	strncpy(copypath, path, sizeof(copypath));
	copypath[sizeof(copypath) - 1] = 0;

	pp = copypath;
	while (rc == 0 && (sp = strchr(pp, '/')) != 0) {
		if (sp != pp) {
			/* Neither root nor double slash in path */
			*sp = '\0';
			rc = mkdirInternal(copypath, mode);
			*sp = '/';
		}
		pp = sp + 1;
	}

	if (rc == 0)
		rc = mkdirInternal(path, mode);

	return rc;
}

static std::string getWalletDir(const std::string &dir) {
	std::string result = dir;

	if (dir.empty()) {
		result = getenv("HOME");
		result += "/.wallet";
	}

	if (result.back() == '/' || result.back() == '\\')
		result.pop_back();

	return result;
}

#ifdef HAVE_SYS_RESOURCE_H

#include <sys/resource.h>

static int sys_coredump_set(bool enable) {
	const struct rlimit rlim = {
		enable ? RLIM_INFINITY : 0,
		enable ? RLIM_INFINITY : 0
	};

	return setrlimit(RLIMIT_CORE, &rlim);
}

#endif

static void signalHandler(int signum) {
	walletCleanup();
	delete manager;
	exit(-1);
}

static void usage(void)
{
	printf("SPV Wallet, a command line wallet.\n");
	printf("Usage: wallet [OPTION]...\n");
	printf("\n");
	printf("  -d, --data=DIR            Wallet data directory.\n");
	printf("  -n, --network=NETWORK     Network name or config file.\n");
	printf("                            MainNet TestNet RegTest,\n");
	printf("                            or config file for private network.\n");
	printf("  -v, --verbose             Verbose output.\n");
	printf("      --debug               Wait for debugger attach after start.\n");
	printf("\n");
}

int main(int argc, char *argv[]) {
	char cmdLine[4096];
	char *cmdArgs[512];
	int numArgs;
	int waitForAttach = 0;
    std::string tmp;

	int opt;
	int idx;
	struct option options[] = {
		{"data",            required_argument, NULL, 'd'},
		{"network",         required_argument, NULL, 'n' },
        {"config",          required_argument, NULL, 'c'},
		{"verbose",         no_argument,       NULL, 'v'},
		{"debug",           no_argument,       NULL, 1},
		{"help",            no_argument,       NULL, 'h'},
		{NULL,              0,                 NULL, 0}
	};

#ifdef HAVE_SYS_RESOURCE_H
	sys_coredump_set(true);
#endif

	while ((opt = getopt_long(argc, argv, "d:n:c:vh?", options, &idx)) != -1) {
		switch (opt) {
			case 'd':
				walletRoot = optarg;
				break;

			case 'n':
				network = optarg;
				break;

		    case 'c':
		        tmp = optarg;
                config = nlohmann::json::parse(tmp);
		        break;

			case 'v':
				verboseMode = true;
				break;

			case 1:
				waitForAttach = 1;
				break;

			case 'h':
			case '?':
			default:
				usage();
				exit(-1);
		}
	}

	if (waitForAttach) {
		printf("Wait for debugger attaching, process id is: %d.\n", getpid());
#ifndef _MSC_VER
		printf("After debugger attached, press any key to continue......");
		getchar();
#else
		DebugBreak();
#endif
	}

	signal(SIGINT, signalHandler);
	signal(SIGTERM, signalHandler);
	signal(SIGKILL, signalHandler);
	signal(SIGHUP, signalHandler);

	walletRoot = getWalletDir(walletRoot);
	if (mkdirs(walletRoot.c_str(), S_IRWXU) < 0) {
		std::cerr << "Create wallet data directory '" << walletRoot << "' failed" << std::endl;
		return -1;
	} else {
		std::cout << "Wallet data directory: " << walletRoot << std::endl;
	}

	try {
		manager = new MasterWalletManager(walletRoot, network, config);
	} catch (const std::exception &e) {
		std::cout << e.what() << std::endl;
		return -1;
	}

	walletInit();

	while (1) {
		fprintf(stdout, "-> wallet $ ");
		if (NULL == fgets(cmdLine, sizeof(cmdLine), stdin))
			continue;

		numArgs = parseCmdLine(cmdLine, cmdArgs);
		if (numArgs == 0)
			continue;

		if (strcmp(cmdArgs[0], "quit") == 0 ||
			strcmp(cmdArgs[0], "exit") == 0)
			break;

		run_cmd(numArgs, cmdArgs);
	}

	walletCleanup();
	delete manager;

	return 0;
}
