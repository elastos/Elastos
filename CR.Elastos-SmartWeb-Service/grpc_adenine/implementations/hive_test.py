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
from grpc_adenine.implementations.hive import Hive
from grpc_adenine.stubs.python import hive_pb2, hive_pb2_grpc


@contextmanager
def mock_server(cls):
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
    hive_pb2_grpc.add_HiveServicer_to_server(cls(), server)
    port = server.add_insecure_port('[::]:0')
    server.start()

    try:
        with grpc.insecure_channel('localhost:%d' % port) as channel:
            yield hive_pb2_grpc.HiveStub(channel)
    finally:
        server.stop(None)


class HiveTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        log = logging.getLogger("HiveTest")
        log.debug("Setting up HiveTest")

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
        log = logging.getLogger("HiveTest")
        log.debug("Tearing down HiveTest")

    # testing Generate API Key with valid response
    def test_upload_and_sign(self):
        log = logging.getLogger("HiveTest")
        with mock_server(Hive) as stub:
            response = upload_and_sign(self, stub)
            log.debug(f"Method: test_upload_and_sign, Expected: True, Actual: {response.status}")
            self.assertEqual(response.status, True)

    def test_verify_and_show(self):
        log = logging.getLogger("HiveTest")
        with mock_server(Hive) as stub:
            # First, prepare for the test by uploading the file and sign
            response = upload_and_sign(self, stub)
            output = jwt.decode(response.output, key=self.api_key_to_use, algorithms=['HS256']).get('jwt_info')
            result = output["result"]
            msg = result["msg"]
            pub = result["pub"]
            sig = result["sig"]
            file_hash = result["hash"]

            # Finally, do the actual test
            jwt_info = {
                'network': self.network,
                'msg': msg,
                'pub': pub,
                'sig': sig,
                'hash': file_hash,
                'privateKey': self.private_key_to_use
            }

            jwt_token = jwt.encode({
                'jwt_info': jwt_info,
                'exp': datetime.datetime.utcnow() + datetime.timedelta(hours=settings.TOKEN_EXPIRATION)
            }, self.api_key_to_use, algorithm='HS256')

            response = stub.VerifyAndShow(hive_pb2.Request(input=jwt_token), timeout=settings.REQUEST_TIMEOUT,
                                          metadata=[('did', self.did_to_use)])
            log.debug(f"Method: test_verify_and_show, Expected: True, Actual: {response.status}")
            self.assertEqual(response.status, True)


def upload_and_sign(cls, stub):
    filename = 'test/sample.txt'
    with open(filename, 'rb') as myfile:
        file_content = myfile.read()

    # generate JWT token
    jwt_info = {
        'network': cls.network,
        'privateKey': cls.private_key_to_use
    }

    jwt_token = jwt.encode({
        'jwt_info': jwt_info,
        'exp': datetime.datetime.utcnow() + datetime.timedelta(hours=settings.TOKEN_EXPIRATION)
    }, cls.api_key_to_use, algorithm='HS256')

    response = stub.UploadAndSign(hive_pb2.Request(input=jwt_token, file_content=file_content),
                                  timeout=settings.REQUEST_TIMEOUT, metadata=[('did', cls.did_to_use)])
    return response


if __name__ == "__main__":
    logging.basicConfig(stream=sys.stderr)
    logging.getLogger("HiveTest").setLevel(logging.DEBUG)
    unittest.main()
