import grpc
import jwt
import datetime
from decouple import config
from .stubs import node_rpc_pb2, node_rpc_pb2_grpc
from elastos_adenine.settings import REQUEST_TIMEOUT, TOKEN_EXPIRATION, GRPC_SERVER_CRT


class NodeRpc:

    def __init__(self, host, port, production):
        if not production:
            self._channel = grpc.insecure_channel('{}:{}'.format(host, port))
        else:
            with open(GRPC_SERVER_CRT, 'rb') as f:
                trusted_certs = f.read()
            # create credentials
            credentials = grpc.ssl_channel_credentials(root_certificates=trusted_certs)
            self._channel = grpc.secure_channel('{}:{}'.format(host, port), credentials)

        self.stub = node_rpc_pb2_grpc.NodeRpcStub(self._channel)

    def close(self):
        self._channel.close()

    # TODO: Common method for mainchain only
    def get_current_current_council(self, network):
        params = {"state": "all"}
        return self.rpc_method(network, "mainchain", "listcurrentcrs", params)

    # TODO: Common method for mainchain only
    def get_current_crc_candidates(self, network):
        params = {"start": 0, "state": "all"}
        return self.rpc_method(network, "mainchain", "listcrcandidates", params)

    # TODO: Common method for mainchain only
    def get_current_dpos_supernodes(self, network):
        params = {"start": 0, "state": "all"}
        return self.rpc_method(network, "mainchain", "listproducers", params)

    # Common method for mainchain only
    def get_current_arbitrator_group(self, network):
        params = {"height": str(self.get_current_height(network, "mainchain"))}
        return self.rpc_method(network, "mainchain", "getarbitratorgroupbyheight", params)

    # Common method for mainchain only
    def get_arbitrator_group(self, network, height):
        params = {"height": height}
        return self.rpc_method(network, "mainchain", "getarbitratorgroupbyheight", params)

    # Common method for mainchain only
    def get_current_arbitrators_info(self, network):
        return self.rpc_method(network, "mainchain", "getarbitersinfo", {})

    # Common method for mainchain only
    def get_current_block_confirm(self, network):
        params = {"height": self.get_current_height(network, "mainchain"), "verbosity": 1}
        return self.rpc_method(network, "mainchain", "getconfirmbyheight", params)

    # Common method for mainchain only
    def get_block_confirm(self, network, height):
        params = {"height": height, "verbosity": 1}
        return self.rpc_method(network, "mainchain", "getconfirmbyheight", params)

    # Common method for mainchain only
    def get_current_mining_info(self, network):
        return self.rpc_method(network, "mainchain", "getmininginfo", {})

    # Common method for mainchain, did sidechain and token sidechain
    def get_current_block_details(self, network, chain):
        params = {"height": str(self.get_current_height(network, chain))}
        return self.rpc_method(network, chain, "getblockbyheight", params)

    # Common method for mainchain, did sidechain and token sidechain
    def get_block_details(self, network, chain, height):
        params = {"height": height}
        return self.rpc_method(network, chain, "getblockbyheight", params)

    # Common method for mainchain, did sidechain and token sidechain
    def get_current_balance(self, network, chain, address):
        params = {"address": address}
        return self.rpc_method(network, chain, "getreceivedbyaddress", params)

    # Common method for mainchain, did sidechain and token sidechain
    def get_current_height(self, network, chain):
        node_state = self.get_current_node_state(network, chain)
        current_height = node_state["height"]
        return current_height

    # Common method for mainchain, did sidechain and token sidechain
    def get_current_node_state(self, network, chain):
        return self.rpc_method(network, chain, "getnodestate", {})

    def rpc_method(self, network, chain, method, params):
        secret_key = config('SHARED_SECRET_ADENINE')
        req_data = {
            'chain': chain,
            'method': method,
            'params': params
        }

        jwt_info = {
            'network': network,
            'request_input': req_data
        }

        jwt_token = jwt.encode({
            'jwt_info': jwt_info,
            'exp': datetime.datetime.utcnow() + datetime.timedelta(hours=TOKEN_EXPIRATION)
        }, secret_key, algorithm='HS256')

        response = self.stub.RpcMethod(node_rpc_pb2.Request(input=jwt_token), timeout=REQUEST_TIMEOUT)
        data = {}

        if response.status:
            output = jwt.decode(response.output, key=secret_key, algorithms=['HS256']).get('jwt_info')
            data = output['result']
        
        return data

