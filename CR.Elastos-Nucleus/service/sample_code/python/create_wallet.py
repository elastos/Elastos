import json

from elastos_adenine.wallet import Wallet

def main():
    api_key = "9A5Fy8jDxsJQSDdU4thLZs9fwDmtVzBU"
    try:
        wallet = Wallet()
        print("\n--> Create Wallet")
        response = wallet.create_wallet(api_key)
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

if __name__ == '__main__':
    main()
