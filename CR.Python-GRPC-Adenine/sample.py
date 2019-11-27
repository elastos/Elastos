from adenine.common import Common
from adenine.did_sidechain import DidSidechain

def run():
    
	common = Common()
	print("--> Generate Api Request")
	response = common.generate_api_request('kAE&&ZeRLJ*6j7NfbYHs38CLVQWA%A@GN6j?&#LXwWQ6@y^G2rB3BSqqnHEZAg@s', 'qhfiueq98dqwbd')
    
	if(response.status==True):
		print("Api Key: "+response.api_key)
	elif(response.status==False):
		print("Error Message: "+response.status_message)

    #did_sidechain =  DidSidechain()
    #print("\n--> Sign a message")
    #did_sidechain.sign('shguBwHIVOi3EkHDP9V4JJb5GN', 'shguBwHIVO2wdw3EkHDPJb5GN', 'hey')

	#if(response.status==True):
	#	print("Message: ",response.status)
	#	print("Public Key: ",response.pub_key)
	#	print("Signature: ",response.sig)
	#elif(response.status==False):
	#	print("Error Message: ",response.status_message)


if __name__ == '__main__':
    run()
