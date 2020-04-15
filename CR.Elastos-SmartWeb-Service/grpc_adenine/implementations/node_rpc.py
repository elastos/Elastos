import json
import logging
import datetime
import jwt
from requests import Session
from decouple import config
from sqlalchemy.orm import sessionmaker

from grpc_adenine import settings
from grpc_adenine.database import db_engine
from grpc_adenine.implementations.rate_limiter import RateLimiter
from grpc_adenine.implementations.utils import get_api_from_did, validate_api_key
from grpc_adenine.settings import REQUEST_TIMEOUT
from grpc_adenine.stubs.python import node_rpc_pb2, node_rpc_pb2_grpc


class NodeRpc(node_rpc_pb2_grpc.NodeRpcServicer):

    def __init__(self):
        headers = {
            'Accepts': 'application/json',
            'Content-Type': 'application/json'
        }
        self.session = Session()
        self.session.headers.update(headers)
        session_maker = sessionmaker(bind=db_engine)
        self.rate_limiter = RateLimiter(session_maker())

    def RpcMethod(self, request, context):

        metadata = dict(context.invocation_metadata())
        did = metadata["did"]
        api_key = get_api_from_did(did)

        try:
            jwt_info = jwt.decode(request.input, key=api_key, algorithms=['HS256']).get('jwt_info')
        except:
            status_message = 'Authentication Error'
            logging.debug(f"{did} : {api_key} : {status_message}")
            return node_rpc_pb2.Response(output='', status_message=status_message, status=False)

        if type(jwt_info) == str:
            jwt_info = json.loads(jwt_info)

        # Validate the API Key
        api_status = validate_api_key(api_key)
        if not api_status:
            status_message = 'API Key could not be verified'
            logging.debug(f"{did} : {api_key} : {status_message}")
            return node_rpc_pb2.Response(output='', status_message=status_message,
                                         status=False)

        # Check whether the user is able to use this API by checking their rate limiter
        response = self.rate_limiter.check_rate_limit(settings.NODE_RPC_LIMIT, api_key,
                                                      self.RpcMethod.__name__)
        if response:
            status_message = f'Number of daily access limit exceeded {response["result"]["daily_limit"]}'
            logging.debug(f"{did} : {api_key} : {status_message}")
            return node_rpc_pb2.Response(output=json.dumps(response),
                                         status_message=status_message,
                                         status=False)

        network = jwt_info['network']
        chain = jwt_info["chain"]
        method = jwt_info["method"]
        params = jwt_info["params"]

        d = {"method": method}
        if params:
            d["params"] = params
        if chain == "mainchain" and not (
                method in settings.NODE_COMMON_RPC_METHODS or method in settings.NODE_MAIN_RPC_METHODS):
            status_message = f'The method {method} is not available for the chain {chain}'
            logging.debug(f"{api_key} : {status_message}")
            return node_rpc_pb2.Response(output=json.dumps({}),
                                         status_message=status_message,
                                         status=False)

        if chain != "mainchain" and chain != "eth" and method not in settings.NODE_COMMON_RPC_METHODS:
            status_message = f'The method {method} is not available for the chain {chain}'
            logging.debug(f"{api_key} : {status_message}")
            return node_rpc_pb2.Response(output=json.dumps({}),
                                         status_message=status_message,
                                         status=False)

        if chain == "eth" and method not in settings.NODE_SIDECHAIN_ETH_RPC_METHODS:
            status_message = f'The method {method} is not available for the chain {chain}'
            logging.debug(f"{api_key} : {status_message}")
            return node_rpc_pb2.Response(output=json.dumps({}),
                                         status_message=status_message,
                                         status=False)

        if network == "mainnet":
            if chain == "mainchain":
                url = config('MAIN_NET_MAINCHAIN_RPC_PORT')
            elif chain == "did":
                url = config('MAIN_NET_SIDECHAIN_DID_RPC_PORT')
            elif chain == "token":
                url = config('MAIN_NET_SIDECHAIN_TOKEN_RPC_PORT')
            elif chain == "eth":
                url = config('MAIN_NET_SIDECHAIN_ETH_RPC_PORT')
                d["id"] = 1
            else:
                status_message = f'The chain {chain} is not supported for the network {network}'
                logging.debug(f"{api_key} : {status_message}")
                return node_rpc_pb2.Response(output=json.dumps({}),
                                             status_message=status_message,
                                             status=False)
        else:
            if chain == "mainchain":
                url = config('PRIVATE_NET_MAINCHAIN_RPC_PORT')
            elif chain == "did":
                url = config('PRIVATE_NET_SIDECHAIN_DID_RPC_PORT')
            elif chain == "token":
                url = config('PRIVATE_NET_SIDECHAIN_TOKEN_RPC_PORT')
            elif chain == "eth":
                url = config('PRIVATE_NET_SIDECHAIN_ETH_RPC_PORT')
                d["id"] = 1
            else:
                status_message = f'The chain {chain} is not supported for the network {network}'
                logging.debug(f"{api_key} : {status_message}")
                return node_rpc_pb2.Response(output=json.dumps({}),
                                             status_message=status_message,
                                             status=False)

        response = self.session.post(url, data=json.dumps(d), timeout=REQUEST_TIMEOUT)
        data = json.loads(response.text)

        # generate jwt token
        jwt_info = {
            'result': data['result']
        }

        jwt_token = jwt.encode({
            'jwt_info': jwt_info,
            'exp': datetime.datetime.utcnow() + datetime.timedelta(hours=settings.TOKEN_EXPIRATION)
        }, api_key, algorithm='HS256')

        return node_rpc_pb2.Response(output=jwt_token,
                                     status_message=f'Successfully called the method {method} for the chain {chain} in network {network}',
                                     status=True)
