from __future__ import print_function
import logging

import json
import grpc
from decouple import config

from .stubs import adenine_io_pb2
from .stubs import adenine_io_pb2_grpc

class DidSidechain():
    def sign(self, api_key, private_key, message):
        # NOTE(gRPC Python Team): .close() is possible on a channel and should be
        # used in circumstances in which the with statement does not fit the needs
        # of the code.
        host = config('GRPC_SERVER_HOST')
        port = config('GRPC_SERVER_PORT')

        production = config('PRODUCTION', default=False, cast=bool)
        if production == False:
            with grpc.insecure_channel('{}:{}'.format(host, port)) as channel:
                stub = adenine_io_pb2_grpc.AdenineIoStub(channel)
                req_data = 	{
            				"privateKey": private_key,
            				"msg": message
            			}
                response = stub.Sign(adenine_io_pb2.Request(api_key=api_key, input=json.dumps(req_data)))
                return response
        else:
            credentials = grpc.ssl_channel_credentials()
            with grpc.secure_channel('{}:{}'.format(host, port), credentials) as channel:
                stub = adenine_io_pb2_grpc.AdenineIoStub(channel)
                req_data = 	{
            				"privateKey": private_key,
            				"msg": message
            			}
                response = stub.Sign(adenine_io_pb2.Request(api_key=api_key, input=json.dumps(req_data)))
                return response
