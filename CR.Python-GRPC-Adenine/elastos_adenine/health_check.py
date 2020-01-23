from __future__ import print_function

import grpc
from decouple import config

from .stubs import health_check_pb2, health_check_pb2_grpc


class HealthCheck:

    def __init__(self):
        host = config('GRPC_SERVER_HOST')
        port = config('GRPC_SERVER_PORT')
        production = config('PRODUCTION', default=False, cast=bool)
        if not production:
            self._channel = grpc.insecure_channel('{}:{}'.format(host, port))
        else:
            credentials = grpc.ssl_channel_credentials()
            self._channel = grpc.secure_channel('{}:{}'.format(host, port), credentials)
        self.stub = health_check_pb2_grpc.HealthStub(self._channel)

    def close(self):
        self._channel.close()

    def check(self):
        request = health_check_pb2.HealthCheckRequest()
        response = self.stub.Check(request)
        return response
