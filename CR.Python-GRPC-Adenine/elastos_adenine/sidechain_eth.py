import json
import grpc
from decouple import config

from solidity_parser import parser

from .stubs import sidechain_eth_pb2
from .stubs import sidechain_eth_pb2_grpc


class SidechainEth:

    def __init__(self):
        host = config('GRPC_SERVER_HOST')
        port = config('GRPC_SERVER_PORT')
        production = config('PRODUCTION', default=False, cast=bool)
        if not production:
            self._channel = grpc.insecure_channel('{}:{}'.format(host, port))
        else:
            credentials = grpc.ssl_channel_credentials()
            self._channel = grpc.secure_channel('{}:{}'.format(host, port), credentials)
        self.stub = sidechain_eth_pb2_grpc.SidechainEthStub(self._channel)

    def close(self):
        self._channel.close()

    def deploy_eth_contract(self, api_key, eth_account_address, eth_account_password, filename):
        with open(filename, 'r') as myfile:
            contract_source = myfile.read()
        contract_metadata = parser.parse_file(filename)
        req_data = {
            'eth_account_address': eth_account_address,
            'eth_account_password': eth_account_password,
            'filename': filename,
            'contract_source': contract_source,
            'contract_metadata': contract_metadata,
        }
        response = self.stub.DeployEthContract(sidechain_eth_pb2.Request(api_key=api_key, input=json.dumps(req_data)))
        return response
