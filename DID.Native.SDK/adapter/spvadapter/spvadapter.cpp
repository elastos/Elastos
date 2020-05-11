#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <MasterWalletManager.h>
#include <IMasterWallet.h>
#include <ISubWallet.h>
#include <IMainchainSubWallet.h>
#include <IIDChainSubWallet.h>

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <mutex>
#include <condition_variable>

#include "spvadapter.h"

using namespace Elastos::ElaWallet;

static const char *ID_CHAIN = "IDChain";

class SubWalletCallback;

struct SpvDidAdapter {
    bool synced;
    IMasterWalletManager *manager;
    IMasterWallet *masterWallet;
    IIDChainSubWallet *idWallet;
    SubWalletCallback *callback;
};

enum TransactionStatus { DELETED, ADDED, UPDATED };

class TransactionCallback {
private:
    TransactionStatus status;
    int confirms;
    SpvTransactionCallback *callback;
    void *context;

public:
    ~TransactionCallback() {}
    TransactionCallback() :
        status(DELETED), confirms(0), callback(NULL),  context(NULL) {}
    TransactionCallback(int _confirms, SpvTransactionCallback *_callback, void *_context) :
        status(DELETED), confirms(_confirms), callback(_callback), context(_context) {}

    int getConfirms() {
        return confirms;
    }

    void setStatus(TransactionStatus _status) {
        status = _status;
    }

    TransactionStatus getStatus(void) {
        return status;
    }

    void success(const std::string txid) {
        callback(txid.c_str(), 0, NULL, context);
    }

    void failed(const std::string txid, int status, std::string &msg) {
        callback(txid.c_str(), status, msg.c_str(), context);
    }
};

class SubWalletCallback : public ISubWalletCallback {
private:
    SpvDidAdapter *adapter;
    std::map<const std::string, TransactionCallback> txCallbacks;

    void RemoveTransactionCallback(const std::string &tx) {
        txCallbacks.erase(tx);
    }

public:
    ~SubWalletCallback() {
        txCallbacks.clear();
    }

    SubWalletCallback(SpvDidAdapter *_adapter) : adapter(_adapter) {}

    virtual void OnBlockSyncStarted() {
    }

    virtual void OnBlockSyncProgress(const nlohmann::json &progressInfo) {
        if (progressInfo["Progress"] == 100)
            adapter->synced = true;
    }

    virtual void OnBlockSyncStopped() {
    }

    virtual void OnBalanceChanged(const std::string &asset,
            const std::string &balance) {
    }

    virtual void OnTransactionStatusChanged(
            const std::string &txid, const std::string &status,
            const nlohmann::json &desc, uint32_t confirms) {
        if (txCallbacks.find(txid) == txCallbacks.end())
            return;

        // Ignore the other status except Updated when first confirm.
        TransactionCallback &callback = txCallbacks[txid];

        if (status.compare("Updated") == 0) {
            // TODO: bug in SPV SDK
            if (confirms > 1000)
                return;

            // expected confirms >= 1
            if (callback.getConfirms() > 0 && confirms >= callback.getConfirms()) {
                callback.success(txid);
                RemoveTransactionCallback(txid);
            }
        } else if (status.compare("Added") == 0) {
            // expected confirms == 0, Published to tx pool
            if (callback.getConfirms() == 0 ) {
                callback.success(txid);
                RemoveTransactionCallback(txid);
            }
        }
    }

    virtual void OnTxPublished(const std::string &txid,
            const nlohmann::json &result) {
        if (txCallbacks.find(txid) == txCallbacks.end())
            return;

        TransactionCallback &callback = txCallbacks[txid];
        int rc = result["Code"];
        if (rc != 0) {
            std::string msg = result["Reason"];
            callback.failed(txid, rc, msg);
            RemoveTransactionCallback(txid);
        }
    }

    virtual void OnAssetRegistered(const std::string &asset,
            const nlohmann::json &info) {
    }

    virtual void OnConnectStatusChanged(const std::string &status) {
    }

    void RegisterTransactionCallback(const std::string &tx, int confirms,
            SpvTransactionCallback *callback, void *context) {
        TransactionCallback txCallback(confirms, callback, context);
        txCallbacks[tx] = txCallback;
    }
};

class Semaphore {
public:
    Semaphore (int _count = 0) : count(_count) {}

    inline void notify() {
        std::unique_lock<std::mutex> lock(mtx);
        count++;
        cv.notify_one();
    }

    inline void wait() {
        std::unique_lock<std::mutex> lock(mtx);
        while (count == 0)
            cv.wait(lock);

        count--;
    }

private:
    std::mutex mtx;
    std::condition_variable cv;
    int count;
};

static void SyncStart(SpvDidAdapter *adapter)
{
    auto subWallets = adapter->masterWallet->GetAllSubWallets();
    for (auto it = subWallets.begin(); it != subWallets.end(); ++it) {
        try {
           if ((*it)->GetChainID() == ID_CHAIN)
                (*it)->AddCallback(adapter->callback);
            (*it)->SyncStart();
        } catch (...) {
            // ignore
        }
    }

    while (!adapter->synced) {
        // Waiting for sync to latest block height.
        sleep(1);
    }

}

static void SyncStop(SpvDidAdapter *adapter)
{
    auto subWallets = adapter->masterWallet->GetAllSubWallets();
    for (auto it = subWallets.begin(); it != subWallets.end(); ++it) {
        try {
            if ((*it)->GetChainID() == ID_CHAIN)
               (*it)->SyncStop();
            (*it)->RemoveCallback();
        } catch (...) {
            // ignore
        }
    }

    try {
        adapter->manager->FlushData();
    } catch (...) {
        // ignore
    }
}

SpvDidAdapter *SpvDidAdapter_Create(const char *walletDir, const char *walletId,
        const char *network)
{
    nlohmann::json netConfig;
    IMasterWallet *masterWallet;
    int syncState = 0;

    if (!walletDir || !walletId)
        return NULL;

    if (!network)
        network = "MainNet";

    if (strcmp(network, "MainNet") == 0 || strcmp(network, "TestNet") == 0 ||
            strcmp(network, "RegTest") == 0) {
        netConfig = nlohmann::json();
    } else {
        try {
            std::ifstream in(network);
            netConfig = nlohmann::json::parse(in);
        } catch (...) {
            // error number?
            return NULL;
        }

        network = "PrvNet";
    }

    IMasterWalletManager *manager = new MasterWalletManager(
            walletDir, network, netConfig);
    if (!manager)
        return NULL;

    IIDChainSubWallet *idWallet = NULL;

    try {
        masterWallet = manager->GetMasterWallet(walletId);
        std::vector<ISubWallet *> subWallets = masterWallet->GetAllSubWallets();
        for (auto it = subWallets.begin(); it != subWallets.end(); ++it) {
            if ((*it)->GetChainID() == ID_CHAIN)
                idWallet = dynamic_cast<IIDChainSubWallet *>(*it);
        }
    } catch (...) {
    }

    if (!idWallet) {
        delete manager;
        return NULL;
    }

    SpvDidAdapter *adapter = new SpvDidAdapter;
    SubWalletCallback *callback = new SubWalletCallback(adapter);

    adapter->synced = false;
    adapter->manager = manager;
    adapter->masterWallet = masterWallet;
    adapter->idWallet = idWallet;
    adapter->callback = callback;

    SyncStart(adapter);

    return adapter;
}

void SpvDidAdapter_Destroy(SpvDidAdapter *adapter)
{
    if (!adapter)
        return;

    SyncStop(adapter);

    delete adapter->callback;
    delete adapter->manager;
    delete adapter;
}

int SpvDidAdapter_IsAvailable(SpvDidAdapter *adapter)
{
    if (!adapter)
        return 0;

    try {
        // Force sync
        adapter->idWallet->SyncStart();

        auto result = adapter->idWallet->GetAllTransaction(0, 1, "");
        auto count = result["MaxCount"];
        if (count < 1)
            return 1;

        auto tx = result["Transactions"][0];
        int confirm = tx["ConfirmStatus"];
        if (confirm >= 2)
            return 1;
    } catch (...) {
        return 0;
    }
    return 0;
}

class TransactionResult {
private:
    char txid[128];
    int status;
    char message[256];
    Semaphore sem;

public:
    TransactionResult() : status(0) {
        *txid = 0;
        *message = 0;
    }

    const char *getTxid(void) {
        return txid;
    }

    int getStatus(void) {
        return status;
    }

    const char *getMessage(void) {
        return message;
    }

    void update(const char *_txid, int _status, const char *_message) {
        status = _status;

        if (_txid)
            strcpy(txid, _txid);

        if (_message)
            strcpy(message, _message);

        sem.notify();
    }

    void wait(void) {
        sem.wait();
    }
};

static void TransactionCallback(const char *txid, int status,
        const char *msg, void *context)
{
    TransactionResult *tr = (TransactionResult *)context;
    tr->update(txid, status, msg);
}

const char *SpvDidAdapter_CreateIdTransaction(SpvDidAdapter *adapter,
        const char *payload, const char *memo, const char *password)
{
    if (!adapter || !payload || !password)
        return NULL;

    if (!memo)
        memo = "";

    TransactionResult tr;

    SpvDidAdapter_CreateIdTransactionEx(adapter, payload, memo, 1,
        TransactionCallback, &tr, password);

    tr.wait();
    if (tr.getStatus() != 0)
        return NULL;
    else
        return strdup(tr.getTxid());
}

void SpvDidAdapter_CreateIdTransactionEx(SpvDidAdapter *adapter,
        const char *payload, const char *memo, int confirms,
        SpvTransactionCallback *txCallback, void *context,
        const char *password)
{
    if (!memo)
        memo = "";

    try {
        auto payloadJson = nlohmann::json::parse(payload);

        auto tx = adapter->idWallet->CreateIDTransaction(payloadJson, memo);
        tx = adapter->idWallet->SignTransaction(tx, password);
        tx = adapter->idWallet->PublishTransaction(tx);
        std::string txid = tx["TxHash"];
        adapter->callback->RegisterTransactionCallback(txid,
                confirms, txCallback, context);
    } catch (...) {
        txCallback(NULL, -1, "SPV adapter internal error.", context);
    }
}

void SpvDidAdapter_FreeMemory(SpvDidAdapter *adapter, void *mem)
{
    (void)(adapter);

    free(mem);
}
