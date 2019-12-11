from __future__ import print_function

import json
import grpc
from decouple import config

from .stubs import adenine_io_pb2
from .stubs import adenine_io_pb2_grpc


class DidSidechain():

    def __init__(self):
        host = config('GRPC_SERVER_HOST')
        port = config('GRPC_SERVER_PORT')
        production = config('PRODUCTION', default=False, cast=bool)
        if not production:
            self._channel = grpc.insecure_channel('{}:{}'.format(host, port))
        else:
            credentials = grpc.ssl_channel_credentials()
            self._channel = grpc.secure_channel('{}:{}'.format(host, port), credentials)
        self.stub = adenine_io_pb2_grpc.AdenineIoStub(self._channel)

    def close(self):
        self._channel.close()

    def sign(self, api_key, private_key, message):
        req_data = 	{
            "privateKey": private_key,
            "msg": message
        }
        response = self.stub.Sign(adenine_io_pb2.Request(api_key=api_key, input=json.dumps(req_data)))
        return response
