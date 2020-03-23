import json
import logging
import datetime
import jwt
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

        secret_key = config('SHARED_SECRET_ADENINE')
        try:
            jwt_info = jwt.decode(request.input, key=secret_key, algorithms=['HS256']).get('jwt_info')
        except:
            return hive_pb2.Response(output='', status_message='Authentication Error', status=False)

        network = jwt_info['network']
        request_input = jwt_info['request_input']

        chain = request_input["chain"]
        method = request_input["method"]
        params = request_input["params"]

        d = {"method": method}
        if params:
            d["params"] = params
        if chain == "mainchain" and not (method in settings.NODE_COMMON_RPC_METHODS or method in settings.NODE_MAIN_RPC_METHODS):
            return node_rpc_pb2.Response(output=json.dumps({}),
                                         status_message=f'The method {method} is not available for the chain {chain}',
                                         status=False)

        if chain != "mainchain" and method not in settings.NODE_COMMON_RPC_METHODS:
            return node_rpc_pb2.Response(output=json.dumps({}),
                                         status_message=f'The method {method} is not available for the chain {chain}',
                                         status=False)

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

        #generate jwt token
        jwt_info = {
            'result': response
        }

        jwt_token = jwt.encode({
            'jwt_info': jwt_info,
            'exp': datetime.datetime.utcnow() + datetime.timedelta(hours=settings.TOKEN_EXPIRATION)
        }, secret_key, algorithm='HS256')

        return node_rpc_pb2.Response(output=jwt_token,
                                     status_message=f'Successfully called the method {method} for the chain {chain} in network {network}',
                                     status=True)
