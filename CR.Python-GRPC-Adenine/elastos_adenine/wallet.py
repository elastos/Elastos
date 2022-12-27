import json
import grpc
import jwt
import datetime

from .stubs import wallet_pb2, wallet_pb2_grpc
from elastos_adenine.settings import REQUEST_TIMEOUT, TOKEN_EXPIRATION


class Wallet:

    def __init__(self, host, port, production):
        if not production:
            self._channel = grpc.insecure_channel('{}:{}'.format(host, port))
        else:
            credentials = grpc.ssl_channel_credentials()
            self._channel = grpc.secure_channel('{}:{}'.format(host, port), credentials)

        self.stub = wallet_pb2_grpc.WalletStub(self._channel)

    def close(self):
        self._channel.close()

    def create_wallet(self, api_key, did, network):
        jwt_info = {
            'network': network
        }

        jwt_token = jwt.encode({
            'jwt_info': jwt_info,
            'exp': datetime.datetime.utcnow() + datetime.timedelta(hours=TOKEN_EXPIRATION)
        }, api_key, algorithm='HS256')

        response = self.stub.CreateWallet(wallet_pb2.Request(input=jwt_token), timeout=REQUEST_TIMEOUT, metadata=[('did', did)])
        
        if response.status:
            output = jwt.decode(response.output, key=api_key, algorithms=['HS256']).get('jwt_info')
            result = {
                'output': json.dumps(output),
                'status_message': response.status_message,
                'status': response.status
            }
            return result
        else:
            result = {
                'output': '',
                'status_message': response.status_message,
                'status': response.status
            }
            return result

    def request_ela(self, api_key, did, chain, address):
        jwt_info = {
            'address': address,
            'chain': chain
        }

        jwt_token = jwt.encode({
            'jwt_info': jwt_info,
            'exp': datetime.datetime.utcnow() + datetime.timedelta(hours=TOKEN_EXPIRATION)
        }, api_key, algorithm='HS256')

        response = self.stub.RequestELA(wallet_pb2.Request(input=jwt_token), timeout=REQUEST_TIMEOUT, metadata=[('did', did)])
        
        if response.status:
            output = jwt.decode(response.output, key=api_key, algorithms=['HS256']).get('jwt_info')
            result = {
                'output': json.dumps(output),
                'status_message': response.status_message,
                'status': response.status
            }
            return result
        else:
            result = {
                'output': '',
                'status_message': response.status_message,
                'status': response.status
            }
            return result
