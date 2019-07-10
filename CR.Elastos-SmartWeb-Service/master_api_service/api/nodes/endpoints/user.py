import logging
import requests

from flask import request
from flask_restplus import Resource
from master_api_service.api.nodes.business import create_user
from master_api_service.api.nodes.serializers import user
from master_api_service.api.restplus import api
from master_api_service.database.models import User

log = logging.getLogger(__name__)

ns = api.namespace('testbed/user', description='List of users')


@ns.route('/')
class UserList(Resource):

   # @api.marshal_list_with(user)
    def get(self):
        """
        Returns list of user.
        """
        api_url_base = "http://localhost:10012/api/v1/asset/balances/EQ4QhsYRwuBbNBXc8BPW972xA9ANByKt6U"
        json_data = requests.get(api_url_base).json()
        print(json_data)

        users = User.query.all()
        #users.name = json_data
        return json_data

    @api.response(201, 'User successfully created.')
    #@api.expect(json_data)
    def user(self):
        """
        Creates a new user.
        """
        data = request.json
        create_user(data)
        return None, 201
