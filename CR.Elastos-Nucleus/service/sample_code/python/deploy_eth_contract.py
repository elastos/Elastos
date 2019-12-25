import json

from elastos_adenine.sidechain_eth import SidechainEth

def main():
    api_key = "9A5Fy8jDxsJQSDdU4thLZs9fwDmtVzBU"
    eth_address = "0x4505b967d56f84647eb3a40f7c365f7d87a88bc3"
    eth_private_key = "0xf98fa0f1e6b6772077591ba9eefe68b227c59d9103477a4db3c411feec919abb"
    eth_gas = 200000
    smart_contract_file = "test/HelloWorld.sol"
    try:
        sidechain_eth = SidechainEth()
        print("\n--> Deploy ETH Contract")
        response = sidechain_eth.deploy_eth_contract(api_key, eth_address, eth_private_key, eth_gas, smart_contract_file)
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
