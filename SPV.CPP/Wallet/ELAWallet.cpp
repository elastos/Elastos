#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <getopt.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include <signal.h>

#include <MasterWalletManager.h>
#include <IMasterWallet.h>
#include <ISubWallet.h>
#include <IMainchainSubWallet.h>
#include <IIDChainSubWallet.h>

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

using namespace Elastos::ElaWallet;

#define SELA_PER_ELA 100000000UL
static const std::string CHAINID_ELA = "ELA";
static const std::string CHAINID_ID = "IDChain";

static MasterWalletManager *manager = nullptr;
static IMasterWallet *currentWallet = nullptr;

static bool verboseMode = false;
static const char *SplitLine = "------------------------------------------------------------------------------------------------------------------------";

class WalletData {
public:
	WalletData() :
		_callback(nullptr),
		_syncProgress(0),
		_speed(0),
		_lastBlockTime(0)
		{}

	void SetCallback(ISubWalletCallback *callback) {
		_callback = callback;
	}

	ISubWalletCallback *GetCallback() const {
		return _callback;
	}

	void SetSyncProgress(int progress) {
		_syncProgress = progress;
	}

	int GetSyncProgress() const {
		return _syncProgress;
	}

	void SetDownloadPeer(const std::string &peer) {
		_downloadPeer = peer;
	}

	const std::string &GetDownloadPeer() const {
		return _downloadPeer;
	}

	void SetSpeed(int speed) {
		_speed = speed;
	}

	int GetSpeed() const {
		return _speed;
	}

	void SetLastBlockTime(time_t t) {
		_lastBlockTime = t;
	}

	time_t GetLastBlockTime() const {
		return _lastBlockTime;
	}

	void SetLastPublishedTx(const std::string &txHash) {
		_lastPublishedTx = txHash;
	}

	const std::string &GetLastPublishedTx() const  {
		return _lastPublishedTx;
	}

	void SetLastPublishedTxResult(const std::string &result) {
		_lastPublishedTxResult = result;
	}

	const std::string &GetLastPublishedTxResult() const {
		return _lastPublishedTxResult;
	}

private:
	ISubWalletCallback *_callback;
	int _syncProgress;
	std::string _downloadPeer;
	int _speed;
	time_t _lastBlockTime;

	std::string _lastPublishedTx;
	std::string _lastPublishedTxResult;
};

typedef std::map<std::string, WalletData> SubWalletData;
typedef std::map<std::string, SubWalletData> MasterWalletData;

static MasterWalletData masterWalletData;

class SubWalletCallback : public ISubWalletCallback {
public:
	~SubWalletCallback() {}

	SubWalletCallback(const std::string &masterWalletID, const std::string &walletID) :
		_walletID(masterWalletID),
		_chainID(walletID) {}

	virtual void OnTransactionStatusChanged(
		const std::string &txid, const std::string &status,
		const nlohmann::json &desc, uint32_t confirms) {
		if (verboseMode)
			std::cout << "*** Wallet " << _walletID << ":" << _chainID << " transaction: "
					  << txid << " changed to " << status << ", confirms: "
					  << confirms << std::endl;
	}

	virtual void OnBlockSyncStarted() {
		if (verboseMode)
			std::cout << "*** Wallet " << _walletID << ":" << _chainID << " sycn started." << std::endl;
	}

	virtual void OnBlockSyncProgress(const nlohmann::json &progressInfo) {
		if (verboseMode)
			std::cout << "*** Wallet " << _walletID << ":" << _chainID << " syncing:"
					  << progressInfo.dump() << std::endl;

		masterWalletData[_walletID][_chainID].SetSyncProgress(progressInfo["Progress"].get<int>());
		masterWalletData[_walletID][_chainID].SetDownloadPeer(progressInfo["DownloadPeer"].get<std::string>());
		masterWalletData[_walletID][_chainID].SetSpeed(progressInfo["BytesPerSecond"].get<int>());
		masterWalletData[_walletID][_chainID].SetLastBlockTime(progressInfo["LastBlockTime"].get<time_t>());
	}

	virtual void OnBlockSyncStopped() {
		if (verboseMode)
			std::cout << "*** Wallet " << _chainID << " sync stopped." << std::endl;
	}

	virtual void OnBalanceChanged(const std::string &asset, const std::string &balance) {
		if (verboseMode)
			std::cout << "*** Wallet " << _chainID << " balance changed: "
					  << balance << std::endl;
	}

	virtual void OnTxPublished(const std::string &hash, const nlohmann::json &result) {
		if (verboseMode)
			std::cout << "*** Wallet " << _chainID << " public a new transaction: "
					  << hash << std::endl;
	}

	virtual void OnAssetRegistered(const std::string &asset, const nlohmann::json &info) {
	}

	virtual void OnConnectStatusChanged(const std::string &status) {
		if (verboseMode)
			std::cout << "*** Wallet " << _chainID << " connection status: "
					  << status << std::endl;
	}

private:
	std::string _walletID;
	std::string _chainID;
};

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

static void signAndPublishTx(ISubWallet *subWallet, const nlohmann::json &tx) {
	std::string password = getpass("Enter payment password: ");
	nlohmann::json signedTx = subWallet->SignTransaction(tx, password);
	subWallet->PublishTransaction(signedTx);
}

static std::string convertAmount(const std::string &amount) {
	return std::to_string((uint64_t) (std::stod(amount) * SELA_PER_ELA));
}

static void subWalletOpen(IMasterWallet *masterWallet, ISubWallet *subWallet) {
	WalletData walletData;
	SubWalletData subWalletData;
	ISubWalletCallback *callback = new SubWalletCallback(masterWallet->GetID(), subWallet->GetChainID());

	walletData.SetCallback(callback);
	subWalletData[subWallet->GetChainID()] = walletData;
	masterWalletData[masterWallet->GetID()] = subWalletData;

	subWallet->AddCallback(callback);
	subWallet->SyncStart();
}

static void subWalletClose(IMasterWallet *masterWallet, ISubWallet *subWallet) {
	std::string walletName = masterWallet->GetID();
	std::string chainID = subWallet->GetChainID();

	subWallet->SyncStop();
	subWallet->RemoveCallback();

	SubWalletCallback *callback = static_cast<SubWalletCallback *>(masterWalletData[walletName][chainID].GetCallback());
	delete callback;
	callback = nullptr;

	masterWalletData[walletName].erase(chainID);
	if (masterWalletData[walletName].empty())
		masterWalletData.erase(walletName);
}

static void walletInit(void) {
	auto masterWallets = manager->GetAllMasterWallets();
	for (IMasterWallet *masterWallet : masterWallets) {
		if (currentWallet == nullptr)
			currentWallet = masterWallet;

		auto subWallets = masterWallet->GetAllSubWallets();
		for (ISubWallet *subWallet : subWallets) {
			subWalletOpen(masterWallet, subWallet);
		}
	}
}

static void walletCleanup(void) {
	auto masterWallets = manager->GetAllMasterWallets();
	for (IMasterWallet *masterWallet : masterWallets) {
		auto subWallets = masterWallet->GetAllSubWallets();
		for (ISubWallet *subWallet : subWallets) {
			subWalletClose(masterWallet, subWallet);
		}
	}

	manager->FlushData();
}

static void invalidCmdError() {
	std::cerr << "Invalid command syntax." << std::endl;
}

static void exceptionError(const std::exception &e) {
	std::cerr << "exception: " << e.what() << std::endl;
}

static void create(int argc, char *argv[]) {
	if (argc != 2) {
		invalidCmdError();
		return;
	}

	try {
		std::string walletID = argv[1];
		if (nullptr != manager->GetMasterWallet(walletID)) {
			std::cerr << "Wallet '" << walletID << "' already exist." << std::endl;
			return;
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
			return;
		}

		if (0 > createPaymentPassword(password)) {
			std::cerr << "Create failed!" << std::endl;
			return;
		}

		IMasterWallet *masterWallet = manager->CreateMasterWallet(walletID,
																  mnemonic, passphrase, password, false);
		if (!masterWallet) {
			std::cerr << "Create master wallet failed." << std::endl;
			return;
		}

		ISubWallet *subWallet = masterWallet->CreateSubWallet(CHAINID_ELA);
		if (!subWallet) {
			std::cerr << "Create main chain wallet failed." << std::endl;
			return;
		}

		subWalletOpen(masterWallet, subWallet);
		std::cout << "Wallet create success." << std::endl;
	} catch (const std::exception &e) {
		exceptionError(e);
	}
}

// import [walletName] [m[nemonic] | k[eystore]]
static void import(int argc, char *argv[]) {
	if (argc != 3) {
		invalidCmdError();
		return;
	}

	std::string walletName = argv[1];
	std::string importWhat = argv[2];

	try {
		IMasterWallet *masterWallet = manager->GetMasterWallet(walletName);
		if (masterWallet) {
			std::cerr << "Wallet '" << walletName << "' already exist." << std::endl;
			return;
		}

		if (importWhat == "mnemonic" || importWhat == "m") {
			std::string mnemonic;
			std::cout << "Enter mnemonic: ";
			std::getline(std::cin, mnemonic);

			std::string password, passphrase;
			if (0 > createPassphrase(passphrase)) {
				std::cerr << "Create failed!" << std::endl;
				return;
			}

			if (0 > createPaymentPassword(password)) {
				std::cerr << "Create failed!" << std::endl;
				return;
			}

			masterWallet = manager->ImportWalletWithMnemonic(walletName, mnemonic, passphrase, password, false);
			if (!masterWallet) {
				std::cerr << "Import master wallet failed." << std::endl;
				return;
			}

			ISubWallet *subWallet = masterWallet->CreateSubWallet(CHAINID_ELA);
			if (!subWallet) {
				std::cerr << "Create main chain wallet failed." << std::endl;
				return;
			}
		} else if (importWhat == "keystore" || importWhat == "k") {
			std::string keystore;
			std::cout << "Enter keystore: ";

			std::string backupPassword = getpass("Enter backup password: ");

			std::string password;
			if (0 > createPaymentPassword(password)) {
				std::cerr << "Import wallet failed!" << std::endl;
				return;
			}

			masterWallet = manager->ImportWalletWithKeystore(walletName, nlohmann::json::parse(keystore),
															 backupPassword, password);
			if (!masterWallet) {
				std::cerr << "Import wallet failed!" << std::endl;
				return;
			}
		} else {
			invalidCmdError();
			return;
		}

		auto subWallets = masterWallet->GetAllSubWallets();
		for (ISubWallet *subWallet : subWallets) {
			subWalletOpen(masterWallet, subWallet);
		}

		std::cout << "Wallet import success." << std::endl;
	} catch (const std::exception &e) {
		exceptionError(e);
	}
}

// remove [walletName]
static void remove(int argc, char *argv[]) {
	if (argc != 2) {
		invalidCmdError();
		return;
	}

	std::string walletName = argv[1];

	try {
		auto masterWallet = manager->GetMasterWallet(walletName);
		if (!masterWallet) {
			std::cerr << walletName << " not found" << std::endl;
			return;
		}

		std::string password = getpass("Enter payment password: ");
		if (!masterWallet->VerifyPayPassword(password)) {
			std::cerr << "Wrong password!" << std::endl;
			return;
		}

		auto subWallets = masterWallet->GetAllSubWallets();

		for (ISubWallet *subWallet : subWallets) {
			subWalletClose(masterWallet, subWallet);
		}

		manager->DestroyWallet(walletName);
		std::cout << "Wallet '" << walletName << "' removed." << std::endl;
	} catch (const std::exception &e) {
		exceptionError(e);
	}
}

static void list(int argc, char *argv[]) {
	auto masterWallets = manager->GetAllMasterWallets();
	std::vector<std::string> subWalletIDList = {CHAINID_ELA, CHAINID_ID};

	printf("%-20s", "WalletName");
	for (std::string chainID : subWalletIDList)
		printf("%50s", chainID.c_str());
	printf("\n%s\n", SplitLine);

	for (IMasterWallet *masterWallet : masterWallets) {
		struct tm tm;
		time_t lastBlockTime;
		int progress;
		double balance;
		ISubWallet *subWallet;
		char info[256];
		char buf[100] = {0};

		printf(" %c %-17s", masterWallet == currentWallet ? '*' : ' ', masterWallet->GetID().c_str());
		for (std::string chainID: subWalletIDList) {
			subWallet = masterWallet->GetSubWallet(chainID);
			if (subWallet) {
				balance = std::stod(subWallet->GetBalance()) / SELA_PER_ELA;

				lastBlockTime = masterWalletData[masterWallet->GetID()][chainID].GetLastBlockTime();
				progress = masterWalletData[masterWallet->GetID()][chainID].GetSyncProgress();
				localtime_r(&lastBlockTime, &tm);
				strftime(buf, sizeof(buf), "%F %T", &tm);

				snprintf(info, sizeof(info), "%lf  [%s  %3d%%]", balance, buf, progress);
			} else {
				snprintf(info, sizeof(info), "--");
			}

			printf("%50s", info);
		}
		printf("\n");
	}
}

// switch [walletName]
static void _switch(int argc, char *argv[]) {
	if (argc != 2) {
		invalidCmdError();
		return;
	}

	const std::string walletName = argv[1];
	try {
		auto masterWallet = manager->GetMasterWallet(walletName);
		if (masterWallet == nullptr) {
			std::cerr << walletName << " not found" << std::endl;
			return;
		}

		currentWallet = masterWallet;
	} catch (const std::exception &e) {
		exceptionError(e);
	}
}

// address [chainID]
static void address(int argc, char *argv[]) {
	std::string chain;

	if (argc != 2) {
		invalidCmdError();
		return;
	}

	if (!currentWallet) {
		std::cerr << "no wallet actived" << std::endl;
		return;
	}
	std::string chainID = argv[1];

	try {
		auto subWallet = currentWallet->GetSubWallet(chainID);
		if (!subWallet) {
			std::cerr << chainID << " not found" << std::endl;
			return;
		}

		int cntPerPage = 20;
		int curPage = 1;
		int start, max;
		std::string cmd;
		bool show = true;

		do {
			if (show) {
				start = cntPerPage * (curPage - 1);
				nlohmann::json addrJosn = subWallet->GetAllAddress(start, cntPerPage);
				nlohmann::json addr = addrJosn["Addresses"];
				max = addrJosn["MaxCount"];

				for (nlohmann::json::iterator it = addr.begin(); it != addr.end(); ++it)
					std::cout << *it << std::endl;
			}

			std::cout << "'n' Next Page, 'b' Previous Page, 'q' Exit: ";
			std::getline(std::cin, cmd);

			if (cmd == "n") {
				if (curPage < (max + cntPerPage) / cntPerPage) {
					curPage++;
					show = true;
				} else {
					std::cout << "already last page" << std::endl;
					show = false;
				}
			} else if (cmd == "b") {
				if (curPage > 1) {
					curPage--;
					show = true;
				} else {
					std::cout << "already first page" << std::endl;
					show = false;
				}
			} else if (cmd == "q") {
				break;
			} else {
				std::cout << "invalid input" << std::endl;
				show = false;
			}
		} while (cmd != "q");
	} catch (const std::exception &e) {
		exceptionError(e);
	}
}

// open [walletID] [chainID]
static void _open(int argc, char *argv[]) {
	if (argc != 3) {
		invalidCmdError();
		return;
	}

	std::string walletName = argv[1];
	std::string chainID = argv[2];
	try {
		auto masterWallet = manager->GetMasterWallet(walletName);
		if (!masterWallet) {
			std::cerr << walletName << " not found" << std::endl;
			return;
		}

		ISubWallet *subWallet = masterWallet->CreateSubWallet(chainID);

		subWalletOpen(masterWallet, subWallet);
	} catch (const std::exception &e) {
		exceptionError(e);
	}
}

// close [walletID] [chainID]
static void _close(int argc, char *argv[]) {
	if (argc != 3) {
		invalidCmdError();
		return;
	}

	std::string walletName = argv[1];
	std::string chainID = argv[2];

	try {
		auto masterWallet = manager->GetMasterWallet(walletName);
		if (!masterWallet) {
			std::cerr << walletName << " not found" << std::endl;
			return;
		}

		auto subWallet = masterWallet->GetSubWallet(chainID);
		if (!subWallet) {
			std::cerr << chainID << " not found" << std::endl;
			return;
		}

		subWalletClose(masterWallet, subWallet);
		masterWallet->DestroyWallet(chainID);
	} catch (const std::exception &e) {
		exceptionError(e);
	}
}

// deposit [chainID] [amount] [address]
static void deposit(int argc, char *argv[]) {
	if (argc != 4) {
		invalidCmdError();
		return;
	}

	if (!currentWallet) {
		std::cerr << "no wallet actived" << std::endl;
		return;
	}

	std::string chainID = argv[1];
	std::string amount = convertAmount(argv[2]);
	std::string sideChainAddress = argv[3];

	try {
		if (chainID == CHAINID_ELA) {
			std::cerr << "ELA is not a sidechain." << std::endl;
			return;
		}

		IMainchainSubWallet *mainchainSubWallet = dynamic_cast<IMainchainSubWallet *>(currentWallet->GetSubWallet(
			CHAINID_ELA));
		if (mainchainSubWallet == NULL) {
			std::cerr << "Can not get mainchain wallet." << std::endl;
			return;
		}


		nlohmann::json tx = mainchainSubWallet->CreateDepositTransaction(
			"", chainID, amount, sideChainAddress, "");

		signAndPublishTx(mainchainSubWallet, tx);
	} catch (const std::exception &e) {
		exceptionError(e);
	}
}

// withdraw [chainID] [amount] [address]
static void withdraw(int argc, char *argv[]) {
	if (argc != 4) {
		invalidCmdError();
		return;
	}

	if (!currentWallet) {
		std::cerr << "no wallet actived" << std::endl;
		return;
	}

	std::string chainID = argv[1];
	std::string amount = convertAmount(argv[2]);
	std::string mainChainAddress = argv[3];

	try {
		if (chainID == CHAINID_ELA) {
			std::cerr << "ELA is not a sidechain." << std::endl;
			return;
		}

		ISidechainSubWallet *sidechainSubWallet = dynamic_cast<ISidechainSubWallet *>(currentWallet->GetSubWallet(
			chainID));
		if (sidechainSubWallet == NULL) {
			std::cerr << "Can not get sidechain wallet for: " << chainID << std::endl;
			return;
		}

		nlohmann::json tx = sidechainSubWallet->CreateWithdrawTransaction(
			"", amount, mainChainAddress, "");

		signAndPublishTx(sidechainSubWallet, tx);
	} catch (const std::exception &e) {
		exceptionError(e);
	}
}

// diddoc [walletName]
static void diddoc(int argc, char *argv[]) {
	if (argc != 2) {
		invalidCmdError();
		return;
	}

	std::string walletName = argv[1];

	try {
		auto masterWallet = manager->GetMasterWallet(walletName);
		if (!masterWallet) {
			std::cerr << walletName << " not found" << std::endl;
			return;
		}

		IIDChainSubWallet *subWallet = dynamic_cast<IIDChainSubWallet *>(masterWallet->GetSubWallet(CHAINID_ID));
		if (!subWallet) {
			std::cerr << "open '" << CHAINID_ID << "' first" << std::endl;
			return;
		}

		std::string id;
		std::string didName;
		std::string operation;
		std::string publicKey;
		uint64_t expires;
		nlohmann::json pubKey = R"({"id":"#primary"})"_json;
		nlohmann::json publicKeys;

		std::cout << "Enter id: ";
		std::cin >> id;

		std::cout << "Enter did name: ";
		std::cin >> didName;

		std::cout << "Enter operation: ";
		std::cin >> operation;

		std::cout << "Enter public key: ";
		std::cin >> publicKey;
		pubKey["publicKey"] = publicKey;
		publicKeys.push_back(pubKey);

		std::cout << "Enter expires date: ";
		std::cin >> expires;

		nlohmann::json j;
		j["id"] = id;
		j["didName"] = didName;
		j["operation"] = operation;
		j["publicKey"] = publicKeys;
		j["expires"] = expires;

		std::string password = getpass("Enter payment password: ");
		nlohmann::json payload = subWallet->GenerateDIDInfoPayload(j, password);

		std::cout << payload.dump() << std::endl;
	} catch (const std::exception &e) {
		exceptionError(e);
	}
}

// didtx
static void didtx(int argc, char *argv[]) {
	if (argc != 1) {
		invalidCmdError();
		return;
	}

	if (!currentWallet) {
		std::cerr << "no wallet actived" << std::endl;
		return;
	}

	try {
		IIDChainSubWallet *subWallet = dynamic_cast<IIDChainSubWallet *>(currentWallet->GetSubWallet(CHAINID_ID));
		if (!subWallet) {
			std::cerr << "open '" << CHAINID_ID << "' first" << std::endl;
			return;
		}

		std::cout << "Enter id document: ";
		std::string didDoc;
		std::cin >> didDoc;

		nlohmann::json tx = subWallet->CreateIDTransaction(nlohmann::json::parse(didDoc), "");
		signAndPublishTx(subWallet, tx);
	} catch (const std::exception &e) {
		exceptionError(e);
	}
}

// didsign [DID] [digest]
static void didsign(int argc, char *argv[]) {
	if (argc != 3) {
		invalidCmdError();
		return ;
	}

	std::string did = argv[1];
	std::string digest = argv[2];
	try {
		if (!currentWallet) {
			std::cerr << "no wallet actived" << std::endl;
			return;
		}

		IIDChainSubWallet *subWallet = dynamic_cast<IIDChainSubWallet *>(currentWallet->GetSubWallet(CHAINID_ID));
		if (!subWallet) {
			std::cerr << "open " << CHAINID_ID << " first" << std::endl;
			return;
		}

		std::string password = getpass("Enter payment password: ");
		std::string signature = subWallet->SignDigest(did, digest, password);
		std::cout << signature << std::endl;
	} catch (const std::exception &e) {
		exceptionError(e);
	}
}

// registercr [cr | dpos]
static void _register(int argc, char *argv[]) {
	if (argc != 2) {
		invalidCmdError();
		return;
	}

	std::string registerWhat = argv[1];

	try {
		if (!currentWallet) {
			std::cerr << "no wallet actived" << std::endl;
			return;
		}

		IMainchainSubWallet *subWallet = dynamic_cast<IMainchainSubWallet *>(currentWallet->GetSubWallet(CHAINID_ELA));
		if (!subWallet) {
			std::cerr << "open '" << CHAINID_ELA << "' first" << std::endl;
			return;
		}
		nlohmann::json tx;

		if (registerWhat == "cr") {
			std::string crPublicKey = subWallet->GetCROwnerPublicKey();
			std::string nickName, url;
			uint64_t location;

			std::cout << "DID public key: " << crPublicKey << std::endl;
			std::cout << "Enter nick name: ";
			std::cin >> nickName;

			std::cout << "Enter url: ";
			std::cin >> url;

			std::cout << "Enter location code (example 86): ";
			std::cin >> location;

			std::string password = getpass("Enter payment password: ");
			nlohmann::json payload = subWallet->GenerateCRInfoPayload(crPublicKey, nickName, url, location, password);
			tx = subWallet->CreateRegisterCRTransaction("", payload, convertAmount("5000"), "");
		} else if (registerWhat == "dpos") {
			std::string ownerPubkey = subWallet->GetOwnerPublicKey();
			std::string nickName, nodePubkey, url;
			uint64_t location;

			std::cout << "Owner public key: " << ownerPubkey << std::endl;
			std::cout << "Enter node public key (empty will set to owner public key): ";
			std::cin >> nodePubkey;
			if (nodePubkey.empty())
				nodePubkey = ownerPubkey;

			std::cout << "Enter nick name: ";
			std::cin >> nickName;

			std::cout << "Enter url: ";
			std::cin >> url;

			std::string locationString;
			std::cout << "Enter location code (example 86): ";
			std::cin >> locationString;
			location = std::stol(locationString);

			std::string password = getpass("Enter payment password: ");
			nlohmann::json payload = subWallet->GenerateProducerPayload(ownerPubkey, nodePubkey, nickName, url, "", location, password);
			tx = subWallet->CreateRegisterCRTransaction("", payload, convertAmount("5000"), "");
		} else {
			invalidCmdError();
			return;
		}

		signAndPublishTx(subWallet, tx);
	} catch (const std::exception &e) {
		exceptionError(e);
	}
}

// proposal [sponsor | crsponsor]
static void proposal(int argc, char *argv[]) {
	if (argc != 2) {
		invalidCmdError();
		return;
	}

	std::string what = argv[1];

	try {
		if (!currentWallet) {
			std::cerr << "no wallet actived" << std::endl;
			return;
		}

		IMainchainSubWallet *mainchainSubWallet = dynamic_cast<IMainchainSubWallet *>(currentWallet->GetSubWallet(CHAINID_ELA));
		if (!mainchainSubWallet) {
			std::cerr << "open " << CHAINID_ELA << " first" << std::endl;
			return;
		}

		IIDChainSubWallet *iidChainSubWallet = dynamic_cast<IIDChainSubWallet *>(currentWallet->GetSubWallet(CHAINID_ID));
		if (!iidChainSubWallet) {
			std::cerr << "open " << CHAINID_ID << " first" << std::endl;
			return;
		}

		if (what == "sponsor") {
			int type = 0;
			std::string draftHash;
			std::string budgetList;
			std::string receiptAddress;
			std::string crSponsorDID;

			std::string sponsorPubKey = iidChainSubWallet->GetAllPublicKeys(0, 1)["PublicKeys"][0];
			std::cout << "sponsor public key: " << sponsorPubKey << std::endl;

			std::string sponsorDID = iidChainSubWallet->GetPublicKeyDID(sponsorPubKey);
			std::cout << "sponsor DID: " << sponsorDID << std::endl;

			std::cout << "Enter proposal draft hash: ";
			std::cin >> draftHash;

			std::cout << "Enter budget list (eg: [100.2, 200.3, 300.4]): ";
			std::cin >> budgetList;
			nlohmann::json budgetJson = nlohmann::json::parse(budgetList);
			std::vector<std::string> budgets;
			for (nlohmann::json::iterator it = budgetJson.begin(); it != budgetJson.end(); ++it)
				budgets.push_back(std::to_string((uint64_t)(*it).get<double>() * SELA_PER_ELA));

			std::cout << "Enter receipt address: ";
			std::cin >> receiptAddress;

			nlohmann::json proposalJson = mainchainSubWallet->SponsorProposalDigest(type, sponsorPubKey, draftHash, budgets,
																					receiptAddress);
			std::string digest = proposalJson["Digest"].get<std::string>();

			std::string password = getpass("Enter payment password: ");
			std::string signature = iidChainSubWallet->SignDigest(sponsorDID, digest, password);
			proposalJson["Signature"] = signature;

			std::cout << proposalJson.dump() << std::endl;
		} else if (what == "crsponsor") {
			std::string tmp;
			std::cout << "Enter sponsor signed proposal: " << std::endl;
			std::cin >> tmp;

			nlohmann::json sponsorSignedProposal = nlohmann::json::parse(tmp);

			std::string crSponsorPubKey = iidChainSubWallet->GetAllPublicKeys(0, 1)["PublicKeys"][0];
			std::cout << "CR sponsor public key: " << crSponsorPubKey << std::endl;
			std::string crSponsorDID = iidChainSubWallet->GetPublicKeyDID(crSponsorPubKey);
			std::cout << "CR sponsor DID: " << crSponsorDID << std::endl;

			nlohmann::json proposalJson = mainchainSubWallet->CRSponsorProposalDigest(sponsorSignedProposal, crSponsorDID);
			std::string digest = proposalJson["Digest"].get<std::string>();

			std::string password = getpass("Enter payment password: ");
			std::string signature = iidChainSubWallet->SignDigest(crSponsorDID, digest, password);
			proposalJson["CRSignature"] = signature;

			nlohmann::json tx = mainchainSubWallet->CreateCRCProposalTransaction(proposalJson, "");

			signAndPublishTx(mainchainSubWallet, tx);
		} else {
			invalidCmdError();
			return;
		}
	} catch (const std::exception &e) {
		exceptionError(e);
	}
}

// vote [cr | dpos]
static void vote(int argc, char *argv[]) {
	if (argc != 2) {
		invalidCmdError();
		return;
	}

	std::string voteType = argv[1];

	try {
		if (!currentWallet) {
			std::cerr << "no wallet actived" << std::endl;
			return;
		}

		IMainchainSubWallet *subWallet = dynamic_cast<IMainchainSubWallet *>(currentWallet->GetSubWallet(CHAINID_ELA));
		if (!subWallet) {
			std::cerr << "open '" << CHAINID_ELA << "' first" << std::endl;
			return;
		}
		nlohmann::json tx;
		if (voteType == "cr") {
			std::cout << "Enter vote cr with JSON format: " << std::endl;
			std::string voteJson;
			std::cin >> voteJson;

			tx = subWallet->CreateVoteCRTransaction("", nlohmann::json::parse(voteJson), "");
		} else if (voteType == "dpos") {
			std::cout << "Enter number of votes:";
			int64_t stake;
			std::cin >> stake;
			std::string num = std::to_string(stake);

			std::cout << "Enter vote producer public keys with JSON format:\n";
			std::string pubKeys;
			std::cin >> pubKeys;

			tx = subWallet->CreateVoteProducerTransaction("", num, nlohmann::json::parse(pubKeys), "");
		} else {
			invalidCmdError();
			return;
		}

		signAndPublishTx(subWallet, tx);
	} catch (const std::exception &e) {
		exceptionError(e);
	}

}

// export [mnemonic | keystore]
static void _export(int argc, char *argv[]) {
	if (argc != 2) {
		invalidCmdError();
		return;
	}

	if (currentWallet == nullptr) {
		std::cerr << "no wallet actived" << std::endl;
		return;
	}

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
				return;
			}

			std::string password = getpass("Enter payment password: ");
			nlohmann::json keystore = currentWallet->ExportKeystore(backupPassword, password);
			std::cout << keystore.dump(4) << std::endl;
		} else {
			invalidCmdError();
		}
	} catch (const std::exception &e) {
		std::cerr << "Export mnemonic error: " << e.what() << std::endl;
	}
}

// passwd
static void passwd(int argc, char *argv[]) {
	if (!currentWallet) {
		std::cerr << "no wallet actived" << std::endl;
		return;
	}

	try {
		std::string oldPassword = getpass("Old Password: ");
		if (!currentWallet->VerifyPayPassword(oldPassword)) {
			std::cerr << "Wrong password" << std::endl;
			return;
		}

		std::string newPassword = getpass("New Password: ");
		std::string newPasswordVerify = getpass("Retype New Password: ");
		if (newPassword != newPasswordVerify) {
			std::cerr << "Password not match" << std::endl;
			return;
		}

		currentWallet->ChangePassword(oldPassword, newPassword);
	} catch (const std::exception &e) {
		exceptionError(e);
	}
}

// send [chainID] [address] [amount]
static void _send(int argc, char *argv[]) {
	if (argc != 4) {
		invalidCmdError();
		return;
	}

	if (!currentWallet) {
		std::cerr << "no wallet actived" << std::endl;
		return;
	}

	std::string chainID = argv[1];
	std::string addr = argv[2];
	std::string amount = convertAmount(argv[3]);

	try {
		auto subWallet = currentWallet->GetSubWallet(chainID);
		if (!subWallet) {
			std::cerr << "open " << chainID << " first" << std::endl;
			return;
		}

		nlohmann::json tx = subWallet->CreateTransaction("", addr, amount, "");
		signAndPublishTx(subWallet, tx);
	} catch (const std::exception &e) {
		exceptionError(e);
	}
}

// receive [chainID]
static void _receive(int argc, char *argv[]) {
	if (argc != 2) {
		invalidCmdError();
		return;
	}

	if (!currentWallet) {
		std::cerr << "no wallet actived" << std::endl;
		return;
	}

	std::string chainID = argv[1];
	try {
		auto subWallet = currentWallet->GetSubWallet(chainID);
		if (!subWallet) {
			std::cerr << "open " << chainID << " first" << std::endl;
			return;
		}

		std::cout << subWallet->CreateAddress() << std::endl;
	} catch (const std::exception &e) {
		exceptionError(e);
	}
}

// sync [walletName] [chainID] [start | stop]
static void _sync(int argc, char *argv[]) {
	if (argc != 4) {
		invalidCmdError();
		return;
	}

	std::string walletName = argv[1];
	std::string chainID = argv[2];
	std::string op = argv[3];

	try {
		auto masterWallet = manager->GetMasterWallet(walletName);
		if (!masterWallet) {
			std::cerr << walletName << " not found" << std::endl;
			return;
		}

		auto subWallet = masterWallet->GetSubWallet(chainID);
		if (!subWallet) {
			std::cerr << chainID << " not found" << std::endl;
			return;
		}

		if (op == "start") {
			subWallet->SyncStart();
		} else if (op == "stop") {
			subWallet->SyncStop();
		} else {
			invalidCmdError();
		}
	} catch (const std::exception &e) {
		exceptionError(e);
	}
}

// fixpeer [chainID] [ip] [port]
static void fixpeer(int argc, char *argv[]) {
	if (argc != 4) {
		invalidCmdError();
		return;
	}

	try {
		std::string chainID = argv[1];
		std::string ip = argv[2];
		uint16_t port = (uint16_t)std::stoi(argv[3]);

		if (!currentWallet) {
			std::cerr << "no wallet actived" << std::endl;
			return;
		}

		auto subWallet = currentWallet->GetSubWallet(chainID);
		if (!subWallet) {
			std::cerr << "open " << chainID << " first" << std::endl;
			return;
		}

		if (!subWallet->SetFixedPeer(ip, port))
			std::cerr << "set fixed peer failed" << std::endl;
	} catch (const std::exception &e) {
		exceptionError(e);
	}
}

// tx [chainID]
static void _tx(int argc, char *argv[]) {
	if (argc != 2) {
		invalidCmdError();
		return;
	}

	if (!currentWallet) {
		std::cerr << "no wallet actived" << std::endl;
		return;
	}

	std::string chainID = argv[1];
	try {
		auto subWallet = currentWallet->GetSubWallet(chainID);
		if (!subWallet) {
			std::cerr << chainID << " not found" << std::endl;
			return;
		}

		int cntPerPage = 20;
		int curPage = 1;
		int start, max;
		std::string cmd;
		bool show = true;

		do {
			if (show) {
				start = cntPerPage * (curPage - 1);
				nlohmann::json txJosn = subWallet->GetAllTransaction(start, cntPerPage, "");
				nlohmann::json tx = txJosn["Transactions"];
				max = txJosn["MaxCount"];

				char buf[100];
				struct tm tm;
				for (nlohmann::json::iterator it = tx.begin(); it != tx.end(); ++it) {
					std::string txHash = (*it)["TxHash"];
					std::string confirm = (*it)["ConfirmStatus"];
					time_t t = (*it)["Timestamp"];
					std::string dir = (*it)["Direction"];
					std::string amount = (*it)["Amount"];

					localtime_r(&t, &tm);
					strftime(buf, sizeof(buf), "%F %T", &tm);
					printf("%s  %s  %s  %s  %s\n", txHash.c_str(), dir.c_str(), buf, amount.c_str(), confirm.c_str());
				}
			}

			std::cout << "'n' Next Page, 'b' Previous Page, 'q' Exit: ";
			std::getline(std::cin, cmd);

			if (cmd == "n") {
				if (curPage < (max + cntPerPage) / cntPerPage) {
					curPage++;
					show = true;
				} else {
					std::cout << "already last page" << std::endl;
					show = false;
				}
			} else if (cmd == "b") {
				if (curPage > 1) {
					curPage--;
					show = true;
				} else {
					std::cout << "already first page" << std::endl;
					show = false;
				}
			} else if (cmd == "q") {
				break;
			} else {
				std::cout << "invalid input" << std::endl;
				show = false;
			}
		} while (cmd != "q");
	} catch (const std::exception &e) {
		exceptionError(e);
	}
}

static void verbose(int argc, char *argv[]) {
	bool v = !verboseMode;

	if (argc > 2) {
		invalidCmdError();
		return;
	}

	if (argc > 1) {
		if (strcmp(argv[1], "on") == 0)
			v = true;
		else if (strcmp(argv[1], "off") == 0)
			v = false;
		else {
			std::cerr << "Unknown verbse options: " << argv[1] << std::endl;
			return;
		}
	}

	std::string opt = v ? "on" : "off";
	std::cout << "Verbose mode: " << opt << std::endl;
	verboseMode = v;
}

static void help(int argc, char *argv[]);

struct command {
	const char *cmd;

	void (*function)(int argc, char *argv[]);

	const char *help;
} commands[] = {
	{"help",     help,           "[command]                               Display available command list, or usage description for specific command."},
	{"create",   create,         "[walletName]                            Create a new wallet with given name."},
	{"import",   import,         "[walletName] [m[nemonic] | [k[eystore]] Import wallet with given name and mnemonic or keystore."},
	{"remove",   remove,         "[walletName]                            Remove specified wallet."},
	{"open",     _open,          "[walletName] [chainID]                  Open wallet of `chainID`."},
	{"close",    _close,         "[walletName] [chainID]                  Close wallet of `chainID`."},
	{"sync",     _sync,          "[walletName] [chainID] [start | stop]   Start or stop sync of wallet"},
	{"switch",   _switch,        "[walletName]                            Switch current wallet."},
	{"list",     list,           "                                        List all wallets."},
	{"passwd",   passwd,         "                                        Change password of wallet."},

	{"diddoc",   diddoc,         "                                        Create DID document."},
	{"didtx",    didtx,          "                                        Create DID transaction."},
	{"didsign",  didsign,        "[DID] [digest]                          Sign `digest` with private key of DID."},

	{"tx",       _tx,            "[chainID]                               List all tx records."},
	{"fixpeer",  fixpeer,        "[chainID] [ip] [port]                   Set fixed peer to sync."},
	{"send",     _send,          "[chainID] [address] [amount]            Send ELA from `chainID`."},
	{"receive",  _receive,       "[chainID]                               Get receive address of `chainID`."},
	{"address",  address,        "[chainID]                               Get the revceive address of chainID."},
	{"proposal", proposal,       "[sponsor | crsponsor]                   Sponsor sign proposal or cr sponsor create proposal tx."},
	{"deposit",  deposit,        "[chainID] [amount] [address]            Deposit to sidechain from mainchain."},
	{"withdraw", withdraw,       "[chainID] [amount] [address]            Withdraw from sidechain to mainchain."},
	{"export",   _export,        "[m[nemonic] | [k[eystore]]              Export mnemonic or keystore."},
	{"register", _register,      "[cr | dpos]                             Register CR or DPoS with specified wallet."},
	{"vote",     vote,           "[cr | dpos]                             CR/DPoS vote."},
	{"verbose",  verbose,        "[on | off]                              Set verbose mode."},
	{"exit", NULL,               "                                        Quit wallet."},
	{"quit", NULL,               "                                        Quit wallet."},
	{NULL,   NULL, NULL}
};

static void help(int argc, char *argv[]) {
	struct command *p;

	std::cout << std::endl << "Usage: [Command] [Arguments]..." << std::endl << std::endl;
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
				return;
			}
		}

		std::cerr << "Unknown command: " << argv[1] << std::endl;
	}
}

int run_cmd(int argc, char *argv[]) {
	struct command *p;

	for (p = commands; p->cmd; p++) {
		if (strcmp(argv[0], p->cmd) == 0) {
			p->function(argc, argv);
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

static const char *getWalletDir(const char *dir) {
	static char walletDir[PATH_MAX];
	struct stat st;

	if (!dir) {
		sprintf(walletDir, "%s/.elawallet", getenv("HOME"));
		dir = (const char *) walletDir;
	}

	return dir;
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

int main(int argc, char *argv[]) {
	const char *walletRoot = NULL;
	char cmdLine[4096];
	char *cmdArgs[512];
	int numArgs;
	int waitForAttach = 0;

	int opt;
	int idx;
	struct option options[] = {
		{"data",    required_argument, NULL, 'd'},
		{"verbose", no_argument,       NULL, 'v'},
		{"debug",   no_argument,       NULL, 1},
		{"help",    no_argument,       NULL, 'h'},
		{NULL, 0,                      NULL, 0}
	};

#ifdef HAVE_SYS_RESOURCE_H
	sys_coredump_set(true);
#endif

	while ((opt = getopt_long(argc, argv, "d:vh?", options, &idx)) != -1) {
		switch (opt) {
			case 'd':
				walletRoot = optarg;
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
				//usage();
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
	if (mkdirs(walletRoot, S_IRWXU) < 0) {
		fprintf(stderr, "Create wallet data directory '%s' failed.\n", walletRoot);
		return -1;
	} else {
		fprintf(stdout, "Wallet data directory: %s\n", walletRoot);
	}

	manager = new MasterWalletManager(walletRoot, "MainNet");
	walletInit();

	while (1) {
		fprintf(stdout, "-> ELAWallet $ ");
		fgets(cmdLine, sizeof(cmdLine), stdin);

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
}
