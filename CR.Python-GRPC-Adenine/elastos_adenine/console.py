from __future__ import print_function
import logging

import json
import grpc
from decouple import config

from adenine.stubs import adenine_io_pb2
from adenine.stubs import adenine_io_pb2_grpc

class Console():
    def upload_and_sign(self, api_key, private_key, file):
        # NOTE(gRPC Python Team): .close() is possible on a channel and should be
        # used in circumstances in which the with statement does not fit the needs
        # of the code.
        host = config('GRPC_SERVER_HOST')
        port = config('GRPC_SERVER_PORT')

        production = config('PRODUCTION', default=False, cast=bool)
        if production == False:
            with grpc.insecure_channel('{}:{}'.format(host, port)) as channel:
                stub = adenine_io_pb2_grpc.AdenineIoStub(channel)
                request_file = open(file, "rb")
                file_contents = request_file.read().decode('utf-8')
                req_data = 	{
            				    'private_key': private_key,
            				    'file': file_contents
            			     }
                response = stub.UploadAndSign(adenine_io_pb2.Request(api_key=api_key, input=json.dumps(req_data)))
                return response
        else:
            credentials = grpc.ssl_channel_credentials()
            with grpc.secure_channel('{}:{}'.format(host, port), credentials) as channel:
                stub = adenine_io_pb2_grpc.AdenineIoStub(channel)
                request_file = open(file, "rb")
                file_contents = request_file.read().decode('utf-8')
                req_data =  {
                            "private_key": private_key,
                            "file": file_contents
                        }
                response = stub.UploadAndSign(adenine_io_pb2.Request(api_key=api_key, input=json.dumps(req_data)))
                return response

    def verify_and_show(self, api_key, request_input):
        # NOTE(gRPC Python Team): .close() is possible on a channel and should be
        # used in circumstances in which the with statement does not fit the needs
        # of the code.
        host = config('GRPC_SERVER_HOST')
        port = config('GRPC_SERVER_PORT')

        production = config('PRODUCTION', default=False, cast=bool)
        if production == False:
            with grpc.insecure_channel('{}:{}'.format(host, port)) as channel:
                stub = adenine_io_pb2_grpc.AdenineIoStub(channel)
                response = stub.VerifyAndShow(adenine_io_pb2.Request(api_key=api_key, input=json.dumps(request_input)))
                return response
        else:
            credentials = grpc.ssl_channel_credentials()
            with grpc.secure_channel('{}:{}'.format(host, port), credentials) as channel:
                stub = adenine_io_pb2_grpc.AdenineIoStub(channel)
                response = stub.VerifyAndShow(adenine_io_pb2.Request(api_key=api_key, input=json.dumps(request_input)))
                return response
