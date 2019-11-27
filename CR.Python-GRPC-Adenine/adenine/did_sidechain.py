from __future__ import print_function
import logging

import grpc
from decouple import config

from adenine.stubs import adenine_io_pb2
from adenine.stubs import adenine_io_pb2_grpc

class DidSidechain():
    def sign(self, api_key, private_key, message):
        # NOTE(gRPC Python Team): .close() is possible on a channel and should be
        # used in circumstances in which the with statement does not fit the needs
        # of the code.
        host = config('GRPC_SERVER_HOST')
        port = config('GRPC_SERVER_PORT')

        credentials = grpc.ssl_channel_credentials()

        with grpc.secure_channel('{}:{}'.format(host, port), credentials) as channel:
            stub = adenine_io_pb2_grpc.AdenineIoStub(channel)
            req_data = 	{
        				"privateKey": private_key,
        				"msg": message
        			} 
            response = stub.sendRequest(adenine_io_pb2.Request(api_key=api_key, input=req_data))
            return response 
