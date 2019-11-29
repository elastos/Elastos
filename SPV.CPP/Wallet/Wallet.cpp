#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
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
#include <IMainchainSubWallet.h>
#include <IIDChainSubWallet.h>

#include <string>
#include <iostream>
#include <fstream>
#include <vector>

using namespace Elastos::ElaWallet;

#define ERRNO_APP -255
#define ERRNO_CMD -1

#define SELA_PER_ELA 100000000UL
static const std::string CHAINID_ELA = "ELA";
static const std::string CHAINID_ID = "IDChain";
static std::string walletRoot;
static std::string network;

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

static void signAndPublishTx(ISubWallet *subWallet, const nlohmann::json &tx, const std::string &payPasswd = "") {
	std::string password = payPasswd;

	std::cout << "Fee: " << tx["Fee"] << std::endl;
	if (payPasswd.empty()) {
		password = getpass("Enter payment password: ");
	}
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

	auto callback = static_cast<SubWalletCallback *>(masterWalletData[walletName][chainID].GetCallback());
	delete callback;
	callback = nullptr;

	masterWalletData[walletName].erase(chainID);
	if (masterWalletData[walletName].empty())
		masterWalletData.erase(walletName);
}

static void walletInit(void) {
	auto masterWallets = manager->GetAllMasterWallets();
	for (IMasterWallet *masterWallet : masterWallets) {
		if (!currentWallet)
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

		subWalletOpen(masterWallet, subWallet);
		std::cout << "Wallet create success." << std::endl;

		if (!currentWallet) {
			currentWallet = masterWallet;
		}
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

		if (!currentWallet) {
			currentWallet = masterWallet;
		}

		auto subWallets = masterWallet->GetAllSubWallets();
		for (ISubWallet *subWallet : subWallets) {
			subWalletOpen(masterWallet, subWallet);
		}

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

		auto subWallets = masterWallet->GetAllSubWallets();

		for (ISubWallet *subWallet : subWallets) {
			subWalletClose(masterWallet, subWallet);
		}

		manager->DestroyWallet(walletName);
		std::cout << "Wallet '" << walletName << "' removed." << std::endl;

		if (needUpdate)  {
			auto masterWallets = manager->GetAllMasterWallets();
			if (!masterWallets.empty()) {
				currentWallet = masterWallets[0];
			} else {
				currentWallet = nullptr;
			}
		}
	} catch (const std::exception &e) {
		exceptionError(e);
		return ERRNO_APP;
	}
	return 0;
}

// list [all]
static int list(int argc, char *argv[]) {
	std::vector<std::string> subWalletIDList = {CHAINID_ELA, CHAINID_ID};
	if (argc == 1) {
		checkCurrentWallet();
	} else if (argc >= 2) {
		if (std::string(argv[1]) != "all") {
			invalidCmdError();
			return ERRNO_CMD;
		}
	}

	try {
		auto masterWallets = manager->GetAllMasterWallets();
		printf("%-20s", "WalletName");
		for (const std::string &chainID : subWalletIDList)
			printf("%50s", chainID.c_str());
		printf("\n%s\n", SplitLine);

		for (IMasterWallet *masterWallet : masterWallets) {
			if (argc == 1 && currentWallet != masterWallet)
				continue;

			struct tm tm;
			time_t lastBlockTime;
			int progress;
			double balance;
			ISubWallet *subWallet;
			char info[256];
			char buf[100] = {0};

			printf(" %c %-17s", masterWallet == currentWallet ? '*' : ' ', masterWallet->GetID().c_str());
			for (const std::string& chainID: subWalletIDList) {
				subWallet = masterWallet->GetSubWallet(chainID);
				if (subWallet) {
					balance = std::stod(subWallet->GetBalance()) / SELA_PER_ELA;

					lastBlockTime = masterWalletData[masterWallet->GetID()][chainID].GetLastBlockTime();
					progress = masterWalletData[masterWallet->GetID()][chainID].GetSyncProgress();
					localtime_r(&lastBlockTime, &tm);
					strftime(buf, sizeof(buf), "%F %T", &tm);

					snprintf(info, sizeof(info), "%.8lf  [%s  %3d%%]", balance, buf, progress);
				} else {
					snprintf(info, sizeof(info), "--");
				}

				printf("%50s", info);
			}
			printf("\n");
		}
	} catch (const std::exception &e) {
		exceptionError(e);
		return ERRNO_APP;
	}

	return 0;
}

// switch walletName
static int _switch(int argc, char *argv[]) {
	checkParam(2);

	const std::string walletName = argv[1];
	try {
		auto masterWallet = manager->GetMasterWallet(walletName);
		if (masterWallet == nullptr) {
			std::cerr << walletName << " not found" << std::endl;
			return ERRNO_APP;
		}

		currentWallet = masterWallet;
	} catch (const std::exception &e) {
		exceptionError(e);
		return ERRNO_APP;
	}
	return 0;
}

// address chainID
static int address(int argc, char *argv[]) {
	checkParam(2);
	checkCurrentWallet();

	std::string chainID = argv[1];

	try {
		ISubWallet *subWallet;
		getSubWallet(subWallet, currentWallet, chainID);

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
		auto subWallet = currentWallet->CreateSubWallet(chainID);

		subWalletOpen(currentWallet, subWallet);
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
		ISubWallet *subWallet;
		getSubWallet(subWallet, currentWallet, chainID);
		subWalletClose(currentWallet, subWallet);
		currentWallet->DestroyWallet(chainID);
	} catch (const std::exception &e) {
		exceptionError(e);
		return ERRNO_APP;
	}
	return 0;
}

// deposit amount address
static int deposit(int argc, char *argv[]) {
	checkParam(3);
	checkCurrentWallet();

	std::string amount = convertAmount(argv[1]);
	std::string sideChainAddress = argv[2];

	try {
		IMainchainSubWallet *subWallet;
		getSubWallet(subWallet, currentWallet, CHAINID_ELA);

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
			for (const std::string &id: supportChainID)
				std::cout << "  " << id << std::endl;
			std::cout << "Which side chain do you want to top up: ";
			std::cin >> chainID;
		} else {
			chainID = supportChainID[0];
		}

		nlohmann::json tx = subWallet->CreateDepositTransaction(
			"", chainID, amount, sideChainAddress, "");

		std::cout << "Top up " << sideChainAddress << ":" << amount << " to " << chainID << std::endl;

		signAndPublishTx(subWallet, tx);
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
			std::cin >> chainID;
		} else {
			chainID = supportChainID[0];
		}

		ISidechainSubWallet *subWallet;
		getSubWallet(subWallet, currentWallet, chainID);

		nlohmann::json tx = subWallet->CreateWithdrawTransaction(
			"", amount, mainChainAddress, "");

		std::cout << "Withdraw " << mainChainAddress << ":" << amount << " from " << chainID << std::endl;

		signAndPublishTx(subWallet, tx);
	} catch (const std::exception &e) {
		exceptionError(e);
		return ERRNO_APP;
	}
	return 0;
}

// diddoc
static int diddoc(int argc, char *argv[]) {
	checkCurrentWallet();

	try {
		IIDChainSubWallet *subWallet;
		getSubWallet(subWallet, currentWallet, CHAINID_ID);

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
		if (argc == 2) {
			std::string filepath = argv[1];
			std::ifstream is(filepath);
			std::string line;
			while (!is.eof()) {
				is >> line;
				didDoc += line;
				line.clear();
			}
		} else {
			struct termios told = enableLongInput();
			std::cout << "Enter id document: ";
			std::getline(std::cin, didDoc);
			recoveryTTYSetting(&told);
		}

		nlohmann::json tx = subWallet->CreateIDTransaction(nlohmann::json::parse(didDoc), "");
		signAndPublishTx(subWallet, tx);
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

// registercr (cr | dpos)
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

			std::string crPublicKey, nickName, url, did;
			uint64_t location;

			crPublicKey = idSubWallet->GetAllPublicKeys(0, 1)["PublicKeys"][0];
			did = idSubWallet->GetPublicKeyDID(crPublicKey);

			std::cout << "DID public key: " << crPublicKey << std::endl;
			std::cout << "DID: " << did << std::endl;

			std::cout << "Enter nick name: ";
			std::cin >> nickName;

			std::cout << "Enter url: ";
			std::cin >> url;

			std::cout << "Enter location code (example 86): ";
			std::cin >> location;

			nlohmann::json payload = subWallet->GenerateCRInfoPayload(crPublicKey, nickName, url, location);

			std::string digest = payload["Digest"].get<std::string>();

			password = getpass("Enter payment password: ");

			std::string signature = idSubWallet->SignDigest(did, digest, password);
			payload["Signature"] = signature;

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

			password = getpass("Enter payment password: ");
			nlohmann::json payload = subWallet->GenerateProducerPayload(ownerPubkey, nodePubkey, nickName, url, "", location, password);
			tx = subWallet->CreateRegisterProducerTransaction("", payload, convertAmount("5000"), "");
		} else {
			invalidCmdError();
			return ERRNO_APP;
		}

		signAndPublishTx(subWallet, tx, password);
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

			std::string did = idSubWallet->GetAllDID(0, 1)["DID"][0];

			nlohmann::json payload = subWallet->GenerateUnregisterCRPayload(did);
			std::string digest = payload["Digest"].get<std::string>();

			password = getpass("Enter payment password: ");

			std::string signature = idSubWallet->SignDigest(did, digest, password);
			payload["Signature"] = signature;

			tx = subWallet->CreateUnregisterCRTransaction("", payload, "");
		} else if (registerWhat == "dpos") {
			std::string ownerPubkey = subWallet->GetOwnerPublicKey();
			password = getpass("Enter payment password: ");
			nlohmann::json payload = subWallet->GenerateCancelProducerPayload(ownerPubkey, password);
			tx = subWallet->CreateCancelProducerTransaction("", payload, "");
		} else {
			invalidCmdError();
			return ERRNO_APP;
		}

		signAndPublishTx(subWallet, tx, password);
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

			std::string crPublicKey = idSubWallet->GetAllPublicKeys(0, 1)["PublicKeys"][0];
			std::string did = idSubWallet->GetPublicKeyDID(crPublicKey);

			std::cout << "DID public key: " << crPublicKey << std::endl;
			std::cout << "DID: " << did << std::endl;

			std::cout << "Enter retrieve amount: ";
			std::string amount;
			std::cin >> amount;

			tx = subWallet->CreateRetrieveCRDepositTransaction(crPublicKey, convertAmount(amount), "");

		} else if (registerWhat == "dpos") {

			std::cout << "Enter retrieve amount: ";
			std::string amount;
			std::cin >> amount;

			tx = subWallet->CreateRetrieveDepositTransaction(convertAmount(amount), "");
		} else {
			invalidCmdError();
			return ERRNO_APP;
		}

		signAndPublishTx(subWallet, tx);
	} catch (const std::exception &e) {
		exceptionError(e);
		return ERRNO_APP;
	}
	return 0;
}

// proposal (sponsor | crsponsor)
static int proposal(int argc, char *argv[]) {
	checkParam(2);
	checkCurrentWallet();
	std::string what = argv[1];

	try {
		IMainchainSubWallet *subWallet;
		getSubWallet(subWallet, currentWallet, CHAINID_ELA);

		IIDChainSubWallet *idSubWallet;
		getSubWallet(idSubWallet, currentWallet, CHAINID_ID);

		if (what == "sponsor") {
			int type = 0;
			std::string draftHash;
			std::string budgetList;
			std::string receiptAddress;
			std::string crSponsorDID;

			std::string sponsorPubKey = idSubWallet->GetAllPublicKeys(0, 1)["PublicKeys"][0];
			std::cout << "sponsor public key: " << sponsorPubKey << std::endl;

			std::string sponsorDID = idSubWallet->GetPublicKeyDID(sponsorPubKey);
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

			nlohmann::json proposalJson = subWallet->SponsorProposalDigest(type, sponsorPubKey, draftHash, budgets,
																					receiptAddress);
			std::string digest = proposalJson["Digest"].get<std::string>();

			std::string password = getpass("Enter payment password: ");
			std::string signature = idSubWallet->SignDigest(sponsorDID, digest, password);
			proposalJson["Signature"] = signature;

			std::cout << proposalJson.dump() << std::endl;
		} else if (what == "crsponsor") {
			std::string tmp;
			std::cout << "Enter sponsor signed proposal: " << std::endl;
			std::cin >> tmp;

			nlohmann::json sponsorSignedProposal = nlohmann::json::parse(tmp);

			std::string crSponsorPubKey = idSubWallet->GetAllPublicKeys(0, 1)["PublicKeys"][0];
			std::cout << "CR sponsor public key: " << crSponsorPubKey << std::endl;
			std::string crSponsorDID = idSubWallet->GetPublicKeyDID(crSponsorPubKey);
			std::cout << "CR sponsor DID: " << crSponsorDID << std::endl;

			nlohmann::json proposalJson = subWallet->CRSponsorProposalDigest(sponsorSignedProposal, crSponsorDID);
			std::string digest = proposalJson["Digest"].get<std::string>();

			std::string password = getpass("Enter payment password: ");
			std::string signature = idSubWallet->SignDigest(crSponsorDID, digest, password);
			proposalJson["CRSignature"] = signature;

			nlohmann::json tx = subWallet->CreateCRCProposalTransaction(proposalJson, "");

			signAndPublishTx(subWallet, tx);
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

// tracking (leader | newLeader | secretaryGeneral)
static int tracking(int argc, char *argv[]) {
	checkParam(2);
	checkCurrentWallet();

	std::string what = argv[1];

	try {
		IMainchainSubWallet *subWallet;
		getSubWallet(subWallet, currentWallet, CHAINID_ELA);

		IIDChainSubWallet *idSubWallet;
		getSubWallet(idSubWallet, currentWallet, CHAINID_ID);

		if (what == "leader") {
			int type = 0;
			std::string proposalHash, documentHash, appropriation, leaderPubKey, newLeaderPubKey;
			uint8_t stage;

			std::cout << "Select proposal tracking type: " << std::endl;
			std::cout << "0 (Default) :" << "common" << std::endl;
			std::cout << "1 :" << "progress" << std::endl;
			std::cout << "2 :" << "progressReject" << std::endl;
			std::cout << "3 :" << "terminated" << std::endl;
			std::cout << "4 :" << "proposalLeader" << std::endl;
			std::cout << "5 :" << "appropriation" << std::endl;
			std::cout << "Enter tracking type (empty for default): ";

			std::string trackingType;
			std::getline(std::cin, trackingType);
			if (!trackingType.empty())
				type = std::stoi(trackingType);

			std::cout << "Enter proposal hash: ";
			std::cin >> proposalHash;

			std::cout << "Enter tracking document hash: ";
			std::cin >> documentHash;

			std::cout << "Enter tracking proposal stage: ";
			std::cin >> stage;

			std::cout << "Enter tracking proposal appropriation: ";
			std::cin >> appropriation;

			leaderPubKey = idSubWallet->GetAllPublicKeys(0, 1)["PublicKeys"][0];
			std::string leaderDID = idSubWallet->GetPublicKeyDID(leaderPubKey);
			std::cout << "leader DID: " << leaderDID << std::endl;
			std::cout << "leader leaderPubKey: " << leaderPubKey << std::endl;

			if (type == 4) {
				std::cout << "Enter proposal new leader public key: ";
				std::cin >> newLeaderPubKey;

				std::string newLeaderDID = idSubWallet->GetPublicKeyDID(newLeaderPubKey);
				std::cout << "new leader DID: " << newLeaderDID << std::endl;
			}

			nlohmann::json payloadJson = subWallet->LeaderProposalTrackDigest(type, proposalHash, documentHash,
					stage, appropriation, leaderPubKey, newLeaderPubKey);
			std::string digest = payloadJson["Digest"].get<std::string>();

			std::string password = getpass("Enter payment password: ");
			std::string signature = idSubWallet->SignDigest(leaderDID, digest, password);
			payloadJson["LeaderSign"] = signature;

			std::cout << payloadJson.dump() << std::endl;
		} else if (what == "newLeader") {
			std::string tmp;
			std::cout << "Enter leader signed proposal tracking json: " << std::endl;
			std::cin >> tmp;

			nlohmann::json leaderSignedProposalTracking = nlohmann::json::parse(tmp);

			std::string newLeaderPubKey = idSubWallet->GetAllPublicKeys(0, 1)["PublicKeys"][0];
			std::cout << "new leader of sponsor public key: " << newLeaderPubKey << std::endl;
			std::string newLeaderDID = idSubWallet->GetPublicKeyDID(newLeaderPubKey);
			std::cout << "New leader DID: " << newLeaderDID << std::endl;

			nlohmann::json payloadJson = subWallet->NewLeaderProposalTrackDigest(leaderSignedProposalTracking);
			std::string digest = payloadJson["Digest"].get<std::string>();

			std::string password = getpass("Enter payment password: ");
			std::string signature = idSubWallet->SignDigest(newLeaderDID, digest, password);
			payloadJson["NewLeaderSign"] = signature;

			std::cout << payloadJson.dump() << std::endl;
		} else if (what == "secretaryGeneral") {
			std::string tmp;
			std::cout << "Enter leader signed proposal tracking json: " << std::endl;
			std::cin >> tmp;

			nlohmann::json leaderSignedProposalTracking = nlohmann::json::parse(tmp);

			std::string did = idSubWallet->GetAllDID(0, 1)["DID"][0];
			std::cout << "SecretaryGeneral DID: " << did << std::endl;

			nlohmann::json payloadJson = subWallet->SecretaryGeneralProposalTrackDigest(leaderSignedProposalTracking);
			std::string digest = payloadJson["Digest"].get<std::string>();

			std::string password = getpass("Enter payment password: ");
			std::string signature = idSubWallet->SignDigest(did, digest, password);
			payloadJson["SecretaryGeneralSign"] = signature;

			nlohmann::json tx = subWallet->CreateProposalTrackingTransaction(payloadJson, "");
			signAndPublishTx(subWallet, tx, password);
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

// vote (cr | dpos)
static int vote(int argc, char *argv[]) {
	checkParam(2);
	checkCurrentWallet();
	std::string voteType = argv[1];

	try {
		IMainchainSubWallet *subWallet;
		getSubWallet(subWallet, currentWallet, CHAINID_ELA);
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
			return ERRNO_APP;
		}

		signAndPublishTx(subWallet, tx);
	} catch (const std::exception &e) {
		exceptionError(e);
		return ERRNO_APP;
	}
	return 0;
}

// export (mnemonic | keystore)
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
			std::cout << keystore.dump(4) << std::endl;
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

// passwd
static int passwd(int argc, char *argv[]) {
	checkCurrentWallet();
	try {
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
	} catch (const std::exception &e) {
		exceptionError(e);
		return ERRNO_APP;
	}
	return 0;
}

// transfer chainID address amount
static int transfer(int argc, char *argv[]) {
	checkParam(4);
	checkCurrentWallet();

	std::string chainID = argv[1];
	std::string addr = argv[2];
	std::string amount = convertAmount(argv[3]);

	try {
		ISubWallet *subWallet;
		getSubWallet(subWallet, currentWallet, chainID);

		nlohmann::json tx = subWallet->CreateTransaction("", addr, amount, "");
		signAndPublishTx(subWallet, tx);
	} catch (const std::exception &e) {
		exceptionError(e);
		return ERRNO_APP;
	}
	return 0;
}

// receive chainID
static int _receive(int argc, char *argv[]) {
	checkParam(2);
	checkCurrentWallet();

	std::string chainID = argv[1];
	try {
		ISubWallet *subWallet;
		getSubWallet(subWallet, currentWallet, chainID);

		std::cout << subWallet->CreateAddress() << std::endl;
	} catch (const std::exception &e) {
		exceptionError(e);
		return ERRNO_APP;
	}
	return 0;
}

// sync chainID (start | stop)
static int _sync(int argc, char *argv[]) {
	checkParam(3);
	checkCurrentWallet();

	std::string chainID = argv[1];
	std::string op = argv[2];

	try {
		ISubWallet *subWallet;
		getSubWallet(subWallet, currentWallet, chainID);
		if (op == "start") {
			subWallet->SyncStart();
		} else if (op == "stop") {
			subWallet->SyncStop();
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

// fixpeer chainID ip port
static int fixpeer(int argc, char *argv[]) {
	checkParam(4);
	checkCurrentWallet();

	try {
		std::string chainID = argv[1];
		std::string ip = argv[2];
		uint16_t port = (uint16_t)std::stoi(argv[3]);

		ISubWallet *subWallet;
		getSubWallet(subWallet, currentWallet, chainID);

		if (!subWallet->SetFixedPeer(ip, port)) {
			std::cerr << "set fixed peer failed" << std::endl;
			return ERRNO_APP;
		}
	} catch (const std::exception &e) {
		exceptionError(e);
		return ERRNO_APP;
	}
	return 0;
}

// tx [chainID]
static int _tx(int argc, char *argv[]) {
	checkParam(2);
	checkCurrentWallet();

	std::string chainID = argv[1];
	try {
		ISubWallet *subWallet;
		getSubWallet(subWallet, currentWallet, chainID);

		int cntPerPage = 20;
		int curPage = 1;
		int start, max;
		std::string cmd;
		bool show = true;
		std::string txHash;

		do {
			if (show) {
				start = cntPerPage * (curPage - 1);
				nlohmann::json txJosn = subWallet->GetAllTransaction(start, cntPerPage, txHash);
				nlohmann::json tx = txJosn["Transactions"];
				max = txJosn["MaxCount"];

				printf("%d / %d\n", curPage, (max + cntPerPage) / cntPerPage);
				char buf[100];
				struct tm tm;
				for (nlohmann::json::iterator it = tx.begin(); it != tx.end(); ++it) {
					if (txHash.empty()) {
						std::string txHash = (*it)["TxHash"];
						std::string confirm = (*it)["ConfirmStatus"];
						time_t t = (*it)["Timestamp"];
						std::string dir = (*it)["Direction"];
						double amount = std::stod((*it)["Amount"].get<std::string>()) / SELA_PER_ELA;

						localtime_r(&t, &tm);
						strftime(buf, sizeof(buf), "%F %T", &tm);
						printf("%s  %2s  %8s  %s  %.8lf\n", txHash.c_str(), confirm.c_str(), dir.c_str(), buf, amount);
					} else {
						std::cout << (*it).dump(4) << std::endl;
					}
				}
				txHash.clear();
			}

			std::cout << "'txid' Detail, 'n' Next Page, 'b' Previous Page, 'q' Exit: ";
			std::getline(std::cin, cmd);

			if (cmd == "n") {
				if (curPage < (max + cntPerPage) / cntPerPage) {
					curPage++;
					show = true;
				} else {
					std::cout << "already last page" << std::endl;
					show = true;
				}
			} else if (cmd == "b") {
				if (curPage > 1) {
					curPage--;
					show = true;
				} else {
					std::cout << "already first page" << std::endl;
					show = true;
				}
			} else if (cmd == "q") {
				break;
			} else if (cmd.size() == 64) {
				txHash = cmd;
				show = true;
			} else {
				std::cout << "invalid input" << std::endl;
				show = false;
			}
		} while (cmd != "q");
	} catch (const std::exception &e) {
		exceptionError(e);
		return ERRNO_APP;
	}
	return 0;
}

// utxo chainID
static int utxo(int argc, char *argv[]) {
	checkParam(2);
	checkCurrentWallet();

	std::string chainID = argv[1];
	try {
		ISubWallet *subWallet;
		getSubWallet(subWallet, currentWallet, chainID);

		int cntPerPage = 20;
		int curPage = 1;
		int start, max;
		std::string cmd;
		bool show = true;

		do {
			if (show) {
				start = cntPerPage * (curPage - 1);
				nlohmann::json utxoJosn = subWallet->GetAllUTXOs(start, cntPerPage, "");
				nlohmann::json utxo = utxoJosn["UTXOs"];
				max = utxoJosn["MaxCount"];

				printf("%d / %d\n", curPage, (max + cntPerPage) / cntPerPage);
				char buf[100];
				struct tm tm;
				for (nlohmann::json::iterator it = utxo.begin(); it != utxo.end(); ++it) {
					std::string txHash = (*it)["Hash"];
					int index = (*it)["Index"];
					double amount = std::stod((*it)["Amount"].get<std::string>()) / SELA_PER_ELA;

					printf("%s:%-5d  %.8lf\n", txHash.c_str(), index, amount);
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
		return ERRNO_APP;
	}
	return 0;
}

// balance chainID
static int balance(int argc, char *argv[]) {
	checkParam(2);
	checkCurrentWallet();

	std::string chainID = argv[1];
	try {
		ISubWallet *subWallet;
		getSubWallet(subWallet, currentWallet, chainID);
		nlohmann::json balance = subWallet->GetBalanceInfo();

		std::cout << balance.dump(4) << std::endl;
	} catch(const std::exception &e) {
		exceptionError(e);
		return ERRNO_APP;
	}
	return 0;
}

// network [netType]
static int _network(int argc, char *argv[]) {

	if (argc == 1) {
		std::cout << "Network: " << network << std::endl;
	} else {
		checkParam(2);
		nlohmann::json netConfig;
		std::string netType = argv[1];
		if (netType == "PrvNet") {
			std::string filepath;
			std::cout << "Enter config file for private network (empty for default): ";
			std::getline(std::cin, filepath);
			std::cout << "filepath: " << filepath << std::endl;
			if (!filepath.empty()) {
				try {
					std::ifstream in(filepath);
					netConfig = nlohmann::json::parse(in);
				} catch (const std::exception &e) {
					std::cerr << "Load network config '" << filepath << "' failed: " << e.what() << std::endl;
					return ERRNO_APP;
				}
			}
		} else if (netType != "MainNet" && netType != "TestNet" && netType != "RegTest") {
			invalidCmdError();
			return ERRNO_CMD;
		}

		network = netType;

		walletCleanup();
		delete manager;

		manager = new MasterWalletManager(walletRoot, network, netConfig);
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
	{"help",     help,           "[command]                                        Display available command list, or usage description for specific command."},
	{"create",   create,         "walletName                                       Create a new wallet with given name."},
	{"import",   import,         "walletName (m[nemonic] | k[eystore]) [filePath]  Import wallet with given name and mnemonic or keystore."},
	{"remove",   remove,         "walletName                                       Remove specified wallet."},
	{"switch",   _switch,        "walletName                                       Switch current wallet."},
	{"list",     list,           "[all]                                            List all wallets or current wallet."},
	{"passwd",   passwd,         "                                                 Change password of wallet."},
	{"diddoc",   diddoc,         "                                                 Create DID document."},
	{"didtx",    didtx,          "[didDocFilepath]                                 Create DID transaction."},
	{"didsign",  didsign,        "DID digest                                       Sign `digest` with private key of DID."},
	{"sync",     _sync,          "chainID (start | stop)                           Start or stop sync of wallet"},
	{"open",     _open,          "chainID                                          Open wallet of `chainID`."},
	{"close",    _close,         "chainID                                          Close wallet of `chainID`."},
	{"tx",       _tx,            "chainID                                          List all tx records."},
	{"utxo",     utxo,           "chainID                                          List all utxos"},
	{"balance",  balance,        "chainID                                          List balance info"},
	{"fixpeer",  fixpeer,        "chainID ip port                                  Set fixed peer to sync."},
	{"transfer", transfer,       "chainID address amount                           Transfer ELA from `chainID`."},
	{"receive",  _receive,       "chainID                                          Get receive address of `chainID`."},
	{"address",  address,        "chainID                                          Get the revceive address of chainID."},
	{"proposal", proposal,       "(sponsor | crsponsor)                            Sponsor sign proposal or cr sponsor create proposal tx."},
	{"deposit",  deposit,        "amount address                                   Deposit to sidechain from mainchain."},
	{"withdraw", withdraw,       "amount address                                   Withdraw from sidechain to mainchain."},
	{"export",   _export,        "(m[nemonic] | k[eystore])                        Export mnemonic or keystore."},
	{"register", _register,      "(cr | dpos)                                      Register CR or DPoS with specified wallet."},
	{"unregister", unregister,   "(cr | dpos)                                      Unregister CR or DPoS with specified wallet."},
	{"retrieve", retrieve,       "(cr | dpos)                                      Retrieve Pledge with specified wallet."},
	{"vote",     vote,           "(cr | dpos)                                      CR/DPoS vote."},
	{"network",  _network,       "[netType]                                        Show current net type or set net type: 'MainNet', 'TestNet', 'RegTest', 'PrvNet'"},
	{"verbose",  verbose,        "(on | off)                                       Set verbose mode."},
	{"tracking", tracking,       "(leader | newLeader | secretaryGeneral)          Leader of sponsor sign tracking proposal or secretaryGeneral create tracking proposal tx."},
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
	nlohmann::json netConfig;

	int opt;
	int idx;
	struct option options[] = {
		{"data",            required_argument, NULL, 'd'},
		{"network",         required_argument, NULL, 'n' },
		{"verbose",         no_argument,       NULL, 'v'},
		{"debug",           no_argument,       NULL, 1},
		{"help",            no_argument,       NULL, 'h'},
		{NULL,              0,                 NULL, 0}
	};

#ifdef HAVE_SYS_RESOURCE_H
	sys_coredump_set(true);
#endif

	while ((opt = getopt_long(argc, argv, "d:n:vh?", options, &idx)) != -1) {
		switch (opt) {
			case 'd':
				walletRoot = optarg;
				break;

			case 'n':
				network = optarg;
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

	if (!network.empty()) {
		// user config
		if (network == "MainNet" || network == "TestNet" || network == "RegTest") {
			netConfig = nlohmann::json();
		} else {
			try {
				std::ifstream in(network);
				netConfig = nlohmann::json::parse(in);
				network = "PrvNet";
			} catch (const std::exception &e) {
				std::cerr << "Load network config '" << network << "' failed: " << e.what() << std::endl;
				return -1;
			}
		}
	} else {
		// load from local Config.json
		try {
			std::ifstream in(walletRoot + "/Config.json");
			netConfig = nlohmann::json::parse(in);
			network = netConfig["NetType"];
		} catch (...) {
			// ignore exception, use default "MainNet" as config
			netConfig = nlohmann::json();
			network.clear();
		}
	}

	if (network.empty()) {
		network = "MainNet";
		netConfig = nlohmann::json();
	}

	manager = new MasterWalletManager(walletRoot, network, netConfig);
	walletInit();

	while (1) {
		fprintf(stdout, "-> wallet $ ");
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

	return 0;
}
