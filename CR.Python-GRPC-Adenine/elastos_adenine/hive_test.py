import pytest
import sys
from decouple import config
import json
import argparse

from elastos_adenine.stubs import health_check_pb2
from elastos_adenine.health_check import HealthCheck
from elastos_adenine.hive import Hive
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

def test_upload_and_sign():
	hive = Hive(host, port, production)
	response = hive.upload_and_sign(api_key_to_use, did_to_use, network, private_key_to_use, 'test/sample.txt')
	if response['status']:
		json_output = json.loads(response['output'])
		for i in json_output['result']:
			assert json_output['result'][i]!=None, "In Upload and Sign-> "+response['status_message']
	assert response['status']==True, "In Upload and Sign-> "+response['status_message']

def test_verify_and_show():
	hive = Hive(host, port, production)
	request_input = {
		"msg": "516D5A445272666E76485648354E363277674C505171316D3575586D7459684A5A4475666D4679594B476F745535",
		"pub": "022316EB57646B0444CB97BE166FBE66454EB00631422E03893EE49143B4718AB8",
		"sig": "327C64F47047B71F1AA235CE465D5A80EB823648C7355E8A3EFBF3DE9AA25D443588E101EE0E693BE80A4C7D200CBB65ED838296EE3A8088401C342C0FBCD4E7",
		"hash": "QmZDRrfnvHVH5N62wgLPQq1m5uXmtYhJZDufmFyYKGotU5",
		"privateKey": private_key_to_use
	}
	response = hive.verify_and_show(api_key_to_use, did_to_use,  network, request_input)
	if response['status']:
		assert response['file_content']!=None, "In Verify and Show-> "+response['status_message']
	assert response['status']==True, "In Verify and Show-> "+response['status_message']
