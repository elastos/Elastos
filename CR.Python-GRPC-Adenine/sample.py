from decouple import config
import json
import argparse

from elastos_adenine.common import Common
from elastos_adenine.hive import Hive
from elastos_adenine.sidechain_eth import SidechainEth
from elastos_adenine.wallet import Wallet


def main():
    parser = argparse.ArgumentParser(description="sample.py", add_help=False)
    parser.add_argument('-h', '--help', action='help', default=argparse.SUPPRESS,
                        help='Types of services supported: generate_api, upload_and_sign, verify_and_show, '
                             'create_wallet, view_wallet, request_ela, deploy_eth_contract, watch_eth_contract')
    parser.add_argument('-s', action="store", dest="service")

    results = parser.parse_args()
    service = results.service

    if service == "generate_api":
        try:
            common = Common()
            # Generate API Key
            print("--> Generate Api Request")
            response = common.generate_api_request(config('SHARED_SECRET_ADENINE'), 'qhfiueq98dqwbd')
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
            response = hive.upload_and_sign('9A5Fy8jDxsJQSDdU4thLZs9fwDmtVzBU',
                                            '1F54BCD5592709B695E85F83EBDA515971723AFF56B32E175F14A158D5AC0D99',
                                            'test/sample.txt')
            json_output = json.loads(response.output)
            if response.status:
                for i in json_output['result']:
                    print(i, ':', json_output['result'][i])
            else:
                print("Error Message: ", response.status_message)
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
                "msg": "516D5A554C6B43454C6D6D6B35584E664367546E437946674156784252425879444847474D566F4C464C6958454E",
                "pub": "022316EB57646B0444CB97BE166FBE66454EB00631422E03893EE49143B4718AB8",
                "sig": "15FDD2752B686AF7CABE8DF72FCCC91AC25577C6AFB70A81A1D987DAACAE298621E227D585B7020100228AEF96D22AD0403612FFAEDCD7CA3A2070455418181C",
                "hash": "QmZULkCELmmk5XNfCgTnCyFgAVxBRBXyDHGGMVoLFLiXEN",
                "private_key": "1F54BCD5592709B695E85F83EBDA515971723AFF56B32E175F14A158D5AC0D99"
            }
            response = hive.verify_and_show('9A5Fy8jDxsJQSDdU4thLZs9fwDmtVzBU', request_input)
            if response.status:
                print('File Content:', response.output)
            else:
                print("Error Message: ", response.status_message)
        except Exception as e:
            print(e)
        finally:
            hive.close()
    elif service == "create_wallet":
        try:
            wallet = Wallet()
            print("\n--> Create Wallet")
            response = wallet.create_wallet('9A5Fy8jDxsJQSDdU4thLZs9fwDmtVzBU', 'elastos-privnet')
            json_output = json.loads(response.output)
            if response.status:
                for i in json_output['result']:
                    print(i, ':', json_output['result'][i])
            else:
                print("Error Message: ", response.status_message)
        except Exception as e:
            print(e)
        finally:
            wallet.close()
    elif service == "view_wallet":
        try:
            wallet = Wallet()
            print("\n--> View Wallet")
            # Mainchain preloaded wallet address on private net
            response = wallet.view_wallet('9A5Fy8jDxsJQSDdU4thLZs9fwDmtVzBU', 'mainchain', 'EQeMkfRk3JzePY7zpUSg5ZSvNsWedzqWXN')
            json_output = json.loads(response.output)
            if response.status:
                for i in json_output['result']:
                    print(i, ':', json_output['result'][i])
            else:
                print("Error Message: ", response.status_message)

            # DID sidechain preloaded wallet address on private net
            response = wallet.view_wallet('9A5Fy8jDxsJQSDdU4thLZs9fwDmtVzBU', 'did', 'EQeMkfRk3JzePY7zpUSg5ZSvNsWedzqWXN')
            json_output = json.loads(response.output)
            if response.status:
                for i in json_output['result']:
                    print(i, ':', json_output['result'][i])
            else:
                print("Error Message: ", response.status_message)

            # Token sidechain preloaded wallet address on private net
            response = wallet.view_wallet('9A5Fy8jDxsJQSDdU4thLZs9fwDmtVzBU', 'token', 'EQeMkfRk3JzePY7zpUSg5ZSvNsWedzqWXN')
            json_output = json.loads(response.output)
            if response.status:
                for i in json_output['result']:
                    print(i, ':', json_output['result'][i])
            else:
                print("Error Message: ", response.status_message)

            # Eth sidechain preloaded wallet address on private net
            response = wallet.view_wallet('9A5Fy8jDxsJQSDdU4thLZs9fwDmtVzBU', 'eth', '0x282c2795B9722d638778f5a1A0045c60b330F1A0')
            json_output = json.loads(response.output)
            if response.status:
                for i in json_output['result']:
                    print(i, ':', json_output['result'][i])
            else:
                print("Error Message: ", response.status_message)
        except Exception as e:
            print(e)
        finally:
            wallet.close()
    elif service == "request_ela":
        try:
            wallet = Wallet()
            print("\n--> Request ELA")
            # Mainchain preloaded wallet address on private net
            response = wallet.request_ela('9A5Fy8jDxsJQSDdU4thLZs9fwDmtVzBU', 'mainchain', 'EQeMkfRk3JzePY7zpUSg5ZSvNsWedzqWXN')
            json_output = json.loads(response.output)
            if response.status:
                for i in json_output['result']:
                    print(i, ':', json_output['result'][i])
            else:
                print("Error Message: ", response.status_message)

            # DID sidechain preloaded wallet address on private net
            response = wallet.request_ela('9A5Fy8jDxsJQSDdU4thLZs9fwDmtVzBU', 'did', 'EQeMkfRk3JzePY7zpUSg5ZSvNsWedzqWXN')
            json_output = json.loads(response.output)
            if response.status:
                for i in json_output['result']:
                    print(i, ':', json_output['result'][i])
            else:
                print("Error Message: ", response.status_message)

            # Token sidechain preloaded wallet address on private net
            response = wallet.request_ela('9A5Fy8jDxsJQSDdU4thLZs9fwDmtVzBU', 'token', 'EQeMkfRk3JzePY7zpUSg5ZSvNsWedzqWXN')
            json_output = json.loads(response.output)
            if response.status:
                for i in json_output['result']:
                    print(i, ':', json_output['result'][i])
            else:
                print("Error Message: ", response.status_message)
            
            # Eth sidechain preloaded wallet address on private net
            response = wallet.request_ela('9A5Fy8jDxsJQSDdU4thLZs9fwDmtVzBU', 'eth', '0x282c2795B9722d638778f5a1A0045c60b330F1A0')
            json_output = json.loads(response.output)
            if response.status:
                for i in json_output['result']:
                    print(i, ':', json_output['result'][i])
            else:
                print("Error Message: ", response.status_message)
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
            response = sidechain_eth.deploy_eth_contract('9A5Fy8jDxsJQSDdU4thLZs9fwDmtVzBU',
                                                         '0x4505b967d56f84647eb3a40f7c365f7d87a88bc3', 'elastos-privnet',
                                                         'test/HelloWorld.sol')
            json_output = json.loads(response.output)
            if response.status:
                for i in json_output['result']:
                    print(i, ':', json_output['result'][i])
            else:
                print("Error Message: ", response.status_message)
        except Exception as e:
            print(e)
        finally:
            sidechain_eth.close()
    elif service == "watch_eth_contract":
        try:
            sidechain_eth = SidechainEth()
            print("\n--> Watch ETH Contract")
            response = sidechain_eth.watch_eth_contract('9A5Fy8jDxsJQSDdU4thLZs9fwDmtVzBU',
                                                         '0x099E99A9f9668Cc6176c27F73da0b11B7DF42705', 'test/HelloWorld.sol',
                                                         'HelloWorld', 'QmRCn3tQem7UugGLE7tkchXudp4prqLtDhMRs828mUED34')
            json_output = json.loads(response.output)
            if response.status:
                for i in json_output['result']:
                    print(i, ':', json_output['result'][i])
            else:
                print("Error Message: ", response.status_message)
        except Exception as e:
            print(e)
        finally:
            sidechain_eth.close()


if __name__ == '__main__':
    main()
