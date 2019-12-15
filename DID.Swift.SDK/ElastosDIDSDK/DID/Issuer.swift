import Foundation

public class Issuer {
    
    public var didDocument: DIDDocument?
    public var signKey: DIDURL!
    public var target: DID?
    public var credential: VerifiableCredential!

    public init(_ doc: DIDDocument, _ signKey: DIDURL) throws {
       _ = try Issuer(doc, signKey)
    }
    
    public init(_ doc: DIDDocument) throws {
        _ = try Issuer(doc, nil)
    }
    
    public init(_ did: DID, _ signKey: DIDURL?) throws {
        self.signKey = signKey
        self.target = did
        self.didDocument = try did.resolve()
        guard self.didDocument != nil else {
            throw DIDError.failue("Can not resolve DID.")
        }
        self.credential = VerifiableCredential()
        self.credential.subject = CredentialSubject(self.target!)
        self.credential.issuer = didDocument?.subject
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
    
    private init(_ doc: DIDDocument, _ signKey: DIDURL?) throws {
        
        self.didDocument = doc
        self.signKey = signKey
        if (signKey == nil) {
            self.signKey = didDocument!.getDefaultPublicKey()
        } else {
            if (try !(didDocument!.isAuthenticationKey((self.signKey)))){
                throw DIDError.failue("Invalid sign key id.")
            }
            
            if (try !didDocument!.hasPrivateKey(self.signKey)){
                throw DIDError.failue("No private key.")
            }
        }
    }
    
    public func getDid() -> DID {
        return (didDocument?.subject!)!
    }

    public func getSignKey() -> DIDURL {
        return signKey
    }
    
    public func sign(_ passphrase: String) throws -> VerifiableCredential {
        
        self.credential.issuanceDate = Date()
        let json: String = self.credential.toJsonForSign(false)
        guard !json.isEmpty else {
            throw DIDError.failue("No json.")
        }
        let inputs: [CVarArg] = [json, json.count]
        let count = inputs.count / 2
        let sig: String = (try didDocument?.sign(signKey, passphrase, count, inputs))!
        let proof: Proof = Proof(Constants.defaultPublicKeyType, signKey, sig)
        credential.proof = proof
        
        // Should clean credential member
        let vc: VerifiableCredential = credential
        // TODO: CLEAR
        return vc
    }
}
