
public class DIDDocumentBuilder {
    var document: DIDDocument?
    
    var subject: DID? {
        get {
            return document!.subject
        }
    }
    
    init(did: DID, store: DIDStore) {
        self.document = DIDDocument(did)
        self.document!.meta.store = store
    }
    
    init(doc: DIDDocument) {
        // Make a copy.
        self.document = DIDDocument(doc)
    }
    
    public func addPublicKey(_ id: DIDURL, _ controller: DID, _ pk: String) throws -> Bool {
        guard Base58.bytesFromBase58(pk).count == HDKey.PUBLICKEY_BYTES else {
            throw DIDError.failue("Invalid public key.")
        }
        let key: DIDPublicKey = DIDPublicKey(id, controller, pk)
        return document!.addPublicKey(key)
    }
    
    public func addPublicKey(_ id: String, _ controller: String, _ pk: String) throws -> Bool {
        return try addPublicKey(DIDURL(subject!, id), DID(controller), pk)
    }
    
    public func removePublicKey(_ id: DIDURL, _ force: Bool) throws -> Bool {
        return try document!.removePublicKey(id, force)
    }
    
    public func removePublicKey(_ id: String, _ force: Bool) throws -> Bool {
        return try removePublicKey(DIDURL(subject!, id), force)
    }
    
    public func removePublicKey(_ id: DIDURL) throws -> Bool {
        return try removePublicKey(id, false)
    }
    
    public func removePublicKey(_ id: String) throws -> Bool {
        return try removePublicKey(DIDURL(id), false)
    }
    
    public func addAuthenticationKey(_ id: DIDURL) throws -> Bool {
        let pk = try document!.getPublicKey(id)
        guard pk != nil else {
            return false
        }
        return try document!.addAuthenticationKey(pk!)
    }
    
    public func addAuthenticationKey(_ id: String) throws -> Bool {
        return try addAuthenticationKey(DIDURL(subject!, id))
    }
    
    public func addAuthenticationKey(_ id: DIDURL, _ pk: String) throws -> Bool {
        guard Base58.bytesFromBase58(pk).count == HDKey.PUBLICKEY_BYTES else {
            throw DIDError.failue("Invalid public key.")
        }
        let key: DIDPublicKey = DIDPublicKey(id, subject!, pk)
        return try document!.addAuthenticationKey(key)
    }
    
    public func addAuthenticationKey(_ id: String, _ pk: String) throws -> Bool {
        return try addAuthenticationKey(DIDURL(subject!, id), pk)
    }
    
    public func removeAuthenticationKey(_ id: DIDURL) -> Bool {
        return document!.removeAuthenticationKey(id)
    }
    
    public func removeAuthenticationKey(_ id: String) throws -> Bool {
        return try removeAuthenticationKey(DIDURL(subject!, id))
    }
    
    public func addAuthorizationKey(_ id: DIDURL) throws -> Bool {
        let pk = try document!.getPublicKey(id)
        guard pk != nil else {
            return false
        }
        return try document!.addAuthorizationKey(pk!)
    }
    
    public func addAuthorizationKey(_ id: String) throws -> Bool {
        return try  addAuthorizationKey(DIDURL(subject!, id))
    }
    
    public func addAuthorizationKey(_ id: DIDURL, _ controller: DID, _ pk: String) throws -> Bool {
        guard Base58.bytesFromBase58(pk).count == HDKey.PUBLICKEY_BYTES else {
            throw DIDError.malFormedDocumentError(_desc: "Invalid public key.")
        }
        let key: DIDPublicKey = DIDPublicKey(id, controller, pk)
        return try document!.addAuthorizationKey(key)
    }
    
    public func addAuthorizationKey(_ id: String, _ controller: String, _ pk: String) throws -> Bool {
        return try addAuthorizationKey(DIDURL(subject!, id), DID(controller), pk)
    }
    
    public func authorizationDid(_ id: DIDURL, _ controller: DID, key: DIDURL? = nil) throws -> Bool {
        let doc = try controller.resolve()
        if doc == nil {
            return false
        }
        
        // Can not authorize to self
        if controller.isEqual(subject) {
            return false
        }
        var k: DIDURL
        if key == nil {
            k = doc!.getDefaultPublicKey()
        }else {
            k = key!
        }
        let targetPk = try doc!.getAuthenticationKey(k)
        
        // The public key should belongs to controller
        if (targetPk == nil) {
            return false
        }
        
        let pk: DIDPublicKey = DIDPublicKey(id, targetPk!.type, controller, targetPk!.publicKeyBase58)
        return try document!.addAuthorizationKey(pk)
    }
    
    public func authorizationDid(_ id: String, _ controller: String, key: String? = nil) throws -> Bool {
        let controllerId: DID = try DID(controller)
        let keyid = (key == nil ? nil : try DIDURL(controllerId, key!))
        return try authorizationDid(DIDURL(subject!, id), controllerId, key: keyid)
    }
    
    public func removeAuthorizationKey(_ id: DIDURL) -> Bool {
        return document!.removeAuthorizationKey(id)
    }
    
    public func removeAuthorizationKey(_ id: String) throws -> Bool {
        return try removeAuthorizationKey(DIDURL(subject!, id))
    }
    
    public func addCredential(_ vc: VerifiableCredential) -> Bool {
        return document!.addCredential(vc)
    }
    
    public func removeCredential(_ id: DIDURL) -> Bool {
        return document!.removeCredential(id)
    }
    
    public func removeCredential(_ id: String) throws -> Bool {
        return removeCredential(try DIDURL(subject!, id))
    }
    
    public func addService(_ id: DIDURL, _ type: String, _ endpoint: String) throws -> Bool {
        let svc: Service = Service(id, type, endpoint)
        return try document!.addService(svc)
    }
    
    public func addService(_ id: String, _ type: String, _ endpoint: String) throws -> Bool {
        return try addService(DIDURL(subject!, id), type, endpoint)
    }
    
    public func removeService(_ id: DIDURL) -> Bool {
        return document!.removeService(id)
    }
    
    public func removeService(_ id: String) throws -> Bool {
        return removeService(try DIDURL(subject!, id))
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
    
    public func setDefaultExpires() {
        document!.expires = DateFormater.currentDateToWantDate(MAX_VALID_YEARS)
    }
    
    public func setExpires(_ expiresDate: Date) -> Bool {
        let MaxExpires = DateFormater.currentDateToWantDate(MAX_VALID_YEARS)
        if DateFormater.comporsDate(expiresDate, MaxExpires) {
            document!.expires = DateFormater.setExpires(expiresDate)
            return true
        }
        return false
    }
    
    public func seal(storepass: String) throws -> DIDDocument {
        
        if document!.expires == nil {
            setDefaultExpires()
        }
        
        let signKey: DIDURL = document!.getDefaultPublicKey()
        let json = document!.toJson(true, forSign: true)
        let inputs: [CVarArg] = [json, json.count]
        let count = inputs.count / 2
        let sig = try document!.sign(signKey, storepass, count, inputs)
        //        _ = try document!.verify(signKey, sig, count, inputs)
        
        let proof = DIDDocumentProof(signKey, sig)
        document!.proof = proof
        
        // Invalidate builder
        let doc: DIDDocument = document!
        document = nil
        return doc
    }
}
