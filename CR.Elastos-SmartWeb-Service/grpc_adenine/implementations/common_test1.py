import pytest
import sys
from decouple import config
import grpc
from concurrent import futures
from contextlib import contextmanager
import unittest

from grpc_adenine.stubs.python import common_pb2, common_pb2_grpc
from grpc_adenine.implementations.common import get_api_key, generate_api_key
from grpc_adenine.implementations.rate_limiter import RateLimiter

import codecs
from codecs import open
import psycopg2
from sqlalchemy import create_engine
from sqlalchemy.orm import sessionmaker


secret_key = config('SHARED_SECRET_ADENINE')

network = "gmunet"
mnemonic_to_use = 'obtain pill nest sample caution stone candy habit silk husband give net'
did_to_use = 'n84dqvIK9O0LIPXi27uL0aRnoR45Exdxl218eQyPDD4lW8RPov'
api_key_to_use = ''
private_key_to_use = '1F54BCD5592709B695E85F83EBDA515971723AFF56B32E175F14A158D5AC0D99'

# Connect to the database
db_name = config('TEST_DB_NAME')
db_user = config('DB_USER')
db_password = config('DB_PASSWORD')
db_host = config('DB_HOST')
db_port = config('TEST_DB_PORT')

database_uri = f"postgresql://{db_user}:{db_password}@{db_host}:{db_port}/{db_name}"
create_scripts = "grpc_adenine/database/scripts/create_table_scripts.sql"
reset_scripts = "grpc_adenine/database/scripts/reset_database.sql"


def create_database_tables():
	# Connect to the PostgreSQL database and create tables
	postgres_connection = psycopg2.connect(database_uri)
	cursor = postgres_connection.cursor()

	sql = open(create_scripts, mode='r', encoding='utf-8-sig').read()
	cursor.execute(sql)
	postgres_connection.commit()
	cursor.close()
	postgres_connection.close()


def reset_database_tables():
	# reset database tables
	postgres_connection = psycopg2.connect(database_uri)
	cursor = postgres_connection.cursor()

	sql = open(reset_scripts, mode='r', encoding='utf-8-sig').read()
	cursor.execute(sql)
	postgres_connection.commit()
	cursor.close()
	postgres_connection.close()


class RunTest(unittest.TestCase):
	def setUp(self):
		create_database_tables()
		db_engine = create_engine(database_uri)
		session_maker = sessionmaker(bind=db_engine)
		self.session = session_maker()
		self.rate_limiter = RateLimiter(self.session)

	# testing Generate API Key with valid DID
	def test_generate_api_key(self):
		response = generate_api_key(self.session, self.rate_limiter, did_to_use)
		self.assertEqual(response.status, True)

	# testing Get API Key with valid DID
	def test_get_api_key(self):
		generate_api_key(self.session, self.rate_limiter, did_to_use)
		response = get_api_key(self.session, did_to_use)
		self.assertEqual(response.status, True)

	def tearDown(self):
		self.session.close()
		reset_database_tables()

		


	

