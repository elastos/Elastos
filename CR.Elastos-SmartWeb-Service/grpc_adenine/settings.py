# Node RPC
NODE_COMMON_RPC_METHODS = ['getreceivedbyaddress', 'getnodestate', 'getblockbyheight',
                           'getarbitratorgroupbyheight']
NODE_MAIN_RPC_METHODS = ['getconfirmbyheight', 'getmininginfo', 'getarbitersinfo', 'listproducers', 'listcrcandidates', 'listcurrentcrs']
NODE_SIDECHAIN_ETH_RPC_METHODS = ['eth_getBalance', 'eth_blockNumber', 'eth_getBlockByNumber']

# Mainchain RPC
MAINCHAIN_RPC_GET_BALANCE = 'getreceivedbyaddress'  # Get the balance of an address

# DID Sidechain RPC
DID_SIDECHAIN_RPC_GET_BALANCE = 'getreceivedbyaddress'  # Get the balance of an address

# Token Sidechain RPC
TOKEN_SIDECHAIN_RPC_GET_BALANCE = 'getreceivedbyaddress'  # Get the balance of an address

# Wallet Service[mainchain, did sidechain]
WALLET_API_GENERATE_MNEMONIC = '/1/eng/mnemonic'
WALLET_API_RETRIEVE_WALLET_FROM_MNEMONIC = '/1/hd'
WALLET_API_CREATE_WALLET = '/1/createWallet'
WALLET_API_TRANSFER = '/1/transfer'
WALLET_API_CROSSCHAIN_TRANSFER = '/1/cross/m2d/transfer'
WALLET_API_GET_TRANSACTION = '/1/tx/'

# Wallet Service[mainchain, token sidechain]
WALLET_API_CROSSCHAIN_TRANSFER_TOKENSIDECHAIN = '/1/cross/m2d/transfer'

# Sidechain Service
DID_SERVICE_RETRIEVE_WALLET_FROM_MNEMONIC = '/1/hd'
DID_SERVICE_API_CREATE_DID = '/1/gen/did'
DID_SERVICE_API_RETRIEVE_DID = '/1/did/'
DID_SERVICE_API_SIGN = '/1/sign'
DID_SERVICE_API_VERIFY = '/1/verify'

# Hive
HIVE_API_ADD_FILE = '/api/v0/file/add'
HIVE_API_RETRIEVE_FILE = '/api/v0/file/cat?arg='

# Requests Timeout
REQUEST_TIMEOUT = 30

# Rate Limiter
GENERATE_API_LIMIT = 10
NODE_RPC_LIMIT = 100000
UPLOAD_AND_SIGN_LIMIT = 100
VERIFY_AND_SHOW_LIMIT = 1000
CREATE_WALLET_LIMIT = 100
VIEW_WALLET_LIMIT = 100000
REQUEST_ELA_LIMIT = 10
DEPLOY_ETH_CONTRACT_LIMIT = 100
WATCH_ETH_CONTRACT_LIMIT = 1000

# Limit on other items
FILE_UPLOAD_SIZE_LIMIT = 5000000

# JWT Settings
TOKEN_EXPIRATION = 24 * 30
