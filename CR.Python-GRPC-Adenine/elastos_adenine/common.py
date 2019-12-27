from __future__ import print_function

import grpc
from decouple import config

from .stubs import common_pb2
from .stubs import common_pb2_grpc
from elastos_adenine.settings import REQUEST_TIMEOUT


class Common:

    def __init__(self):
        host = config('GRPC_SERVER_HOST')
        port = config('GRPC_SERVER_PORT')
        production = config('PRODUCTION', default=False, cast=bool)
        if not production:
            self._channel = grpc.insecure_channel('{}:{}'.format(host, port))
        else:
            credentials = grpc.ssl_channel_credentials()
            self._channel = grpc.secure_channel('{}:{}'.format(host, port), credentials)
        self.stub = common_pb2_grpc.CommonStub(self._channel)

    def close(self):
        self._channel.close()

    def generate_api_request(self, secret_key, did):
        response = self.stub.GenerateAPIRequest(common_pb2.Request(secret_key=secret_key, did=did), timeout=REQUEST_TIMEOUT)
        return response

    def get_api_key_request(self, secret_key, did):
        response = self.stub.GetAPIKey(common_pb2.Request(secret_key=secret_key, did=did), timeout=REQUEST_TIMEOUT)
        return response
