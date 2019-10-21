#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <getopt.h>
#include <sys/stat.h>

#include <iostream>

#include <MasterWalletManager.h>
#include <IMasterWallet.h>
#include <ISubWallet.h>
#include <IMainchainSubWallet.h>
#include <IIDChainSubWallet.h>

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
            (*it)->AddCallback(new SubWalletCallback((*it)->GetChainID()));
            (*it)->SyncStart();
        }
    }
}

static void syncStop(void)
{
    auto masterWallets = manager->GetAllMasterWallets();
    for (auto it = masterWallets.begin(); it != masterWallets.end(); ++it) {
        auto subWallets = (*it)->GetAllSubWallets();
        for (auto it = subWallets.begin(); it != subWallets.end(); ++it) {
            (*it)->SyncStop();
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

    const std::string mnemonic = manager->GenerateMnemonic("english");

    std::cout << "Please write down the following mnemonic words." << std::endl;
    std::cout << "Mnemonic: " << mnemonic << std::endl;
    std::cout << "Then press enter to continue...";
    std::cin.get();

    std::string password;
    std::cout << "Payment password: ";
    std::getline(std::cin, password);

    IMasterWallet *masterWallet = manager->CreateMasterWallet(walletId,
            mnemonic, "", password, false);
    if (!masterWallet) {
        std::cerr << "Create master wallet failed." << std::endl;
        return;
    }

    ISubWallet *subWallet = masterWallet->CreateSubWallet(MAIN_CHAIN);
    if (!subWallet) {
        std::cerr << "Create main chain wallet failed." << std::endl;
        return;
    }

    subWallet = masterWallet->CreateSubWallet(ID_CHAIN);
    if (!subWallet) {
        std::cerr << "Create ID sidechain wallet failed." << std::endl;
        return;
    }

    std::cout << "Wallet create success." << std::endl;

    syncStart();
}

static void import(int argc, char *argv[])
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
    std::cout << "Payment password: ";
    std::getline(std::cin, password);

    IMasterWallet *masterWallet = manager->ImportWalletWithMnemonic(walletId,
            mnemonic, "", password, false);
    if (!masterWallet) {
        std::cerr << "Import master wallet failed." << std::endl;
        return;
    }

    ISubWallet *subWallet = masterWallet->CreateSubWallet(MAIN_CHAIN);
    if (!subWallet) {
        std::cerr << "Create main chain wallet failed." << std::endl;
        return;
    }

    subWallet = masterWallet->CreateSubWallet(ID_CHAIN);
    if (!subWallet) {
        std::cerr << "Create ID sidechain wallet failed." << std::endl;
        return;
    }

    std::cout << "Wallet import success." << std::endl;

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
        std::string lockedAddress = sidechainSubWallet->GetGenesisAddress();

        nlohmann::json tx = mainchainSubWallet->CreateDepositTransaction(
            "", lockedAddress, std::to_string(amount), sidechainSubWallet->CreateAddress(), "");

        nlohmann::json signedTx = mainchainSubWallet->SignTransaction(tx, password);
        mainchainSubWallet->PublishTransaction(signedTx);
    } catch (...) {
        std::cerr << "Create deposit transaction failed." << std::endl;
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
        nlohmann::json tx = sidechainSubWallet->CreateWithdrawTransaction(
            "", std::to_string(amount), mainchainSubWallet->CreateAddress(), "");

        nlohmann::json signedTx = sidechainSubWallet->SignTransaction(tx, password);
        sidechainSubWallet->PublishTransaction(signedTx);
    } catch (...) {
        std::cerr << "Create withdraw transaction failed." << std::endl;
    }
}

static void exportm(int argc, char *argv[])
{
    std::string walletId;
    std::string password;

    if (argc != 3) {
        std::cerr << "Invalid command syntax." << std::endl;
        return;
    }

    walletId = argv[1];
    password = argv[2];

    IMasterWallet *masterWallet = manager->GetMasterWallet(walletId);
    if (!masterWallet) {
        std::cerr << "Can not find wallet: " << walletId << std::endl;
        return;
    }

    std::string mnemonic = manager->ExportWalletWithMnemonic(masterWallet, password);
    std::cout << "Mnemonic: " << mnemonic << std::endl;
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
    { "help",       help,                   "help [command]\n  Display available command list, or usage description for specific command." },
    { "init",       init,                   "init walletName\n  Create a new wallet with given name." },
    { "import",     import,                 "import walletName\n  Import wallet with given name and mnemonic." },
    { "list",       list,                   "list\n  List all wallets." },
    { "address",    address,                "address chainId\n  Get the revceive address for specified chainId." },
    { "deposit",    deposit,                "deposit walletName sidechain amount password\n  Deposit to sidechain from mainchain." },
    { "withdraw",   withdraw,               "withdraw walletName sidechain amount password\n  Withdraw from sidechain to mainchain." },
    { "export",     exportm,                "export walletName password\n  Export mnemonic from specified wallet." },
    { "remove",     remove,                 "remove walletName\n  Remove specified wallet."},
    { "verbose",    verbose,                "verbose [on | off]\n Set verbose mode." },
    { "exit",       NULL,                   "exit\n  Quit wallet." },
    { "quit",       NULL,                   "quit\n  Quit wallet." },
    { NULL,         NULL,                   NULL }
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

int main(int argc, char *argv[])
{
    const char *walletRoot = NULL;
    char cmdLine[4096];
    char *cmdArgs[256];
    int numArgs;
    int waitForAttach = 0;

    int opt;
    int idx;
    struct option options[] = {
        { "data",           required_argument,  NULL, 'd' },
        { "verbose",        no_argument,        NULL, 'v' },
        { "debug",          no_argument,        NULL, 1 },
        { "help",           no_argument,        NULL, 'h' },
        { NULL,             0,                  NULL, 0 }
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

    manager = new MasterWalletManager(walletRoot);
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
