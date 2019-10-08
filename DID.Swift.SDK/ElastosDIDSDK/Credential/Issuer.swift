import Foundation

public class Issuer {
    
    public var didDocument: DIDDocument?
    public var defaultSignKey: DIDURL!
    
    public init(_ did: DID, _ defaultSignKey: DIDURL?) throws {
        let store = DIDStore.shareInstance()
        self.didDocument = try store?.resolveDid(did)
        guard self.didDocument != nil else {
            // TODO: THROWS ERROR
            return
        }
        if defaultSignKey == nil {
            let pk: DIDPublicKey = (didDocument?.getDefaultPublicKey()!)!
            self.defaultSignKey = pk.id
        }else {
            guard didDocument?.getAuthenticationKey(defaultSignKey!) != nil else {
                // TODO: THROW ERROR
                return
            }
        }
        
        guard !(try store?.containsPrivateKey(did, defaultSignKey!))! else {
            // TODO: THROWS ERROR
            return
        }
        self.defaultSignKey = defaultSignKey
    }
    
    
}
