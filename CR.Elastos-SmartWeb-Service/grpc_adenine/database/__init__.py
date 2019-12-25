from grpc_adenine import settings
from sqlalchemy_wrapper import SQLAlchemy
from decouple import config
from sqlalchemy import create_engine

# Connect to the database
DATABASE_URI = config('SQLALCHEMY_DATABASE_URI')
connection = SQLAlchemy(DATABASE_URI)
db_engine = create_engine(DATABASE_URI)