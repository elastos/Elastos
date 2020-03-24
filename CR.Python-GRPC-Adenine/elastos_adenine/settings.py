from decouple import config

# GRPC Server Public Certificate
GRPC_SERVER_CRT = config('GRPC_SERVER_CRT')

# Timeout to use
REQUEST_TIMEOUT = 30

# JWT Settings
TOKEN_EXPIRATION = 24 * 30