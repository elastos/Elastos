import json
import grpc
import jwt
import datetime
from solidity_parser import parser

from .stubs import sidechain_eth_pb2, sidechain_eth_pb2_grpc
from elastos_adenine.settings import REQUEST_TIMEOUT, TOKEN_EXPIRATION


class SidechainEth:

    def __init__(self, host, port, production):
        if not production:
            self._channel = grpc.insecure_channel('{}:{}'.format(host, port))
        else:
            credentials = grpc.ssl_channel_credentials()
            self._channel = grpc.secure_channel('{}:{}'.format(host, port), credentials)

        self.stub = sidechain_eth_pb2_grpc.SidechainEthStub(self._channel)

    def close(self):
        self._channel.close()

    def deploy_eth_contract(self, api_key, did, network, eth_account_address, eth_private_key, eth_gas, filename):
        with open(filename, 'r') as myfile:
            contract_source = myfile.read()
        contract_metadata = parser.parse_file(filename)
        contract_name = contract_metadata['children'][1]['name']
        
        req_data = {
            'eth_account_address': eth_account_address,
            'eth_private_key': eth_private_key,
            'eth_gas': eth_gas,
            'contract_source': contract_source,
            'contract_name': contract_name,
        }

        jwt_info = {
            'network': network,
            'request_input': req_data
        }

        jwt_token = jwt.encode({
            'jwt_info': jwt_info,
            'exp': datetime.datetime.utcnow() + datetime.timedelta(hours=TOKEN_EXPIRATION)
        }, api_key, algorithm='HS256')

        response = self.stub.DeployEthContract(sidechain_eth_pb2.Request(input=jwt_token), timeout=REQUEST_TIMEOUT, metadata=[('did', did)])
        
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

    def watch_eth_contract(self, api_key, did, network, contract_address, contract_name, contract_code_hash):
        req_data = {
            'contract_address': contract_address,
            'contract_name': contract_name,
            'contract_code_hash': contract_code_hash,
        }

        jwt_info = {
            'network': network,
            'request_input': req_data
        }

        jwt_token = jwt.encode({
            'jwt_info': jwt_info,
            'exp': datetime.datetime.utcnow() + datetime.timedelta(hours=TOKEN_EXPIRATION)
        }, api_key, algorithm='HS256')

        response = self.stub.WatchEthContract(sidechain_eth_pb2.Request(input=jwt_token), timeout=REQUEST_TIMEOUT, metadata=[('did', did)])
        
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
