import Foundation

public class Issuer {
    
    public var didDocument: DIDDocument?
    public var defaultSignKey: DIDURL!
    public var target: DID?
    public var vc: VerifiableCredential = VerifiableCredential()
    
    public init(_ did: DID, _ defaultSignKey: DIDURL?) throws {
        self.defaultSignKey = defaultSignKey
        self.target = did
        let store = try DIDStore.shareInstance()
        self.didDocument = try store?.resolveDid(did)
        guard self.didDocument != nil else {
            throw DIDError.failue("Can not resolve DID.")
        }
        if defaultSignKey == nil {
            self.defaultSignKey = self.didDocument?.getDefaultPublicKey()
        }else {
            guard didDocument?.getAuthenticationKey(defaultSignKey!) != nil else {
                throw DIDError.failue("Invalid sign key id.")
            }
        }
        
        guard !(try store?.containsPrivateKey(did, defaultSignKey!))! else {
            throw DIDError.failue("No private key.")
        }
    }

    public convenience init(_ did: DID) throws {
        try self.init(did, nil)
        
    }
    
    public func sign(_ passphrase: String) throws -> VerifiableCredential {
        
        self.vc.issuanceDate = Date()
        let json: String = self.vc.toJsonForSign(false)
        guard !json.isEmpty else {
            throw DIDError.failue("No json.")
        }
        let inputs: [CVarArg] = [json, json.count]
        let sig: String = (try DIDStore.shareInstance()?.sign(self.didDocument!.subject!, defaultSignKey, passphrase, inputs))!
        let proof: Proof = Proof(Constants.defaultPublicKeyType, defaultSignKey, sig)
        vc.proof = proof
        return vc
    }
}
