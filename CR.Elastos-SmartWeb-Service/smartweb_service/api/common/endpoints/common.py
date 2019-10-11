import logging
import requests
import random
import string
import datetime
import json

from flask import request
from flask import Response
from flask_restplus import Resource
from smartweb_service import settings
from smartweb_service.api.restplus import api
from smartweb_service.api.common.common_service import getTime

log = logging.getLogger(__name__)

ns = api.namespace('1/common', description='Implements common services')

@ns.route('/generateAPIKey')
class Generate(Resource):

	def get(self):
		"""
		Returns API key of length 20.
		"""
		stringLength = 32
		api_key = ''.join(random.choice(string.ascii_letters + string.digits) for i in range(stringLength))
		data = {"API Key":api_key,"status":200, "timestamp":getTime(),"path":request.url}
		return Response(json.dumps(data), 
				status=200,
				mimetype='application/json'
			)


  
