import Foundation

class Constants {
    static let DEFAULT_PUBLICKEY_TYPE = "ECDSAsecp256r1"

    // DID method
    static let METHOD = "elastos"

    static let ID = "id"
    static let TYPE = "type"
    static let PUBLICKEY = "publicKey"
    static let CONTROLLER = "controller"
    static let AUTHENTICATION = "authentication"
    static let AUTHORIZATION = "authorization"
    static let PUBLICKEY_BASE58 = "publicKeyBase58"
    static let SERVICE_ENDPOINT = "serviceEndpoint"
    static let VERIFIABLE_CREDENTIAL = "verifiableCredential"
    static let SERVICE = "service"
    static let EXPIRES = "expires"
    static let PROOF = "proof"

    static let CREATOR = "creator";
    static let CREATED = "created";
    static let SIGNATURE_VALUE = "signatureValue";

    // Verifiable Credential
    static let ISSUER = "issuer"
    static let ISSUANCE_DATE = "issuanceDate"
    static let EXPIRATION_DATE = "expirationDate"
    static let CREDENTIAL_SUBJECT = "credentialSubject"

    // Verifiable Credential Builder
    static let MAX_VALID_YEARS = 5

    static let SIGNATURE = "signature"
    static let VERIFICATION_METHOD = "verificationMethod"

    static let DEFAULT_PRESENTATION_TYPE = "VerifiablePresentation"
    static let NONCE = "nonce"
    static let REALM = "realm"

    // FileSystemStorage
    static let PRIVATE_DIR = "private"
    static let HDKEY_FILE = "key"
    static let HDPUBKEY_FILE = "key.pub"
    static let INDEX_FILE = "index"
    static let MNEMONIC_FILE = "mnemonic";

    static let DID_DIR = "ids"
    static let DOCUMENT_FILE = "document"
    static let CREDENTIALS_DIR = "credentials"
    static let CREDENTIAL_FILE = "credential"
    static let PRIVATEKEYS_DIR = "privatekeys"

    static let META_FILE = ".meta"
    static let JOURNAL_SUFFIX = ".journal"
    static let DEPRECATED_SUFFIX = ".deprecated"

    // IDTransactionInfo
    static let TXID = "txid"
    static let TIMESTAMP = "timestamp"
    static let OPERATION = "operation"

    // IDChainRequest
    static let HEADER = "header"
    static let SPECIFICATION = "specification"
    static let PREVIOUS_TXID = "previousTxid"
    static let PAYLOAD = "payload"
    static let KEY_TYPE = "type"

    // ResolveResult
    static let DID = "did"
    static let STATUS = "status"
    static let TRANSACTION = "transaction"

    // MetaData
    static let EXTRA_PREFIX = "X-"

    // DIDMeta: TXID/TIMESTAMP/ALIAS/DEACTIVATED
    static let ALIAS = "alias"
    static let DEACTIVATED = "deactivated"

    // CredentialMeta: ALIAS

    // DIDBackend
    static let RESULT = "result"
    static let ERROR = "error"
    static let ERROR_CODE = "code"
    static let ERROR_MESSAGE = "message"
    static let DEFAULT_TTL = 24 * 60 * 60 * 1000
}
