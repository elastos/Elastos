import json
import grpc

from .stubs import node_rpc_pb2, node_rpc_pb2_grpc
from elastos_adenine.settings import REQUEST_TIMEOUT


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

    def get_current_height(self, network, chain):
        current_height = None
        req_data = {
            'chain': chain,
            'method': "getnodestate",
            'params': {}
        }
        response = self.stub.RpcMethod(node_rpc_pb2.Request(network=network, input=json.dumps(req_data)), timeout=REQUEST_TIMEOUT)
        if response.status:
            data = json.loads(response.output)["result"]
            current_height = data["height"]
        return current_height
