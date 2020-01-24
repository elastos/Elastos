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


def main():
    parser = argparse.ArgumentParser(description="sample.py", add_help=False)
    parser.add_argument('-h', '--help', action='help', default=argparse.SUPPRESS,
                        help='Types of services supported: generate_api_key, get_api_key, upload_and_sign, '
                             'verify_and_show, create_wallet, view_wallet, request_ela, deploy_eth_contract, '
                             'watch_eth_contract')
    parser.add_argument('-s', action="store", dest="service")

    results = parser.parse_args()
    service = results.service

    network = "gmunet"
    mnemonic_to_use = 'obtain pill nest sample caution stone candy habit silk husband give net'
    did_to_use = 'n84dqvIK9O0LIPXi27uL0aRnoR45Exdxl218eQyPDD4lW8RPov'
    api_key_to_use = 'lBVwdmOhKT7czkUX0kHbnyxJnC1yyHkIcIospwv4yOIDsTtF1ygmgl06CAhlMxS0'
    private_key_to_use = '1F54BCD5592709B695E85F83EBDA515971723AFF56B32E175F14A158D5AC0D99'

    # Check whether grpc server is healthy first
    try:
        health_check = HealthCheck()
        print("--> Checking the health status of grpc server")
        response = health_check.check()
        if response.status != health_check_pb2.HealthCheckResponse.SERVING:
            print("grpc server is not running properly")
        else:
            print("grpc server is running")
    except Exception as e:
        print(e)
        sys.exit(1)

    if service == "generate_api_key":
        try:
            common = Common()
            # Generate API Key
            print("--> Generate API Key - SHARED_SECRET_ADENINE")
            response = common.generate_api_request(config('SHARED_SECRET_ADENINE'), did_to_use)
            if response.status:
                print("Api Key: " + response.api_key)
            else:
                print("Error Message: " + response.status_message)
            print("--> Generate API Key - MNEMONICS")
            response = common.generate_api_request_mnemonic(mnemonic_to_use)
            if response.status:
                print("Api Key: " + response.api_key)
            else:
                print("Error Message: " + response.status_message)
        except Exception as e:
            print(e)
        finally:
            common.close()
    elif service == "get_api_key":
        try:
            common = Common()
            # Get API Key
            print("--> Get API Key - SHARED_SECRET_ADENINE")
            response = common.get_api_key_request(config('SHARED_SECRET_ADENINE'), did_to_use)
            if response.status:
                print("Api Key: " + response.api_key)
            else:
                print("Error Message: " + response.status_message)
            print("--> Get API Key - MNEMONICS")
            response = common.get_api_request_mnemonic(mnemonic_to_use)
            if response.status:
                print("Api Key: " + response.api_key)
            else:
                print("Error Message: " + response.status_message)
        except Exception as e:
            print(e)
        finally:
            common.close()
    elif service == "upload_and_sign":
        try:
            hive = Hive()
            # Upload and Sign
            print("\n--> Upload and Sign")
            response = hive.upload_and_sign(api_key_to_use, network, private_key_to_use, 'test/sample.txt')
            if response.output:
                json_output = json.loads(response.output)
                print("Status Message :", response.status_message)
                for i in json_output['result']:
                    print(i, ':', json_output['result'][i])
        except Exception as e:
            print(e)
        finally:
            hive.close()
    elif service == "verify_and_show":
        try:
            hive = Hive()
            # Verify and Show
            print("\n--> Verify and Show")
            request_input = {
                "msg": "516D5666706D5977704B4375635570776773564141366936424459454B4A3271365058377A355234437A4C62746E",
                "pub": "022316EB57646B0444CB97BE166FBE66454EB00631422E03893EE49143B4718AB8",
                "sig": "569F046EDF421EC9EBE7E1EA33B181DE62A3F31AF433F3D34761B624A5E9753959711E0B9CB7E9E4FDC8CEF43388C73C2BF866EBDDF66B38AA8B1BA65BA90377",
                "hash": "QmVfpmYwpKCucUpwgsVAA6i6BDYEKJ2q6PX7z5R4CzLbtn",
                "private_key": private_key_to_use
            }
            response = hive.verify_and_show(api_key_to_use, network, request_input)
            if response.output:
                json_output = json.loads(response.output)
                print("Status Message :", response.status_message)
                for i in json_output['result']:
                    print(i, ':', json_output['result'][i])
        except Exception as e:
            print(e)
        finally:
            hive.close()
    elif service == "create_wallet":
        try:
            wallet = Wallet()
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
            wallet = Wallet()
            print("\n--> View Wallet")
            # Mainchain
            response = wallet.view_wallet(api_key_to_use, network, 'mainchain', 'EQeMkfRk3JzePY7zpUSg5ZSvNsWedzqWXN')
            if response.output:
                json_output = json.loads(response.output)
                print("Status Message :", response.status_message)
                for i in json_output['result']:
                    print(i, ':', json_output['result'][i])

            # DID sidechain
            response = wallet.view_wallet(api_key_to_use, network, 'did', 'EQeMkfRk3JzePY7zpUSg5ZSvNsWedzqWXN')
            if response.output:
                json_output = json.loads(response.output)
                print("Status Message :", response.status_message)
                for i in json_output['result']:
                    print(i, ':', json_output['result'][i])

            # Token sidechain
            response = wallet.view_wallet(api_key_to_use, network, 'token', 'EQeMkfRk3JzePY7zpUSg5ZSvNsWedzqWXN')
            if response.output:
                json_output = json.loads(response.output)
                print("Status Message :", response.status_message)
                for i in json_output['result']:
                    print(i, ':', json_output['result'][i])

            # Eth sidechain
            response = wallet.view_wallet(api_key_to_use, network, 'eth', '0x48F01b2f2b1a546927ee99dD03dCa37ff19cB84e')
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
            wallet = Wallet()
            print("\n--> Request ELA")
            # Mainchain
            response = wallet.request_ela(api_key_to_use, 'mainchain', 'EQeMkfRk3JzePY7zpUSg5ZSvNsWedzqWXN')
            if response.output:
                json_output = json.loads(response.output)
                print("Status Message :", response.status_message)
                for i in json_output['result']:
                    print(i, ':', json_output['result'][i])

            # DID sidechain
            response = wallet.request_ela(api_key_to_use, 'did', 'EQeMkfRk3JzePY7zpUSg5ZSvNsWedzqWXN')
            if response.output:
                json_output = json.loads(response.output)
                print("Status Message :", response.status_message)
                for i in json_output['result']:
                    print(i, ':', json_output['result'][i])

            # Token sidechain
            response = wallet.request_ela(api_key_to_use, 'token', 'EQeMkfRk3JzePY7zpUSg5ZSvNsWedzqWXN')
            if response.output:
                json_output = json.loads(response.output)
                print("Status Message :", response.status_message)
                for i in json_output['result']:
                    print(i, ':', json_output['result'][i])

            # Eth sidechain
            response = wallet.request_ela(api_key_to_use, 'eth', '0x48F01b2f2b1a546927ee99dD03dCa37ff19cB84e')
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
            sidechain_eth = SidechainEth()
            # Deploy ETH Contract
            # The eth account addresses below is used from that of privatenet. In order to test this,
            # you must first run https://github.com/cyber-republic/elastos-privnet locally
            # For production GMUnet, this won't work
            print("\n--> Deploy ETH Contract")
            response = sidechain_eth.deploy_eth_contract(api_key_to_use, network, '0x48F01b2f2b1a546927ee99dD03dCa37ff19cB84e',
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
            sidechain_eth = SidechainEth()
            print("\n--> Watch ETH Contract")
            response = sidechain_eth.watch_eth_contract(api_key_to_use, network,
                                                        '0x02D283dbBC6Fa60B45EC1029672029229C95Be4C', 'HelloWorld',
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
