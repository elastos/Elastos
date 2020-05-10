import grpc
import jwt
import datetime

from ast import literal_eval
from .stubs import node_rpc_pb2, node_rpc_pb2_grpc
from elastos_adenine.settings import REQUEST_TIMEOUT, TOKEN_EXPIRATION


class NodeRpc:

    def __init__(self, host, port, production):
        if not production:
            self._channel = grpc.insecure_channel('{}:{}'.format(host, port))
        else:
            credentials = grpc.ssl_channel_credentials()
            self._channel = grpc.secure_channel('{}:{}'.format(host, port), credentials)

        self.stub = node_rpc_pb2_grpc.NodeRpcStub(self._channel)

    def close(self):
        self._channel.close()

    # Common method for mainchain only
    def get_current_crc_council(self, api_key, did, network):
        params = {"state": "all"}
        return self.rpc_method(api_key, did, network, "mainchain", "listcurrentcrs", params)

    # Common method for mainchain only
    def get_current_crc_candidates(self, api_key, did, network):
        params = {"start": 0, "state": "all"}
        return self.rpc_method(api_key, did, network, "mainchain", "listcrcandidates", params)

    # Common method for mainchain only
    def get_current_dpos_supernodes(self, api_key, did, network):
        params = {"start": 0, "state": "all"}
        return self.rpc_method(api_key, did, network, "mainchain", "listproducers", params)

    # Common method for mainchain only
    def get_current_arbitrator_group(self, api_key, did, network):
        params = {"height": str(self.get_current_height(api_key, did, network, "mainchain"))}
        return self.rpc_method(api_key, did, network, "mainchain", "getarbitratorgroupbyheight", params)

    # Common method for mainchain only
    def get_arbitrator_group(self, api_key, did, network, height):
        params = {"height": height}
        return self.rpc_method(api_key, did, network, "mainchain", "getarbitratorgroupbyheight", params)

    # Common method for mainchain only
    def get_current_arbitrators_info(self, api_key, did, network):
        return self.rpc_method(api_key, did, network, "mainchain", "getarbitersinfo", {})

    # Common method for mainchain only
    def get_current_block_confirm(self, api_key, did, network):
        height = self.get_current_height(api_key, did, network, "mainchain")
        return self.get_block_confirm(api_key, did, network, height)

    # Common method for mainchain only
    def get_block_confirm(self, api_key, did, network, height):
        params = {"height": height, "verbosity": 1}
        return self.rpc_method(api_key, did, network, "mainchain", "getconfirmbyheight", params)

    # Common method for mainchain only
    def get_current_mining_info(self, api_key, did, network):
        return self.rpc_method(api_key, did, network, "mainchain", "getmininginfo", {})

    # Common method for mainchain, did sidechain, token and eth sidechain
    def get_current_block_info(self, api_key, did, network, chain):
        height = self.get_current_height(api_key, did, network, chain)
        return self.get_block_info(api_key, did, network, chain, height)

    # Common method for mainchain, did sidechain, token and eth sidechain
    def get_block_info(self, api_key, did, network, chain, height):
        if chain == "eth":
            method = "eth_getBlockByNumber"
            params = [hex(height), True]
        else:
            method = "getblockbyheight"
            params = {"height": str(height)}
        return self.rpc_method(api_key, did, network, chain, method, params)

    # Common method for mainchain, did sidechain, token and eth sidechain
    def get_current_balance(self, api_key, did, network, chain, address):
        if chain == "eth":
            method = "eth_getBalance"
            params = [address, "latest"]
            current_balance = self.rpc_method(api_key, did, network, chain, method, params)
            current_balance = literal_eval(current_balance) / 1000000000000000000
        else:
            method = "getreceivedbyaddress"
            params = {"address": address}
            current_balance = self.rpc_method(api_key, did, network, chain, method, params)
        return current_balance

    # Common method for mainchain, did sidechain, token and eth sidechain
    def get_current_height(self, api_key, did, network, chain):
        if chain == "eth":
            current_height = literal_eval(self.rpc_method(api_key, did, network, chain, "eth_blockNumber", {}))
        else:
            node_state = self.get_current_node_state(api_key, did, network, chain)
            current_height = node_state["height"]
        return current_height

    # Common method for mainchain, did sidechain and token sidechain
    def get_current_node_state(self, api_key, did, network, chain):
        return self.rpc_method(api_key, did, network, chain, "getnodestate", {})

    def rpc_method(self, api_key, did, network, chain, method, params):
        jwt_info = {
            'network': network,
            'chain': chain,
            'method': method,
            'params': params
        }

        jwt_token = jwt.encode({
            'jwt_info': jwt_info,
            'exp': datetime.datetime.utcnow() + datetime.timedelta(hours=TOKEN_EXPIRATION)
        }, api_key, algorithm='HS256')

        response = self.stub.RpcMethod(node_rpc_pb2.Request(input=jwt_token), timeout=REQUEST_TIMEOUT, metadata=[('did', did)])
        data = ''

        if response.status:
            output = jwt.decode(response.output, key=api_key, algorithms=['HS256']).get('jwt_info')
            data = output['result']

        return data

