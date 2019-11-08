from __future__ import print_function
import logging

import grpc

from adenine.stubs import common_pb2
from adenine.stubs import common_pb2_grpc
from adenine.stubs import did_pb2
from adenine.stubs import did_pb2_grpc

def run():
    # NOTE(gRPC Python Team): .close() is possible on a channel and should be
    # used in circumstances in which the with statement does not fit the needs
    # of the code.
    with grpc.insecure_channel('localhost:50051') as channel:
        stub = common_pb2_grpc.CommonStub(channel)
        response = stub.GenerateAPIRequest(common_pb2.ApiRequest(name='hey'))

        stub = did_pb2_grpc.DidStub(channel)
        response1 = stub.Sign(did_pb2.ApiRequest(api_key='shguBwHIVO2ziKMQi3EkHDP9V4JJb5GN', private_key='shguBwHIVO2ziKMQi3EkHDP9V4JJb5GN', message='hey'))
    
    print("SmartWeb client received: "+response.message)
    print("SmartWeb client received: ",response1.status)


if __name__ == '__main__':
    logging.basicConfig()
    run()