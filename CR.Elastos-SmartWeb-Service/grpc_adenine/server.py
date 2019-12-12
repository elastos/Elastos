from concurrent import futures

import logging
import grpc

from grpc_adenine.stubs import common_pb2_grpc
from grpc_adenine.stubs import wallet_pb2_grpc
from grpc_adenine.stubs import hive_pb2_grpc
from grpc_adenine.stubs import sidechain_eth_pb2_grpc

from grpc_adenine.implementations.common import Common
from grpc_adenine.implementations.wallet import Wallet
from grpc_adenine.implementations.hive import Hive
from grpc_adenine.implementations.sidechain_eth import SidechainEth


def serve():
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
    
    common_pb2_grpc.add_CommonServicer_to_server(Common(), server)
    wallet_pb2_grpc.add_WalletServicer_to_server(Wallet(), server)
    hive_pb2_grpc.add_HiveServicer_to_server(Hive(), server)
    sidechain_eth_pb2_grpc.add_SidechainEthServicer_to_server(SidechainEth(), server)
    
    server.add_insecure_port('[::]:50051')
    server.start()
    server.wait_for_termination()


if __name__ == '__main__':
    logging.basicConfig()
    serve()
