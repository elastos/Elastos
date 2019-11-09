from __future__ import print_function
import logging

import grpc

from adenine.stubs import did_pb2
from adenine.stubs import did_pb2_grpc

class DidSidechain():
    def sign(self, api_key, private_key, message):
        # NOTE(gRPC Python Team): .close() is possible on a channel and should be
        # used in circumstances in which the with statement does not fit the needs
        # of the code.
        with grpc.insecure_channel('localhost:50051') as channel:
            stub = did_pb2_grpc.DidStub(channel)
            response = stub.Sign(did_pb2.ApiRequest(api_key=api_key, private_key=private_key, message=message))

        if(response.status==200):
            print("Message: ",response.status)
            print("Public Key: ",response.pub_key)
            print("Signature: ",response.sig)
