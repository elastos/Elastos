import pytest
import requests
import json
def test_file1_method1():
	api_url_base = 'http://localhost:8888/api/1/common/generateAPIKey'
	myResponse = requests.get(api_url_base).json()
	assert myResponse['status'] == 200,"test failed"
def test_file1_method2():
	api_url_base = 'http://localhost:8888/api/1/common/generateAPIKey'
	myResponse = requests.get(api_url_base).json()
	assert len(myResponse['API Key']) == 200,"test failed"