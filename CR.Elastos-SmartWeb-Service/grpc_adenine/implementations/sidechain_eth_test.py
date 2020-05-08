import json
import sys
import unittest
import jwt
import logging
import grpc
import datetime

from concurrent import futures
from contextlib import contextmanager
from decouple import Config, RepositoryEnv
from grpc_adenine import settings

from solidity_parser import parser
from grpc_adenine.implementations.utils import get_api_from_did
from grpc_adenine.implementations.sidechain_eth import SidechainEth
from grpc_adenine.stubs.python import sidechain_eth_pb2, sidechain_eth_pb2_grpc


@contextmanager
def mock_server(cls):
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
    sidechain_eth_pb2_grpc.add_SidechainEthServicer_to_server(cls(), server)
    port = server.add_insecure_port('[::]:0')
    server.start()

    try:
        with grpc.insecure_channel('localhost:%d' % port) as channel:
            yield sidechain_eth_pb2_grpc.SidechainEthStub(channel)
    finally:
        server.stop(None)


class SidechainEthTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        log = logging.getLogger("SidechainEthTest")
        log.debug("Setting up SidechainEthTest")

        DOTENV_FILE = '.env.test'
        env_config = Config(RepositoryEnv(DOTENV_FILE))

        cls.network = env_config('NETWORK')
        cls.ela_to_use = env_config('ELA_TO_USE')
        cls.ela_eth_to_use = env_config('ELA_ETH_TO_USE')
        cls.did_to_use = env_config('DID_TO_USE')
        cls.api_key_to_use = get_api_from_did(cls.did_to_use)
        cls.private_key_to_use = env_config('PRIVATE_KEY_TO_USE')

    @classmethod
    def tearDownClass(cls):
        log = logging.getLogger("SidechainEthTest")
        log.debug("Tearing down SidechainEthTest")

    def test_deploy_eth_contract(self):
        log = logging.getLogger("SidechainEthTest")
        with mock_server(SidechainEth) as stub:
            filename = 'test/HelloWorld.sol'
            with open(filename, 'r') as myfile:
                contract_source = myfile.read()
            contract_metadata = parser.parse_file(filename)
            contract_name = contract_metadata['children'][1]['name']
            
            jwt_info = {
                'network': self.network,
                'eth_account_address': self.ela_eth_to_use,
                'eth_private_key': '0x35a12175385b24b2f906d6027d440aac7bd31e1097311fa8e3cf21ceac7c4809',
                'eth_gas': 2000000,
                'contract_source': contract_source,
                'contract_name': contract_name,
            }

            jwt_token = jwt.encode({
                'jwt_info': jwt_info,
                'exp': datetime.datetime.utcnow() + datetime.timedelta(hours=settings.TOKEN_EXPIRATION)
            }, self.api_key_to_use, algorithm='HS256')

            response = stub.DeployEthContract(sidechain_eth_pb2.Request(input=jwt_token), timeout=settings.REQUEST_TIMEOUT, metadata=[('did', self.did_to_use)])
            log.debug(f"Method: test_deploy_eth_contract, Expected: True, Actual: {response.status}")
            self.assertEqual(response.status, True)

    def test_watch_eth_contract(self):
        log = logging.getLogger("SidechainEthTest")
        contract_address = '0xc0ba7D9CF73c0410FfC9FB5b768F5257906B13c1'
        contract_name = 'HelloWorld'
        contract_code_hash = 'QmXYqHg8gRnDkDreZtXJgqkzmjujvrAr5n6KXexmfTGqHd'
        with mock_server(SidechainEth) as stub:
            jwt_info = {
                'network': self.network,
                'contract_address': contract_address,
                'contract_name': contract_name,
                'contract_code_hash': contract_code_hash,
            }

            jwt_token = jwt.encode({
                'jwt_info': jwt_info,
                'exp': datetime.datetime.utcnow() + datetime.timedelta(hours=settings.TOKEN_EXPIRATION)
            }, self.api_key_to_use, algorithm='HS256')

            response = stub.WatchEthContract(sidechain_eth_pb2.Request(input=jwt_token), timeout=settings.REQUEST_TIMEOUT, metadata=[('did', self.did_to_use)])
            log.debug(f"Method: test_watch_eth_contract, Expected: True, Actual: {response.status}")
            self.assertEqual(response.status, True)


if __name__ == "__main__":
    logging.basicConfig(stream=sys.stderr)
    logging.getLogger("SidechainEthTest").setLevel(logging.DEBUG)
    unittest.main()
