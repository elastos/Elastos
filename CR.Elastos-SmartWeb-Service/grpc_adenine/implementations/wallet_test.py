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

from grpc_adenine.implementations.utils import get_api_from_did
from grpc_adenine.implementations.wallet import Wallet
from grpc_adenine.stubs.python import wallet_pb2, wallet_pb2_grpc


@contextmanager
def mock_server(cls):
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
    wallet_pb2_grpc.add_WalletServicer_to_server(cls(), server)
    port = server.add_insecure_port('[::]:0')
    server.start()

    try:
        with grpc.insecure_channel('localhost:%d' % port) as channel:
            yield wallet_pb2_grpc.WalletStub(channel)
    finally:
        server.stop(None)


class WalletTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        log = logging.getLogger("WalletTest")
        log.debug("Setting up WalletTest")

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
        log = logging.getLogger("WalletTest")
        log.debug("Tearing down WalletTest")

    # testing create wallet with valid response
    def test_create_wallet(self):
        log = logging.getLogger("WalletTest")
        with mock_server(Wallet) as stub:
            jwt_info = {
                'network': self.network
            }

            jwt_token = jwt.encode({
                'jwt_info': jwt_info,
                'exp': datetime.datetime.utcnow() + datetime.timedelta(hours=settings.TOKEN_EXPIRATION)
            }, self.api_key_to_use, algorithm='HS256')

            response = stub.CreateWallet(wallet_pb2.Request(input=jwt_token), timeout=settings.REQUEST_TIMEOUT, metadata=[('did', self.did_to_use)])
            log.debug(f"Method: test_create_wallet, Expected: True, Actual: {response.status}")
            self.assertEqual(response.status, True)

    def test_request_ela_mainchain(self):
        log = logging.getLogger("WalletTest")
        with mock_server(Wallet) as stub:
            jwt_info = {
                'address': self.ela_to_use,
                'chain': 'mainchain'
            }

            jwt_token = jwt.encode({
                'jwt_info': jwt_info,
                'exp': datetime.datetime.utcnow() + datetime.timedelta(hours=settings.TOKEN_EXPIRATION)
            }, self.api_key_to_use, algorithm='HS256')

            response = stub.RequestELA(wallet_pb2.Request(input=jwt_token), timeout=settings.REQUEST_TIMEOUT, metadata=[('did', self.did_to_use)])
            log.debug(f"Method: test_request_ela_mainchain, Expected: True, Actual: {response.status}")
            self.assertEqual(response.status, True)

    def test_request_ela_did(self):
        log = logging.getLogger("WalletTest")
        with mock_server(Wallet) as stub:
            jwt_info = {
                'address': self.ela_to_use,
                'chain': 'did'
            }

            jwt_token = jwt.encode({
                'jwt_info': jwt_info,
                'exp': datetime.datetime.utcnow() + datetime.timedelta(hours=settings.TOKEN_EXPIRATION)
            }, self.api_key_to_use, algorithm='HS256')

            response = stub.RequestELA(wallet_pb2.Request(input=jwt_token), timeout=settings.REQUEST_TIMEOUT, metadata=[('did', self.did_to_use)])
            log.debug(f"Method: test_request_ela_did, Expected: True, Actual: {response.status}")
            self.assertEqual(response.status, True)

    def test_request_ela_token(self):
        log = logging.getLogger("WalletTest")
        with mock_server(Wallet) as stub:
            jwt_info = {
                'address': self.ela_to_use,
                'chain': 'token'
            }

            jwt_token = jwt.encode({
                'jwt_info': jwt_info,
                'exp': datetime.datetime.utcnow() + datetime.timedelta(hours=settings.TOKEN_EXPIRATION)
            }, self.api_key_to_use, algorithm='HS256')

            response = stub.RequestELA(wallet_pb2.Request(input=jwt_token), timeout=settings.REQUEST_TIMEOUT, metadata=[('did', self.did_to_use)])
            log.debug(f"Method: test_request_ela_token, Expected: True, Actual: {response.status}")
            self.assertEqual(response.status, True)

    def test_request_ela_eth(self):
        log = logging.getLogger("WalletTest")
        with mock_server(Wallet) as stub:
            jwt_info = {
                'address': self.ela_to_use,
                'chain': 'eth'
            }

            jwt_token = jwt.encode({
                'jwt_info': jwt_info,
                'exp': datetime.datetime.utcnow() + datetime.timedelta(hours=settings.TOKEN_EXPIRATION)
            }, self.api_key_to_use, algorithm='HS256')

            response = stub.RequestELA(wallet_pb2.Request(input=jwt_token), timeout=settings.REQUEST_TIMEOUT, metadata=[('did', self.did_to_use)])
            log.debug(f"Method: test_request_ela_eth, Expected: True, Actual: {response.status}")
            self.assertEqual(response.status, True)

if __name__ == "__main__":
    logging.basicConfig(stream=sys.stderr)
    logging.getLogger("WalletTest").setLevel(logging.DEBUG)
    unittest.main()
