import json
import sys
import unittest
import jwt
import logging
import grpc
import datetime

from concurrent import futures
from contextlib import contextmanager
from decouple import Config, RepositoryEnv
from grpc_adenine import settings

from ast import literal_eval
from grpc_adenine.implementations.utils import get_api_from_did
from grpc_adenine.implementations.node_rpc import NodeRpc
from grpc_adenine.stubs.python import node_rpc_pb2, node_rpc_pb2_grpc


@contextmanager
def mock_server(cls):
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
    node_rpc_pb2_grpc.add_NodeRpcServicer_to_server(cls(), server)
    port = server.add_insecure_port('[::]:0')
    server.start()

    try:
        with grpc.insecure_channel('localhost:%d' % port) as channel:
            yield node_rpc_pb2_grpc.NodeRpcStub(channel)
    finally:
        server.stop(None)


class NodeRpcTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        log = logging.getLogger("NodeRpcTest")
        log.debug("Setting up NodeRpcTest")

        DOTENV_FILE = '.env.test'
        env_config = Config(RepositoryEnv(DOTENV_FILE))

        cls.network = env_config('NETWORK')
        cls.ela_to_use = env_config('ELA_TO_USE')
        cls.ela_eth_to_use = env_config('ELA_ETH_TO_USE')
        cls.did_to_use = env_config('DID_TO_USE')
        cls.api_key_to_use = get_api_from_did(cls.did_to_use)
        cls.private_key_to_use = env_config('PRIVATE_KEY_TO_USE')

    @classmethod
    def tearDownClass(cls):
        log = logging.getLogger("NodeRpcTest")
        log.debug("Tearing down NodeRpcTest")

    def test_node_rpc_curr_height(self):
        log = logging.getLogger("NodeRpcTest")
        with mock_server(NodeRpc) as stub:
            response_data = rpc_method(self, stub, 'mainchain', "getnodestate", {})
            height = response_data["height"]
            log.debug(f"Method: test_node_rpc_curr_height for mainchain, Expected: Value, Actual: {height}")
            self.assertIsNotNone(height)

            response_data = rpc_method(self, stub, 'did', "getnodestate", {})
            height = response_data["height"]
            log.debug(f"Method: test_node_rpc_curr_height for did, Expected: Value, Actual: {height}")
            self.assertIsNotNone(height)

            response_data = rpc_method(self, stub, 'token', "getnodestate", {})
            height = response_data["height"]
            log.debug(f"Method: test_node_rpc_curr_height for token, Expected: Value, Actual: {height}")
            self.assertIsNotNone(height)

            current_height = literal_eval(rpc_method(self, stub, 'eth', "eth_blockNumber", {}))
            log.debug(f"Method: test_node_rpc_curr_height for eth, Expected: Value, Actual: {current_height}")
            self.assertIsNotNone(current_height)

    def test_node_rpc_curr_block_info(self):
        log = logging.getLogger("NodeRpcTest")
        with mock_server(NodeRpc) as stub:
            node_state = rpc_method(self, stub, 'mainchain', "getnodestate", {})
            height = node_state["height"]
            block_info = rpc_method(self, stub, 'mainchain', 'getblockbyheight', {"height": str(height)})
            #log.debug(f"Method: test_node_rpc_curr_block_info for mainchain, Expected: Value, Actual: {block_info}")
            self.assertIsNotNone(block_info)

            node_state = rpc_method(self, stub, 'did', "getnodestate", {})
            height = node_state["height"]
            block_info = rpc_method(self, stub, 'did', 'getblockbyheight', {"height": str(height)})
            #log.debug(f"Method: test_node_rpc_curr_block_info for did, Expected: Value, Actual: {block_info}")
            self.assertIsNotNone(block_info)

            node_state = rpc_method(self, stub, 'token', "getnodestate", {})
            height = node_state["height"]
            block_info = rpc_method(self, stub, 'token', 'getblockbyheight', {"height": str(height)})
            #log.debug(f"Method: test_node_rpc_curr_block_info for token, Expected: Value, Actual: {block_info}")
            self.assertIsNotNone(block_info)

            height = literal_eval(rpc_method(self, stub, 'eth', "eth_blockNumber", {}))
            block_info = rpc_method(self, stub, 'eth', 'eth_getBlockByNumber', [hex(height), True])
            #log.debug(f"Method: test_node_rpc_curr_block_info for eth, Expected: Value, Actual: {block_info}")
            self.assertIsNotNone(block_info)

    def test_node_rpc_current_mining_info(self):
        log = logging.getLogger("NodeRpcTest")
        with mock_server(NodeRpc) as stub:
            response_data = rpc_method(self, stub, 'mainchain', "getmininginfo", {})
            #log.debug(f"Method: test_node_rpc_current_mining_info for mainchain, Expected: Value, Actual: {response_data}")
            self.assertIsNotNone(response_data)

    def test_node_rpc_current_block_confirm(self):
        log = logging.getLogger("NodeRpcTest")
        with mock_server(NodeRpc) as stub:
            node_state = rpc_method(self, stub, 'mainchain', "getnodestate", {})
            height = node_state["height"]
            params = {"height": height, "verbosity": 1}
            response_data = rpc_method(self, stub, 'mainchain', "getconfirmbyheight", params)
            #log.debug(f"Method: test_node_rpc_current_block_confirm for mainchain, Expected: Value, Actual: {response_data}")
            self.assertIsNotNone(response_data)

    def test_node_rpc_current_arbitrators_info(self):
        log = logging.getLogger("NodeRpcTest")
        with mock_server(NodeRpc) as stub:
            response_data = rpc_method(self, stub, 'mainchain', "getarbitersinfo", {})
            #log.debug(f"Method: test_node_rpc_current_arbitrators_info for mainchain, Expected: Value, Actual: {response_data}")
            self.assertIsNotNone(response_data)

    def test_node_rpc_current_arbitrator_group(self):
        log = logging.getLogger("NodeRpcTest")
        with mock_server(NodeRpc) as stub:
            node_state = rpc_method(self, stub, 'mainchain', "getnodestate", {})
            height = node_state["height"]
            params = {"height": str(height)}
            response_data = rpc_method(self, stub, 'mainchain', "getarbitratorgroupbyheight", params)
            #log.debug(f"Method: test_node_rpc_curr_height for mainchain, Expected: Value, Actual: {response_data}")
            self.assertIsNotNone(response_data)

def rpc_method(cls, stub, chain, method, params):
    log = logging.getLogger("NodeRpcTest")
    jwt_info = {
        'network': cls.network,
        'chain': chain,
        'method': method,
        'params': params
    }

    jwt_token = jwt.encode({
        'jwt_info': jwt_info,
        'exp': datetime.datetime.utcnow() + datetime.timedelta(hours=settings.TOKEN_EXPIRATION)
    }, cls.api_key_to_use, algorithm='HS256')

    response = stub.RpcMethod(node_rpc_pb2.Request(input=jwt_token), timeout=settings.REQUEST_TIMEOUT, metadata=[('did', cls.did_to_use)])
    data = ''

    if response.status:
        output = jwt.decode(response.output, key=cls.api_key_to_use, algorithms=['HS256']).get('jwt_info')
        data = output["result"]

    return data

if __name__ == "__main__":
    logging.basicConfig(stream=sys.stderr)
    logging.getLogger("NodeRpcTest").setLevel(logging.DEBUG)
    unittest.main()
