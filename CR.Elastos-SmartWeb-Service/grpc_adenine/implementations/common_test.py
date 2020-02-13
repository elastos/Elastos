import pytest
import sys
from decouple import config
import grpc
from concurrent import futures
from contextlib import contextmanager
import unittest

from grpc_adenine.stubs.python import common_pb2, common_pb2_grpc
from grpc_adenine.implementations.common import Common


host = "localhost"
server_port = 0
secret_key = config('SHARED_SECRET_ADENINE')

network = "gmunet"
mnemonic_to_use = 'obtain pill nest sample caution stone candy habit silk husband give net'
did_to_use = 'n84dqvIK9O0LIPXi27uL0aRnoR45Exdxl218eQyPDD4lW8RPov'
api_key_to_use = ''
private_key_to_use = '1F54BCD5592709B695E85F83EBDA515971723AFF56B32E175F14A158D5AC0D99'

@contextmanager
def mock_server(cls):
	"""Instantiate a mockserver and return a stub for use in tests"""
	server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
	common_pb2_grpc.add_CommonServicer_to_server(Common(), server)
	port = server.add_insecure_port('[::]:{}'.format(server_port))
	server.start()

	try:
		with grpc.insecure_channel('{}:{}'.format(host, port)) as channel:
			yield common_pb2_grpc.CommonStub(channel)
	finally:
		server.stop(None)


class MockServerTest(unittest.TestCase):
	#testing Get API Key with valid DID
	def test_get_api_key_1(self):
		class FakeMockServer(common_pb2_grpc.CommonServicer):
			def GetAPIKey(self, request, context):
				return common_pb2.Response()

		with mock_server(FakeMockServer) as stub:
			response = stub.GetAPIKey(common_pb2.Request(secret_key=secret_key, did=did_to_use), timeout=30)
			self.assertEqual(response.status, True)

	#testing Get API Key with invalid DID
	def test_get_api_key_2(self):
		invalid_did = "n84dqvIK9O0LIPXi27uL0aRnoR45Exdxl218eQyPDD4lW8RPob"
		class FakeMockServer(common_pb2_grpc.CommonServicer):
			def GetAPIKey(self, request, context):
				return common_pb2.Response()

		with mock_server(FakeMockServer) as stub:
			response = stub.GetAPIKey(common_pb2.Request(secret_key=secret_key, did=invalid_did), timeout=30)
			self.assertEqual(response.status, False)

	#testing get api key mnemonic
	def test_get_api_key_mnemonic_1(self):
		class FakeMockServer(common_pb2_grpc.CommonServicer):
			def GetAPIKeyMnemonic(self, request, context):
				return common_pb2.Response()

		with mock_server(FakeMockServer) as stub:
			response = stub.GetAPIKeyMnemonic(common_pb2.RequestMnemonic(mnemonic=mnemonic_to_use), timeout=30)
			self.assertEqual(response.status, True)

	#testing get api key invalid mnemonic
	def test_get_api_key_mnemonic_2(self):
		invalid_mneumonic = "obtain pill nest sample caution stone candy habit silk husband give internet"
		class FakeMockServer(common_pb2_grpc.CommonServicer):
			def GetAPIKeyMnemonic(self, request, context):
				return common_pb2.Response()

		with mock_server(FakeMockServer) as stub:
			response = stub.GetAPIKeyMnemonic(common_pb2.RequestMnemonic(mnemonic=invalid_mneumonic), timeout=30)
			self.assertEqual(response.status, False)

	

