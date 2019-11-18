#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <getopt.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>

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

static const char *MAIN_CHAIN = "ELA";
static const char *ID_CHAIN = "IDChain";

static MasterWalletManager *manager;

static bool verboseMode = false;

class SubWalletCallback : public ISubWalletCallback {
public:
    ~SubWalletCallback() {}
    SubWalletCallback(const std::string &walletID) : _walletID(walletID) {}

    virtual void OnTransactionStatusChanged(
        const std::string &txid,const std::string &status,
        const nlohmann::json &desc,uint32_t confirms) {
        if (verboseMode)
            std::cout << "*** Wallet " << _walletID << " transaction: "
                    << txid << " changed to " << status << ", confirms: "
                    << confirms << std::endl;
    }

    virtual void OnBlockSyncStarted() {
        if (verboseMode)
            std::cout << "*** Wallet " << _walletID << " sycn started." << std::endl;
    }

    virtual void OnBlockSyncProgress(uint32_t currentBlockHeight, uint32_t estimatedHeight, time_t lastBlockTime) {
        if (verboseMode)
            std::cout << "*** Wallet " << _walletID << " syncing:"
                    << currentBlockHeight << "/" << estimatedHeight  << std::endl;
    }

    virtual void OnBlockSyncStopped() {
        if (verboseMode)
            std::cout << "*** Wallet " << _walletID << " sync stopped."  << std::endl;
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

static bool isEnptyWallet(void)
{
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

static void syncStart(void)
{
    auto masterWallets = manager->GetAllMasterWallets();
    for (auto it = masterWallets.begin(); it != masterWallets.end(); ++it) {
        auto subWallets = (*it)->GetAllSubWallets();
        for (auto it = subWallets.begin(); it != subWallets.end(); ++it) {
            // TODO: remove callback on sync stop
            auto subWallet = *it;
            auto chainID = subWallet->GetChainID();
            try {
                (*it)->AddCallback(new SubWalletCallback(chainID));
                (*it)->SyncStart();
            } catch (std::exception e) {
                std::cerr << "Failed start sync blocks for " << chainID
                        << ": " << e.what() << std::endl;
                continue;
            }
        }
    }
}

static void syncStop(void)
{
    auto masterWallets = manager->GetAllMasterWallets();
    for (auto it = masterWallets.begin(); it != masterWallets.end(); ++it) {
        auto subWallets = (*it)->GetAllSubWallets();
        for (auto it = subWallets.begin(); it != subWallets.end(); ++it) {
            auto subWallet = *it;
            try {
                subWallet->SyncStop();
            } catch (std::exception e) {
                // Ignore any exception
                continue;
            }
        }
    }

    manager->FlushData();
}

static void init(int argc, char *argv[])
{
    if (argc != 2) {
        std::cerr << "Invalid command syntax." << std::endl;
        return;
    }

    std::string walletId = argv[1];
    auto ids = manager->GetAllMasterWalletID();
    if (std::find(ids.begin(), ids.end(), walletId) != ids.end()) {
        std::cerr << "Wallet '" << walletId << "' already exist." << std::endl;
        return;
    }

    auto mnemonic = manager->GenerateMnemonic("english");

    std::cout << "Please write down the following mnemonic words." << std::endl;
    std::cout << "Mnemonic: " << mnemonic << std::endl;
    std::cout << "Then press enter to continue...";
    std::cin.get();

    std::string password;
    while (true) {
        std::cout << "Payment password: ";
        std::getline(std::cin, password);
        if (password.length() >= 8)
            break;

        std::cout << "Password should be at least 8 characters long." << std::endl;
    }

    try {
        auto masterWallet = manager->CreateMasterWallet(walletId, mnemonic, "",
                password, false);
        std::cout << "Master wallet '" << walletId << "' created." << std::endl;

        masterWallet->CreateSubWallet(MAIN_CHAIN);
        std::cout << "SubWallet for '" << MAIN_CHAIN << "' created." << std::endl;

        masterWallet->CreateSubWallet(ID_CHAIN);
        std::cout << "SubWallet for '" << ID_CHAIN << "' created." << std::endl;

        std::cout << "Wallet create success." << std::endl;
    } catch (std::exception e) {
        std::cerr << "Create wallet failed: " << e.what() << std::endl;
        return;
    }

    syncStart();
}

static void importMnemonic(int argc, char *argv[])
{
    if (argc != 2) {
        std::cerr << "Invalid command syntax." << std::endl;
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

    std::string password;
    while (true) {
        std::cout << "Payment password: ";
        std::getline(std::cin, password);
        if (password.length() >= 8)
            break;

        std::cout << "Password should be at least 8 characters long." << std::endl;
    }

    try {
        auto *masterWallet = manager->ImportWalletWithMnemonic(walletId,
                mnemonic, "", password, false);
        std::cout << "Master wallet '" << walletId << "' imported." << std::endl;

        masterWallet->CreateSubWallet(MAIN_CHAIN);
        std::cout << "SubWallet for '" << MAIN_CHAIN << "' created." << std::endl;

        masterWallet->CreateSubWallet(ID_CHAIN);
        std::cout << "SubWallet for '" << ID_CHAIN << "' created." << std::endl;

        std::cout << "Wallet import success." << std::endl;
    } catch (std::exception e) {
        std::cerr << "import wallet failed: " << e.what() << std::endl;
        return;
    }

    syncStart();
}

// import walletId keystoreFile storepass paypass
static void importKeystore(int argc, char *argv[])
{
    if (argc != 5) {
        std::cerr << "Invalid command syntax." << std::endl;
        return;
    }

    std::string walletId = argv[1];
    std::string keystore = argv[2];
    std::string storepass = argv[3];
    std::string password = argv[4];

    auto ids = manager->GetAllMasterWalletID();
    if (std::find(ids.begin(), ids.end(), walletId) != ids.end()) {
        std::cerr << "Wallet '" << walletId << "' already exist." << std::endl;
        return;
    }

    try {
        char line[8192];
        std::ifstream is;

        is.open(keystore);
        is.getline(line, sizeof(line));
        nlohmann::json ks = nlohmann::json::parse(line);
        auto *masterWallet = manager->ImportWalletWithKeystore(walletId,
                ks, storepass, password);
        std::cout << "Master wallet '" << walletId << "' imported." << std::endl;

        masterWallet->CreateSubWallet(MAIN_CHAIN);
        std::cout << "SubWallet for '" << MAIN_CHAIN << "' created." << std::endl;

        masterWallet->CreateSubWallet(ID_CHAIN);
        std::cout << "SubWallet for '" << ID_CHAIN << "' created." << std::endl;

        std::cout << "Wallet import success." << std::endl;
    } catch (std::exception e) {
        std::cerr << "import wallet failed: " << e.what() << std::endl;
        return;
    }

    syncStart();
}

static void remove(int argc, char *argv[])
{
    if (argc != 2) {
        std::cerr << "Invalid command syntax." << std::endl;
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

static void list(int argc, char *argv[])
{
    auto masterWallets = manager->GetAllMasterWallets();
    for (auto it = masterWallets.begin(); it != masterWallets.end(); ++it) {
        auto masterWallet = *it;
        std::cout << "Wallet: " << masterWallet->GetID() << std::endl;

        auto subWallets = masterWallet->GetAllSubWallets();
        for (auto it = subWallets.begin(); it != subWallets.end(); ++it) {
            auto subWallet = *it;
            double balance = std::stod(subWallet->GetBalance()) / 100000000;
            std::cout << "  " << subWallet->GetChainID() << ": "
                    << balance << std::endl;
        }
    }
}

static void address(int argc, char *argv[])
{
    std::string chain;
    bool matched = false;

    if (argc > 2) {
        std::cerr << "Invalid command syntax." << std::endl;
        return;
    }

    if (argc == 2)
        chain = argv[1];

    auto masterWallets = manager->GetAllMasterWallets();
    for (auto it = masterWallets.begin(); it != masterWallets.end(); ++it) {
        auto masterWallet = *it;
        auto subWallets = masterWallet->GetAllSubWallets();
        for (auto it = subWallets.begin(); it != subWallets.end(); ++it) {
            auto subWallet = *it;
            if (subWallet->GetChainID() == chain) {
                matched = true;

                std::cout << masterWallet->GetID() << ":"
                        << subWallet->GetChainID() << ": "
                        << subWallet->CreateAddress() << std::endl;
            }
        }
    }

    if (!matched)
        std::cerr << "Can not find the wallet: " << chain << std::endl;
}

// deposit walletId sidechain amount password
static void deposit(int argc, char *argv[])
{
    if (argc != 5) {
        std::cerr << "Invalid command syntax." << std::endl;
        return;
    }

    std::string walletId = argv[1];
    std::string sidechain = argv[2];
    double amount = std::stod(argv[3]) * 100000000;
    std::string password = argv[4];

    auto ids = manager->GetAllMasterWalletID();
    if (std::find(ids.begin(), ids.end(), walletId) == ids.end()) {
        std::cerr << "Wallet '" << walletId << "' not exist." << std::endl;
        return;
    }

    if (sidechain == MAIN_CHAIN) {
        std::cerr << "ELA is not a sidechain." << std::endl;
        return;
    }

    IMainchainSubWallet *mainchainSubWallet = NULL;
    ISidechainSubWallet *sidechainSubWallet = NULL;
    auto masterWallet = manager->GetMasterWallet(walletId);
    std::vector<ISubWallet *> subWallets = masterWallet->GetAllSubWallets();
    for (auto it = subWallets.begin(); it != subWallets.end(); ++it) {
        if ((*it)->GetChainID() == MAIN_CHAIN)
            mainchainSubWallet = dynamic_cast<IMainchainSubWallet *>(*it);
        else if ((*it)->GetChainID() == sidechain)
            sidechainSubWallet = dynamic_cast<ISidechainSubWallet *>(*it);
    }

    if (mainchainSubWallet == NULL) {
        std::cerr << "Can not get mainchain wallet." << std::endl;
        return;
    }

    if (sidechainSubWallet == NULL) {
        std::cerr << "Can not get sidechain wallet for: " << sidechain << std::endl;
        return;
    }

    try {
        auto lockedAddress = sidechainSubWallet->GetGenesisAddress();

        auto tx = mainchainSubWallet->CreateDepositTransaction(
                "", lockedAddress, std::to_string(amount),
                sidechainSubWallet->CreateAddress(), "");

        tx = mainchainSubWallet->SignTransaction(tx, password);
        tx = mainchainSubWallet->PublishTransaction(tx);

        std::cout << "Deposit transaction " << tx["TxHash"] << " created success." << std::endl;
        std::cout << "Deposit transactions need 8 confirms!" << std::endl;
    } catch (std::exception e) {
        std::cerr << "Create deposit transaction failed: " << e.what() << std::endl;
    }
}

// withdraw walletId sidechain amount password
static void withdraw(int argc, char *argv[])
{
    if (argc != 5) {
        std::cerr << "Invalid command syntax." << std::endl;
        return;
    }

    std::string walletId = argv[1];
    std::string sidechain = argv[2];
    int amount = (int)(std::stod(argv[3]) * 100000000);
    std::string password = argv[4];

    auto ids = manager->GetAllMasterWalletID();
    if (std::find(ids.begin(), ids.end(), walletId) == ids.end()) {
        std::cerr << "Wallet '" << walletId << "' not exist." << std::endl;
        return;
    }

    if (sidechain == MAIN_CHAIN) {
        std::cerr << "ELA is not a sidechain." << std::endl;
        return;
    }

    IMainchainSubWallet *mainchainSubWallet = NULL;
    ISidechainSubWallet *sidechainSubWallet = NULL;
    auto masterWallet = manager->GetMasterWallet(walletId);
    std::vector<ISubWallet *> subWallets = masterWallet->GetAllSubWallets();
    for (auto it = subWallets.begin(); it != subWallets.end(); ++it) {
        if ((*it)->GetChainID() == MAIN_CHAIN)
            mainchainSubWallet = dynamic_cast<IMainchainSubWallet *>(*it);
        else if ((*it)->GetChainID() == sidechain)
            sidechainSubWallet = dynamic_cast<ISidechainSubWallet *>(*it);
    }

    if (mainchainSubWallet == NULL) {
        std::cerr << "Can not get mainchain wallet." << std::endl;
        return;
    }

    if (sidechainSubWallet == NULL) {
        std::cerr << "Can not get sidechain wallet for: " << sidechain << std::endl;
        return;
    }

    try {
        auto tx = sidechainSubWallet->CreateWithdrawTransaction(
            "", std::to_string(amount), mainchainSubWallet->CreateAddress(), "");

        tx = sidechainSubWallet->SignTransaction(tx, password);
        tx = sidechainSubWallet->PublishTransaction(tx);

        std::cout << "Withdraw transaction " << tx["TxHash"] << " created success." << std::endl;
        std::cout << "Withdraw transactions need 8 confirms!" << std::endl;
    } catch (std::exception e) {
        std::cerr << "Create withdraw transaction failed: " << e.what() << std::endl;
    }
}

// transfer walletId address amount password
static void transfer(int argc, char *argv[])
{
    if (argc != 5) {
        std::cerr << "Invalid command syntax." << std::endl;
        return;
    }

    std::string walletId = argv[1];
    std::string address = argv[2];
    int amount = (int)(std::stod(argv[3]) * 100000000);
    std::string password = argv[4];

    auto ids = manager->GetAllMasterWalletID();
    if (std::find(ids.begin(), ids.end(), walletId) == ids.end()) {
        std::cerr << "Wallet '" << walletId << "' not exist." << std::endl;
        return;
    }

    IMainchainSubWallet *mainchainSubWallet = NULL;
    auto masterWallet = manager->GetMasterWallet(walletId);
    std::vector<ISubWallet *> subWallets = masterWallet->GetAllSubWallets();
    for (auto it = subWallets.begin(); it != subWallets.end(); ++it) {
        if ((*it)->GetChainID() == MAIN_CHAIN)
            mainchainSubWallet = dynamic_cast<IMainchainSubWallet *>(*it);
    }

    if (mainchainSubWallet == NULL) {
        std::cerr << "Can not get mainchain wallet." << std::endl;
        return;
    }

    try {
        auto tx = mainchainSubWallet->CreateTransaction(
            "", address, std::to_string(amount), "");

        tx = mainchainSubWallet->SignTransaction(tx, password);
        tx = mainchainSubWallet->PublishTransaction(tx);

        std::cout << "Transaction " << tx["TxHash"] << " created success." << std::endl;
    } catch (std::exception e) {
        std::cerr << "Create withdraw transaction failed: " << e.what() << std::endl;
    }
}

// publishdid walletId password
static void publishDid(int argc, char *argv[])
{
    if (argc != 3) {
        std::cerr << "Invalid command syntax." << std::endl;
        return;
    }

    std::string walletId = argv[1];
    std::string password = argv[2];

    auto ids = manager->GetAllMasterWalletID();
    if (std::find(ids.begin(), ids.end(), walletId) == ids.end()) {
        std::cerr << "Wallet '" << walletId << "' not exist." << std::endl;
        return;
    }

    IIDChainSubWallet *idWallet = NULL;
    auto masterWallet = manager->GetMasterWallet(walletId);
    std::vector<ISubWallet *> subWallets = masterWallet->GetAllSubWallets();
    for (auto it = subWallets.begin(); it != subWallets.end(); ++it) {
        if ((*it)->GetChainID() == ID_CHAIN)
            idWallet = dynamic_cast<IIDChainSubWallet *>(*it);
    }

    if (idWallet == NULL) {
        std::cerr << "Can not get idchain wallet." << std::endl;
        return;
    }

    std::string didrequest;
    std::cout << "DID request: ";
    std::getline(std::cin, didrequest);

    if (didrequest.length() == 0)
        return;

    try {
        auto payload = nlohmann::json::parse(didrequest);

        auto tx = idWallet->CreateIDTransaction(payload, "");
        tx = idWallet->SignTransaction(tx, password);
        tx = idWallet->PublishTransaction(tx);

        std::cout << "ID transaction " << tx["TxHash"] << " created success." << std::endl;
    } catch (std::exception e) {
        std::cerr << "Create ID transaction failed: " << e.what() << std::endl;
    }
}

static void exportMnemonic(int argc, char *argv[])
{
    std::string walletId;
    std::string password;

    if (argc != 3) {
        std::cerr << "Invalid command syntax." << std::endl;
        return;
    }

    walletId = argv[1];
    password = argv[2];

    auto masterWallet = manager->GetMasterWallet(walletId);
    if (!masterWallet) {
        std::cerr << "Can not find wallet: " << walletId << std::endl;
        return;
    }

    auto mnemonic = masterWallet->ExportMnemonic(password);
    std::cout << "Mnemonic: " << mnemonic << std::endl;
}

// export walletId keystoreFile storepass paypass
static void exportKeystore(int argc, char *argv[])
{
    if (argc != 5) {
        std::cerr << "Invalid command syntax." << std::endl;
        return;
    }

    std::string walletId = argv[1];
    std::string keystore = argv[2];
    std::string storepass = argv[3];
    std::string password = argv[4];

    auto masterWallet = manager->GetMasterWallet(walletId);
    if (!masterWallet) {
        std::cerr << "Can not find wallet: " << walletId << std::endl;
        return;
    }

    auto json = masterWallet->ExportKeystore(storepass, password);

    std::ofstream os;
    os.open(keystore);
    os << json;
    os.close();
    std::cout << "keystore exported." << std::endl;
}

static void verbose(int argc, char *argv[])
{
    bool v = !verboseMode;

    if (argc > 2) {
        std::cerr << "Invalid command syntax." << std::endl;
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
    { "help",           help,                   "help [command]\n  Display available command list, or usage description for specific command." },
    { "init",           init,                   "init walletName\n  Create a new wallet with given name." },
    { "import",         importMnemonic,         "import walletName\n  Import wallet with given name and mnemonic." },
    { "importkeystore", importKeystore,         "importkeystore walletName keystoreFile storepass paypass\n  Import wallet from keystore." },
    { "list",           list,                   "list\n  List all wallets." },
    { "address",        address,                "address chainId\n  Get the revceive address for specified chainId." },
    { "deposit",        deposit,                "deposit walletName sidechain amount password\n  Deposit to sidechain from mainchain." },
    { "withdraw",       withdraw,               "withdraw walletName sidechain amount password\n  Withdraw from sidechain to mainchain." },
    { "transfer",       transfer,               "transfer walletName address amount password\n  Transter amount of ELA to address."},
    { "publishdid",     publishDid,             "publishdid walletName password\n Create a ID transaction to publish DID"},
    { "export",         exportMnemonic,         "export walletName password\n  Export mnemonic from specified wallet." },
    { "exportkeystore", exportKeystore,         "exportkeystore walletName keystoreFile storepass paypass\n  Export wallet to keystore." },
    { "remove",         remove,                 "remove walletName\n  Remove specified wallet."},
    { "verbose",        verbose,                "verbose [on | off]\n Set verbose mode." },
    { "exit",           NULL,                   "exit\n  Quit wallet." },
    { "quit",           NULL,                   "quit\n  Quit wallet." },
    { NULL,             NULL,                   NULL }
};

static void help(int argc, char *argv[])
{
    struct command *p;

    if (argc == 1) {
        std::cout << "Use *help [Command]* to see usage description for a specific command." << std::endl;
        std::cout << "Available commands list:" << std::endl << std::endl;

        for (p = commands; p->cmd; p++) {
            std::cout << p->help << std::endl << std::endl;
        }
   } else {
        for (p = commands; p->cmd; p++) {
            if (strcmp(argv[1], p->cmd) == 0) {
                std::cout << "Usage: " << p->help << std::endl;
                return;
            }
        }

        std::cerr << "Unknown command: " << argv[1] << std::endl;
    }
}

int run_cmd(int argc, char *argv[])
{
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

static int parseCmdLine(char *cmdLine, char *cmdArgs[])
{
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

static int mkdirInternal(const char *path, mode_t mode)
{
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

static int mkdirs(const char *path, mode_t mode)
{
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

static const char *getWalletDir(const char *dir)
{
    static char walletDir[PATH_MAX];
    struct stat st;

    if (!dir) {
        sprintf(walletDir, "%s/.didwallet", getenv("HOME"));
        dir = (const char *)walletDir;
    }

    return dir;
}

#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>

static int sys_coredump_set(bool enable)
{
    const struct rlimit rlim = {
        enable ? RLIM_INFINITY : 0,
        enable ? RLIM_INFINITY : 0
    };

    return setrlimit(RLIMIT_CORE, &rlim);
}
#endif

static void signalHandler(int signum)
{
    syncStop();
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

int main(int argc, char *argv[])
{
    const char *network = "MainNet";
    const char *walletRoot = NULL;
    char cmdLine[4096];
    char *cmdArgs[256];
    int numArgs;
    int waitForAttach = 0;
    nlohmann::json netConfig;

    int opt;
    int idx;
    struct option options[] = {
        { "data",           required_argument,  NULL, 'd' },
        { "network",        required_argument,  NULL, 'n' },
        { "verbose",        no_argument,        NULL, 'v' },
        { "debug",          no_argument,        NULL, 1 },
        { "help",           no_argument,        NULL, 'h' },
        { NULL,             0,                  NULL, 0 }
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
    if (mkdirs(walletRoot, S_IRWXU) < 0) {
        fprintf(stderr, "Create wallet data directory '%s' failed.\n", walletRoot);
        return -1;
    } else {
        fprintf(stdout, "Wallet data directory: %s\n", walletRoot);
    }

    if (strcmp(network, "MainNet") == 0 || strcmp(network, "TestNet") == 0 ||
            strcmp(network, "RegTest") == 0) {
        netConfig = nlohmann::json();
    } else {
        try {
            std::ifstream in(network);
            netConfig = nlohmann::json::parse(in);
        } catch (...) {
            fprintf(stderr, "Load network config '%s' failed.\n", network);
        }

        network = "PrvNet";
    }

    manager = new MasterWalletManager(walletRoot, network, netConfig);
    syncStart();

    while (1) {
        fprintf(stdout, "$$$ ");
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
