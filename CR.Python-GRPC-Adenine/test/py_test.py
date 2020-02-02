import pytest
import sys
from decouple import config
import json
import argparse

from elastos_adenine.stubs import health_check_pb2
from elastos_adenine.health_check import HealthCheck
from elastos_adenine.common import Common
from elastos_adenine.hive import Hive
from elastos_adenine.sidechain_eth import SidechainEth
from elastos_adenine.wallet import Wallet

host = config('GRPC_SERVER_HOST')
port = config('GRPC_SERVER_PORT')
production = config('PRODUCTION', default=False, cast=bool)

network = "gmunet"
mnemonic_to_use = 'obtain pill nest sample caution stone candy habit silk husband give net'
did_to_use = 'n84dqvIK9O0LIPXi27uL0aRnoR45Exdxl218eQyPDD4lW8RPov'
api_key_to_use = 'ho3PV4wmvJbuMHbi1kxj8pDnor4LbnLWdjgjhsWF2oIGWASynBHrclL98YZFj5US'
private_key_to_use = '1F54BCD5592709B695E85F83EBDA515971723AFF56B32E175F14A158D5AC0D99'

def test_generate_api_key():
	# Generate API Key
	global api_key_to_use
	common = Common(host, port, production)
	response = common.generate_api_request(config('SHARED_SECRET_ADENINE'), did_to_use)
	if response.status:
		api_key_to_use = response.api_key
	assert response.status_message=='Success',"Testing Generate API Key Failed"

def test_upload_and_sign():
	# Upload And Sign
	hive = Hive(host, port, production)
	response = hive.upload_and_sign(api_key_to_use, network, private_key_to_use, 'test/sample.txt')
	assert 'Success' in response.status_message,"Testing Upload And Sign Failed"
