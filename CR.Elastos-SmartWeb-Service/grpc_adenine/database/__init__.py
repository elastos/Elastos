from sqlalchemy import create_engine
from sqlalchemy_wrapper import SQLAlchemy
from decouple import config

# Connect to the database
db_name = config('DB_NAME')
db_user = config('DB_USER')
db_password = config('DB_PASSWORD')
db_host = config('DB_HOST')
db_port = config('DB_PORT')
database_uri = f"postgresql://{db_user}:{db_password}@{db_host}:{db_port}/{db_name}"

db_engine = create_engine(database_uri)
connection = SQLAlchemy(database_uri)
