import pytest
import sys
from decouple import config
import json
import argparse

from elastos_adenine.stubs import health_check_pb2
from elastos_adenine.health_check import HealthCheck
from elastos_adenine.node_rpc import NodeRpc
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

def test_node_rpc_curr_height():
	node_rpc = NodeRpc(host, port, production)
	current_height = node_rpc.get_current_height(api_key_to_use, did_to_use, network, "mainchain")
	assert current_height!=None, "In Current Height-> "+response['status_message']

	current_height = node_rpc.get_current_height(api_key_to_use, did_to_use, network, "did")
	assert current_height!=None, "In Current Height-> "+response['status_message']

	current_height = node_rpc.get_current_height(api_key_to_use, did_to_use, network, "token")
	assert current_height!=None, "In Current Height-> "+response['status_message']

	current_height = node_rpc.get_current_height(api_key_to_use, did_to_use, network, "eth")
	assert current_height!=None, "In Current Height-> "+response['status_message']

def test_node_rpc_curr_balance():
	node_rpc = NodeRpc(host, port, production)
	current_balance = node_rpc.get_current_balance(api_key_to_use, did_to_use, network, "mainchain", ela_to_use)
	assert current_balance!=None, "In Current Balance-> "+response['status_message']

	current_balance = node_rpc.get_current_balance(api_key_to_use, did_to_use, network, "did", ela_to_use)
	assert current_balance!=None, "In Current Balance-> "+response['status_message']

	current_balance = node_rpc.get_current_balance(api_key_to_use, did_to_use, network, "token", ela_to_use)
	assert current_balance!=None, "In Current Balance-> "+response['status_message']

	current_balance = node_rpc.get_current_balance(api_key_to_use, did_to_use, network, "eth", ela_to_use)
	assert current_balance!=None, "In Current Balance-> "+response['status_message']

def test_node_rpc_curr_block_info():
	node_rpc = NodeRpc(host, port, production)
	current_block_info = node_rpc.get_current_block_info(api_key_to_use, did_to_use, network, "mainchain")
	assert current_block_info!=None, "In Current Block Info-> "+response['status_message']

	current_block_info = node_rpc.get_current_block_info(api_key_to_use, did_to_use, network, "did")
	assert current_block_info!=None, "In Current Block Info-> "+response['status_message']

	current_block_info = node_rpc.get_current_block_info(api_key_to_use, did_to_use, network, "token")
	assert current_block_info!=None, "In Current Block Info-> "+response['status_message']

	current_block_info = node_rpc.get_current_block_info(api_key_to_use, did_to_use, network, "eth")
	assert current_block_info!=None, "In Current Block Info-> "+response['status_message']

def test_node_rpc_curr_mining_info():
	node_rpc = NodeRpc(host, port, production)
	current_mining_info = node_rpc.get_current_mining_info(api_key_to_use, did_to_use, network)
	assert current_mining_info!=None, "In Current Mining Info-> "+response['status_message']

def test_node_rpc_curr_block_confirm():
	node_rpc = NodeRpc(host, port, production)
	current_block_confirm = node_rpc.get_current_block_confirm(api_key_to_use, did_to_use, network)
	assert current_block_confirm!=None, "In Current Block Confirm-> "+response['status_message']

def test_node_rpc_curr_arbitrator_info():
	node_rpc = NodeRpc(host, port, production)
	current_arbitrator_info = node_rpc.get_current_arbitrators_info(api_key_to_use, did_to_use, network)
	assert current_arbitrator_info!=None, "In current arbitrator info-> "+response['status_message']

def test_node_rpc_curr_arbitrator_group():
	node_rpc = NodeRpc(host, port, production)
	current_arbitrator_group = node_rpc.get_current_arbitrator_group(api_key_to_use, did_to_use, network)
	assert current_arbitrator_group!=None, "In current arbitrator group-> "+response['status_message']
