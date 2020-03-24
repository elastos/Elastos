import grpc

from .settings import GRPC_SERVER_CRT
from .stubs import health_check_pb2, health_check_pb2_grpc


class HealthCheck:

    def __init__(self, host, port, production):
        if not production:
            self._channel = grpc.insecure_channel('{}:{}'.format(host, port))
        else:
            with open(GRPC_SERVER_CRT, 'rb') as f:
                trusted_certs = f.read()
            # create credentials
            credentials = grpc.ssl_channel_credentials(root_certificates=trusted_certs)
            self._channel = grpc.secure_channel('{}:{}'.format(host, port), credentials)

        self.stub = health_check_pb2_grpc.HealthStub(self._channel)

    def close(self):
        self._channel.close()

    def check(self):
        request = health_check_pb2.HealthCheckRequest()
        response = self.stub.Check(request)
        return response
