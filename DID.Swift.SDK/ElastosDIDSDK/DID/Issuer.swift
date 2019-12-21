import Foundation

public class Issuer {
    
    public var didDocument: DIDDocument?
    public var signKey: DIDURL!
    public var target: DID?

    public init(_ doc: DIDDocument, _ signKey: DIDURL? = nil) throws {
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
    
    public init(_ did: DID, _ signKey: DIDURL? = nil) throws {
        self.signKey = signKey
        self.target = did
        self.didDocument = try did.resolve()
        guard self.didDocument != nil else {
            throw DIDError.failue("Can not resolve DID.")
        }
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
    
    public func getDid() -> DID {
        return (didDocument?.subject!)!
    }

    public func getSignKey() -> DIDURL {
        return signKey
    }
    
    public func seal(for did : DID, _ id: String, _ type: Array<String>, _ properties: Dictionary<String, String>, _ storepass: String) throws -> VerifiableCredential {
        let credential: VerifiableCredential = VerifiableCredential()
        credential.issuer = didDocument?.subject
        credential.subject = CredentialSubject(did)
        credential.id = try DIDURL(did, id)
        credential.types = type
        let date = DateFormater.currentDate()
        credential.issuanceDate = date
        if credential.expirationDate == nil {
            let edate = DateFormater.currentDateToWantDate(Constants.MAX_VALID_YEARS)
            credential.expirationDate = edate
        }
        let dic = credential.toJson(true, true)
        let json = JsonHelper.creatJsonString(dic: dic)
        let inputs: [CVarArg] = [json, json.count]
        let count: Int = inputs.count / 2
        let sig: String = try (didDocument?.sign(signKey, storepass, count, inputs))!
        
        let proof = Proof.init(Constants.defaultPublicKeyType, signKey, sig)
        credential.proof = proof
        return credential
    }
}
