from concurrent import futures

import logging
import random
import string
import datetime
import json

import grpc

from grpc_adenine.implementations.common import Common
from grpc_adenine.stubs import common_pb2
from grpc_adenine.stubs import common_pb2_grpc

from grpc_adenine.implementations.did_sidechain import Did
from grpc_adenine.stubs import adenine_io_pb2
from grpc_adenine.stubs import adenine_io_pb2_grpc

def serve():
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
    
    common_pb2_grpc.add_CommonServicer_to_server(Common(), server)
    adenine_io_pb2_grpc.add_AdenineIoServicer_to_server(Did(), server)
    
    server.add_insecure_port('[::]:50051')
    server.start()
    server.wait_for_termination()

if __name__ == '__main__':
    logging.basicConfig()
    serve()