import Foundation

public class Issuer {
    
    public var didDocument: DIDDocument?
    public var signKey: DIDURL!
    public var target: DID?
    public var vc: VerifiableCredential!
    
    public init(_ did: DID, _ signKey: DIDURL?) throws {
        self.signKey = signKey
        self.target = did
        self.didDocument = try did.resolve()
        guard self.didDocument != nil else {
            throw DIDError.failue("Can not resolve DID.")
        }
        self.vc = VerifiableCredential()
        self.vc.subject = CredentialSubject(self.target!)
        self.vc.issuer = didDocument?.subject
        if signKey == nil {
            self.signKey = self.didDocument?.getDefaultPublicKey()
        } else {
            guard try didDocument?.isAuthenticationKey(self.signKey) != nil else {
                throw DIDError.failue("Invalid sign key id.")
            }
        }
        
        guard (try self.didDocument!.hasPrivateKey(self.signKey)) else {
            throw DIDError.failue("No private key.")
        }
    }

    public convenience init(_ did: DID) throws {
        try self.init(did, nil)
    }
    
    public func getDid() -> DID {
        return (didDocument?.subject!)!
    }

    public func getSignKey() -> DIDURL {
        return signKey
    }
    
    public func sign(_ passphrase: String) throws -> VerifiableCredential {
        
        self.vc.issuanceDate = Date()
        let json: String = self.vc.toJsonForSign(false)
        guard !json.isEmpty else {
            throw DIDError.failue("No json.")
        }
        let inputs: [CVarArg] = [json, json.count]
        let count = inputs.count / 2
        let sig: String = (try didDocument?.sign(signKey, passphrase, count, inputs))!
        let proof: Proof = Proof(Constants.defaultPublicKeyType, signKey, sig)
        vc.proof = proof
        return vc
    }
}
