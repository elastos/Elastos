import json
import secrets
import logging
import jwt
import datetime
from requests import Session
from decouple import config
from sqlalchemy.orm import sessionmaker

from web3 import Web3, HTTPProvider
from web3.middleware import geth_poa_middleware

from grpc_adenine import settings
from grpc_adenine.database import db_engine
from grpc_adenine.implementations import WalletAddresses, WalletAddressesETH
from grpc_adenine.implementations.rate_limiter import RateLimiter
from grpc_adenine.implementations.utils import validate_api_key, get_api_from_did
from grpc_adenine.settings import REQUEST_TIMEOUT
from grpc_adenine.stubs.python import wallet_pb2, wallet_pb2_grpc


class Wallet(wallet_pb2_grpc.WalletServicer):

    def __init__(self):
        headers = {
            'Accepts': 'application/json',
            'Content-Type': 'application/json'
        }
        self.session = Session()
        self.session.headers.update(headers)
        session_maker = sessionmaker(bind=db_engine)
        self.rate_limiter = RateLimiter(session_maker())

    def CreateWallet(self, request, context):

        metadata = dict(context.invocation_metadata())
        did = metadata["did"]
        api_key = get_api_from_did(did)

        try:
            jwt_info = jwt.decode(request.input, key=api_key, algorithms=['HS256']).get('jwt_info')
        except Exception as e:
            status_message = 'Authentication Error'
            logging.debug(f"CreateWallet : {did} : {api_key} : {status_message} : {e}")
            return wallet_pb2.Response(output='', status_message=status_message, status=False)

        if type(jwt_info) == str:
            jwt_info = json.loads(jwt_info)

        network = jwt_info['network']

        # Validate the API Key
        api_status = validate_api_key(api_key)
        if not api_status:
            status_message = 'API Key could not be verified'
            logging.debug(f"{did} : {api_key} : {status_message}")
            return wallet_pb2.Response(output='', status_message=status_message,
                                       status=False)

        # Check whether the user is able to use this API by checking their rate limiter
        response = self.rate_limiter.check_rate_limit(settings.CREATE_WALLET_LIMIT, api_key,
                                    self.CreateWallet.__name__)
        if response:
            status_message = f'Number of daily access limit exceeded {response["result"]["daily_limit"]}'
            logging.debug(f"{did} : {api_key} : {status_message}")
            return wallet_pb2.Response(output=json.dumps(response),
                                       status_message=status_message,
                                       status=False)

        # Create wallets
        wallet_mainchain = create_wallet_mainchain(self.session, network)
        if wallet_mainchain is None:
            status_message = 'Error: Mainchain wallet could not created'
            logging.debug(f"{did} : {api_key} : {status_message}")
            return wallet_pb2.Response(output="", status_message=status_message, status=False)

        wallet_sidechain_did = create_wallet_sidechain_did(self.session, network)
        if wallet_sidechain_did is None:
            status_message = 'Error: DID Sidechain wallet could not created'
            logging.debug(f"{did} : {api_key} : {status_message}")
            return wallet_pb2.Response(output="", status_message=status_message, status=False)

        wallet_sidechain_token = wallet_mainchain

        wallet_sidechain_eth = create_wallet_sidechain_eth(network)
        if wallet_sidechain_eth is None:
            status_message = 'Error: Eth Sidechain wallet could not created'
            logging.debug(f"{did} : {api_key} : {status_message}")
            return wallet_pb2.Response(output="", status_message=status_message, status=False)

        # generate jwt token
        jwt_info = {
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

        jwt_token = jwt.encode({
            'jwt_info': jwt_info,
            'exp': datetime.datetime.utcnow() + datetime.timedelta(hours=settings.TOKEN_EXPIRATION)
        }, api_key, algorithm='HS256')

        return wallet_pb2.Response(output=jwt_token, status_message='Successfully created wallets', status=True)

    def RequestELA(self, request, context):

        metadata = dict(context.invocation_metadata())
        did = metadata["did"]
        api_key = get_api_from_did(did)

        try:
            jwt_info = jwt.decode(request.input, key=api_key, algorithms=['HS256']).get('jwt_info')
        except Exception as e:
            status_message = 'Authentication Error'
            logging.debug(f"RequestELA : {did} : {api_key} : {status_message} : {e}")
            return wallet_pb2.Response(output='', status_message=status_message, status=False)

        # Validate the API Key
        api_status = validate_api_key(api_key)
        if not api_status:
            status_message = 'API Key could not be verified'
            logging.debug(f"{did} : {api_key} : {status_message}")
            return wallet_pb2.Response(output='', status_message=status_message,
                                       status=False)

        # Check whether the user is able to use this API by checking their rate limiter
        response = self.rate_limiter.check_rate_limit(settings.REQUEST_ELA_LIMIT, api_key, self.RequestELA.__name__)
        if response:
            status_message = f'Number of daily access limit exceeded {response["result"]["daily_limit"]}'
            logging.debug(f"{did} : {api_key} : {status_message}")
            return wallet_pb2.Response(output=json.dumps(response),
                                       status_message=status_message,
                                       status=False)

        if type(jwt_info) == str:
            jwt_info = json.loads(jwt_info)

        chain = jwt_info['chain']
        address = jwt_info['address']

        status_message = 'Successfully deposited ELA. Please wait 2-4 minutes for the ELA to arrive on main chain and ' \
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

        # generate jwt token
        jwt_info = {
            'result': {
                'address': address,
                'deposit_amount': "{0} {1}".format(amount, currency_representation)
            }
        }

        jwt_token = jwt.encode({
            'jwt_info': jwt_info,
            'exp': datetime.datetime.utcnow() + datetime.timedelta(hours=settings.TOKEN_EXPIRATION)
        }, api_key, algorithm='HS256')

        return wallet_pb2.Response(output=jwt_token, status_message=status_message, status=status)


def create_wallet_mainchain(session, network):
    result = {}
    # Generate mnemonics
    if network == "mainnet":
        generate_mnemonics_url = config('MAIN_NET_WALLET_SERVICE_URL') + settings.WALLET_API_GENERATE_MNEMONIC
    else:
        generate_mnemonics_url = config('PRIVATE_NET_WALLET_SERVICE_URL') + settings.WALLET_API_GENERATE_MNEMONIC
    response = session.get(generate_mnemonics_url, timeout=REQUEST_TIMEOUT)
    data = json.loads(response.text)
    mnemonic = data['result']
    result['mnemonic'] = mnemonic

    # Retrieve wallet from mnemonics
    if network == "mainnet":
        retrieve_wallet_url = config('MAIN_NET_WALLET_SERVICE_URL') + settings.WALLET_API_RETRIEVE_WALLET_FROM_MNEMONIC
    else:
        retrieve_wallet_url = config('PRIVATE_NET_WALLET_SERVICE_URL') + settings.WALLET_API_RETRIEVE_WALLET_FROM_MNEMONIC
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


def create_wallet_sidechain_did(session, network):
    result = {}
    # Create DID
    if network == "mainnet":
        create_did_url = config('MAIN_NET_DID_SERVICE_URL') + settings.DID_SERVICE_API_CREATE_DID
    else:
        create_did_url = config('PRIVATE_NET_DID_SERVICE_URL') + settings.DID_SERVICE_API_CREATE_DID
    response = session.get(create_did_url, timeout=REQUEST_TIMEOUT)
    data = json.loads(response.text)['result']
    result['mnemonic'] = ''
    result['private_key'] = data['privateKey']
    result['public_key'] = data['publicKey']
    result['address'] = data['publicAddr']
    result['did'] = data['did']
    return result


def create_wallet_sidechain_eth(network):
    result = {}
    # Create account
    # web3.py instance
    if network == "mainnet":
        web3 = Web3(HTTPProvider(config('MAIN_NET_SIDECHAIN_ETH_RPC_PORT'),
                                 request_kwargs={'timeout': 60}))
    else:
        web3 = Web3(HTTPProvider(config('PRIVATE_NET_SIDECHAIN_ETH_RPC_PORT'),
                                 request_kwargs={'timeout': 60}))
    # We need this since our eth sidechain is POA
    web3.middleware_onion.inject(geth_poa_middleware, layer=0)
    account = web3.eth.account.create(secrets.token_hex(32))
    result['address'] = account.address
    result['private_key'] = account.privateKey.hex()
    return result

