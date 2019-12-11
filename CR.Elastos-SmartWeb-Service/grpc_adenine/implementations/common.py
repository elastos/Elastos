import random
import string
import datetime
from decouple import config
from sqlalchemy import create_engine
from grpc_adenine.database.user import Users
from grpc_adenine.stubs import common_pb2
from grpc_adenine.stubs import common_pb2_grpc
from sqlalchemy.orm import sessionmaker


class Common(common_pb2_grpc.CommonServicer):

    def GenerateAPIRequest(self, request, context):
        stringLength = 32
        secret_key = config('SHARED_SECRET_ADENINE')
        check_did = request.did

        DATABASE_URI = config('SQLALCHEMY_DATABASE_URI')
        engine = create_engine(DATABASE_URI)
        Session = sessionmaker(bind=engine)
        date_now = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S %z")

        user = Users(
            did=request.did,
            name='Elastos User',
            email='elastos_user@elastos.org',
            created_on=date_now,
            last_logged_on=date_now
        )

        s = Session()
        s.add(user)
        s.commit()
        # print("ID value ",s.id)
        s.close()

        if secret_key == request.secret_key:
            api_key = ''.join(random.choice(string.ascii_letters + string.digits) for i in range(stringLength))
            return common_pb2.Response(api_key=api_key, status_message='Success', status=True)
        else:
            return common_pb2.Response(api_key='', status_message='Authentication Error', status=False)
