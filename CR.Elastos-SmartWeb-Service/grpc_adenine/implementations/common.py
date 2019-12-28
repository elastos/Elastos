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


class Common(common_pb2_grpc.CommonServicer):

    def __init__(self):
        session_maker = sessionmaker(bind=db_engine)
        self.session = session_maker()

    def GenerateAPIRequest(self, request, context):
        string_length = 64
        secret_key = config('SHARED_SECRET_ADENINE')
        check_did = request.did

        date_now = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S %z")

        if secret_key == request.secret_key:
            api_key = ''.join(random.choice(string.ascii_letters + string.digits) for i in range(string_length))
            api_present = self.session.query(UserApiRelations).filter_by(api_key=api_key).first()

            if api_present:
                while api_present.api_key == api_key:
                    api_key = ''.join(random.choice(string.ascii_letters + string.digits) for i in range(string_length))

            result = self.session.query(Users).filter_by(did=check_did).first()

            if result:
                insert = self.session.query(UserApiRelations).filter_by(user_id=result.id).first()
                insert.api_key = api_key
                self.session.commit()
                self.session.close()
            else:
                user = Users(
                    did=check_did,
                    created_on=date_now,
                    last_logged_on=date_now
                )
                self.session.add(user)
                self.session.commit()
                insert = self.session.query(Users).filter_by(did=check_did).first()
                user_api = UserApiRelations(
                    user_id=insert.id,
                    api_key=api_key
                )
                self.session.add(user_api)
                self.session.commit()
                self.session.close()
            return common_pb2.Response(api_key=api_key, status_message='Success', status=True)
        else:
            return common_pb2.Response(api_key='', status_message='Authentication Error', status=False)

    def GetAPIKey(self, request, context):

        secret_key = config('SHARED_SECRET_ADENINE')

        if secret_key == request.secret_key:
            show_did = request.did
            result = self.session.query(Users).filter_by(did=show_did).first()

            if result:
                get_api_key = self.session.query(UserApiRelations).filter_by(user_id=result.id).first()
                got_api_key = get_api_key.api_key
                self.session.close()
                return common_pb2.Response(api_key=got_api_key, status_message='Success', status=True)
            else:
                return common_pb2.Response(api_key='', status_message='No such API Key exists', status=False)
        else:
            return common_pb2.Response(api_key='', status_message='Authentication Error', status=False)
