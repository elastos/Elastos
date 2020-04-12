import pytest
import sys
from decouple import config
import json
import argparse

from elastos_adenine.stubs import health_check_pb2
from elastos_adenine.health_check import HealthCheck
from elastos_adenine.wallet import Wallet
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

def test_create_wallet():
	wallet = Wallet(host, port, production)
	response = wallet.create_wallet(api_key_to_use, did_to_use, network)
	if response['status']:
		json_output = json.loads(response['output'])
		for i in json_output['result']:
			assert json_output['result'][i]!=None, "In Create Wallet-> "+response['status_message']
	assert response['status']==True, "In Create Wallet-> "+response['status_message']

def test_view_mainchain_wallet():
	wallet = Wallet(host, port, production)
	response = wallet.view_wallet(api_key_to_use, did_to_use, network, 'mainchain', ela_to_use)
	if response['status']:
		json_output = json.loads(response['output'])
		for i in json_output['result']:
			assert json_output['result'][i]!=None, "In View Mainchain Wallet-> "+response['status_message']
	assert response['status']==True, "In View Mainchain Wallet-> "+response['status_message']

def test_view_did_wallet():
	wallet = Wallet(host, port, production)
	response = wallet.view_wallet(api_key_to_use, did_to_use, network, 'did', ela_to_use)
	if response['status']:
		json_output = json.loads(response['output'])
		for i in json_output['result']:
			assert json_output['result'][i]!=None, "In View Did Wallet-> "+response['status_message']
	assert response['status']==True, "In View Did Wallet-> "+response['status_message']

def test_view_token_wallet():
	wallet = Wallet(host, port, production)
	response = wallet.view_wallet(api_key_to_use, did_to_use, network, 'token', ela_to_use)
	if response['status']:
		json_output = json.loads(response['output'])
		for i in json_output['result']:
			assert json_output['result'][i]!=None, "In View Token Wallet-> "+response['status_message']
	assert response['status']==True, "In View Token Wallet-> "+response['status_message']

def test_view_eth_wallet():
	wallet = Wallet(host, port, production)
	response = wallet.view_wallet(api_key_to_use, did_to_use, network, 'eth', ela_to_use)
	if response['status']:
		json_output = json.loads(response['output'])
		for i in json_output['result']:
			assert json_output['result'][i]!=None, "In View Eth Wallet-> "+response['status_message']
	assert response['status']==True, "In View Eth Wallet-> "+response['status_message']

def test_request_ela_mainchain():
	wallet = Wallet(host, port, production)
	response = wallet.request_ela(api_key_to_use, did_to_use, 'mainchain', ela_to_use)
	if response['status']:
		json_output = json.loads(response['output'])
		for i in json_output['result']:
			assert json_output['result'][i]!=None, "In Request Ela Mainchain-> "+response['status_message']
	assert response['status']==True, "In Request Ela Mainchain-> "+response['status_message']

def test_request_ela_did():
	wallet = Wallet(host, port, production)
	response = wallet.request_ela(api_key_to_use, did_to_use, 'did', ela_to_use)
	if response['status']:
		json_output = json.loads(response['output'])
		for i in json_output['result']:
			assert json_output['result'][i]!=None, "In Request Ela Did-> "+response['status_message']
	assert response['status']==True, "In Request Ela Did-> "+response['status_message']

def test_request_ela_token():
	wallet = Wallet(host, port, production)
	response = wallet.request_ela(api_key_to_use, did_to_use, 'token', ela_to_use)
	if response['status']:
		json_output = json.loads(response['output'])
		for i in json_output['result']:
			assert json_output['result'][i]!=None, "In Request Ela Token-> "+response['status_message']
	assert response['status']==True, "In Request Ela Token-> "+response['status_message']

def test_request_ela_eth():
	wallet = Wallet(host, port, production)
	response = wallet.request_ela(api_key_to_use, did_to_use, 'eth', ela_to_use)
	if response['status']:
		json_output = json.loads(response['output'])
		for i in json_output['result']:
			assert json_output['result'][i]!=None, "In Request Ela Eth-> "+response['status_message']
	assert response['status']==True, "In Request Ela Eth-> "+response['status_message']




