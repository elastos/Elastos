import Foundation

public class VerifiableCredential: DIDObject {
    public var types: Array<String>!
    public var issuer: DID!
    public var issuanceDate: Date!
    public var expiationDate: Date!
    public var subject: CredentialSubject!
    public var proof: Proof!
    
    init(_ vc: VerifiableCredential) {
        super.init(vc.id, vc.type)
        self.id = vc.id
        self.types = vc.types
        self.issuer = vc.issuer
        self.issuanceDate = vc.issuanceDate
        self.expiationDate = vc.expiationDate
        self.subject = vc.subject
        self.proof = vc.proof
    }
}
