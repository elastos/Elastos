from flask_restplus import fields
from master_api_service.api.restplus import api

user = api.model('User', {
    'id': fields.Integer(readOnly=True, description='User ID'),
    'name': fields.String(required=True, description='User name'),
})

