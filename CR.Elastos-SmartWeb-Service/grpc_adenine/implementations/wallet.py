import json
from requests import Session

from decouple import config

from web3 import Web3, HTTPProvider
from web3.middleware import geth_poa_middleware

from grpc_adenine import settings
from grpc_adenine.stubs import wallet_pb2
from grpc_adenine.stubs import wallet_pb2_grpc


class Wallet(wallet_pb2_grpc.WalletServicer):

    def __init__(self):
        headers = {
            'Accepts': 'application/json',
            'Content-Type': 'application/json'
        }
        self.session = Session()
        self.session.headers.update(headers)
        # web3.py instance
        self.web3 = Web3(
            HTTPProvider("{0}{1}".format(config('PRIVATE_NET_IP_ADDRESS'), config('SIDECHAIN_ETH_RPC_PORT')),
                         request_kwargs={'timeout': 60}))
        # We need this since our eth sidechain is POA
        self.web3.middleware_onion.inject(geth_poa_middleware, layer=0)

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
        wallet_mainchain = create_wallet_mainchain(self.session)
        wallet_sidechain_did = create_wallet_sidechain_did(self.session)
        wallet_sidechain_token = wallet_mainchain
        wallet_sidechain_eth = create_wallet_sidechain_eth(self.web3, eth_password)

        response = {
            'result': {
                'mainchain': {
                    'mnemonic': wallet_mainchain['mnemonic'],
                    'private_key': wallet_mainchain['private_key'],
                    'public_key': wallet_mainchain['public_key'],
                    'address': wallet_mainchain['address']
                },
                'sidechain': {
                    'did': {
                        'mnemonic': wallet_sidechain_did['mnemonic'],
                        'private_key': wallet_sidechain_did['private_key'],
                        'public_key': wallet_sidechain_did['public_key'],
                        'address': wallet_sidechain_did['address'],
                        'did': wallet_sidechain_did['did'],
                    },
                    'token': {
                        'mnemonic': wallet_sidechain_token['mnemonic'],
                        'private_key': wallet_sidechain_token['private_key'],
                        'public_key': wallet_sidechain_token['public_key'],
                        'address': wallet_sidechain_token['address'],
                    },
                    'eth': {
                        'address': wallet_sidechain_eth['address'],
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

        request_input = json.loads(request.input)
        chain = request_input['chain']
        address = request_input['address']
        if chain == "eth":
            balance = view_wallet_eth(self.web3, address)
        else:
            balance = view_wallet_general(self.session, address, chain)

        response = {
            'result': {
                'address': address,
                'balance': balance,
            }
        }
        status_message = 'Successfuly retrieved wallet balance'
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


def create_wallet_mainchain(session):
    result = {}
    # Generate mnemonics
    generate_mnemonics_url = config('PRIVATE_NET_IP_ADDRESS') + config(
        'WALLET_SERVICE_URL') + settings.WALLET_API_GENERATE_MNEMONIC
    response = session.get(generate_mnemonics_url)
    data = json.loads(response.text)
    mnemonic = data['result']
    result['mnemonic'] = mnemonic

    # Retrieve wallet from mnemonics
    retrieve_wallet_url = config('PRIVATE_NET_IP_ADDRESS') + config(
        'WALLET_SERVICE_URL') + settings.WALLET_API_RETRIEVE_WALLET_FROM_MNEMONIC
    req_data = {
        "mnemonic": mnemonic,
        "index": 1
    }
    response = session.post(retrieve_wallet_url, data=json.dumps(req_data))
    data = json.loads(response.text)['result']
    result['private_key'] = data['privateKey']
    result['public_key'] = data['publicKey']
    result['address'] = data['publicAddress']
    return result


def create_wallet_sidechain_did(session):
    result = {}
    # Create DID
    create_did_url = config('PRIVATE_NET_IP_ADDRESS') + config('DID_SERVICE_URL') + settings.DID_SERVICE_API_CREATE_DID
    response = session.get(create_did_url)
    data = json.loads(response.text)['result']
    result['mnemonic'] = ''
    result['private_key'] = data['privateKey']
    result['public_key'] = data['publicKey']
    result['address'] = data['publicAddr']
    result['did'] = data['did']
    return result


def create_wallet_sidechain_eth(w3, password):
    result = {}
    if not w3.isConnected():
        return result
    # Create account
    eth_address = w3.parity.personal.newAccount(password)
    result['address'] = eth_address
    result['password'] = password
    return result


def view_wallet_general(session, address, chain):
    if chain == "mainchain":
        get_balance_url = config('PRIVATE_NET_IP_ADDRESS') + config('MAINCHAIN_RPC_PORT')
        d = {
            "method": settings.MAINCHAIN_RPC_GET_BALANCE,
            "params": {
                "address": address
            }
        }
    elif chain == "did":
        get_balance_url = config('PRIVATE_NET_IP_ADDRESS') + config('SIDECHAIN_DID_RPC_PORT')
        d = {
            "method": settings.DID_SIDECHAIN_RPC_GET_BALANCE,
            "params": {
                "address": address
            }
        }
    elif chain == "token":
        get_balance_url = config('PRIVATE_NET_IP_ADDRESS') + config('SIDECHAIN_TOKEN_RPC_PORT')
        d = {
            "method": settings.TOKEN_SIDECHAIN_RPC_GET_BALANCE,
            "params": {
                "address": address
            }
        }
    response = session.post(get_balance_url, data=json.dumps(d))
    data = json.loads(response.text)
    balance = data['result']
    return balance


def view_wallet_eth(w3, address):
    balance = None
    if not w3.isConnected():
        return balance
    address = w3.toChecksumAddress(address)
    balance = w3.eth.getBalance(address)
    balance = w3.fromWei(balance, 'ether')
    return str(balance)
