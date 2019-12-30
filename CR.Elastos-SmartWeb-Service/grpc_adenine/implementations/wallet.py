import json
import secrets

from requests import Session
from decouple import config

from web3 import Web3, HTTPProvider
from web3.middleware import geth_poa_middleware

from grpc_adenine import settings
from grpc_adenine.implementations import WalletAddresses, WalletAddressesETH
from grpc_adenine.implementations.rate_limiter import RateLimiter
from grpc_adenine.implementations.utils import validate_api_key, check_rate_limit
from grpc_adenine.settings import REQUEST_TIMEOUT
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
        self.rate_limiter = RateLimiter()

    def CreateWallet(self, request, context):

        api_key = request.api_key
        # Validate the API Key
        api_status = validate_api_key(api_key)
        if not api_status:
            response = {
                'result': {
                    'API_Key': api_key
                }
            }
            return wallet_pb2.Response(output=json.dumps(response), status_message='API Key could not be verified', status=False)

        # Check whether the user is able to use this API by checking their rate limiter
        response = check_rate_limit(self.rate_limiter, settings.CREATE_WALLET_LIMIT, api_key, self.CreateWallet.__name__)
        if response:
            return wallet_pb2.Response(output=json.dumps(response),
                                       status_message=f'Number of daily access limit exceeded {response["result"]["daily_limit"]}',
                                       status=False)

        # Create wallets
        wallet_mainchain = create_wallet_mainchain(self.session)
        wallet_sidechain_did = create_wallet_sidechain_did(self.session)
        wallet_sidechain_token = wallet_mainchain
        wallet_sidechain_eth = create_wallet_sidechain_eth(self.web3)

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
                        'private_key': wallet_sidechain_eth['private_key']
                    }
                },
            }
        }
        status_message = 'Successfully created wallets'
        status = True

        return wallet_pb2.Response(output=json.dumps(response), status_message=status_message, status=status)

    def ViewWallet(self, request, context):

        api_key = request.api_key
        # Validate the API Key
        api_status = validate_api_key(api_key)
        if not api_status:
            response = {
                'result': {
                    'API_Key': api_key
                }
            }
            return wallet_pb2.Response(output=json.dumps(response), status_message='API Key could not be verified', status=False)

        # Check whether the user is able to use this API by checking their rate limiter
        response = check_rate_limit(self.rate_limiter, settings.VIEW_WALLET_LIMIT, api_key, self.ViewWallet.__name__)
        if response:
            return wallet_pb2.Response(output=json.dumps(response),
                                       status_message=f'Number of daily access limit exceeded {response["result"]["daily_limit"]}',
                                       status=False)

        request_input = json.loads(request.input)
        chain = request_input['chain']
        address = request_input['address']
        if chain == "eth":
            balance = view_wallet_eth(self.web3, address)
        else:
            balance = view_wallet_general(self.session, chain, address)

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

        api_key = request.api_key
        # Validate the API Key
        api_status = validate_api_key(api_key)
        if not api_status:
            response = {
                'result': {
                    'API_Key': api_key
                }
            }
            return wallet_pb2.Response(output=json.dumps(response), status_message='API Key could not be verified', status=False)

        # Check whether the user is able to use this API by checking their rate limiter
        response = check_rate_limit(self.rate_limiter, settings.REQUEST_ELA_LIMIT, api_key, self.RequestELA.__name__)
        if response:
            return wallet_pb2.Response(output=json.dumps(response),
                                       status_message=f'Number of daily access limit exceeded {response["result"]["daily_limit"]}',
                                       status=False)

        request_input = json.loads(request.input)
        chain = request_input['chain']
        address = request_input['address']

        status_message = 'Successfuly deposited ELA. Please wait 2-4 minutes for the ELA to arrive on main chain and ' \
                         '12-15 minutes to arrive on PoW sidechains'
        status = True
        amount = 5
        if chain == "eth":
            currency_representation = "ELA/ETHSC"
            if len(WalletAddressesETH) < 10000:
                WalletAddressesETH.add(address)
            else:
                status_message = "Could not deposit ELA at this time. Please try again later"
                status = False
        else:
            if len(WalletAddresses) < 10000:
                WalletAddresses.add((chain, address))
            else:
                status_message = "Could not deposit ELA at this time. Please try again later"
                status = False
            if chain == "mainchain":
                amount = 10
                currency_representation = "ELA"
            if chain == "did":
                currency_representation = "ELA/DIDSC"
            elif chain == "token":
                currency_representation = "ELA/TOKENSC"

        response = {
            'result': {
                'address': address,
                'deposit_amount': "{0} {1}".format(amount, currency_representation)
            }
        }

        return wallet_pb2.Response(output=json.dumps(response), status_message=status_message, status=status)


def create_wallet_mainchain(session):
    result = {}
    # Generate mnemonics
    generate_mnemonics_url = config('PRIVATE_NET_IP_ADDRESS') + config(
        'WALLET_SERVICE_URL') + settings.WALLET_API_GENERATE_MNEMONIC
    response = session.get(generate_mnemonics_url, timeout=REQUEST_TIMEOUT)
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
    response = session.post(retrieve_wallet_url, data=json.dumps(req_data), timeout=REQUEST_TIMEOUT)
    data = json.loads(response.text)['result']
    result['private_key'] = data['privateKey']
    result['public_key'] = data['publicKey']
    result['address'] = data['publicAddress']
    return result


def create_wallet_sidechain_did(session):
    result = {}
    # Create DID
    create_did_url = config('PRIVATE_NET_IP_ADDRESS') + config('DID_SERVICE_URL') + settings.DID_SERVICE_API_CREATE_DID
    response = session.get(create_did_url, timeout=REQUEST_TIMEOUT)
    data = json.loads(response.text)['result']
    result['mnemonic'] = ''
    result['private_key'] = data['privateKey']
    result['public_key'] = data['publicKey']
    result['address'] = data['publicAddr']
    result['did'] = data['did']
    return result


def create_wallet_sidechain_eth(w3):
    result = {}
    # Create account
    account = w3.eth.account.create(secrets.token_hex(32))
    result['address'] = account.address
    result['private_key'] = account.privateKey.hex()
    return result


def view_wallet_general(session, chain, address):
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
    response = session.post(get_balance_url, data=json.dumps(d), timeout=REQUEST_TIMEOUT)
    data = json.loads(response.text)
    balance = data['result']
    return balance


def view_wallet_eth(w3, address):
    address = w3.toChecksumAddress(address)
    balance = w3.eth.getBalance(address)
    balance = w3.fromWei(balance, 'ether')
    return str(balance)
