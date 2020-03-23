import sys

from decouple import config
import json
import argparse
import jwt
from elastos_adenine.stubs import health_check_pb2

from elastos_adenine.health_check import HealthCheck
from elastos_adenine.node_rpc import NodeRpc
from elastos_adenine.common import Common
from elastos_adenine.hive import Hive
from elastos_adenine.sidechain_eth import SidechainEth
from elastos_adenine.wallet import Wallet


def main():
    parser = argparse.ArgumentParser(description="sample.py", add_help=False)
    parser.add_argument('-h', '--help', action='help', default=argparse.SUPPRESS,
                        help='Types of services supported: node_rpc_methods, generate_api_key, get_api_key, '
                             'upload_and_sign, verify_and_show, create_wallet, view_wallet, request_ela, '
                             'deploy_eth_contract, watch_eth_contract')
    parser.add_argument('-s', action="store", dest="service")

    results = parser.parse_args()
    service = results.service

    host = config('GRPC_SERVER_HOST')
    port = config('GRPC_SERVER_PORT')
    production = config('PRODUCTION', default=False, cast=bool)

    network = "gmunet"
    mnemonic_to_use = 'obtain pill nest sample caution stone candy habit silk husband give net'
    ela_to_use = 'EQeMkfRk3JzePY7zpUSg5ZSvNsWedzqWXN'
    ela_eth_to_use = '0x48F01b2f2b1a546927ee99dD03dCa37ff19cB84e'
    did_to_use = 'n84dqvIK9O0LIPXi27uL0aRnoR45Exdxl218eQyPDD4lW8RPov'
    api_key_to_use = 'KfBoNvibcRrhec9IE1sAl3pIL7SecOLyE1oe8VuUgM21IjwRybibAmBPB8PoVokj'
    private_key_to_use = '1F54BCD5592709B695E85F83EBDA515971723AFF56B32E175F14A158D5AC0D99'

    # Check whether grpc server is healthy first
    try:
        health_check = HealthCheck(host, port, production)
        print("--> Checking the health status of grpc server")
        response = health_check.check()
        if response.status != health_check_pb2.HealthCheckResponse.SERVING:
            print("grpc server is not running properly")
        else:
            print("grpc server is running")
    except Exception as e:
        print(e)
        sys.exit(1)

    if service == "node_rpc_methods":
        try:
            node_rpc = NodeRpc(host, port, production)
            print("--> Get current height")
            current_height = node_rpc.get_current_height(network, "mainchain")
            print("Current Height - mainchain: ", current_height)
            current_height = node_rpc.get_current_height(network, "did")
            print("Current Height - did sidechain: ", current_height)
            current_height = node_rpc.get_current_height(network, "token")
            print("Current Height - token sidechain: ", current_height)
            print("--> Get current balance")
            current_balance = node_rpc.get_current_balance(network, "mainchain", ela_to_use)
            print("Current balance - mainchain:", current_balance)
            current_balance = node_rpc.get_current_balance(network, "did", ela_to_use)
            print("Current balance - did sidechain:", current_balance)
            current_balance = node_rpc.get_current_balance(network, "token", ela_to_use)
            print("Current balance - token sidechain:", current_balance)
            print("--> Get current block details")
            current_block_details = node_rpc.get_current_block_details(network, "mainchain")
            print("Current block details - mainchain: ", current_block_details)
            current_block_details = node_rpc.get_current_block_details(network, "did")
            print("Current block details - did sidechain: ", current_block_details)
            current_block_details = node_rpc.get_current_block_details(network, "token")
            print("Current block details - token sidechain: ", current_block_details)

            print("--> Get current mining info - mainchain")
            current_mining_info = node_rpc.get_current_mining_info(network)
            print("Current mining info: ", current_mining_info)
            print("--> Get current block confirm - mainchain")
            current_block_confirm = node_rpc.get_current_block_confirm(network)
            print("Current block confirm: ", current_block_confirm)
            print("--> Get current arbitrator info - mainchain")
            current_arbitrator_info = node_rpc.get_current_arbitrators_info(network)
            print("Current arbitrator info: ", current_arbitrator_info)
            print("--> Get current arbitrator group - mainchain")
            current_arbitrator_group = node_rpc.get_current_arbitrator_group(network)
            print("Current arbitrator group: ", current_arbitrator_group)
        except Exception as e:
            print(e)
        finally:
            node_rpc.close()
    elif service == "generate_api_key":
        try:
            common = Common(host, port, production)
            # Generate API Key
            print("--> Generate API Key - SHARED_SECRET_ADENINE")
            response = common.generate_api_request(config('SHARED_SECRET_ADENINE'), did_to_use)
            if response.status:
                jwt_info = jwt.decode(response.output, key=config('SHARED_SECRET_ADENINE'), algorithms=['HS256']).get('jwt_info')
                print("Api Key: " + jwt_info['api_key'])
            else:
                print("Error Message: " + response.status_message)

            # Generate API Key - MNEMONICS
            print("--> Generate API Key - MNEMONICS")
            response = common.generate_api_request_mnemonic(mnemonic_to_use, did_to_use)
            if response.status:
                jwt_info = jwt.decode(response.output, key=config('SHARED_SECRET_ADENINE'), algorithms=['HS256']).get('jwt_info')
                print("Api Key: " + jwt_info['api_key'])
            else:
                print("Error Message: " + response.status_message)
        except Exception as e:
            print(e)
        finally:
            common.close()
    elif service == "get_api_key":
        try:
            common = Common(host, port, production)
            # Get API Key
            print("--> Get API Key - SHARED_SECRET_ADENINE")
            response = common.get_api_key_request(config('SHARED_SECRET_ADENINE'), did_to_use)
            if response.status:
                jwt_info = jwt.decode(response.output, key=config('SHARED_SECRET_ADENINE'), algorithms=['HS256']).get('jwt_info')
                print("Api Key: " + jwt_info['api_key'])
            else:
                print("Error Message: " + response.status_message)
            
            # Get API Key - MNEMONICS
            print("--> Get API Key - MNEMONICS")
            response = common.get_api_request_mnemonic(mnemonic_to_use, did_to_use)
            if response.status:
                jwt_info = jwt.decode(response.output, key=config('SHARED_SECRET_ADENINE'), algorithms=['HS256']).get('jwt_info')
                print("Api Key: " + jwt_info['api_key'])
            else:
                print("Error Message: " + response.status_message)
        except Exception as e:
            print(e)
        finally:
            common.close()
    elif service == "upload_and_sign":
        try:
            hive = Hive(host, port, production)
            # Upload and Sign
            print("\n--> Upload and Sign")
            response = hive.upload_and_sign(api_key_to_use, did_to_use, network, private_key_to_use, 'test/sample.txt')
            if response.status:
                jwt_info = jwt.decode(response.output, key=api_key_to_use, algorithms=['HS256']).get('jwt_info')
                print("Status Message :", response.status_message)
                for i in jwt_info['result']:
                    print(i, ':', jwt_info['result'][i])
            else:
                print("Status Message :", response.status_message)
        except Exception as e:
            print(e)
        finally:
            hive.close()
    elif service == "verify_and_show":
        try:
            hive = Hive(host, port, production)
            # Verify and Show
            print("\n--> Verify and Show")
            request_input = {
                "msg": "516D614C55377A6B56784554396451316D4544464C677A7466564B6D53683343523641656265533774557764766A",
                "pub": "022316EB57646B0444CB97BE166FBE66454EB00631422E03893EE49143B4718AB8",
                "sig": "15FD3D976CF619CCF372F5752754A4EB25D861BF56D143FB8E07E45836D16795BBA80D070955BAF425995BA29AD6A11381A5B67D8E3CDA53F943CC080C693638",
                "hash": "QmaLU7zkVxET9dQ1mEDFLgztfVKmSh3CR6AebeS7tUwdvj",
                "privateKey": private_key_to_use
            }
            response = hive.verify_and_show(api_key_to_use, did_to_use,  network, request_input)

            if response['status']:
                download_path = 'test/sample_from_hive.txt'
                print("Status Message :", response['status_message'])
                print("File Path :", download_path)
                
                with open(download_path, 'wb') as file:
                    file.write(response['file_content'])
            else:
                print("Error Message: " + response['status_message'])
        except Exception as e:
            print(e)
        finally:
            hive.close()
    elif service == "create_wallet":
        try:
            wallet = Wallet(host, port, production)
            print("\n--> Create Wallet")
            response = wallet.create_wallet(api_key_to_use, network)
            if response.output:
                json_output = json.loads(response.output)
                print("Status Message :", response.status_message)
                for i in json_output['result']:
                    print(i, ':', json_output['result'][i])
        except Exception as e:
            print(e)
        finally:
            wallet.close()
    elif service == "view_wallet":
        try:
            wallet = Wallet(host, port, production)
            print("\n--> View Wallet")
            # Mainchain
            response = wallet.view_wallet(api_key_to_use, network, 'mainchain', ela_to_use)
            if response.output:
                json_output = json.loads(response.output)
                print("Status Message :", response.status_message)
                for i in json_output['result']:
                    print(i, ':', json_output['result'][i])

            # DID sidechain
            response = wallet.view_wallet(api_key_to_use, network, 'did', ela_to_use)
            if response.output:
                json_output = json.loads(response.output)
                print("Status Message :", response.status_message)
                for i in json_output['result']:
                    print(i, ':', json_output['result'][i])

            # Token sidechain
            response = wallet.view_wallet(api_key_to_use, network, 'token', ela_to_use)
            if response.output:
                json_output = json.loads(response.output)
                print("Status Message :", response.status_message)
                for i in json_output['result']:
                    print(i, ':', json_output['result'][i])

            # Eth sidechain
            response = wallet.view_wallet(api_key_to_use, network, 'eth', ela_eth_to_use)
            if response.output:
                json_output = json.loads(response.output)
                print("Status Message :", response.status_message)
                for i in json_output['result']:
                    print(i, ':', json_output['result'][i])
        except Exception as e:
            print(e)
        finally:
            wallet.close()
    elif service == "request_ela":
        try:
            wallet = Wallet(host, port, production)
            print("\n--> Request ELA")
            # Mainchain
            response = wallet.request_ela(api_key_to_use, 'mainchain', ela_to_use)
            if response.output:
                json_output = json.loads(response.output)
                print("Status Message :", response.status_message)
                for i in json_output['result']:
                    print(i, ':', json_output['result'][i])

            # DID sidechain
            response = wallet.request_ela(api_key_to_use, 'did', ela_to_use)
            if response.output:
                json_output = json.loads(response.output)
                print("Status Message :", response.status_message)
                for i in json_output['result']:
                    print(i, ':', json_output['result'][i])

            # Token sidechain
            response = wallet.request_ela(api_key_to_use, 'token', ela_to_use)
            if response.output:
                json_output = json.loads(response.output)
                print("Status Message :", response.status_message)
                for i in json_output['result']:
                    print(i, ':', json_output['result'][i])

            # Eth sidechain
            response = wallet.request_ela(api_key_to_use, 'eth', ela_eth_to_use)
            if response.output:
                json_output = json.loads(response.output)
                print("Status Message :", response.status_message)
                for i in json_output['result']:
                    print(i, ':', json_output['result'][i])
        except Exception as e:
            print(e)
        finally:
            wallet.close()
    elif service == "deploy_eth_contract":
        try:
            sidechain_eth = SidechainEth(host, port, production)
            # Deploy ETH Contract
            # The eth account addresses below is used from that of privatenet. In order to test this,
            # you must first run https://github.com/cyber-republic/elastos-privnet locally
            # For production GMUnet, this won't work
            print("\n--> Deploy ETH Contract")
            response = sidechain_eth.deploy_eth_contract(api_key_to_use, network,
                                                         ela_eth_to_use,
                                                         '0x35a12175385b24b2f906d6027d440aac7bd31e1097311fa8e3cf21ceac7c4809',
                                                         2000000, 'test/HelloWorld.sol')
            if response.output:
                json_output = json.loads(response.output)
                print("Status Message :", response.status_message)
                for i in json_output['result']:
                    print(i, ':', json_output['result'][i])
        except Exception as e:
            print(e)
        finally:
            sidechain_eth.close()
    elif service == "watch_eth_contract":
        try:
            sidechain_eth = SidechainEth(host, port, production)
            print("\n--> Watch ETH Contract")
            response = sidechain_eth.watch_eth_contract(api_key_to_use, network,
                                                        '0xc0ba7D9CF73c0410FfC9FB5b768F5257906B13c1', 'HelloWorld',
                                                        'QmXYqHg8gRnDkDreZtXJgqkzmjujvrAr5n6KXexmfTGqHd')
            if response.output:
                json_output = json.loads(response.output)
                print("Status Message :", response.status_message)
                for i in json_output['result']:
                    print(i, ':', json_output['result'][i])
        except Exception as e:
            print(e)
        finally:
            sidechain_eth.close()


if __name__ == '__main__':
    main()
