import random
import string
import os
import grpc
from decouple import config

from grpc_adenine.database import (connection as db)
from grpc_adenine.stubs import common_pb2
from grpc_adenine.stubs import common_pb2_grpc

class Common(common_pb2_grpc.CommonServicer):

	def GenerateAPIRequest(self, request, context):
		stringLength = 32
		secret_key = config('SHARED_SECRET_ADENINE')
		
		if(secret_key==request.secret_key):
			api_key = ''.join(random.choice(string.ascii_letters + string.digits) for i in range(stringLength))
			return common_pb2.Response(api_key=api_key, status_message='Success', status=True)
		else:
			return common_pb2.Response(api_key='', status_message='Authentication Error', status=False)


