# SQLAlchemy settings
SQLALCHEMY_DATABASE_URI = 'postgresql://postgres:postgres@localhost:5432/smartweb_master'

#GMU Net
GMU_NET_IP_ADDRESS = 'http://10.192.113.16:'

#Wallet Service
WALLET_SERVICE_URL = 'http://10.192.113.16:8091/api'
WALLET_API_BALANCE = '/1/balance/'
WALLET_API_CREATE = '/1/createWallet'
WALLET_API_DPOS_VOTE = '/1/dpos/vote'
WALLET_API_TRANSACTIONS = '/1/tx'
WALLET_API_TRANSACTION_HISTORY = '/1/history/'
WALLET_API_TRANSFER = '/1/transfer'
WALLET_API_MNEMONIC = '/1/eng/mnemonic'

#DID sidechain
DID_SERVICE_URL = 'http://10.192.113.16:8092/api'
DID_SERVICE_GEN_DID = '/1/gen/did'
DID_SERVICE_SET_DID_INFO = '/1/setDidInfo'
DID_SERVICE_SIGN = '/1/sign'
DID_SERVICE_VERIFY = '/1/verify'

#Mainchain
MAINCHAIN_RPC_URL = 'http://10.192.113.16:10014'

#Sidechain
SIDECHAIN_RPC_URL = 'http://10.192.113.16:30113'

#Hive
HIVE_PORT = '9095'
HIVE_ADD = '/api/v0/file/add'
SHOW_CONTENT = '/api/v0/file/cat?arg='