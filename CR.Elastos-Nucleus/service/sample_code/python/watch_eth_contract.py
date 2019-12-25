import json

from elastos_adenine.sidechain_eth import SidechainEth

def main():
    api_key = "9A5Fy8jDxsJQSDdU4thLZs9fwDmtVzBU"
    smart_contract_address = "0xdf29327c95b12A37089A6e230d5ce50F23237671"
    smart_contract_name = "HelloWorld"
    smart_contract_code_hash = "QmRCn3tQem7UugGLE7tkchXudp4prqLtDhMRs828mUED34"
    try:
        sidechain_eth = SidechainEth()
        print("\n--> Watch ETH Contract")
        response = sidechain_eth.watch_eth_contract(api_key, smart_contract_address, smart_contract_name, smart_contract_code_hash)
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
