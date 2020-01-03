import logging

from sqlalchemy import create_engine
from sqlalchemy_wrapper import SQLAlchemy
from decouple import config

# Set up logging
logging.basicConfig(
    format='%(asctime)s %(levelname)-8s %(message)s',
    level=logging.DEBUG,
    datefmt='%Y-%m-%d %H:%M:%S'
)

# Connect to the database
db_name = config('DB_NAME')
db_user = config('DB_USER')
db_password = config('DB_PASSWORD')
db_host = config('DB_HOST')
db_port = config('DB_PORT')

database_uri = f"postgresql://{db_user}:{db_password}@{db_host}:{db_port}/{db_name}"
gce = config('GCE', default=False, cast=bool)
if gce:
    database_uri = f"postgres+pg8000://{db_user}:{db_password}@{db_name}?unix_sock=/cloudsql/{db_host}/.s.PGSQL.{db_port}"

try:
    db_engine = create_engine(database_uri)
    connection = SQLAlchemy(database_uri)
except Exception as e:
    logging.debug(f"Encountered error while connecting to postgresql instance: database_uri: {database_uri} Error: {e}")
