from adenine.common import Common
from adenine.did_sidechain import DidSidechain
from adenine.console import Console
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

	#Signing using Did Sidechain
	did_sidechain = DidSidechain()
	print("\n--> Sign a message")
	#response1 = did_sidechain.sign('9A5Fy8jDxsJQSDdU4thLZs9fwDmtVzBU', '1F54BCD5592709B695E85F83EBDA515971723AFF56B32E175F14A158D5AC0D99', 'hey hi!')
	#json_output1 = json.loads(response1.output)

	#if(response1.status==True):
	#	for i in json_output1 :
	#		print(i,':', json_output1[i])
	#elif(response1.status==False):
	#	print("Error Message: ",json_output1)

	#Upload and Sign
	console = Console()
	print("\n--> Upload and Sign")
	response2 = console.upload_and_sign('9A5Fy8jDxsJQSDdU4thLZs9fwDmtVzBU', '1F54BCD5592709B695E85F83EBDA515971723AFF56B32E175F14A158D5AC0D99', './sample.txt')
	json_output2 = json.loads(response2.output)

	if(response2.status==True):
		for i in json_output2['result'] :
			print(i,':', json_output2['result'][i])
	elif(response2.status==False):
		print("Error Message: ",response2.status_message)

	#Verify and Show
	print("\n--> Verify and Show")
	request_input = {
		"msg" : "516D656D64314B6A42535956785963524B674A336666665972613246644273316967445638394878577A704A6252",
		"pub" : "022316EB57646B0444CB97BE166FBE66454EB00631422E03893EE49143B4718AB8",
		"sig" : "EDB237447C7234219239DACEC88BC3AC5FAB7321D4AC49E18E259F6A5732C78021681298B2C7817105D6CF6F212B3746DE1238FC0FECD25CE23D165AF2957F40",
		"hash" : "Qmemd1KjBSYVxYcRKgJ3fffYra2FdBs1igDV89HxWzpJbR",
		"private_key" : "1F54BCD5592709B695E85F83EBDA515971723AFF56B32E175F14A158D5AC0D99"
	}
	response3 = console.verify_and_show('9A5Fy8jDxsJQSDdU4thLZs9fwDmtVzBU', request_input)

	if(response3.status==True):
		print('File Content:', response3.output)
	elif(response3.status==False):
		print("Error Message: ",response3.status_message)


if __name__ == '__main__':
    run()
