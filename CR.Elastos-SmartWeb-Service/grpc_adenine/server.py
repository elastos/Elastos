from concurrent import futures
from decouple import config

import grpc
import logging

from grpc_adenine.stubs.python import health_check_pb2, health_check_pb2_grpc
from grpc_adenine.stubs.python import common_pb2_grpc
from grpc_adenine.stubs.python import node_rpc_pb2_grpc
from grpc_adenine.stubs.python import wallet_pb2_grpc
from grpc_adenine.stubs.python import hive_pb2_grpc
from grpc_adenine.stubs.python import sidechain_eth_pb2_grpc

from grpc_adenine.implementations.health_check import HealthServicer
from grpc_adenine.implementations.node_rpc import NodeRpc
from grpc_adenine.implementations.common import Common
from grpc_adenine.implementations.wallet import Wallet
from grpc_adenine.implementations.hive import Hive
from grpc_adenine.implementations.sidechain_eth import SidechainEth

from grpc_adenine.implementations.cron import Cron


def serve():
    # Set up logging
    logging.basicConfig(
        format='%(asctime)s %(levelname)-8s %(message)s',
        level=logging.DEBUG,
        datefmt='%Y-%m-%d %H:%M:%S'
    )

    logging.debug("Initializing the grpc server")

    # Initialize the server
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=10), maximum_concurrent_rpcs=10000)

    health_servicer = HealthServicer()
    health_servicer.set('', health_check_pb2.HealthCheckResponse.SERVING)
    health_check_pb2_grpc.add_HealthServicer_to_server(health_servicer, server)
    logging.debug("Added Health Check Service to server")

    secret_key = config('SHARED_SECRET_ADENINE')
    common_pb2_grpc.add_CommonServicer_to_server(Common(secret_key), server)
    logging.debug("Added Common Service to server")

    node_rpc_pb2_grpc.add_NodeRpcServicer_to_server(NodeRpc(), server)
    logging.debug("Added Node Rpc Service to server")

    hive_pb2_grpc.add_HiveServicer_to_server(Hive(), server)
    logging.debug("Added Hive Service to server")

    wallet_pb2_grpc.add_WalletServicer_to_server(Wallet(), server)
    logging.debug("Added Wallet Service to server")

    sidechain_eth_pb2_grpc.add_SidechainEthServicer_to_server(SidechainEth(), server)
    logging.debug("Added Eth Sidechain Service to server")

    port = config('GRPC_SERVER_PORT')
    server.add_insecure_port('[::]:{}'.format(port))
    server.start()

    # Initialize cron job
    cron = Cron()
    cron.cronjob_send_ela()
    cron.cronjob_send_ela_ethsc()

    server.wait_for_termination()


if __name__ == '__main__':
    serve()
