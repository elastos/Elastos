import json
from decouple import config
from requests import Session
from pathlib import Path

from web3 import Web3, HTTPProvider
from web3.middleware import geth_poa_middleware
from solc import compile_standard

from grpc_adenine import settings
from grpc_adenine.stubs import sidechain_eth_pb2
from grpc_adenine.stubs import sidechain_eth_pb2_grpc


class SidechainEth(sidechain_eth_pb2_grpc.SidechainEthServicer):

    def __init__(self):
        headers = {
            'Content-Disposition': 'multipart/form-data;boundary=--------------------------608819652137318562927303'
        }
        self.session = Session()
        self.session.headers.update(headers)
        # web3.py instance
        self.web3 = Web3(
            HTTPProvider("{0}{1}".format(config('PRIVATE_NET_IP_ADDRESS'), config('SIDECHAIN_ETH_RPC_PORT')),
                         request_kwargs={'timeout': 60}))
        # We need this since our eth sidechain is POA
        self.web3.middleware_onion.inject(geth_poa_middleware, layer=0)

    def DeployEthContract(self, request, context):

        # Validate the API Key
        api_key = request.api_key
        # api_status = validate_api_key(api_key)
        # if not api_status:
        #       return adenine_io_pb2.Response(output='', status_message='API Key could not be verified', status=False)

        # reading the file content
        request_input = json.loads(request.input)
        eth_account_address = request_input['eth_account_address']
        eth_account_password = request_input['eth_account_password']
        contract_metadata = request_input['contract_metadata']
        contract_source = request_input['contract_source']

        # upload smart contract code to hive
        hive_upload_url = config('PRIVATE_NET_IP_ADDRESS') + config('HIVE_PORT') + settings.HIVE_API_ADD_FILE
        response = self.session.get(hive_upload_url, files={'file': contract_source})
        data = json.loads(response.text)
        hive_hash = data['Hash']

        if not response:
            status_message = 'Error: Smart contract code could not be uploaded'
            status = False
            return sidechain_eth_pb2.Response(output="", status_message=status_message, status=status)

        if not self.web3.isConnected():
            status_message = 'Error: Could not connect to Eth Sidechain node'
            status = False
            return sidechain_eth_pb2.Response(output="", status_message=status_message, status=status)

        contract_name = contract_metadata['children'][1]['name']
        compiled_sol = compile_standard({
            "language": "Solidity",
            "sources": {
                contract_name: {
                    "content": contract_source,
                },
            },
            "settings": {
                'evmVersion': 'byzantium',
                "outputSelection": {
                    "*": {"*": ["*"]}
                }
            }
        })

        # set pre-funded account as sender
        self.web3.eth.defaultAccount = self.web3.toChecksumAddress(eth_account_address)

        # Have to unlock the account before we can transact with it
        self.web3.parity.personal.unlockAccount(self.web3.eth.defaultAccount, eth_account_password, 3600)

        # get bytecode
        bytecode = compiled_sol['contracts'][contract_name][contract_name]['evm']['bytecode']['object']

        # get abi
        abi = json.loads(compiled_sol['contracts'][contract_name][contract_name]['metadata'])['output']['abi']

        contract = self.web3.eth.contract(abi=abi, bytecode=bytecode)

        # Submit the transaction that deploys the contract
        tx_hash = contract.constructor().transact()

        # Wait for the transaction to be mined, and get the transaction receipt
        tx_receipt = self.web3.eth.waitForTransactionReceipt(tx_hash)

        contract_details = self.web3.eth.contract(
            address=tx_receipt.contractAddress,
            abi=abi
        )

        response = {
            'result': {
                'contract_address': contract_details.address,
                'contract_name': contract_name,
                'contract_code_hash': hive_hash,
            }
        }
        status_message = 'Successfuly deployed Eth Smart Contract'
        status = True

        return sidechain_eth_pb2.Response(output=json.dumps(response), status_message=status_message, status=status)

    def WatchEthContract(self, request, context):

        # Validate the API Key
        api_key = request.api_key
        # api_status = validate_api_key(api_key)
        # if not api_status:
        #       return adenine_io_pb2.Response(output='', status_message='API Key could not be verified', status=False)

        # reading the file content
        request_input = json.loads(request.input)
        contract_address = request_input['contract_address']
        contract_name = request_input['contract_name']
        contract_code_hash = request_input['contract_code_hash']

        # show smart contract code from Hive
        hive_show_url = config('PRIVATE_NET_IP_ADDRESS') + config('HIVE_PORT') + settings.HIVE_API_RETRIEVE_FILE + "{}"
        response = self.session.get(hive_show_url.format(contract_code_hash))
        contract_source = response.text

        if not response:
            status_message = 'Error: Smart contract code could not be retrieved from Hive'
            status = False
            return sidechain_eth_pb2.Response(output="", status_message=status_message, status=status)

        # Get smart contract details
        compiled_sol = compile_standard({
            "language": "Solidity",
            "sources": {
                contract_name: {
                    "content": contract_source,
                },
            },
            "settings": {
                'evmVersion': 'byzantium',
                "outputSelection": {
                    "*": {"*": ["*"]}
                }
            }
        })
        abi = json.loads(compiled_sol['contracts'][contract_name][contract_name]['metadata'])['output']['abi']
        contract = self.web3.eth.contract(address=contract_address, abi=abi)
        functions = contract.all_functions()
        contract_functions = []
        for function in functions:
            contract_function = repr(function)[1:-1].split()[1]
            contract_functions.append(contract_function)

        response = {
            'result': {
                'contract_address': contract_address,
                'contract_functions': contract_functions,
                'contract_source': contract_source
            }
        }
        status_message = 'Successfuly viewed Eth Smart Contract'
        status = True

        return sidechain_eth_pb2.Response(output=json.dumps(response), status_message=status_message, status=status)