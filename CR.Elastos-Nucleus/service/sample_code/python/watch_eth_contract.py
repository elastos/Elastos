import json

from elastos_adenine.sidechain_eth import SidechainEth

def main():
    host = "localhost"
    port = 8001
    production = False

    api_key = "9A5Fy8jDxsJQSDdU4thLZs9fwDmtVzBU"
    did = "iHdasfhasdflkHdasfasdfD"
    network = "gmunet"
    smart_contract_address = "0xdf29327c95b12A37089A6e230d5ce50F23237671"
    smart_contract_name = "HelloWorld"
    smart_contract_code_hash = "QmRCn3tQem7UugGLE7tkchXudp4prqLtDhMRs828mUED34"

    try:
        sidechain_eth = SidechainEth(host, port, production)
        print("\n--> Watch ETH Contract")
        response = sidechain_eth.watch_eth_contract(api_key, did, network, smart_contract_address, smart_contract_name,
                                                    smart_contract_code_hash)
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
