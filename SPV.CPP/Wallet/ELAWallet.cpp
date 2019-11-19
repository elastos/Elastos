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

#define SELA_PER_ELA 100000000
static const char *MAIN_CHAIN = "ELA";
static const char *ID_CHAIN = "IDChain";

static MasterWalletManager *manager;

static bool verboseMode = false;
static const char *SplitLine = "------------------------------------------------------------";

class SubWalletCallback : public ISubWalletCallback {
public:
	~SubWalletCallback() {}

	SubWalletCallback(const std::string &walletID) : _walletID(walletID) {}

	virtual void OnTransactionStatusChanged(
		const std::string &txid, const std::string &status,
		const nlohmann::json &desc, uint32_t confirms) {
		if (verboseMode)
			std::cout << "*** Wallet " << _walletID << " transaction: "
					  << txid << " changed to " << status << ", confirms: "
					  << confirms << std::endl;
	}

	virtual void OnBlockSyncStarted() {
		if (verboseMode)
			std::cout << "*** Wallet " << _walletID << " sycn started." << std::endl;
	}

	virtual void OnBlockSyncProgress(const nlohmann::json &progressInfo) {
		if (verboseMode)
			std::cout << "*** Wallet " << _walletID << " syncing:"
					  << progressInfo.dump() << std::endl;
	}

	virtual void OnBlockSyncStopped() {
		if (verboseMode)
			std::cout << "*** Wallet " << _walletID << " sync stopped." << std::endl;
	}

	virtual void OnBalanceChanged(const std::string &asset, const std::string &balance) {
		if (verboseMode)
			std::cout << "*** Wallet " << _walletID << " balance changed: "
					  << balance << std::endl;
	}

	virtual void OnTxPublished(const std::string &hash, const nlohmann::json &result) {
		if (verboseMode)
			std::cout << "*** Wallet " << _walletID << " public a new transaction: "
					  << hash << std::endl;
	}

	virtual void OnAssetRegistered(const std::string &asset, const nlohmann::json &info) {
	}

	virtual void OnConnectStatusChanged(const std::string &status) {
		if (verboseMode)
			std::cout << "*** Wallet " << _walletID << " connection status: "
					  << status << std::endl;
	}

private:
	std::string _walletID;
};

static int getPaymentPassword(std::string &password) {
	std::string p1, p2;

	int tryagain = 3;
	while (tryagain--) {
		p1 = getpass("Enter payment password: ");
		p2 = getpass("Enter same payment password again: ");
		if (p1 == p2 && !p1.empty()) {
			break;
		}

		if (p1 != p2)
			std::cerr << "Password not match, try again" << std::endl;
		else
			std::cerr << "Password should not be empty, try again" << std::endl;
	}

	if (tryagain <= 0)
		return -1;

	password = p1;

	return 0;
}

static int getBackupPassword(std::string &password) {
	std::string p1, p2;

	int tryagain = 3;
	while (tryagain--) {
		p1 = getpass("Enter backup password: ");
		p2 = getpass("Enter same backup password again: ");
		if (p1 == p2 && !p1.empty()) {
			break;
		}

		if (p1 != p2)
			std::cerr << "Password not match, try again" << std::endl;
		else
			std::cerr << "Password should not be empty, try again" << std::endl;
	}

	if (tryagain <= 0)
		return -1;

	password = p1;

	return 0;
}

static int getPassphrase(std::string &passphrase) {
	std::string passphrase1, passphrase2;

	int tryagain = 3;
	while (tryagain--) {
		passphrase1 = getpass("Enter passphrase (empty for no passphrase): ");
		passphrase2 = getpass("Enter same passphrase again: ");
		if (passphrase1 == passphrase2) {
			break;
		}

		std::cerr << "Password not match, try again" << std::endl;
	}

	if (tryagain <= 0)
		return -1;

	passphrase = passphrase1;

	return 0;
}

static bool isEnptyWallet(void) {
	bool empty = true;

	auto masterWallets = manager->GetAllMasterWallets();
	for (auto it = masterWallets.begin(); it != masterWallets.end(); ++it) {
		auto subWallets = (*it)->GetAllSubWallets();
		for (auto it = subWallets.begin(); it != subWallets.end(); ++it) {
			empty = false;
		}
	}

	return empty;
}

static void syncStart(void) {
	auto masterWallets = manager->GetAllMasterWallets();
	for (auto it = masterWallets.begin(); it != masterWallets.end(); ++it) {
		auto subWallets = (*it)->GetAllSubWallets();
		for (auto it = subWallets.begin(); it != subWallets.end(); ++it) {
			// TODO: remove callback on sync stop
			(*it)->AddCallback(new SubWalletCallback((*it)->GetChainID()));
			(*it)->SyncStart();
		}
	}
}

static void syncStop(void) {
	auto masterWallets = manager->GetAllMasterWallets();
	for (auto it = masterWallets.begin(); it != masterWallets.end(); ++it) {
		auto subWallets = (*it)->GetAllSubWallets();
		for (auto it = subWallets.begin(); it != subWallets.end(); ++it) {
			(*it)->SyncStop();
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

static ISubWallet *getSubWallet(const std::string &masterWalletID, const std::string &chainID) {
	auto masterWallet = manager->GetMasterWallet(masterWalletID);
	if (!masterWallet) {
		std::cerr << "wallet '" << masterWalletID << "' not found" << std::endl;
		return nullptr;
	}
	auto subWallet = masterWallet->GetSubWallet(chainID);
	if (!subWallet) {
		std::cerr << "chain '" << chainID << "' not found" << std::endl;
		return nullptr;
	}

	return subWallet;
}

static void create(int argc, char *argv[]) {
	if (argc != 2) {
		invalidCmdError();
		return;
	}

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

	const std::string mnemonic = manager->GenerateMnemonic(language);

	std::cout << "Please write down the following mnemonic words." << std::endl;
	std::cout << "Mnemonic: " << mnemonic << std::endl;
	std::cout << "Then press enter to continue...";
	std::cin.get();

	std::string password, passphrase;

	if (0 > getPaymentPassword(password)) {
		std::cerr << "Create failed!" << std::endl;
		return;
	}

	if (0 > getPassphrase(passphrase)) {
		std::cerr << "Create failed!" << std::endl;
		return;
	}

	IMasterWallet *masterWallet = manager->CreateMasterWallet(walletID,
															  mnemonic, passphrase, password, false);
	if (!masterWallet) {
		std::cerr << "Create master wallet failed." << std::endl;
		return;
	}

	ISubWallet *subWallet = masterWallet->CreateSubWallet(MAIN_CHAIN);
	if (!subWallet) {
		std::cerr << "Create main chain wallet failed." << std::endl;
		return;
	}
	subWallet->SyncStart();

	std::cout << "Wallet create success." << std::endl;
}

static void import(int argc, char *argv[]) {
	if (argc != 2) {
		invalidCmdError();
		return;
	}

	std::string walletId = argv[1];
	auto ids = manager->GetAllMasterWalletID();
	if (std::find(ids.begin(), ids.end(), walletId) != ids.end()) {
		std::cerr << "Wallet '" << walletId << "' already exist." << std::endl;
		return;
	}

	std::string mnemonic;
	std::cout << "Mnemonic: ";
	std::getline(std::cin, mnemonic);

	std::string password, passphrase;
	if (0 > getPaymentPassword(password)) {
		std::cerr << "Create failed!" << std::endl;
		return;
	}

	if (0 > getPassphrase(passphrase)) {
		std::cerr << "Create failed!" << std::endl;
		return;
	}

	IMasterWallet *masterWallet = manager->ImportWalletWithMnemonic(walletId,
																	mnemonic, passphrase, password, false);
	if (!masterWallet) {
		std::cerr << "Import master wallet failed." << std::endl;
		return;
	}

	ISubWallet *subWallet = masterWallet->CreateSubWallet(MAIN_CHAIN);
	if (!subWallet) {
		std::cerr << "Create main chain wallet failed." << std::endl;
		return;
	}
	subWallet->SyncStart();

	std::cout << "Wallet import success." << std::endl;
}

static void remove(int argc, char *argv[]) {
	if (argc != 2) {
		invalidCmdError();
		return;
	}

	std::string walletId = argv[1];
	auto ids = manager->GetAllMasterWalletID();
	if (std::find(ids.begin(), ids.end(), walletId) == ids.end()) {
		std::cerr << "Wallet '" << walletId << "' not exist." << std::endl;
		return;
	}

	manager->DestroyWallet(walletId);
	std::cout << "Wallet '" << walletId << "' removed." << std::endl;
}

static void list(int argc, char *argv[]) {
	auto masterWallets = manager->GetAllMasterWallets();
	printf("%20s%20s%20s\n", "WalletID", MAIN_CHAIN, ID_CHAIN);
	printf("%s\n", SplitLine);

	for (auto it = masterWallets.begin(); it != masterWallets.end(); ++it) {
		ISubWallet *subWallet;
		std::string elaBalance = "--";
		std::string idBalance = "--";

		subWallet = (*it)->GetSubWallet(MAIN_CHAIN);
		if (subWallet)
			elaBalance = std::to_string(std::stod(subWallet->GetBalance()) / SELA_PER_ELA);

		subWallet = (*it)->GetSubWallet(ID_CHAIN);
		if (subWallet)
			idBalance = std::to_string(std::stod(subWallet->GetBalance()) / SELA_PER_ELA);

		printf("%20s%20s%20s\n", (*it)->GetID().c_str(), elaBalance.c_str(), idBalance.c_str());
	}
}

// address [walletID] [chainID]
static void address(int argc, char *argv[]) {
	std::string chain;

	if (argc != 3) {
		invalidCmdError();
		return;
	}

	try {
		auto subWallet = getSubWallet(argv[1], argv[2]);
		if (!subWallet)
			return;

		std::cout << subWallet->CreateAddress() << std::endl;
	} catch (const std::exception &e) {
		exceptionError(e);
	}
}

// open [walletID] [chainID]
static void openw(int argc, char *argv[]) {
	if (argc != 3) {
		invalidCmdError();
		return;
	}

	try {
		auto masterWallet = manager->GetMasterWallet(argv[1]);
		if (!masterWallet) {
			std::cerr << argv[1] << " not found" << std::endl;
			return;
		}

		masterWallet->CreateSubWallet(argv[2]);
	} catch (const std::exception &e) {
		exceptionError(e);
	}
}

// close [walletID] [chainID]
static void closew(int argc, char *argv[]) {
	if (argc != 3) {
		invalidCmdError();
		return;
	}

	try {
		auto masterWallet = manager->GetMasterWallet(argv[1]);
		if (!masterWallet) {
			std::cerr << argv[1] << " not found" << std::endl;
			return;
		}

		auto subWallet = masterWallet->GetSubWallet(argv[2]);
		if (!subWallet) {
			std::cerr << argv[2] << " not found" << std::endl;
			return;
		}

		masterWallet->DestroyWallet(subWallet);
	} catch (const std::exception &e) {
		exceptionError(e);
	}
}

// deposit walletID sidechain amount
static void deposit(int argc, char *argv[]) {
	if (argc != 4) {
		invalidCmdError();
		return;
	}

	std::string walletID = argv[1];
	std::string sidechain = argv[2];
	double amount = std::stod(argv[3]) * SELA_PER_ELA;

	try {
		auto masterWallet = manager->GetMasterWallet(walletID);
		if (masterWallet == nullptr) {
			std::cerr << "wallet " << walletID << " do not exist" << std::endl;
			return;
		}

		if (sidechain == MAIN_CHAIN) {
			std::cerr << "ELA is not a sidechain." << std::endl;
			return;
		}

		IMainchainSubWallet *mainchainSubWallet = dynamic_cast<IMainchainSubWallet *>(masterWallet->GetSubWallet(MAIN_CHAIN));
		if (mainchainSubWallet == NULL) {
			std::cerr << "Can not get mainchain wallet." << std::endl;
			return;
		}

		ISidechainSubWallet *sidechainSubWallet = dynamic_cast<ISidechainSubWallet *>(masterWallet->GetSubWallet(ID_CHAIN));
		if (sidechainSubWallet == NULL) {
			std::cerr << "Can not get sidechain wallet for: " << sidechain << std::endl;
			return;
		}

		std::string lockedAddress = sidechainSubWallet->GetGenesisAddress();

		nlohmann::json tx = mainchainSubWallet->CreateDepositTransaction(
			"", lockedAddress, std::to_string(amount), sidechainSubWallet->CreateAddress(), "");

		std::string password = getpass("Enter payment password: ");

		nlohmann::json signedTx = mainchainSubWallet->SignTransaction(tx, password);
		mainchainSubWallet->PublishTransaction(signedTx);
	} catch (const std::exception &e) {
		exceptionError(e);
	}
}

// withdraw [walletName] [sidechain] [amount]
static void withdraw(int argc, char *argv[]) {
	if (argc != 4) {
		invalidCmdError();
		return;
	}

	std::string walletID = argv[1];
	std::string sidechain = argv[2];
	int amount = (int) (std::stod(argv[3]) * SELA_PER_ELA);

	try {
		auto masterWallet = manager->GetMasterWallet(walletID);
		if (masterWallet == nullptr) {
			std::cerr << "Wallet '" << walletID << "' not exist." << std::endl;
			return;
		}

		if (sidechain == MAIN_CHAIN) {
			std::cerr << "ELA is not a sidechain." << std::endl;
			return;
		}

		IMainchainSubWallet *mainchainSubWallet = dynamic_cast<IMainchainSubWallet *>(masterWallet->GetSubWallet(MAIN_CHAIN));
		if (mainchainSubWallet == NULL) {
			std::cerr << "Can not get mainchain wallet." << std::endl;
			return;
		}

		ISidechainSubWallet *sidechainSubWallet = dynamic_cast<ISidechainSubWallet *>(masterWallet->GetSubWallet(ID_CHAIN));
		if (sidechainSubWallet == NULL) {
			std::cerr << "Can not get sidechain wallet for: " << sidechain << std::endl;
			return;
		}

		nlohmann::json tx = sidechainSubWallet->CreateWithdrawTransaction(
			"", std::to_string(amount), mainchainSubWallet->CreateAddress(), "");

		std::string password = getpass("Enter payment password: ");

		nlohmann::json signedTx = sidechainSubWallet->SignTransaction(tx, password);
		sidechainSubWallet->PublishTransaction(signedTx);
	} catch (const std::exception &e) {
		exceptionError(e);
	}
}

static void createDID(int argc, char *argv[]) {
	std::string walletId = argv[1];

	std::string id;
	std::string didName;
	std::string operation;
	std::string publicKey;
	uint64_t expires;
	nlohmann::json jsonData;
	nlohmann::json pubKey = R"({"id":"#primary"})"_json;
	nlohmann::json publicKeys;
	std::string payPasswd;

	std::cout << "Input id:";
	std::cin >> id;

	std::cout << "Input didName:";
	std::cin >> didName;

	std::cout << "Input operation:";
	std::cin >> operation;

	std::cout << "Input publicKey:";
	std::cin >> publicKey;
	pubKey["publicKey"] = publicKey;
	publicKeys.push_back(pubKey);

	std::cout << "Input expires date:";
	std::cin >> expires;

	std::cout << "Input pay password:";
	std::cin >> payPasswd;

	jsonData["id"] = id;
	jsonData["didName"] = didName;
	jsonData["operation"] = operation;
	jsonData["publicKey"] = publicKeys;
	jsonData["expires"] = expires;

	IIDChainSubWallet *iidChainSubWallet = nullptr;
	auto masterWallet = manager->GetMasterWallet(walletId);
	std::vector<ISubWallet *> subWallets = masterWallet->GetAllSubWallets();
	for (auto it = subWallets.begin(); it != subWallets.end(); ++it) {
		if ((*it)->GetChainID() == ID_CHAIN) {
			iidChainSubWallet = dynamic_cast<IIDChainSubWallet *>(*it);
			break;
		}
	}

	if (iidChainSubWallet == nullptr) {
		std::cerr << "Can not get sidechain wallet for: " << iidChainSubWallet << std::endl;
		return;
	}

	try {
		nlohmann::json payload = iidChainSubWallet->GenerateDIDInfoPayload(jsonData, payPasswd);
		nlohmann::json tx = iidChainSubWallet->CreateIDTransaction(payload, "");

		nlohmann::json signedTx = iidChainSubWallet->SignTransaction(tx, payPasswd);
		nlohmann::json res = iidChainSubWallet->PublishTransaction(signedTx);
		std::cout << res.dump() << std::endl;

	} catch (...) {
		std::cerr << "Create id transaction failed." << std::endl;
	}

}

// idtx [walletName]
static void idtx(int argc, char *argv[]) {
	if (argc != 2) {
		invalidCmdError();
		return;
	}

	auto masterWallet = manager->GetMasterWallet(argv[1]);
	if (!masterWallet) {
		std::cerr << argv[1] << "not found" << std::endl;
		return;
	}

	IIDChainSubWallet *subWallet = dynamic_cast<IIDChainSubWallet *>(masterWallet->GetSubWallet(ID_CHAIN));
	if (!subWallet) {
		std::cerr << "open '" << ID_CHAIN << "' first" << std::endl;
		return;
	}

	std::cout << "Enter id document: ";
	std::string iddoc;
	std::cin >> iddoc;

	try {
		nlohmann::json tx = subWallet->CreateIDTransaction(nlohmann::json::parse(iddoc), "");
		std::string password = getpass("Enter payment password: ");
		nlohmann::json signedTx = subWallet->SignTransaction(tx, password);
		subWallet->PublishTransaction(signedTx);
	} catch (const std::exception &e) {
		exceptionError(e);
	}
}

static void createProposal(int argc, char *argv[]) {
	std::string walletId = argv[1];
	std::string payPasswd = "qqqqqqqq1";

	IIDChainSubWallet *iidChainSubWallet = nullptr;
	IMainchainSubWallet *mainchainSubWallet = nullptr;
	auto masterWallet = manager->GetMasterWallet(walletId);
	std::vector<ISubWallet *> subWallets = masterWallet->GetAllSubWallets();
	for (auto it = subWallets.begin(); it != subWallets.end(); ++it) {
		if ((*it)->GetChainID() == ID_CHAIN) {
			iidChainSubWallet = dynamic_cast<IIDChainSubWallet *>(*it);
		} else if ((*it)->GetChainID() == MAIN_CHAIN) {
			mainchainSubWallet = dynamic_cast<IMainchainSubWallet *>(*it);
		}
	}

	if (iidChainSubWallet == nullptr) {
		std::cerr << "Can not get sidechain wallet for: " << iidChainSubWallet << std::endl;
		return;
	}

	if (mainchainSubWallet == nullptr) {
		std::cerr << "Can not get mainchain wallet for: " << mainchainSubWallet << std::endl;
		return;
	}

	int type = 0;
	std::string publicKey;
	std::string draftHash;
	std::string budgetList;
	std::string receiptAddress;
	std::string crSponsorDID;

	std::cout << "Input sponsor public key:";
	std::cin >> publicKey;
	std::string sponsorDID = iidChainSubWallet->GetPublicKeyDID(publicKey);

	std::cout << "Input draft hash:";
	std::cin >> draftHash;

	std::cout << "Input budget list, split by ,:";
	std::cin >> budgetList;
	std::vector<std::string> budgets;
	boost::algorithm::split(budgets, budgetList, boost::is_any_of(","), boost::token_compress_on);

	std::cout << "Input receipt address:";
	std::cin >> receiptAddress;

	nlohmann::json proposal = mainchainSubWallet->SponsorProposalDigest(type, publicKey, draftHash, budgets,
																		receiptAddress);
	std::string digest = proposal["Digest"].get<std::string>();

	std::string sponsorSignature = iidChainSubWallet->SignDigest(sponsorDID, digest, payPasswd);
	proposal["Signature"] = sponsorSignature;

	std::cout << "Input cr did:";
	std::cin >> crSponsorDID;

	proposal = mainchainSubWallet->CRSponsorProposalDigest(proposal, crSponsorDID);
	digest = proposal["Digest"].get<std::string>();

	std::string crSignature = iidChainSubWallet->SignDigest(crSponsorDID, digest, payPasswd);
	proposal["CRSignature"] = crSignature;

	nlohmann::json tx = mainchainSubWallet->CreateCRCProposalTransaction(proposal, "");

	nlohmann::json signedTx = mainchainSubWallet->SignTransaction(tx, payPasswd);
	nlohmann::json res = mainchainSubWallet->PublishTransaction(signedTx);
	std::cout << res.dump() << std::endl;
}

static void exportm(int argc, char *argv[]) {
	if (argc != 2) {
		std::cerr << "Invalid command syntax." << std::endl;
		return;
	}

	std::string password = getpass("Enter payment password: ");

	try {
		IMasterWallet *masterWallet = manager->GetMasterWallet(argv[1]);
		if (!masterWallet) {
			std::cerr << argv[1] << " not found" << std::endl;
			return;
		}

		std::string mnemonic = masterWallet->ExportMnemonic(password);
		std::cout << mnemonic << std::endl;
	} catch (const std::exception &e) {
		std::cerr << "Export mnemonic error: " << e.what() << std::endl;
	}
}

// exportk [walletName]
static void exportk(int argc, char *argv[]) {
	if (argc != 2) {
		invalidCmdError();
		return;
	}

	std::string backupPassword;
	if (0 > getBackupPassword(backupPassword)) {
		std::cerr << "Export keystore failed!" << std::endl;
		return;
	}

	std::string password = getpass("Enter payment password: ");

	try {
		auto masterWallet = manager->GetMasterWallet(argv[1]);
		if (!masterWallet) {
			std::cerr << argv[1] << " not found" << std::endl;
			return;
		}

		nlohmann::json keystore = masterWallet->ExportKeystore(backupPassword, password);
		std::cout << keystore.dump(4) << std::endl;
	} catch (const std::exception &e) {
		exceptionError(e);
	}
}

// passwd [walletName]
static void passwd(int argc, char *argv[]) {
	if (argc != 2) {
		invalidCmdError();
		return;
	}

	try {
		auto masterWallet = manager->GetMasterWallet(argv[1]);
		if (!masterWallet) {
			std::cerr << argv[1] << " not found" << std::endl;
			return;
		}

		std::string oldPassword = getpass("Old Password: ");
		if (!masterWallet->VerifyPayPassword(oldPassword)) {
			std::cerr << "Wrong password" << std::endl;
			return;
		}

		std::string newPassword = getpass("New Password: ");
		std::string newPasswordVerify = getpass("Retype New Password: ");
		if (newPassword != newPasswordVerify) {
			std::cerr << "Password not match" << std::endl;
			return;
		}

		masterWallet->ChangePassword(oldPassword, newPassword);
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
	{"help",           help,           "help      [command]                         Display available command list, or usage description for specific command."},
	{"create",         create,         "create    [walletName]                      Create a new wallet with given name."},
	{"import",         import,         "import    [walletName]                      Import wallet with given name and mnemonic."},
	{"address",        address,        "address   [walletName] [chainID]            Get the revceive address for specified chainId."},
	{"open",           openw,          "open      [walletName] [chainID]            Open wallet of `chainID`."},
	{"close",          closew,         "close     [walletName] [chainID]            Close wallet of `chainID`."},
	{"deposit",        deposit,        "deposit   [walletName] [sidechain] [amount] Deposit to sidechain from mainchain."},
	{"withdraw",       withdraw,       "withdraw  [walletName] [sidechain] [amount] Withdraw from sidechain to mainchain."},
	{"exportm",        exportm,        "exportm   [walletName]                      Export mnemonic of specified wallet."},
	{"exportk",        exportk,        "exportk   [walletName]                      Export keystore of specified wallet."},
	{"passwd",         passwd,         "passwd    [walletName]                      Change password of specified wallet."},
	{"remove",         remove,         "remove    [walletName]                      Remove specified wallet."},
	{"createDID",      createDID,      "createDID [walletName]                      Create DID with IDChain."},
	{"idtx",           idtx,           "idtx      [walletName]                      Create id transaction."},
	{"proposal",       createProposal, "proposal  [walletName]                      Create CRC proposal transaction."},
	{"verbose",        verbose,        "verbose   [on | off]                        Set verbose mode."},
	{"list",           list,           "list                                        List all wallets."},
	{"exit", NULL,             "exit                                        Quit wallet."},
	{"quit", NULL,             "quit                                        Quit wallet."},
	{NULL,   NULL, NULL}
};

static void help(int argc, char *argv[]) {
	struct command *p;

	std::cout << std::endl << "Usage: [Command] [Arguments]..." << std::endl << std::endl;
	std::cout << "Command list:" << std::endl;

	if (argc == 1) {
		for (p = commands; p->cmd; p++)
			std::cout << "  " << p->help << std::endl;

		std::cout << std::endl;
	} else {
		for (p = commands; p->cmd; p++) {
			if (strcmp(argv[1], p->cmd) == 0) {
				std::cout << "  " << p->help << std::endl << std::endl;
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
	syncStop();
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
	syncStart();

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

	syncStop();
	delete manager;
}
