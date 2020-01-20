import Foundation

public class Issuer {
    
    public var didDocument: DIDDocument?
    public var signKey: DIDURL!
    public var target: DID?
    
    public init(_ doc: DIDDocument, signKey: DIDURL? = nil) throws {
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
    
    public init(_ did: DID, signKey: DIDURL? = nil, _ store: DIDStore) throws {
        let doc = try store.loadDid(did)
        guard (doc != nil) else {
            throw DIDError.failue("Can not resolve DID.")
        }
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
    
    public func issueFor(did: DID) -> CredentialBuilder{
        return CredentialBuilder(did: did, doc: self.didDocument!, signKey: self.signKey);
    }
}
