import json

from elastos_adenine.wallet import Wallet

def main():
    api_key = "9A5Fy8jDxsJQSDdU4thLZs9fwDmtVzBU"
    address = "EQeMkfRk3JzePY7zpUSg5ZSvNsWedzqWXN"
    address_eth = "0x282c2795B9722d638778f5a1A0045c60b330F1A0"

    try:
        wallet = Wallet()
        print("\n--> Request ELA")
        # Mainchain
        response = wallet.request_ela(api_key, 'mainchain', address)
        if response.output:
            json_output = json.loads(response.output)
            print("Status Message :", response.status_message)
            for i in json_output['result']:
                print(i, ':', json_output['result'][i])

        # DID sidechain
        response = wallet.request_ela(api_key, 'did', address)
        if response.output:
            json_output = json.loads(response.output)
            print("Status Message :", response.status_message)
            for i in json_output['result']:
                print(i, ':', json_output['result'][i])

        # Token sidechain
        response = wallet.request_ela(api_key, 'token', address)
        if response.output:
            json_output = json.loads(response.output)
            print("Status Message :", response.status_message)
            for i in json_output['result']:
                print(i, ':', json_output['result'][i])

        # Eth sidechain
        response = wallet.request_ela(api_key, 'eth', address_eth)
        if response.output:
            json_output = json.loads(response.output)
            print("Status Message :", response.status_message)
            for i in json_output['result']:
                print(i, ':', json_output['result'][i])
    except Exception as e:
        print(e)
    finally:
        wallet.close()

if __name__ == '__main__':
    main()
