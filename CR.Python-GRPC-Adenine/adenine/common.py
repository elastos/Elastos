from __future__ import print_function
import logging

import grpc
from decouple import config

from adenine.stubs import common_pb2
from adenine.stubs import common_pb2_grpc

class Common():
    def generate_api_request(self, secret_key, did):
        # NOTE(gRPC Python Team): .close() is possible on a channel and should be
        # used in circumstances in which the with statement does not fit the needs
        # of the code
        host = config('GRPC_SERVER_HOST')
        port = config('GRPC_SERVER_PORT')

        production = config('PRODUCTION', default=False, cast=bool)
        if production == False:
            with grpc.insecure_channel('{}:{}'.format(host, port)) as channel:
                stub = common_pb2_grpc.CommonStub(channel)
                response = stub.GenerateAPIRequest(common_pb2.Request(secret_key=secret_key, did=did))
                return response
        else:
            credentials = grpc.ssl_channel_credentials()
            with grpc.secure_channel('{}:{}'.format(host, port), credentials) as channel:
                stub = common_pb2_grpc.CommonStub(channel)
                response = stub.GenerateAPIRequest(common_pb2.Request(secret_key=secret_key, did=did))
                return response

