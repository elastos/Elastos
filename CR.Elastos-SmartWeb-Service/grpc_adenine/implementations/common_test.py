import datetime
import sys
from concurrent import futures
from contextlib import contextmanager

import grpc
from decouple import config

import unittest
import jwt
import logging

from grpc_adenine.implementations.common import Common
from grpc_adenine.implementations.rate_limiter import RateLimiter

from sqlalchemy import create_engine
from sqlalchemy.orm import sessionmaker

from grpc_adenine.stubs.python import common_pb2, common_pb2_grpc


@contextmanager
def mock_server(cls, session, rate_limiter):
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
    common_pb2_grpc.add_CommonServicer_to_server(cls(session=session, rate_limiter=rate_limiter), server)
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
        cls.secret_key = config('SHARED_SECRET_ADENINE')
        cls.did_to_use = 'n84dqvIK9O0LIPXi27uL0aRnoR45Exdxl218eQyPDD4lW8RPov'
        # Connect to the database
        db_name = config('TEST_DB_NAME')
        db_user = config('DB_USER')
        db_password = config('DB_PASSWORD')
        db_host = config('DB_HOST')
        db_port = config('TEST_DB_PORT')
        database_uri = f"postgresql://{db_user}:{db_password}@{db_host}:{db_port}/{db_name}"
        db_engine = create_engine(database_uri)
        session_maker = sessionmaker(bind=db_engine)
        cls.session = session_maker()
        cls.rate_limiter = RateLimiter(cls.session)

    @classmethod
    def tearDownClass(cls):
        log = logging.getLogger("CommonTest")
        log.debug("Tearing down CommonTest")
        cls.session.close()

    # testing Generate API Key with valid response
    def test_generate_api_key(self):
        log = logging.getLogger("CommonTest")
        with mock_server(Common, self.session, self.rate_limiter) as stub:
            jwt_token = jwt.encode({
                'jwt_info': {},
            }, self.secret_key, algorithm='HS256')
            response = stub.GenerateAPIRequest(common_pb2.Request(input=jwt_token), metadata=[('did', self.did_to_use)])
            log.debug(f"Method: test_generate_api_key, Expected: True, Actual: {response.status}")
            self.assertEqual(response.status, True)

    # testing Get API Key with valid response
    def test_get_api_key(self):
        log = logging.getLogger("CommonTest")
        with mock_server(Common, self.session, self.rate_limiter) as stub:
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
