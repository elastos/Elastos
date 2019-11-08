#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>

#include <MasterWalletManager.h>
#include <IMasterWallet.h>
#include <ISubWallet.h>
#include <IMainchainSubWallet.h>
#include <IIDChainSubWallet.h>

#include <string>
#include <fstream>
#include <vector>

#include "spvadaptor.h"

using namespace Elastos::ElaWallet;

static const char *ID_CHAIN = "IDChain";

class SubWalletCallback : public ISubWalletCallback {
public:
    ~SubWalletCallback() {}
    SubWalletCallback() {}

    virtual void OnTransactionStatusChanged(
        const std::string &txid,const std::string &status,
        const nlohmann::json &desc,uint32_t confirms) {
    }

    virtual void OnBlockSyncStarted() {
    }

    virtual void OnBlockSyncProgress(uint32_t currentBlockHeight, uint32_t estimatedHeight, time_t lastBlockTime) {
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
};

struct SpvDidAdaptor {
    IMasterWalletManager *manager;
    IIDChainSubWallet *idWallet;
    SubWalletCallback *callback;
};

static
void SyncStart(IMasterWalletManager *manager, ISubWalletCallback *callback)
{
    auto masterWallets = manager->GetAllMasterWallets();
    for (auto it = masterWallets.begin(); it != masterWallets.end(); ++it) {
        auto subWallets = (*it)->GetAllSubWallets();
        for (auto it = subWallets.begin(); it != subWallets.end(); ++it) {
            try {
                (*it)->AddCallback(callback);
                (*it)->SyncStart();
            } catch (...) {
                // ignore
            }
        }
    }
}

static
void SyncStop(IMasterWalletManager *manager, ISubWalletCallback *callback)
{
    auto masterWallets = manager->GetAllMasterWallets();
    for (auto it = masterWallets.begin(); it != masterWallets.end(); ++it) {
        auto subWallets = (*it)->GetAllSubWallets();
        for (auto it = subWallets.begin(); it != subWallets.end(); ++it) {
            try {
                (*it)->SyncStop();
                (*it)->RemoveCallback(callback);
            } catch (...) {
                // ignore
            }
        }
    }

    try {
        manager->FlushData();
    } catch (...) {
        // ignore
    }
}

SpvDidAdaptor *SpvDidAdaptor_Create(const char *walletDir, const char *walletId,
        const char *network)
{
    nlohmann::json netConfig;

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
    IIDChainSubWallet *idWallet = NULL;

    CURLcode rc = curl_global_init(CURL_GLOBAL_ALL);
    if (rc != CURLE_OK)
        return NULL;

    try {
        auto masterWallet = manager->GetMasterWallet(walletId);
        std::vector<ISubWallet *> subWallets = masterWallet->GetAllSubWallets();
        for (auto it = subWallets.begin(); it != subWallets.end(); ++it) {
            if ((*it)->GetChainID() == ID_CHAIN)
                idWallet = dynamic_cast<IIDChainSubWallet *>(*it);
        }
    } catch (...) {
    }

    if (!idWallet) {
        delete manager;

        curl_global_cleanup();
        return NULL;
    }

    SubWalletCallback *callback = new SubWalletCallback();
    SyncStart(manager, callback);

    SpvDidAdaptor *adaptor = new SpvDidAdaptor;
    adaptor->manager = manager;
    adaptor->idWallet = idWallet;
    adaptor->callback = callback;

    return adaptor;
}

void SpvDidAdaptor_Destroy(SpvDidAdaptor *adaptor)
{
    if (!adaptor)
        return;

    SyncStop(adaptor->manager, adaptor->callback);

    delete adaptor->callback;
    delete adaptor->manager;
    delete adaptor;

    curl_global_cleanup();
}

int SpvDidAdaptor_CreateIdTransaction(SpvDidAdaptor *adaptor,
        const char *payload, const char *memo, const char *password)
{
    if (!adaptor || !payload || !password)
        return -1;

    try {
        auto payloadJson = nlohmann::json::parse(payload);

        auto tx = adaptor->idWallet->CreateIDTransaction(payloadJson, memo);
        auto signedTx = adaptor->idWallet->SignTransaction(tx, password);
        adaptor->idWallet->PublishTransaction(signedTx);
    } catch (...) {
        return -1;
    }

    return 0;
}

typedef struct HttpResponseBody {
    size_t used;
    size_t sz;
    void *data;
} HttpResponseBody;

static size_t HttpResponseBodyWriteCallback(char *ptr,
        size_t size, size_t nmemb, void *userdata)
{
    HttpResponseBody *response = (HttpResponseBody *)userdata;
    size_t length = size * nmemb;

    if (response->sz - response->used < length) {
        size_t new_sz;
        size_t last_try;
        void *new_data;

        if (response->sz + length < response->sz) {
            response->used = 0;
            return 0;
        }

        for (new_sz = response->sz ? response->sz << 1 : 512, last_try = response->sz;
            new_sz > last_try && new_sz <= response->sz + length;
            last_try = new_sz, new_sz <<= 1) ;

        if (new_sz <= last_try)
            new_sz = response->sz + length;

        new_data = realloc(response->data, new_sz);
        if (!new_data) {
            response->used = 0;
            return 0;
        }

        response->data = new_data;
        response->sz = new_sz;
    }

    memcpy((char *)response->data + response->used, ptr, length);
    response->used += length;

    return length;
}

// Caller need free the pointer
const char *SpvDidAdaptor_Resolve(SpvDidAdaptor *adaptor, const char *did)
{
    if (!adaptor || !did)
        return NULL;

    HttpResponseBody response;

    // TODO: make the real URL for DID resolve
    CURL *curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, "http://example.com");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, HttpResponseBodyWriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    memset(&response, 0, sizeof(response));
    CURLcode rc = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    if (rc != CURLE_OK) {
        if (response.data)
            free(response.data);

        return NULL;
    }

    return (const char *)response.data;
}

void SpvDidAdaptor_FreeMemory(SpvDidAdaptor *adaptor, void *mem)
{
    (void)(adaptor);

    free(mem);
}
