import logging
import requests

from flask import request
from flask_restplus import Resource
from master_api_service.api.restplus import api

log = logging.getLogger(__name__)

ns = api.namespace('wallet', description='Has wallet services')


@ns.route('/createWallet')
class Wallet(Resource):

    @api.marshal_list_with(user)
    def get(self):
        """
        Returns list of user.
        """
        #api_url_base = "http://localhost:10012/api/v1/asset/balances/EQ4QhsYRwuBbNBXc8BPW972xA9ANByKt6U"
        #json_data = requests.get(api_url_base).json()
        #print(json_data)

        #users = User.query.all()
        #users.name = json_data
        #return users

    @api.response(201, 'User successfully created.')
    @api.expect(user)
    def user(self):
        """
        Creates a new user.
        """
        #data = request.json
        #create_user(data)
        #return None, 201
