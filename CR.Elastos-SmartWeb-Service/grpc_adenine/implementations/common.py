import random
import string
import datetime

from decouple import config
from grpc_adenine.database import db_engine
from grpc_adenine.database.user import Users
from grpc_adenine.database.user_api_relation import UserApiRelations
from grpc_adenine.stubs import common_pb2
from grpc_adenine.stubs import common_pb2_grpc
from sqlalchemy.orm import sessionmaker
from grpc_adenine import settings
from grpc_adenine.implementations.utils import check_rate_limit
from grpc_adenine.implementations.rate_limiter import RateLimiter


class Common(common_pb2_grpc.CommonServicer):

    def __init__(self):
        session_maker = sessionmaker(bind=db_engine)
        self.session = session_maker()
        self.rate_limiter = RateLimiter()

    def GenerateAPIRequest(self, request, context):
        string_length = 64
        secret_key = config('SHARED_SECRET_ADENINE')
        check_did = request.did

        date_now = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S %z")

        # Generate the api key
        if secret_key == request.secret_key:
            api_key = ''.join(random.choice(string.ascii_letters + string.digits) for i in range(string_length))
            api_present = self.session.query(UserApiRelations).filter_by(api_key=api_key).first()

            # Generate a new api key if its already present
            if api_present:
                while api_present.api_key == api_key:
                    api_key = ''.join(random.choice(string.ascii_letters + string.digits) for i in range(string_length))

            result = self.session.query(Users).filter_by(did=check_did).first()

            # If did is already present replace the api key
            if result:
                insert = self.session.query(UserApiRelations).filter_by(user_id=result.id).first()
                # check rate limit
                response = check_rate_limit(self.rate_limiter, settings.GENERATE_API_LIMIT, insert.api_key, self.GenerateAPIRequest.__name__)
                if response:
                    return common_pb2.Response(api_key='', status_message='Number of daily access limit exceeded', status=False)

                # replace the new API Key
                insert.api_key = api_key
                self.session.commit()
                self.session.close()
            # Else insert the did and api key into respective tables
            else:
                # insert to Users table
                user = Users(
                    did=check_did,
                    created_on=date_now,
                    last_logged_on=date_now
                )
                self.session.add(user)
                self.session.commit()
                # insert to UserApiRelations table
                insert = self.session.query(Users).filter_by(did=check_did).first()
                user_api = UserApiRelations(
                    user_id=insert.id,
                    api_key=api_key
                )
                self.session.add(user_api)
                self.session.commit()
                self.session.close()
                # insert into SERVICES LISTS table
                rate_limiter.add_new_access_entry(api_key, self.GenerateAPIRequest.__name__)
            return common_pb2.Response(api_key=api_key, status_message='Success', status=True)
        else:
            return common_pb2.Response(api_key='', status_message='Authentication Error', status=False)

    def GetAPIKey(self, request, context):

        secret_key = config('SHARED_SECRET_ADENINE')

        if secret_key == request.secret_key:
            show_did = request.did
            result = self.session.query(Users).filter_by(did=show_did).first()
            
            # Get the api key for the requested did
            if result:
                get_api_key = self.session.query(UserApiRelations).filter_by(user_id=result.id).first()
                got_api_key = get_api_key.api_key
                self.session.close()
                return common_pb2.Response(api_key=got_api_key, status_message='Success', status=True)
            else:
                return common_pb2.Response(api_key='', status_message='No such API Key exists', status=False)
        else:
            return common_pb2.Response(api_key='', status_message='Authentication Error', status=False)
