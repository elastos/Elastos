import Foundation

public class VerifiableCredential: DIDObject {
    private var types: Array<String>!
    private var issuer: DID!
    private var issuanceDate: Date!
    private var expiationDate: Date!
    private var subject: CredentialSubject!
    private var proof: Proof!
    
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
