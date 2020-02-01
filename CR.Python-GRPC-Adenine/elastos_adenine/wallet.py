import json
import grpc

from .stubs import wallet_pb2, wallet_pb2_grpc
from elastos_adenine.settings import REQUEST_TIMEOUT


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

    def create_wallet(self, api_key, network):
        response = self.stub.CreateWallet(wallet_pb2.Request(api_key=api_key, network=network), timeout=REQUEST_TIMEOUT)
        return response

    def view_wallet(self, api_key, network, chain, address):
        req_data = {
            'address': address,
            'chain': chain
        }
        response = self.stub.ViewWallet(wallet_pb2.Request(api_key=api_key, network=network, input=json.dumps(req_data)), timeout=REQUEST_TIMEOUT)
        return response

    def request_ela(self, api_key, chain, address):
        req_data = {
            'address': address,
            'chain': chain
        }
        response = self.stub.RequestELA(wallet_pb2.Request(api_key=api_key, input=json.dumps(req_data)), timeout=REQUEST_TIMEOUT)
        return response
