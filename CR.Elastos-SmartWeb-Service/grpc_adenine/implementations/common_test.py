import sys
from concurrent import futures
from contextlib import contextmanager

import grpc
from decouple import RepositoryEnv, Config, config

import unittest
import jwt
import logging

from grpc_adenine.implementations.common import Common

from grpc_adenine.stubs.python import common_pb2, common_pb2_grpc


@contextmanager
def mock_server(cls, secret_key):
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
    common_pb2_grpc.add_CommonServicer_to_server(cls(secret_key), server)
    port = server.add_insecure_port('[::]:0')
    server.start()

    try:
        with grpc.insecure_channel('localhost:%d' % port) as channel:
            yield common_pb2_grpc.CommonStub(channel)
    finally:
        server.stop(None)


class CommonTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        log = logging.getLogger("CommonTest")
        log.debug("Setting up CommonTest")

        DOTENV_FILE = '.env.test'
        env_config = Config(RepositoryEnv(DOTENV_FILE))

        cls.secret_key = config('SHARED_SECRET_ADENINE')
        cls.did_to_use = env_config('DID_TO_USE')

    @classmethod
    def tearDownClass(cls):
        log = logging.getLogger("CommonTest")
        log.debug("Tearing down CommonTest")

    # testing Generate API Key with valid response
    def test_generate_api_key(self):
        log = logging.getLogger("CommonTest")
        with mock_server(Common, self.secret_key) as stub:
            jwt_token = jwt.encode({
                'jwt_info': {},
            }, self.secret_key, algorithm='HS256')
            response = stub.GenerateAPIRequest(common_pb2.Request(input=jwt_token), metadata=[('did', self.did_to_use)])
            log.debug(f"Method: test_generate_api_key, Expected: True, Actual: {response.status}")
            self.assertEqual(response.status, True)

    # testing Get API Key with valid response
    def test_get_api_key(self):
        log = logging.getLogger("CommonTest")
        with mock_server(Common, self.secret_key) as stub:
            jwt_token = jwt.encode({
                'jwt_info': {},
            }, self.secret_key, algorithm='HS256')
            response = stub.GetAPIKey(common_pb2.Request(input=jwt_token), metadata=[('did', self.did_to_use)])
            log.debug(f"Method: test_get_api_key, Expected: True, Actual: {response.status}")
            self.assertEqual(response.status, True)


if __name__ == "__main__":
    logging.basicConfig(stream=sys.stderr)
    logging.getLogger("CommonTest").setLevel(logging.DEBUG)
    unittest.main()
