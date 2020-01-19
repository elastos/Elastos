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


public class CredentialBuilder {
    
    private var target: DID
    private var credential: VerifiableCredential
    private var signKey: DIDURL
    private var document: DIDDocument

    public init(did: DID, doc: DIDDocument, signKey:DIDURL) {
        self.target = did
        self.document = doc
        self.signKey = signKey
        self.credential = VerifiableCredential()
        self.credential.issuer = doc.subject
    }

   public func set(id: DIDURL) throws -> CredentialBuilder {
        self.credential.id = id
        return self
    }
    
    public func set(idString: String) throws -> CredentialBuilder {
        return try self.set(id: DIDURL(target, idString))
    }
    
   public func set(types: Array<String>) throws -> CredentialBuilder {
        guard types.count != 0 else {
            throw DIDError.illegalArgument("type is nil.")
        }
        
        self.credential.types = types
        return self
    }
    
//    private func getMaxExpires() {
//        var date: Date
//        if credential?.issuanceDate != nil {
//            date = self.credential!.issuanceDate!
//        }
//        return DateFormater.currentDateToWantDate(MAX_VALID_YEARS)
//
//    }
//
    
   public func set(expirationDate: Date) -> CredentialBuilder {
        return self
    }
    
   public func set(properties: OrderedDictionary<String, String>) throws -> CredentialBuilder {
        guard properties.keys.count != 0 else {
            throw DIDError.illegalArgument("properties count is 0.")
        }
        self.credential.subject = CredentialSubject(self.target)
        self.credential.subject.addProperties(properties)
        return self
    }
    
    public func seal(storepass: String) throws -> VerifiableCredential {
        guard !storepass.isEmpty else {
            throw DIDError.illegalArgument("storepass is empty.")
        }
        
        guard self.credential.id != nil else {
            throw DIDError.illegalArgument("Missing id.")
        }
        
        guard self.credential.subject != nil else {
            throw DIDError.illegalArgument("Missing subject.")
        }
        
        let date = DateFormater.currentDate()
        self.credential.issuanceDate = date
        
        if credential.expirationDate == nil {
            self.credential.expirationDate = DateFormater.currentDateToWantDate(MAX_VALID_YEARS)
            // TODO
        }
        
        let dic = self.credential.toJson(true, true)
        let json = JsonHelper.creatJsonString(dic: dic)
        let inputs: [CVarArg] = [json, json.count]
        let count: Int = inputs.count / 2
        let sig: String = try (self.document.sign(signKey, storepass, count, inputs))
        
        let proof = CredentialProof(DEFAULT_PUBLICKEY_TYPE, signKey, sig)
        self.credential.proof = proof
        
        return self.credential
    }
}
