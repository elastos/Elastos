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

class SubWalletCallback : public ISubWalletCallback {
private:
    SpvDidAdapter *adapter;

public:
    ~SubWalletCallback() {
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
    }

    virtual void OnTxPublished(const std::string &txid,
            const nlohmann::json &result) {
    }

    virtual void OnAssetRegistered(const std::string &asset,
            const nlohmann::json &info) {
    }

    virtual void OnConnectStatusChanged(const std::string &status) {
    }

    virtual void OnETHSCEventHandled(const nlohmann::json &event) {
    }
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

bool SpvDidAdapter_IsAvailable(SpvDidAdapter *adapter)
{
    if (!adapter)
        return false;

    try {
        // Force sync
        adapter->idWallet->SyncStart();

        auto result = adapter->idWallet->GetAllTransaction(0, 1, "");
        auto count = result["MaxCount"];
        if (count < 1)
            return true;

        auto tx = result["Transactions"][0];
        int confirm = tx["ConfirmStatus"];
        if (confirm >= 2)
            return true;
    } catch (...) {
        return false;
    }
    return false;
}

bool SpvDidAdapter_CreateIdTransaction(SpvDidAdapter *adapter,
        const char *payload, const char *memo, const char *password)
{
    if (!adapter || !payload || !password)
        return false;

    if (!memo)
        memo = "";

    try {
        auto payloadJson = nlohmann::json::parse(payload);

        auto tx = adapter->idWallet->CreateIDTransaction(payloadJson, memo);
        tx = adapter->idWallet->SignTransaction(tx, password);
        adapter->idWallet->PublishTransaction(tx);
    } catch (...) {
        return false;
    }

    return true;
}
