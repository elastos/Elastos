import json
from decouple import config

from web3 import Web3, HTTPProvider
from web3.middleware import geth_poa_middleware
from solc import compile_standard

from grpc_adenine.stubs import sidechain_eth_pb2
from grpc_adenine.stubs import sidechain_eth_pb2_grpc


class SidechainEth(sidechain_eth_pb2_grpc.SidechainEthServicer):

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
        filename = request_input['filename']
        contract_metadata = request_input['contract_metadata']
        contract_source = request_input['contract_source']

        # web3.py instance
        w3 = Web3(HTTPProvider("{0}{1}".format(config('PRIVATE_NET_IP_ADDRESS'), config('SIDECHAIN_ETH_RPC_PORT')),
                               request_kwargs={'timeout': 60}))
        # We need this since our eth sidechain is POA
        w3.middleware_onion.inject(geth_poa_middleware, layer=0)

        if not w3.isConnected():
            status_message = 'Error: Could not connect to Eth Sidechain node'
            status = False
            return sidechain_eth_pb2.Response(output="", status_message=status_message, status=status)

        contract_name = contract_metadata['children'][1]['name']
        compiled_sol = compile_standard({
            "language": "Solidity",
            "sources": {
                filename: {
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
        w3.eth.defaultAccount = w3.toChecksumAddress(eth_account_address)

        # Have to unlock the account before we can transact with it
        w3.parity.personal.unlockAccount(w3.eth.defaultAccount, eth_account_password, 3600)

        # get bytecode
        bytecode = compiled_sol['contracts'][filename][contract_name]['evm']['bytecode']['object']

        # get abi
        abi = json.loads(compiled_sol['contracts'][filename][contract_name]['metadata'])['output']['abi']

        contract = w3.eth.contract(abi=abi, bytecode=bytecode)

        # Submit the transaction that deploys the contract
        tx_hash = contract.constructor().transact()

        # Wait for the transaction to be mined, and get the transaction receipt
        tx_receipt = w3.eth.waitForTransactionReceipt(tx_hash)

        contract_details = w3.eth.contract(
            address=tx_receipt.contractAddress,
            abi=abi
        )

        response = {
            'result': {
                'contract_address': contract_details.address
            }
        }
        status_message = 'Successfuly deployed Eth Smart Contract'
        status = True

        return sidechain_eth_pb2.Response(output=json.dumps(response), status_message=status_message, status=status)
