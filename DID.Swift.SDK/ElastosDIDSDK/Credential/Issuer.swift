import Foundation

public class Issuer {
    
    public var didDocument: DIDDocument?
    public var defaultSignKey: DIDURL!
    public var target: DID?
    public var vc: VerifiableCredential?
    
    public init(_ did: DID, _ defaultSignKey: DIDURL?) throws {
        let store = DIDStore.shareInstance()
        self.didDocument = try store?.resolveDid(did)
        guard self.didDocument != nil else {
            throw DIDError.failue("Can not resolve DID.")
        }
        if defaultSignKey == nil {
            let pk: DIDPublicKey = (didDocument?.getDefaultPublicKey()!)!
            self.defaultSignKey = pk.id
        }else {
            guard didDocument?.getAuthenticationKey(defaultSignKey!) != nil else {
                throw DIDError.failue("Invalid sign key id.")
            }
        }
        
        guard !(try store?.containsPrivateKey(did, defaultSignKey!))! else {
            throw DIDError.failue("No private key.")
        }
        self.defaultSignKey = defaultSignKey
    }
    
    public func sign(_ passphrase: String) {
        
        self.vc?.issuanceDate = Date()
        let json: String = self.vc!.toJsonForSign(false)
//        let sig: String = DIDStore.shareInstance()?.sign(self.didDocument?.subject, defaultSignKey, passphrase, json)
        let proof: Proof = Proof(Constants.defaultPublicKeyType, defaultSignKey, "sig")
        vc?.proof = proof
    }
 
}
