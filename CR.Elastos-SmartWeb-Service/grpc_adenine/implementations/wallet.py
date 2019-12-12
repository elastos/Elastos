import json
from decouple import config

from web3 import Web3, HTTPProvider
from web3.middleware import geth_poa_middleware
from solc import compile_standard

from grpc_adenine.stubs import wallet_pb2
from grpc_adenine.stubs import wallet_pb2_grpc


class Wallet(wallet_pb2_grpc.WalletServicer):

    def CreateWallet(self, request, context):

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
