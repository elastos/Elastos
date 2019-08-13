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

ns = api.namespace('1/hive', description='Implements hive storage services')

@ns.route('/add')
class Add(Resource):

	def get(self):
		"""
		Returns Hash key of the content added.
		"""
		api_url_base = 'http://10.192.113.16:9095/api/v0/file/add'
		headers = {'Content-type': 'application/json'}
		req_data = request.form
		print(req_data)
		myResponse = requests.post(api_url_base, data=json.dumps(req_data), headers=headers).json()
		return Response(json.dumps(myResponse), 
        	mimetype='application/json'
        	)


  
