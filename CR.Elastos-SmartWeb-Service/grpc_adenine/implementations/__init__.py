import logging

# Set up logging
logging.basicConfig(
    format='%(asctime)s %(levelname)-8s %(message)s',
    level=logging.DEBUG,
    datefmt='%Y-%m-%d %H:%M:%S'
)

global WalletAddresses, WalletAddressesETH

WalletAddresses = set()
WalletAddressesETH = set()
