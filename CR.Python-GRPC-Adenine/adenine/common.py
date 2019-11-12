from __future__ import print_function
import logging

import grpc

from adenine.stubs import common_pb2
from adenine.stubs import common_pb2_grpc

class Common():
    def generate_api_request(self, secret_key, did):
        # NOTE(gRPC Python Team): .close() is possible on a channel and should be
        # used in circumstances in which the with statement does not fit the needs
        # of the code.
        with grpc.insecure_channel('localhost:50051') as channel:
            stub = common_pb2_grpc.CommonStub(channel)
            response = stub.GenerateAPIRequest(common_pb2.Request(secret_key=secret_key, did=did))
            return response
        
