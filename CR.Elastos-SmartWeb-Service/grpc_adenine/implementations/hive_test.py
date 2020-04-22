import sys
import unittest
import jwt
import logging
import grpc
import datetime

from concurrent import futures
from contextlib import contextmanager
from decouple import Config, RepositoryEnv
from sqlalchemy import create_engine
from requests import Session
from sqlalchemy.orm import sessionmaker

from grpc_adenine.implementations.rate_limiter import RateLimiter
from grpc_adenine import settings

from grpc_adenine.implementations.hive import Hive
from grpc_adenine.stubs.python import hive_pb2, hive_pb2_grpc

from grpc_adenine.implementations.common import Common
from grpc_adenine.stubs.python import common_pb2, common_pb2_grpc


@contextmanager
def mock_server(cls, session, rate_limiter):
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
    hive_pb2_grpc.add_HiveServicer_to_server(cls(session=session, rate_limiter=rate_limiter), server)
    common_pb2_grpc.add_CommonServicer_to_server(cls(session=session, rate_limiter=rate_limiter), server)
    port = server.add_insecure_port('[::]:0')
    server.start()

    try:
        with grpc.insecure_channel('localhost:%d' % port) as channel:
            yield hive_pb2_grpc.HiveStub(channel)
            #yield common_pb2_grpc.CommonStub(channel)
    finally:
        server.stop(None)


class HiveTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        log = logging.getLogger("HiveTest")
        log.debug("Setting up HiveTest")

        DOTENV_FILE = '.env.test'
        env_config = Config(RepositoryEnv(DOTENV_FILE))

        cls.secret_key = env_config('SHARED_SECRET_ADENINE')
        cls.network = env_config('NETWORK')
        cls.ela_to_use = env_config('ELA_TO_USE')
        cls.ela_eth_to_use = env_config('ELA_ETH_TO_USE')
        cls.did = env_config('DID_TO_USE')
        cls.private_key = env_config('PRIVATE_KEY_TO_USE')
        cls.api_key = 'HkwTeqr7WzcyLXWuWRwPKeIViwW6xyiAyvKcl9QZjJqDZkxcqKMzeFTYLaoZatgt'
        
        # Connect to the database
        db_name = env_config('TEST_DB_NAME')
        db_user = env_config('DB_USER')
        db_password = env_config('DB_PASSWORD')
        db_host = env_config('DB_HOST')
        db_port = env_config('TEST_DB_PORT')
        database_uri = f"postgresql://{db_user}:{db_password}@{db_host}:{db_port}/{db_name}"
        db_engine = create_engine(database_uri)
        Session = sessionmaker(bind=db_engine)
        cls.session = Session()
        cls.rate_limiter = RateLimiter(Session())

    @classmethod
    def tearDownClass(cls):
        log = logging.getLogger("HiveTest")
        log.debug("Tearing down HiveTest")
        cls.session.close()

    # Get API Key for testing
    def test_get_api_key(self):
        log = logging.getLogger("CommonTest")
        with mock_server(Common, self.session, self.rate_limiter) as stub:
            jwt_token = jwt.encode({
                'jwt_info': {},
            }, self.secret_key, algorithm='HS256')
            response = stub.GetAPIKey(common_pb2.Request(input=jwt_token), metadata=[('did', self.did)])
            if response['status']:
                json_output = json.loads(response['output'])
                for i in json_output['result']:
                    if(i=='api_key'):
                        self.api_key = json_output['result'][i]
            log.debug(f"Method: test_get_api_key, Expected: True, Actual: {response.status}")
            self.assertEqual(response.status, True)

    # testing Generate API Key with valid response
    def test_upload_and_sign(self):
        log = logging.getLogger("HiveTest")
        with mock_server(Hive, self.session, self.rate_limiter) as stub:
            filename = '../../test/sample.txt'
            with open(filename, 'rb') as myfile:
                file_content = myfile.read()

            # generate JWT token
            jwt_info = {
                'network': self.network,
                'privateKey': self.private_key
            }

            jwt_token = jwt.encode({
                'jwt_info': jwt_info,
                'exp': datetime.datetime.utcnow() + datetime.timedelta(hours=settings.TOKEN_EXPIRATION)
                }, self.api_key, algorithm='HS256')

            response = stub.UploadAndSign(hive_pb2.Request(input=jwt_token, file_content=file_content),
                                           timeout=settings.REQUEST_TIMEOUT, metadata=[('did', self.did)])
            log.debug(f"Method: test_upload_and_sign, Expected: True, Actual: {response.status}")
            self.assertEqual(response.status, True)

    # testing Get API Key with valid response
    def test_verify_and_show(self):
        log = logging.getLogger("HiveTest")
        with mock_server(Hive, self.session, self.rate_limiter) as stub:
            jwt_info = {
                'network': self.network,
                'msg': env_config('msg'),
                'pub': env_config('pub'),
                'sig': env_config('sig'),
                'hash': env_config('hash'),
                'privateKey': self.private_key
            }

            jwt_token = jwt.encode({
                'jwt_info': jwt_info,
                'exp': datetime.datetime.utcnow() + datetime.timedelta(hours=settings.TOKEN_EXPIRATION)
            }, self.api_key, algorithm='HS256')

            response = stub.VerifyAndShow(hive_pb2.Request(input=jwt_token), timeout=settings.REQUEST_TIMEOUT,
                                               metadata=[('did', self.did)])
            log.debug(f"Method: test_verify_and_show, Expected: True, Actual: {response.status}")
            self.assertEqual(response.status, True)


if __name__ == "__main__":
    logging.basicConfig(stream=sys.stderr)
    logging.getLogger("HiveTest").setLevel(logging.DEBUG)
    unittest.main()
