import logging
import requests
import random
import string
import datetime
import json

from flask import request
from flask import Response
from flask_restplus import Resource
from master_api_service import settings
from master_api_service.api.restplus import api
from master_api_service.api.common.common_service import getTime

log = logging.getLogger(__name__)

ns = api.namespace('1/console', description='Implements console functionalities')

@ns.route('/upload')
class Upload(Resource):

	def get(self):
		"""
		Upload a file to hive.
		"""
		stringLength = 20
		api_key = ''.join(random.choice(string.ascii_letters + string.digits) for i in range(stringLength))
		data = {"API Key":api_key,"status":200, "timestamp":getTime(),"path":request.url}
		return Response(json.dumps(data), 
				status=200,
				mimetype='application/json'
			)


  
