# Flask settings
FLASK_SERVER_NAME = 'localhost:8888'
FLASK_DEBUG = True  # Do not use debug mode in production

# Flask-Restplus settings
RESTPLUS_SWAGGER_UI_DOC_EXPANSION = 'list'
RESTPLUS_VALIDATE = True
RESTPLUS_MASK_SWAGGER = False
RESTPLUS_ERROR_404_HELP = False

# SQLAlchemy settings
SQLALCHEMY_DATABASE_URI = 'postgresql://postgres:postgres@localhost:5432/elastos_admin'
SQLALCHEMY_TRACK_MODIFICATIONS = False

#GMU Net

#Wallet Service
WALLET_SERVICE_URL = 'http://localhost:8091/api'
WALLET_API_BALANCE = '/1/balance/EQ4QhsYRwuBbNBXc8BPW972xA9ANByKt6U'
WALLET_API_CREATE = '/1/createWallet'
WALLET_API_DPOS_VOTE = '/1/dpos/vote'
WALLET_API_TRANSACTIONS = '/1/tx'
WALLET_API_TRANSACTION_HISTORY = '/1/history/EQoascGFzdQ1rLKfNEavJKTm3hMRhBaXBT'
WALLET_API_TRANSFER = '/1/transfer'
WALLET_API_MNEMONIC = '/1/eng/mnemonic'

#DID sidechain
DID_SERVICE_URL = 'http://localhost:8092/api'
DID_SERVICE_GEN_DID = '/1/gen/did'
DID_SERVICE_SET_DID_INFO = '/1/setDidInfo'
DID_SERVICE_SIGN = '/1/sign'
DID_SERVICE_VERIFY = '/1/verify'

#Mainchain
MAINCHAIN_RPC_URL = 'http://localhost:10014'

#Sidechain
SIDECHAIN_RPC_URL = 'http://localhost:30113'