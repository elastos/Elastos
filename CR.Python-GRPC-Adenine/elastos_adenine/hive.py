import json
import grpc
from decouple import config

from .stubs import hive_pb2
from .stubs import hive_pb2_grpc
from elastos_adenine.settings import REQUEST_TIMEOUT


class Hive:

    def __init__(self):
        host = config('GRPC_SERVER_HOST')
        port = config('GRPC_SERVER_PORT')
        production = config('PRODUCTION', default=False, cast=bool)
        if not production:
            self._channel = grpc.insecure_channel('{}:{}'.format(host, port))
        else:
            credentials = grpc.ssl_channel_credentials()
            self._channel = grpc.secure_channel('{}:{}'.format(host, port), credentials)
        self.stub = hive_pb2_grpc.HiveStub(self._channel)

    def close(self):
        self._channel.close()

    def sign(self, api_key, private_key, message):
        req_data = {
            "privateKey": private_key,
            "msg": message
        }
        response = self.stub.Sign(hive_pb2.Request(api_key=api_key, input=json.dumps(req_data)), timeout=REQUEST_TIMEOUT)
        return response

    def upload_and_sign(self, api_key, private_key, filename):
        with open(filename, 'rb') as myfile:
            file_contents = myfile.read().decode('utf-8')
        req_data = {
            'private_key': private_key,
            'file': file_contents
        }
        response = self.stub.UploadAndSign(hive_pb2.Request(api_key=api_key, input=json.dumps(req_data)))
        return response

    def verify_and_show(self, api_key, request_input):
        response = self.stub.VerifyAndShow(hive_pb2.Request(api_key=api_key, input=json.dumps(request_input)), timeout=REQUEST_TIMEOUT)
        return response
