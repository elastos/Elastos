import grpc

from .stubs import common_pb2, common_pb2_grpc
from elastos_adenine.settings import REQUEST_TIMEOUT


class Common:

    def __init__(self, host, port, production):
        if not production:
            self._channel = grpc.insecure_channel('{}:{}'.format(host, port))
        else:
            credentials = grpc.ssl_channel_credentials()
            self._channel = grpc.secure_channel('{}:{}'.format(host, port), credentials)
        self.stub = common_pb2_grpc.CommonStub(self._channel)

    def close(self):
        self._channel.close()

    def generate_api_request_mnemonic(self, mnemonic):
        response = self.stub.GenerateAPIRequestMnemonic(common_pb2.RequestMnemonic(mnemonic=mnemonic), timeout=REQUEST_TIMEOUT)
        return response

    def generate_api_request(self, secret_key, did):
        response = self.stub.GenerateAPIRequest(common_pb2.Request(secret_key=secret_key, did=did), timeout=REQUEST_TIMEOUT)
        return response

    def get_api_request_mnemonic(self, mnemonic):
        response = self.stub.GetAPIKeyMnemonic(common_pb2.RequestMnemonic(mnemonic=mnemonic), timeout=REQUEST_TIMEOUT)
        return response

    def get_api_key_request(self, secret_key, did):
        response = self.stub.GetAPIKey(common_pb2.Request(secret_key=secret_key, did=did), timeout=REQUEST_TIMEOUT)
        return response
