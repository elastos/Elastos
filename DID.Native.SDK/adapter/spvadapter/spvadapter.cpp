#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>

#include <MasterWalletManager.h>
#include <IMasterWallet.h>
#include <ISubWallet.h>
#include <IMainchainSubWallet.h>
#include <IIDChainSubWallet.h>

#include <iostream>
#include <string>
#include <fstream>
#include <vector>

#include "spvadapter.h"

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

    virtual void OnBlockSyncProgress(const nlohmann::json &progressInfo) {
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

struct SpvDidAdapter {
    IMasterWalletManager *manager;
    IIDChainSubWallet *idWallet;
    SubWalletCallback *callback;
    char *resolver;
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
                (*it)->RemoveCallback();
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

SpvDidAdapter *SpvDidAdapter_Create(const char *walletDir, const char *walletId,
        const char *network, const char *resolver)
{
    nlohmann::json netConfig;
    char *url = NULL;

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

    if (resolver) {
        url = strdup(resolver);
        if (!url) {
            delete manager;
            return NULL;
        }
    }

    IIDChainSubWallet *idWallet = NULL;

    CURLcode rc = curl_global_init(CURL_GLOBAL_ALL);
    if (rc != CURLE_OK) {
        if (url)
            free(url);
        delete manager;
        return NULL;
    }

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
        if (url)
            free(url);
        delete manager;
        curl_global_cleanup();
        return NULL;
    }

    SubWalletCallback *callback = new SubWalletCallback();
    SyncStart(manager, callback);

    SpvDidAdapter *adapter = new SpvDidAdapter;
    adapter->resolver = url;
    adapter->manager = manager;
    adapter->idWallet = idWallet;
    adapter->callback = callback;

    return adapter;
}

void SpvDidAdapter_Destroy(SpvDidAdapter *adapter)
{
    if (!adapter)
        return;

    SyncStop(adapter->manager, adapter->callback);

    if (adapter->resolver)
        free(adapter->resolver);

    delete adapter->callback;
    delete adapter->manager;
    delete adapter;

    curl_global_cleanup();
}

int SpvDidAdapter_CreateIdTransaction(SpvDidAdapter *adapter,
        const char *payload, const char *memo, const char *password)
{
    if (!adapter || !payload || !password)
        return -1;

    if (!memo)
        memo = "";

    try {
        auto payloadJson = nlohmann::json::parse(payload);

        auto tx = adapter->idWallet->CreateIDTransaction(payloadJson, memo);
        tx = adapter->idWallet->SignTransaction(tx, password);
        tx = adapter->idWallet->PublishTransaction(tx);
        // std::cout << "ID Transaction: " << tx["TxHash"] << std::endl;
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

typedef struct HttpRequestBody {
    size_t used;
    size_t sz;
    char *data;
} HttpRequestBody;

static size_t HttpRequestBodyReadCallback(void *dest, size_t size,
        size_t nmemb, void *userdata)
{
    HttpRequestBody *request = (HttpRequestBody *)userdata;
    size_t length = size * nmemb;
    size_t bytes_copy = request->sz - request->used;

    if (bytes_copy) {
        if(bytes_copy > length)
            bytes_copy = length;

        memcpy(dest, request->data + request->used, bytes_copy);

        request->used += bytes_copy;
        return bytes_copy;
    }

    return 0;
}

#define DID_RESOLVE_REQUEST "{\"method\":\"getidtxspayloads\",\"params\":{\"id\":\"%s\",\"all\":false}}"

// Caller need free the pointer
const char *SpvDidAdapter_Resolve(SpvDidAdapter *adapter, const char *did)
{
    char buffer[256];

    if (!adapter || !did || !adapter->resolver)
        return NULL;

    // TODO: max did length
    if (strlen(did) > 64)
        return NULL;

    HttpRequestBody request;
    HttpResponseBody response;

    request.used = 0;
    request.sz = sprintf(buffer, DID_RESOLVE_REQUEST, did);
    request.data = buffer;

    CURL *curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, adapter->resolver);

    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, HttpRequestBodyReadCallback);
    curl_easy_setopt(curl, CURLOPT_READDATA, &request);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)request.sz);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, HttpResponseBodyWriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "Accept: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    memset(&response, 0, sizeof(response));
    CURLcode rc = curl_easy_perform(curl);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    if (rc != CURLE_OK) {
        if (response.data)
            free(response.data);

        return NULL;
    }

    return (const char *)response.data;
}

void SpvDidAdapter_FreeMemory(SpvDidAdapter *adapter, void *mem)
{
    (void)(adapter);

    free(mem);
}
