import pytest
import sys
from decouple import config
import json
import argparse

from elastos_adenine.stubs import health_check_pb2
from elastos_adenine.health_check import HealthCheck
from elastos_adenine.sidechain_eth import SidechainEth
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

def test_deploy_eth_contract():
	sidechain_eth = SidechainEth(host, port, production)
	response = sidechain_eth.deploy_eth_contract(api_key_to_use, did_to_use, network,
                                                         ela_eth_to_use,
                                                         '0x35a12175385b24b2f906d6027d440aac7bd31e1097311fa8e3cf21ceac7c4809',
                                                         2000000, 'test/HelloWorld.sol')
	if response['status']:
		json_output = json.loads(response['output'])
		for i in json_output['result']:
			assert json_output['result'][i]!=None, "In Deploy Eth Contract-> "+response['status_message']
	assert response['status']==True, "In Deploy Eth Contract-> "+response['status_message']

def test_watch_eth_contract():
	sidechain_eth = SidechainEth(host, port, production)
	response = sidechain_eth.watch_eth_contract(api_key_to_use, did_to_use, network,
                                                        '0xc0ba7D9CF73c0410FfC9FB5b768F5257906B13c1', 'HelloWorld',
                                                        'QmXYqHg8gRnDkDreZtXJgqkzmjujvrAr5n6KXexmfTGqHd')
	if response['status']:
		json_output = json.loads(response['output'])
		for i in json_output['result']:
			assert json_output['result'][i]!=None, "In Watch Eth Contract-> "+response['status_message']
	assert response['status']==True, "In Watch Eth Contract-> "+response['status_message']
