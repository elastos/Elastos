import json

import requests
from decouple import config
import logging
import jwt
import datetime
from web3 import Web3, HTTPProvider
from web3.middleware import geth_poa_middleware
from solc import compile_standard

from grpc_adenine import settings
from grpc_adenine.implementations.rate_limiter import RateLimiter
from grpc_adenine.implementations.utils import get_api_from_did
from grpc_adenine.settings import REQUEST_TIMEOUT
from grpc_adenine.stubs.python import sidechain_eth_pb2, sidechain_eth_pb2_grpc


class SidechainEth(sidechain_eth_pb2_grpc.SidechainEthServicer):

    def __init__(self):
        self.headers = {
            'Content-Disposition': 'multipart/form-data;boundary=--------------------------608819652137318562927303'
        }
        self.rate_limiter = RateLimiter()

    def DeployEthContract(self, request, context):

        metadata = dict(context.invocation_metadata())
        did = metadata["did"]
        api_key = get_api_from_did(did)

        try:
            jwt_info = jwt.decode(request.input, key=api_key, algorithms=['HS256']).get('jwt_info')
        except Exception as e:
            status_message = 'Authentication Error'
            logging.debug(f"DeployEthContract : {did} : {api_key} : {status_message} : {e}")
            return sidechain_eth_pb2.Response(output='', status_message=status_message, status=False)

        if type(jwt_info) == str:
            jwt_info = json.loads(jwt_info)

        network = jwt_info['network']

        # Check whether the user is able to use this API by checking their rate limiter
        response = self.rate_limiter.check_rate_limit(settings.DEPLOY_ETH_CONTRACT_LIMIT, api_key,
                                    self.DeployEthContract.__name__)
        if response:
            status_message = f'Number of daily access limit exceeded {response["result"]["daily_limit"]}'
            logging.debug(f"{did} : {api_key} : {status_message}")
            return sidechain_eth_pb2.Response(output=json.dumps(response),
                                              status_message=status_message,
                                              status=False)

        if network == "mainnet":
            hive_api_url = config('MAIN_NET_HIVE_PORT') + settings.HIVE_API_ADD_FILE
            web3 = Web3(HTTPProvider(config('MAIN_NET_SIDECHAIN_ETH_RPC_PORT'),
                                     request_kwargs={'timeout': 60}))
        else:
            hive_api_url = config('PRIVATE_NET_HIVE_PORT') + settings.HIVE_API_ADD_FILE
            web3 = Web3(HTTPProvider(config('PRIVATE_NET_SIDECHAIN_ETH_RPC_PORT'),
                                     request_kwargs={'timeout': 60}))

        # reading the file content
        eth_account_address = jwt_info['eth_account_address']
        eth_gas = jwt_info['eth_gas']
        eth_private_key = jwt_info['eth_private_key']
        contract_name = jwt_info['contract_name']
        contract_source = jwt_info['contract_source']

        # upload smart contract code to hive
        response = requests.get(hive_api_url, headers=self.headers, files={'file': contract_source}, timeout=REQUEST_TIMEOUT)
        data = json.loads(response.text)
        hive_hash = data['Hash']

        if not response:
            status_message = 'Error: Smart contract code could not be uploaded'
            logging.debug(f"{did} : {api_key} : {status_message}")
            return sidechain_eth_pb2.Response(output="", status_message=status_message, status=False)
        
        try:
            # We need this since our eth sidechain is POA
            web3.middleware_onion.inject(geth_poa_middleware, layer=0)
            if not web3.isConnected():
                status_message = 'Error: Could not connect to Eth Sidechain node'
                logging.debug(f"{did} : {api_key} : {status_message}")
                return sidechain_eth_pb2.Response(output="", status_message=status_message, status=False)

            compiled_sol = compile_standard({
                "language": "Solidity",
                "sources": {
                    contract_name: {
                        "content": contract_source,
                    },
                },
                "settings": {
                    'evmVersion': 'byzantium',
                    "outputSelection": {
                        "*": {"*": ["*"]}
                    }
                }
            })

            # get bytecode
            bytecode = compiled_sol['contracts'][contract_name][contract_name]['evm']['bytecode']['object']

            transaction = {
                'gas': eth_gas,
                'gasPrice': web3.eth.gasPrice,
                'nonce': web3.eth.getTransactionCount(web3.toChecksumAddress(eth_account_address)),
                'data': '0x' + bytecode
            }
            signed_tx = web3.eth.account.sign_transaction(transaction, eth_private_key)
            if signed_tx is None:
                status_message = 'Error: Transaction could not be signed'
                logging.debug(f"{did} : {api_key} : {status_message}")
                return sidechain_eth_pb2.Response(output="", status_message=status_message, status=False)

            # Submit the transaction that deploys the contract
            tx_hash = web3.eth.sendRawTransaction(signed_tx.rawTransaction)
            if tx_hash is None:
                status_message = 'Error: Transaction could not be send to deploy contract'
                logging.debug(f"{did} : {api_key} : {status_message}")
                return sidechain_eth_pb2.Response(output="", status_message=status_message, status=False)

            # Wait for the transaction to be mined, and get the transaction receipt
            tx_receipt = web3.eth.waitForTransactionReceipt(tx_hash)
        except Exception as e:
            status_message = 'Error Occurred while deploying the Ethereum Contract'
            logging.debug(f"DeployEthContract : {did} : {api_key} : {status_message} : {e}")
            return sidechain_eth_pb2.Response(output='', status_message=status_message, status=False)

        # generate jwt token
        jwt_info = {
            'result': {
                'contract_address': tx_receipt.contractAddress,
                'contract_name': contract_name,
                'contract_code_hash': hive_hash,
            }
        }

        jwt_token = jwt.encode({
            'jwt_info': jwt_info,
            'exp': datetime.datetime.utcnow() + datetime.timedelta(hours=settings.TOKEN_EXPIRATION)
        }, api_key, algorithm='HS256')

        return sidechain_eth_pb2.Response(output=jwt_token, status_message='Successfully deployed Eth Smart Contract', status=True)

    def WatchEthContract(self, request, context):

        metadata = dict(context.invocation_metadata())
        did = metadata["did"]
        api_key = get_api_from_did(did)

        try:
            jwt_info = jwt.decode(request.input, key=api_key, algorithms=['HS256']).get('jwt_info')
        except Exception as e:
            status_message = 'Authentication Error'
            logging.debug(f"WatchEthContract : {did} : {api_key} : {status_message} : {e}")
            return sidechain_eth_pb2.Response(output='', status_message=status_message, status=False)

        if type(jwt_info) == str:
            jwt_info = json.loads(jwt_info)

        network = jwt_info['network']

        # Check whether the user is able to use this API by checking their rate limiter
        response = self.rate_limiter.check_rate_limit(settings.WATCH_ETH_CONTRACT_LIMIT, api_key,
                                    self.WatchEthContract.__name__)
        if response:
            status_message = f'Number of daily access limit exceeded {response["result"]["daily_limit"]}'
            logging.debug(f"{did} : {api_key} : {status_message}")
            return sidechain_eth_pb2.Response(output=json.dumps(response),
                                              status_message=status_message,
                                              status=False)

        if network == "mainnet":
            hive_api_url = config('MAIN_NET_HIVE_PORT') + settings.HIVE_API_RETRIEVE_FILE + "{}"
            web3 = Web3(HTTPProvider(config('MAIN_NET_SIDECHAIN_ETH_RPC_PORT'),
                                     request_kwargs={'timeout': 60}))
        else:
            hive_api_url = config('PRIVATE_NET_HIVE_PORT') + settings.HIVE_API_RETRIEVE_FILE + "{}"
            web3 = Web3(HTTPProvider(config('PRIVATE_NET_SIDECHAIN_ETH_RPC_PORT'),
                                     request_kwargs={'timeout': 60}))

        # reading the file content
        contract_address = jwt_info['contract_address']
        contract_name = jwt_info['contract_name']
        contract_code_hash = jwt_info['contract_code_hash']

        # show smart contract code from Hive
        response = requests.get(hive_api_url.format(contract_code_hash), headers=self.headers, timeout=REQUEST_TIMEOUT)
        contract_source = response.text

        if not response:
            status_message = 'Error: Smart contract code could not be retrieved from Hive'
            logging.debug(f"{did} : {api_key} : {status_message}")
            return sidechain_eth_pb2.Response(output="", status_message=status_message, status=False)

        try:
            # Get smart contract details
            compiled_sol = compile_standard({
                "language": "Solidity",
                "sources": {
                    contract_name: {
                        "content": contract_source,
                    },
                },
                "settings": {
                    'evmVersion': 'byzantium',
                    "outputSelection": {
                        "*": {"*": ["*"]}
                    }
                }
            })
            abi = json.loads(compiled_sol['contracts'][contract_name][contract_name]['metadata'])['output']['abi']

            # We need this since our eth sidechain is POA
            web3.middleware_onion.inject(geth_poa_middleware, layer=0)

            contract = web3.eth.contract(address=contract_address, abi=abi)
            functions = contract.all_functions()
            contract_functions = []
            for function in functions:
                contract_function = repr(function)[1:-1].split()[1]
                contract_functions.append(contract_function)
        except Exception as e:
            status_message = 'Error Occurred while watching the Ethereum Contract'
            logging.debug(f"WatchEthContract : {did} : {api_key} : {status_message} : {e}")
            return sidechain_eth_pb2.Response(output='', status_message=status_message, status=False)

        # generate jwt token
        jwt_info = {
            'result': {
                'contract_address': contract_address,
                'contract_functions': contract_functions,
                'contract_source': contract_source
            }
        }

        jwt_token = jwt.encode({
            'jwt_info': jwt_info,
            'exp': datetime.datetime.utcnow() + datetime.timedelta(hours=settings.TOKEN_EXPIRATION)
        }, api_key, algorithm='HS256')

        return sidechain_eth_pb2.Response(output=jwt_token, status_message='Successfully viewed Eth Smart Contract', status=True)
