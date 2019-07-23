import logging
import requests
import random
import string

from flask import request
from flask_restplus import Resource
from master_api_service import settings
from master_api_service.api.restplus import api

log = logging.getLogger(__name__)

ns = api.namespace('common', description='Implements common services')

@ns.route('/generateAPIKey')
class Generate(Resource):

    def get(self):
        """
        Returns API key of length 20.
        """
        return ''.join(random.choice(string.ascii_letters + string.digits) for _ in range(20))


  
