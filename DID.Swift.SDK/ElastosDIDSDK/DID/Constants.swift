import Foundation

    let DEFAULT_PUBLICKEY_TYPE: String = "ECDSAsecp256r1"

    let DATE_FORMAT: String = "yyyy-MM-dd'T'HH:mm:ss'Z'"
    let DATE_FORMAT_ISO_8601: String = "yyyy-MM-dd'T'HH:mm:ss.SSS'Z'"

    public let MAX_VALID_YEARS: Int = 5
    
    let UTC: TimeZone = TimeZone(abbreviation: "UTC")!
    let ID: String = "id"
    let PUBLICKEY: String = "publicKey"
    let TYPE: String = "type"
    let CONTROLLER: String = "controller"
    let PUBLICKEY_BASE58: String = "publicKeyBase58"
    let AUTHENTICATION: String = "authentication"
    let AUTHORIZATION: String = "authorization"
    let SERVICE: String = "service"
    let SERVICE_ENDPOINT: String = "serviceEndpoint"
    let EXPIRES: String = "expires"
    let CREATOR: String = "creator"
    let PROOF: String = "proof"
    let CREATED: String = "created"
    let VERIFIABLE_CREDENTIAL: String = "verifiableCredential"
    let SIGNATURE_VALUE: String = "signatureValue"
    
    let ISSUER: String = "issuer"
    let ISSUANCE_DATE: String = "issuanceDate"
    let EXPIRATION_DATE: String = "expirationDate"
    let CREDENTIAL_SUBJECT: String = "credentialSubject"
    let VERIFICATION_METHOD: String = "verificationMethod"
    let SIGNATURE: String = "signature"

    let NONCE: String = "nonce"
    let REALM: String = "realm"

    let DEFAULT_PRESENTTATION_TYPE: String = "VerifiablePresentation"
