import Foundation

public class Constants {
    public static let DEFAULT_PUBLICKEY_TYPE: String = "ECDSAsecp256r1"

    public static let DATE_FORMAT: String = "yyyy-MM-dd'T'HH:mm:ss'Z'"
    public static let DATE_FORMAT_ISO_8601: String = "yyyy-MM-dd'T'HH:mm:ss.SSS'Z'"

    public static let MAX_VALID_YEARS: Int = 5
    
    public static let UTC: TimeZone = TimeZone(abbreviation: "UTC")!
    public static let ID: String = "id"
    public static let PUBLICKEY: String = "publicKey"
    public static let TYPE: String = "type"
    public static let CONTROLLER: String = "controller"
    public static let PUBLICKEY_BASE58: String = "publicKeyBase58"
    public static let AUTHENTICATION: String = "authentication"
    public static let AUTHORIZATION: String = "authorization"
    public static let SERVICE: String = "service"
    public static let SERVICE_ENDPOINT: String = "serviceEndpoint"
    public static let EXPIRES: String = "expires"
    public static let CREATOR: String = "creator"
    public static let PROOF: String = "proof"
    public static let CREATED: String = "created"
    public static let VERIFIABLE_CREDENTIAL: String = "verifiableCredential"
    public static let SIGNATURE_VALUE: String = "signatureValue"
    
    public static let credential: String = "verifiableCredential"
    public static let issuer: String = "issuer"
    public static let issuanceDate: String = "issuanceDate"
    public static let expirationDate: String = "expirationDate"
    public static let credentialSubject: String = "credentialSubject"
    public static let verificationMethod: String = "verificationMethod"
    public static let signature: String = "signature"

    public static let nonce: String = "nonce"
    public static let realm: String = "realm"

    public static let defaultPublicKeyType: String = "ECDSAsecp256r1"
    public static let defaultPresentationType: String = "VerifiablePresentation"
}
