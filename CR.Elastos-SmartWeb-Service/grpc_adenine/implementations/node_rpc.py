import json
import logging
from requests import Session
from decouple import config
from sqlalchemy.orm import sessionmaker

from grpc_adenine import settings
from grpc_adenine.database import db_engine
from grpc_adenine.settings import REQUEST_TIMEOUT
from grpc_adenine.implementations.rate_limiter import RateLimiter
from grpc_adenine.implementations.utils import validate_api_key, get_did_from_api
from grpc_adenine.stubs.python import node_rpc_pb2, node_rpc_pb2_grpc


class NodeRpc(node_rpc_pb2_grpc.NodeRpcServicer):

    def __init__(self):
        headers = {
            'Accepts': 'application/json',
            'Content-Type': 'application/json'
        }
        self.session = Session()
        self.session.headers.update(headers)

    def RpcMethod(self, request, context):

        network = request.network
        request_input = json.loads(request.input)

        chain = request_input["chain"]
        method = request_input["method"]
        params = request_input["params"]

        if chain == "mainchain" and not (method in settings.NODE_COMMON_RPC_METHODS or method in settings.NODE_MAIN_RPC_METHODS):
            return node_rpc_pb2.Response(output=json.dumps({}),
                                         status_message=f'The method {method} is not available for the chain {chain}',
                                         status=False)

        if method != "mainchain" and method not in settings.NODE_COMMON_RPC_METHODS:
            return node_rpc_pb2.Response(output=json.dumps({}),
                                         status_message=f'The method {method} is not available for the chain {chain}',
                                         status=False)

        d = {"method": method}
        if params:
            d["params"] = params
        if network == "testnet":
            if chain == "mainchain":
                url = config('TEST_NET_MAINCHAIN_RPC_PORT')
            elif chain == "did":
                url = config('TEST_NET_SIDECHAIN_DID_RPC_PORT')
            elif chain == "token":
                url = config('TEST_NET_SIDECHAIN_TOKEN_RPC_PORT')
        else:
            if chain == "mainchain":
                url = config('PRIVATE_NET_MAINCHAIN_RPC_PORT')
            elif chain == "did":
                url = config('PRIVATE_NET_SIDECHAIN_DID_RPC_PORT')
            elif chain == "token":
                url = config('PRIVATE_NET_SIDECHAIN_TOKEN_RPC_PORT')

        response = self.session.post(url, data=json.dumps(d), timeout=REQUEST_TIMEOUT)
        data = json.loads(response.text)
        response = {
            'result': data['result']
        }

        return node_rpc_pb2.Response(output=json.dumps(response),
                                     status_message=f'Successfully called the method {method} for the chain {chain} in network {network}',
                                     status=True)
