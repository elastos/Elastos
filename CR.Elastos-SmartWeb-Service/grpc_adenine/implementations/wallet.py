import json
import requests

from decouple import config

from web3 import Web3, HTTPProvider
from web3.middleware import geth_poa_middleware

from grpc_adenine import settings
from grpc_adenine.stubs import wallet_pb2
from grpc_adenine.stubs import wallet_pb2_grpc


class Wallet(wallet_pb2_grpc.WalletServicer):

    def CreateWallet(self, request, context):
        # Validate the API Key
        api_key = request.api_key
        # api_status = validate_api_key(api_key)
        # if not api_status:
        #       return adenine_io_pb2.Response(output='', status_message='API Key could not be verified', status=False)

        # reading the file content
        request_input = json.loads(request.input)
        eth_password = request_input['eth_password']

        # Create wallets
        wallet_mainchain = create_wallet_mainchain()
        wallet_sidechain_did = create_wallet_sidechain_did()
        wallet_sidechain_token = wallet_mainchain
        wallet_sidechain_eth = create_wallet_sidechain_eth(eth_password)

        response = {
            'result': {
                'mainchain': {
                    'mnemonic': wallet_mainchain['mnemonic'],
                    'private_key': wallet_mainchain['private_key'],
                    'public_key': wallet_mainchain['public_key'],
                    'ela_address': wallet_mainchain['ela_address']
                },
                'sidechain': {
                    'did': {
                        'mnemonic': wallet_sidechain_did['mnemonic'],
                        'private_key': wallet_sidechain_did['private_key'],
                        'public_key': wallet_sidechain_did['public_key'],
                        'ela_address': wallet_sidechain_did['ela_address'],
                        'did': wallet_sidechain_did['did'],
                    },
                    'token': {
                        'mnemonic': wallet_sidechain_token['mnemonic'],
                        'private_key': wallet_sidechain_token['private_key'],
                        'public_key': wallet_sidechain_token['public_key'],
                        'token_address': wallet_sidechain_token['ela_address'],
                    },
                    'eth': {
                        'eth_address': wallet_sidechain_eth['eth_address'],
                        'password': wallet_sidechain_eth['password']
                    }
                },
            }
        }
        status_message = 'Successfuly created wallets'
        status = True

        return wallet_pb2.Response(output=json.dumps(response), status_message=status_message, status=status)

    def ViewWallet(self, request, context):
        # Validate the API Key
        api_key = request.api_key
        # api_status = validate_api_key(api_key)
        # if not api_status:
        #       return adenine_io_pb2.Response(output='', status_message='API Key could not be verified', status=False)

        response = {
            'result': {
                'key': 'value'
            }
        }
        status_message = 'Successfuly viewed wallets'
        status = True

        return wallet_pb2.Response(output=json.dumps(response), status_message=status_message, status=status)

    def RequestELA(self, request, context):
        # Validate the API Key
        api_key = request.api_key
        # api_status = validate_api_key(api_key)
        # if not api_status:
        #       return adenine_io_pb2.Response(output='', status_message='API Key could not be verified', status=False)

        response = {
            'result': {
                'key': 'value'
            }
        }
        status_message = 'Successfuly deposited ELA'
        status = True

        return wallet_pb2.Response(output=json.dumps(response), status_message=status_message, status=status)


def create_wallet_mainchain():
    result = {}
    # Generate mnemonics
    generate_mnemonics_url = config('PRIVATE_NET_IP_ADDRESS') + config(
        'WALLET_SERVICE_URL') + settings.WALLET_API_GENERATE_MNEMONIC
    response = requests.get(generate_mnemonics_url).json()
    mnemonic = response['result']
    result['mnemonic'] = mnemonic

    # Retrieve wallet from mnemonics
    retrieve_wallet_url = config('PRIVATE_NET_IP_ADDRESS') + config(
        'WALLET_SERVICE_URL') + settings.WALLET_API_RETRIEVE_WALLET_FROM_MNEMONIC
    req_data = {
        "mnemonic": mnemonic,
        "index": 1
    }
    headers = {'Content-type': 'application/json'}
    response = requests.post(retrieve_wallet_url, data=json.dumps(req_data), headers=headers).json()
    data = response['result']
    result['private_key'] = data['privateKey']
    result['public_key'] = data['publicKey']
    result['ela_address'] = data['publicAddress']
    return result


def create_wallet_sidechain_did():
    result = {}
    # Create DID
    create_did_url = config('PRIVATE_NET_IP_ADDRESS') + config('DID_SERVICE_URL') + settings.DID_SERVICE_API_CREATE_DID
    response = requests.get(create_did_url).json()
    data = response['result']
    result['mnemonic'] = ''
    result['private_key'] = data['privateKey']
    result['public_key'] = data['publicKey']
    result['ela_address'] = data['publicAddr']
    result['did'] = data['did']
    return result


def create_wallet_sidechain_eth(password):
    result = {}
    # web3.py instance
    w3 = Web3(HTTPProvider("{0}{1}".format(config('PRIVATE_NET_IP_ADDRESS'), config('SIDECHAIN_ETH_RPC_PORT')),
                           request_kwargs={'timeout': 60}))
    # We need this since our eth sidechain is POA
    w3.middleware_onion.inject(geth_poa_middleware, layer=0)
    if not w3.isConnected():
        return result
    # Create account
    eth_address = w3.parity.personal.newAccount(password)
    result['eth_address'] = eth_address
    result['password'] = password
    return result
