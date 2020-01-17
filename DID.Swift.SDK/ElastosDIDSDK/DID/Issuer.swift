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
    
    public func seal(for did : DID, _ id: String, _ type: Array<String>, _ properties: OrderedDictionary<String, String>, _ storepass: String) throws -> VerifiableCredential {
        let credential: VerifiableCredential = VerifiableCredential()
        credential.issuer = didDocument?.subject
        credential.subject = CredentialSubject(did)
        credential.id = try DIDURL(did, id)
        credential.types = type
        let date = DateFormater.currentDate()
        credential.issuanceDate = date
        if credential.expirationDate == nil {
            let edate = DateFormater.currentDateToWantDate(MAX_VALID_YEARS)
            credential.expirationDate = edate
        }
        credential.subject.addProperties(properties)
        let dic = credential.toJson(true, true)
        let json = JsonHelper.creatJsonString(dic: dic)
        let inputs: [CVarArg] = [json, json.count]
        let count: Int = inputs.count / 2
        let sig: String = try (didDocument?.sign(signKey, storepass, count, inputs))!
        
        let proof = CredentialProof(DEFAULT_PUBLICKEY_TYPE, signKey, sig)
        credential.proof = proof
        return credential
    }
}
