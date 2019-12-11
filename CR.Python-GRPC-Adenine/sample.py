from elastos_adenine.common import Common
from elastos_adenine.did_sidechain import DidSidechain
from elastos_adenine.console import Console
from decouple import config
import json

def run():

    #Generate API Key
	common = Common()
	print("--> Generate Api Request")
	response = common.generate_api_request(config('SHARED_SECRET_ADENINE'), 'qhfiueq98dqwbd')

	if(response.status==True):
		print("Api Key: "+response.api_key)
	elif(response.status==False):
		print("Error Message: "+response.status_message)

	'''
	#Signing using Did Sidechain
	did_sidechain = DidSidechain()
	print("\n--> Sign a message")
	response = did_sidechain.sign('9A5Fy8jDxsJQSDdU4thLZs9fwDmtVzBU', '1F54BCD5592709B695E85F83EBDA515971723AFF56B32E175F14A158D5AC0D99', 'hey hi!')
	json_output = json.loads(response.output)

	if(response.status==True):
		for i in json_output :
			print(i,':', json_output[i])
	elif(response.status==False):
		print("Error Message: ",json_output)
	'''

	console = Console()

	#Upload and Sign	
	print("\n--> Upload and Sign")
	response = console.upload_and_sign('9A5Fy8jDxsJQSDdU4thLZs9fwDmtVzBU', '1F54BCD5592709B695E85F83EBDA515971723AFF56B32E175F14A158D5AC0D99', 'test/sample.txt')
	json_output = json.loads(response.output)

	if(response.status==True):
		for i in json_output['result'] :
			print(i,':', json_output['result'][i])
	elif(response.status==False):
		print("Error Message: ",response.status_message)

	#Verify and Show
	print("\n--> Verify and Show")
	request_input = {
		"msg" : "516D5A554C6B43454C6D6D6B35584E664367546E437946674156784252425879444847474D566F4C464C6958454E",
		"pub" : "022316EB57646B0444CB97BE166FBE66454EB00631422E03893EE49143B4718AB8",
		"sig" : "15FDD2752B686AF7CABE8DF72FCCC91AC25577C6AFB70A81A1D987DAACAE298621E227D585B7020100228AEF96D22AD0403612FFAEDCD7CA3A2070455418181C",
		"hash" : "QmZULkCELmmk5XNfCgTnCyFgAVxBRBXyDHGGMVoLFLiXEN",
		"private_key" : "1F54BCD5592709B695E85F83EBDA515971723AFF56B32E175F14A158D5AC0D99"
	}
	response = console.verify_and_show('9A5Fy8jDxsJQSDdU4thLZs9fwDmtVzBU', request_input)

	if(response.status==True):
		print('File Content:', response.output)
	elif(response.status==False):
		print("Error Message: ",response.status_message)

	# Deploy ETH Contract
	# The eth account addresses below is used from that of privatenet. In order to test this,
	# you must first run https://github.com/cyber-republic/elastos-privnet locally
	# For production GMUnet, this won't work
	print("\n--> Deploy ETH Contract")
	response = console.deploy_eth_contract('9A5Fy8jDxsJQSDdU4thLZs9fwDmtVzBU', '0x4505b967d56f84647eb3a40f7c365f7d87a88bc3', 'elastos-privnet', 'test/HelloWorld.sol')
	json_output = json.loads(response.output)

	if(response.status==True):
		for i in json_output['result'] :
			print(i,':', json_output['result'][i])
	elif(response.status==False):
		print("Error Message: ",response.status_message)

if __name__ == '__main__':
    run()
