from adenine.common import Common
from adenine.did_sidechain import DidSidechain
from decouple import config
import json

def run():
    
    #generate API Key
	common = Common()
	print("--> Generate Api Request")
	response = common.generate_api_request(config('SHARED_SECRET_ADENINE'), 'qhfiueq98dqwbd')
    
	if(response.status==True):
		print("Api Key: "+response.api_key)
	elif(response.status==False):
		print("Error Message: "+response.status_message)

	#Signing using Did Sidechain
	did_sidechain =  DidSidechain()
	print("\n--> Sign a message")
	response1 = did_sidechain.sign('9A5Fy8jDxsJQSDdU4thLZs9fwDmtVzBU', '1F54BCD5592709B695E85F83EBDA515971723AFF56B32E175F14A158D5AC0D99', 'hey hi!')
	json_output = json.loads(response1.output)

	if(response1.status==True):
		for i in json_output : 
			print(i,':', json_output[i]) 
	elif(response1.status==False):
		print("Error Message: ",json_output)


if __name__ == '__main__':
    run()
