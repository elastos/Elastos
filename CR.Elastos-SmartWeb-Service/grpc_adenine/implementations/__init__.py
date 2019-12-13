import json

import threading

from grpc_adenine import settings

from requests import Session
from decouple import config

global WalletAddresses

WalletAddresses = set()


def cronjob_send_ela():
    threading.Timer(120, cronjob_send_ela).start()
    if len(WalletAddresses) == 0:
        return
    headers = {
        'Accepts': 'application/json',
        'Content-Type': 'application/json'
    }
    session = Session()
    session.headers.update(headers)

    req_data_mainchain = {
        "sender": [{
            "address": config('MAINCHAIN_WALLET_ADDRESS'),
            "privateKey": config('MAINCHAIN_WALLET_PRIVATE_KEY')
        }],
        "receiver": []
    }
    req_data_did = {
        "sender": [{
            "address": config('MAINCHAIN_WALLET_ADDRESS2'),
            "privateKey": config('MAINCHAIN_WALLET_PRIVATE_KEY2')
        }],
        "receiver": []
    }
    req_data_token = {
        "sender": [{
            "address": config('MAINCHAIN_WALLET_ADDRESS3'),
            "privateKey": config('MAINCHAIN_WALLET_PRIVATE_KEY3')
        }],
        "receiver": []
    }
    for chain, address in WalletAddresses:
        if chain == "mainchain":
            req_data_mainchain["receiver"].append({
                "address": address,
                "amount": "10"
            })
        elif chain == "did":
            req_data_did["receiver"].append({
                "address": address,
                "amount": "5"
            })
        elif chain == "token":
            req_data_token["receiver"].append({
                "address": address,
                "amount": "5"
            })

    # Transfer from mainchain to mainchain
    transfer_ela_url = config('PRIVATE_NET_IP_ADDRESS') + config(
            'WALLET_SERVICE_URL') + settings.WALLET_API_TRANSFER
    response = session.post(transfer_ela_url, data=json.dumps(req_data_mainchain))
    tx_hash = json.loads(response.text)['result']
    print("Transferred ELA to receipient mainchain address. Transaction hash: ", tx_hash)

    # Transfer from mainchain to did sidechain
    transfer_ela_url = config('PRIVATE_NET_IP_ADDRESS') + config(
            'WALLET_SERVICE_URL') + settings.WALLET_API_CROSSCHAIN_TRANSFER
    response = session.post(transfer_ela_url, data=json.dumps(req_data_did))
    tx_hash = json.loads(response.text)['result']
    print("Transferred ELA to receipient mainchain address. Transaction hash: ", tx_hash)

    # Transfer from mainchain to token sidechain
    transfer_ela_url = config('PRIVATE_NET_IP_ADDRESS') + config(
            'WALLET_SERVICE_TOKENSIDECHAIN_URL') + settings.WALLET_API_CROSSCHAIN_TRANSFER
    response = session.post(transfer_ela_url, data=json.dumps(req_data_token))
    tx_hash = json.loads(response.text)['result']
    print("Transferred ELA to receipient mainchain address. Transaction hash: ", tx_hash)

    WalletAddresses.clear()


cronjob_send_ela()
