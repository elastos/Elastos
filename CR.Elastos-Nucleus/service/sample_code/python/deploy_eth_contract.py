import json

from elastos_adenine.sidechain_eth import SidechainEth

def main():
    host = "localhost"
    port = 8001
    production = False

    api_key = "9A5Fy8jDxsJQSDdU4thLZs9fwDmtVzBU"
    did = "iHdasfhasdflkHdasfasdfD"
    network = "gmunet"
    eth_address = "0x4505b967d56f84647eb3a40f7c365f7d87a88bc3"
    eth_private_key = "0xf98fa0f1e6b6772077591ba9eefe68b227c59d9103477a4db3c411feec919abb"
    eth_gas = 2000000
    smart_contract_file = "test/HelloWorld.sol"

    try:
        sidechain_eth = SidechainEth(host, port, production)
        # Deploy ETH Contract
        # The eth account addresses below is used from that of privatenet. In order to test this,
        # you must first run https://github.com/cyber-republic/elastos-privnet locally
        print("\n--> Deploy ETH Contract")
        response = sidechain_eth.deploy_eth_contract(api_key, did, network, eth_address, eth_private_key, eth_gas, smart_contract_file)
        if response['status']:
            json_output = json.loads(response['output'])
            print("Status Message :", response['status_message'])
            for i in json_output['result']:
                print(i, ':', json_output['result'][i])
        else:
            print("Error Message: " + response['status_message'])
    except Exception as e:
        print(e)
    finally:
        sidechain_eth.close()

if __name__ == '__main__':
    main()
