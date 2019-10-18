import pytest
import requests
import json

def test_api_generateAPIKey():
	api_url_base = 'http://localhost:8888/api/1/common/generateAPIKey'
	myResponse = requests.get(api_url_base).json()
	assert myResponse['status'] == 200,"test failed"
	assert len(myResponse['API Key']) == 30,"test failed"

def test_api_createWallet():
	api_url_base = 'http://localhost:8888/api/1/service/mainchain/createWallet'
	myResponse = requests.get(api_url_base).json()
	assert myResponse['status'] == 200,"test failed"