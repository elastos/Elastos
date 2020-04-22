import logging
import random
import string
import datetime
import jwt
from decouple import config
from grpc_adenine.database import db_engine
from grpc_adenine.database.user import Users
from grpc_adenine.database.user_api_relation import UserApiRelations
from grpc_adenine.stubs.python import common_pb2, common_pb2_grpc
from requests import Session
from sqlalchemy.orm import sessionmaker
from grpc_adenine import settings
from grpc_adenine.implementations.rate_limiter import RateLimiter


class Common(common_pb2_grpc.CommonServicer):

    def __init__(self, session=None, rate_limiter=None):
        Session = sessionmaker(bind=db_engine)
        self.session = session if session else Session()
        self.rate_limiter = rate_limiter if rate_limiter else RateLimiter(Session())

    def GenerateAPIRequest(self, request, context):
        metadata = dict(context.invocation_metadata())
        did = metadata["did"]
        secret_key = config('SHARED_SECRET_ADENINE')
        try:
            jwt.decode(request.input, key=secret_key, algorithms=['HS256']).get('jwt_info')
        except Exception as e:
            status_message = 'Authentication Error'
            logging.debug(f"GenerateAPIRequest : {secret_key} : {status_message} : {e}")
            return common_pb2.Response(output='', status_message=status_message, status=False)

        string_length = 64
        date_now = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S %z")
        secret_key = config('SHARED_SECRET_ADENINE')

        api_key = ''.join(random.choice(string.ascii_letters + string.digits) for i in range(string_length))
        api_present = self.session.query(UserApiRelations).filter_by(api_key=api_key).first()

        # Generate a new api key if its already present
        if api_present:
            while api_present.api_key == api_key:
                api_key = ''.join(random.choice(string.ascii_letters + string.digits) for i in range(string_length))

        result = self.session.query(Users).filter_by(did=did).first()

        # If did is already present replace the api key
        if result:
            insert = self.session.query(UserApiRelations).filter_by(user_id=result.id).first()
            # check rate limit
            response = self.rate_limiter.check_rate_limit(settings.GENERATE_API_LIMIT, insert.api_key,
                                                          self.GenerateAPIRequest.__name__)
            if response:
                return common_pb2.Response(output='', status_message='Number of daily access limit exceeded',
                                           status=False)

            # replace the new API Key
            insert.api_key = api_key
            self.session.commit()
        # Else insert the did and api key into respective tables
        else:
            # insert to Users table
            user = Users(
                did=did,
                created_on=date_now,
                last_logged_on=date_now
            )
            self.session.add(user)
            # insert to UserApiRelations table
            insert = self.session.query(Users).filter_by(did=did).first()
            user_api = UserApiRelations(
                user_id=insert.id,
                api_key=api_key
            )
            self.session.add(user_api)
            self.session.commit()
            # insert into SERVICES LISTS table
            self.rate_limiter.add_new_access_entry(api_key, self.GenerateAPIRequest.__name__)

        data = {
            'api_key': api_key
        }

        # generate jwt token
        jwt_info = {
            'result': data
        }

        jwt_token = jwt.encode({
            'jwt_info': jwt_info,
            'exp': datetime.datetime.utcnow() + datetime.timedelta(hours=settings.TOKEN_EXPIRATION)
        }, secret_key, algorithm='HS256')

        response = common_pb2.Response(output=jwt_token, status_message='Success', status=True)
        return response

    def GetAPIKey(self, request, context):
        metadata = dict(context.invocation_metadata())
        did = metadata["did"]
        secret_key = config('SHARED_SECRET_ADENINE')
        try:
            jwt.decode(request.input, key=secret_key, algorithms=['HS256']).get('jwt_info')
        except Exception as e:
            status_message = 'Authentication Error'
            logging.debug(f"GetAPIKey : {secret_key} : {status_message} : {e}")
            return common_pb2.Response(output='', status_message=status_message, status=False)

        secret_key = config('SHARED_SECRET_ADENINE')
        result = self.session.query(Users).filter_by(did=did).first()
        if result:
            api_present = self.session.query(UserApiRelations).filter_by(user_id=result.id).first()
            api_key = api_present.api_key

            data = {
                'api_key': api_key
            }

            # generate jwt token
            jwt_info = {
                'result': data
            }

            jwt_token = jwt.encode({
                'jwt_info': jwt_info,
                'exp': datetime.datetime.utcnow() + datetime.timedelta(hours=settings.TOKEN_EXPIRATION)
            }, secret_key, algorithm='HS256')

            response = common_pb2.Response(output=jwt_token, status_message='Success', status=True)
        else:
            response = common_pb2.Response(output='', status_message='No such API Key exists', status=False)
        return response
