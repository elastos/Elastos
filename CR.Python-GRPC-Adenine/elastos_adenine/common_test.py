import pytest
import sys
from decouple import config
import json
import argparse

from pytest_dependency import depends
from elastos_adenine.stubs import health_check_pb2
from elastos_adenine.health_check import HealthCheck
from elastos_adenine.common import Common

host = config('GRPC_SERVER_HOST')
port = config('GRPC_SERVER_PORT')
production = config('PRODUCTION', default=False, cast=bool)

network = "gmunet"
ela_to_use = 'EQeMkfRk3JzePY7zpUSg5ZSvNsWedzqWXN'
ela_eth_to_use = '0x48F01b2f2b1a546927ee99dD03dCa37ff19cB84e'
did_to_use = 'n84dqvIK9O0LIPXi27uL0aRnoR45Exdxl218eQyPDD4lW8RPov'
api_key_to_use = ''
private_key_to_use = '1F54BCD5592709B695E85F83EBDA515971723AFF56B32E175F14A158D5AC0D99'

@pytest.mark.dependency()
def test_health_check():
	# Health Check
	try:
		health_check = HealthCheck(host, port, production)
		response = health_check.check()
		assert response.status == health_check_pb2.HealthCheckResponse.SERVING, "grpc server is not running properly"
	except Exception as e:
		health_check_status = False
		assert health_check_status == True, "grpc server is not running properly"

@pytest.mark.dependency(depends=["test_health_check"])
def test_generate_api_key():
	# Generate API Key
	global api_key_to_use
	common = Common(host, port, production)
	response = common.generate_api_request(config('SHARED_SECRET_ADENINE'), did_to_use)
	if response['status']:
		json_output = json.loads(response['output'])
		for i in json_output['result']:
			if(i=='api_key'):
				api_key_to_use = json_output['result'][i]
	assert response['status']==True, "In Generate Api Key-> "+response['status_message']
	assert len(api_key_to_use) == 64,"Testing API Key length Failed"

@pytest.mark.dependency(depends=["test_health_check"])
def test_get_api_key():
	# Get API Key
	global api_key_to_use
	common = Common(host, port, production)
	response = common.get_api_key_request(config('SHARED_SECRET_ADENINE'), did_to_use)
	if response['status']:
		json_output = json.loads(response['output'])
		for i in json_output['result']:
			if(i=='api_key'):
				api_key_to_use = json_output['result'][i]
	assert response['status']==True, "In Get Api Key-> "+response['status_message']
	assert len(api_key_to_use) == 64,"Testing API Key length Failed"

