import Foundation

public class DIDStore: NSObject {
    public static let CACHE_INITIAL_CAPACITY = 16
    public static let CACHE_MAX_CAPACITY = 32
    
    public static let DID_HAS_PRIVATEKEY = 0
    public static let DID_NO_PRIVATEKEY = 1
    public static let DID_ALL = 2

    // TODO: didCache
    // TODO: credentialCache
    
    private var _storage: DIDStorage

    init(_ storage: DIDStorage){
        self._storage = storage
    }
    
    public class func open(_ type: String,
                           _ location: String,
                           _ initialCacheCapacity: Int,
                           _ maxCacheCapacity: Int) throws -> DIDStore {
        guard !location.isEmpty else {
            throw DIDError.illegalArgument()
        }
        guard type == "filesystem" else {
            throw DIDError.illegalArgument("Unsupported store type:\(type)")
        }

        return DIDStore(try FileSystemStorage(location))
    }
    
    public class func open(_ type: String,
                           _ location: String) throws -> DIDStore {
        return try open(type, location, CACHE_INITIAL_CAPACITY, CACHE_MAX_CAPACITY)
    }
    
    public func containsPrivateIdentity() throws -> Bool {
        return self._storage.containsPrivateIdentity()
    }

    class func encryptToBase64(_ input: Data, _ storePass: String) throws -> String {
        // TDDO
        return "TODO"
    }

    class func decryptFromBase64(_ input: String, _ storePass: String) throws -> Data {
        // TODO
        return Data()
    }

    // Initialize & create new private identity and save it to DIDStore.
    public func initPrivateIdentity(_ language: Int,
                                    _ mnemonic: String,
                                    _ passPhrase: String?,
                                    _ storePass: String,
                                    _ force: Bool ) throws {
        if !(try Mnemonic.isValid(language, mnemonic)) {
            throw DIDError.illegalArgument("Invalid mnemonic.")
        }

        guard storePass.count > 0 else {
            throw DIDError.illegalArgument()
        }
        guard !(try containsPrivateIdentity()) || force else {
            throw DIDError.didStoreError("Already has private indentity.")
        }

        let _passPhrase: String = (passPhrase != nil) ? passPhrase! : ""
        let privateIdentity = try HDKey.fromMnemonic(mnemonic, _passPhrase)

        // Save seed instead of root private key,
        // keep compatible with Native SDK
        let encryptedIdentity = try DIDStore.encryptToBase64(privateIdentity.seed, storePass)
        try self._storage.storePrivateIdentity(encryptedIdentity)
        
        // Save mnemonic
        let encryptedMnemonic = try DIDStore.encryptToBase64(mnemonic.data(using: .utf8)!, storePass)
        try self._storage.storeMnemonic(encryptedMnemonic)

        // Save index
        try self._storage.storePrivateIdentityIndex(0)

        // TODO: privateIdentity.wipe()
    }
    
    public func initPrivateIdentity(_ language: Int,
                                    _ mnemonic: String,
                                    _ passPhrase: String?,
                                    _ storePass: String) throws {
        try initPrivateIdentity(language, mnemonic, passPhrase, storePass, false)
    }
    
    public func exportMnemonic(using storePass: String) throws -> String {
        guard storePass.count > 0 else {
            throw DIDError.illegalArgument()
        }

        let encryptedMnemonic = try self._storage.loadMnemonic()
        let decryptedMnemonic = try DIDStore.decryptFromBase64(encryptedMnemonic, storePass)
        return String(data: decryptedMnemonic, encoding: .utf8)!
    }

    // initialized from saved private identity in DIDStore.
    func loadPrivateIdentity(_ storePass: String) throws -> HDKey {
        guard try containsPrivateIdentity() else {
            throw DIDError.didStoreError("DID Store contains no private identity.")
        }

        let privityIdentity = try self._storage.loadPrivateIdentity()
        let seed = try DIDStore.decryptFromBase64(privityIdentity, storePass)
        return HDKey(seed)
    }

    public func synchronize(using storePass: String) throws {
        guard !storePass.isEmpty else {
            throw DIDError.illegalArgument()
        }

        let privityIdentity  = try loadPrivateIdentity(storePass)
        let nextIndex = try self._storage.loadPrivateIdentityIndex()
        var blanks = 0
        var index = 0

        while index < nextIndex || blanks < 10 {
            let key: DerivedKey = try privityIdentity.derive(index++)
            let did = DID(Constants.METHOD, key.getAddress())
            let doc: DIDDocument?

            do {
                doc = DIDBackend.shareInstance()?.resolve(did, true)
            } catch {
                // TODO:
                continue
            }

            if doc != nil {
                //save private key.
                storePrivateKey(did, doc?.defaultPublicKey, key.serialize(), storePass)
                try storeDid(doc!)

                if index >= nextIndex {
                    try self._storage.storePrivateIdentityIndex(index)
                }
                blanks = 0
            } else {
                blanks += 1
            }
        }
    }

    public func newDid(_ alias: String, using storePass: String) throws -> DIDDocument {
        guard !storePass.isEmpty else {
            throw DIDError.illegalArgument()
        }

        let privateIdentity = try loadPrivateIdentity(storePass)
        var nextIndex = try self._storage.loadPrivateIdentityIndex()
        let key = try privateIdentity.derive(nextIndex++)
        let did = DID(Constants.METHOD, key.getAddress())
        let id  = try DIDURL(did, "primary")

        try storePrivateKey(did, id, key.serialize(), storePass)

        let builder: DIDDocumentBuilder = DIDDocumentBuilder(did, self)
        _ = try builder.appendAuthenticationKey(id, try key.getPublicKeyBase58())
        let doc = try builder.seal(using: storePass)
        doc.getMeta().setAlias(alias) // TODO
        try storeDid(doc)

        try self._storage.storePrivateIdentityIndex(nextIndex)
        //TODO: privateIdentity.wipe()
        //TODO: key.wipe()

        return doc
    }
    
    public func newDid(using storePass: String) throws -> DIDDocument {
        return try newDid("", using: storePass)
    }
    
    public func publishDid(_ did: DID, using signKey: DIDURL?, storePass: String) throws -> String {
        guard !storePass.isEmpty else {
            throw DIDError.illegalArgument()
        }

        let doc: DIDDocument
        do {
            doc = try loadDid(did)
        } catch {
            throw DIDError.didStoreError("Can not find the document for \(did)")
        }
        guard !doc.isDeactivated else {
            throw DIDError.didStoreError("DID already deactivated.")
        }

        let resolvedDoc = try did.resolve()
        if let _ = resolvedDoc {
            guard !resolvedDoc!.isDeactivated else {
                throw  DIDError.didStoreError("DID already deactivated")
            }
            guard let _ = doc.transactionId else {
                throw DIDError.didStoreError("DID document is not up-to-date")
            }
            guard doc.transactionId! == resolvedDoc!.transactionId else {
                throw DIDError.didStoreError("DID document is not up-to-date")
            }
        }

        let _signKey = (signKey != nil) ? signKey! : doc.defaultPublicKey!
        var lastTransactionId = doc.transactionId
        if  lastTransactionId == nil || lastTransactionId!.isEmpty {
            lastTransactionId = try DIDBackend.shareInstance()!.create(doc, _signKey, storePass)
        } else {
            lastTransactionId = try DIDBackend.shareInstance()!.update(doc, lastTransactionId!, _signKey, storePass)
        }

        if let _ = lastTransactionId {
            // TODO: checkme!!! save transactionId?
            doc.getMeta().setTransactionId(lastTransactionId)
            try self._storage.storeDidMeta(doc.subject, doc.getMeta())
        }

        return lastTransactionId! // TODO: nil if possbile?
    }

    public func publishDid(_ did: String, using signKey: String?, storePass: String) throws -> String {
        let _did = try DID(did)
        let _signKey = (signKey != nil) ? try DIDURL(_did, signKey!) : nil
        return try publishDid(DID(did), using: _signKey, storePass: storePass)
    }

    public func publishDid(_ did: DID, using storePass: String) throws -> String {
        return try publishDid(did, using: nil, storePass: storePass)
    }

    public func publishDid(_ did: String, using storePass: String) throws -> String {
        return try publishDid(DID(did), using: nil, storePass: storePass)
    }
    
    // Deactivate self use authentication keys
    public func deactivateDid(_ did: DID, using signKey: DIDURL?, storePass: String) throws -> String {
        guard !storePass.isEmpty else {
            throw DIDError.didStoreError("Invalid storePass")
        }

        // Document should use the IDChain's copy
        var localCopy = false
        var doc: DIDDocument?

        do {
            doc = try DIDBackend.shareInstance()?.resolve(did)
        } catch {
            throw DIDError.didStoreError("Can not find the document for \(did)")
        }

        if doc == nil {
            // Fail-back: try to load document from local store.
            do {
                doc = try loadDid(did)
            } catch {
                throw DIDError.didStoreError("Can not resolve DID document")
            }
            localCopy = true
        } else {
            doc!.getMeta().setStore(self)
        }

        let _signKey: DIDURL = (signKey != nil) ? signKey! : doc!.defaultPublicKey!
        let transactionId = try DIDBackend.shareInstance()?.deactivate(doc!, _signKey, storePass)

        // Save deactivated status to DID metadata
        if localCopy {
            doc?.getMeta().setDeactivated(true)
            try self._storage.storeDidMeta(did, doc!.getMeta())
        }

        return transactionId!
    }
    
    public func deactivateDid(_ did: String, using signKey: String?, storePass :String) throws -> String {
        let _did = try DID(did)
        let _signKey = (signKey != nil) ? try DIDURL(_did, signKey!) : nil
        return try deactivateDid(_did, using: _signKey, storePass: storePass)
    }

    public func deactivateDid(_ did: DID, using storePass: String) throws -> String {
        return try deactivateDid(did, using: nil, storePass: storePass)
    }

    public func deactivateDid(_ did: String, using storePass: String) throws -> String {
        return try deactivateDid(DID(did), using: nil, storePass: storePass)
    }
    
    // Deactivate target DID with authorization
    public func deactivateDid(_ target: DID, with did: DID, using signKey: DIDURL?, storePass: String) throws -> String {
        guard !storePass.isEmpty else {
            throw DIDError.didStoreError("Invalid storePass")
        }

        // All document should use the IDChain's copy
        var doc: DIDDocument?
        do {
            doc = try DIDBackend.shareInstance()?.resolve(did)
        } catch {
            throw DIDError.didStoreError("Can not find the document for \(did)")
        }

        if doc == nil {
            // Fail-back: try to load document from local store.
            do {
                doc = try loadDid(did)
            } catch {
                throw DIDError.didStoreError("Can not resolve DID document")
            }
        } else {
            doc!.getMeta().setStore(self)
        }

        if let _ = signKey {
            guard doc?.authenticationKey(ofId: signKey!) != nil else {
                throw DIDError.unknownFailure("Not authentication key.") // TODO:
            }
        }

        let targetDoc = try DIDBackend.shareInstance()?.resolve(target)
        guard let _ = targetDoc else {
            throw DIDError.didResolveError("DID \(target) not exist")
        }
        guard targetDoc!.authorizationKeyCount > 0 else {
            throw DIDError.unknownFailure("No authorization.")      // TODO:
        }

        // TODO:
        return "TODO"
    }
    
    public func deactivateDid(_ target: String, with did: String, using signKey: String?, storePass: String) throws -> String {
        let _did = try DID(did)
        let _signKey = (signKey != nil) ? try DIDURL(_did, signKey!) : nil

        return try deactivateDid(DID(target), with: _did, using: _signKey, storePass: storePass)
    }

    public func deactivateDid(_ target: DID, with did: String, using storePass: String) throws -> String {
        return try deactivateDid(target, with: did, using: nil, storePass: storePass)
    }

    public func storeDid(_ doc: DIDDocument, withAlias: String?) throws {
        doc.getMeta().setAlias(withAlias)
        try storeDid(doc)
    }
    
    public func storeDid(_ doc: DIDDocument) throws {
        try self._storage.storeDid(doc)

        let meta = try loadDidMeta(doc.subject)
        try meta.merge(doc.getMeta())
        meta.setStore(self)
        doc.setMeta(meta)

        try self._storage.storeDidMeta(doc.subject, meta)

        for credential in doc.credentials! {
            try storeCredential(credential)
        }
    }
    
    func storeDidMeta(_ meta: DIDMeta, for did: DID) throws {
        try self._storage.storeDidMeta(did, meta)
    }
    
    func storeDidMeta(_ meta: DIDMeta, for did: String) throws {
        try storeDidMeta(meta, for: try DID(did))
    }
    
    func loadDidMeta(_ did: DID) throws -> DIDMeta {
        return try self._storage.loadDidMeta(did)
    }

    func loadDidMeta(_ did: String) throws -> DIDMeta {
        return try loadDidMeta(DID(did))
    }
    
    public func loadDid(_ did: DID) throws -> DIDDocument {
        let doc = try self._storage.loadDid(did)
        doc.setMeta(try self._storage.loadDidMeta(did))
        doc.getMeta().setStore(self)
        return doc
    }

    public func loadDid(_ did: String) throws -> DIDDocument {
        return try loadDid(DID(did))
    }
    
    public func containsDid(_ did: DID) -> Bool {
        return self._storage.containsDid(did)
    }

    public func containsDid(_ did: String) throws -> Bool {
        return containsDid(try DID(did))
    }

    public func deleteDid(_ did: DID) -> Bool {
        return try? self._storage.deleteDid(did) ?? false
    }

    public func deleteDid(_ did: String) throws -> Bool {
        return try deleteDid(DID(did))
    }

    public func listDids(using filter: Int) throws -> Array<DID> {
        let dids = try self._storage.listDids(filter)

        try dids.forEach { did in
            let meta = try loadDidMeta(did)
            meta.setStore(self)
            did.setMeta(meta)
        }

        return dids
    }
    
    public func storeCredential(_ credential: VerifiableCredential, with alias: String?) throws {
        credential.getMeta().setAlias(alias)
        try storeCredential(credential)
    }
    
    public func storeCredential(_ credential: VerifiableCredential) throws {
        try self._storage.storeCredential(credential)

        let meta = try loadCredentialMeta(credential.subject.did, credential.getId())
        try meta.merge(credential.getMeta())
        meta.setStore(self)
        credential.setMeta(meta)
        credential.getMeta().setStore(self) // TODO: really need?
        try self._storage.storeCredentialMeta(credential.subject.did, credential.getId(), meta)
    }

    func storeCredentialMeta(_ did: DID, _ id: DIDURL, _ meta: CredentialMeta) throws {
        try self._storage.storeCredentialMeta(did, id, meta)
    }
    
    func storeCredentialMeta(_ did: String, _ id: String, _ meta: CredentialMeta) throws {
        let _did = try DID(did)
        try storeCredentialMeta(_did, try DIDURL(_did, id), meta)
    }
    
    func loadCredentialMeta(_ did: DID, _ id: DIDURL) throws -> CredentialMeta {
        return try self._storage.loadCredentialMeta(did, id)
    }

    func loadCredentialMeta(_ did: String, _ id: String) throws -> CredentialMeta? {
        let _did = try DID(did)
        return try loadCredentialMeta(_did, DIDURL(_did, id))
    }

    public func loadCredential(_ did: DID, _ id: DIDURL) throws -> VerifiableCredential? {
        return try self._storage.loadCredential(did, id)
    }
    
    public func loadCredential(_ did: String, _ id: String) throws -> VerifiableCredential? {
        let _did: DID = try DID(did)
        return try loadCredential(_did, DIDURL(_did, id))
    }
    
    public func containsCredentials(_ did:DID) throws -> Bool {
        return try self._storage.containsCredentials(did)
    }
    
    public func containsCredentials(_ did: String) throws -> Bool {
        return try containsCredentials(DID(did))
    }
    
    public func containsCredential(_ did: DID, _ id: DIDURL) throws -> Bool {
        return try self._storage.containsCredential(did, id)
    }
    
    public func containsCredential(_ did: String, _ id: String) throws -> Bool {
        let _did: DID = try DID(did)
        return try containsCredential(_did, DIDURL(_did, id))
    }
    
    public func deleteCredential(_ did: DID , _ id: DIDURL) throws -> Bool{
        return try self._storage.deleteCredential(did, id)
    }
    
    public func deleteCredential(_ did: String , _ id: String) throws -> Bool{
        let _did: DID = try DID(did)
        return try deleteCredential(_did, DIDURL(_did, id))
    }
    
    public func listCredentials(_ did: DID) throws -> Array<DIDURL> {
        let ids = try self._storage.listCredentials(did)
        for id in ids {
            let meta = try loadCredentialMeta(did, id)
            meta.setStore(self)
            id.setMeta(meta)
        }
        return ids
    }
    
    public func listCredentials(_ did: String) throws -> Array<DIDURL> {
        return try listCredentials(DID(did))
    }

    public func selectCredentials(_ did: DID, _ id: DIDURL,_ type: Array<Any>) throws -> Array<DIDURL> {
        return try self._storage.selectCredentials(did, id, type)
    }
    
    public func selectCredentials(_ did: String, _ id: String,_ type: Array<Any>) throws -> Array<DIDURL> {
        let _did: DID = try DID(did)
        return try selectCredentials(_did, DIDURL(_did, id), type)
    }
    
    public func storePrivateKey(_ did: DID,_ id: DIDURL, _ privateKey: Data, _ storePass: String) throws {
        guard !storePass.isEmpty else {
            throw DIDError.illegalArgument()
        }

        let encryptedKey = try DIDStore.encryptToBase64(privateKey, storePass)
        try self._storage.storePrivateKey(did, id, encryptedKey)
    }
    
    public func storePrivateKey(_ did: String,_ id: String, _ privateKey: Data, _ storepass: String) throws {
        let _did: DID = try DID(did)
       try storePrivateKey(_did, DIDURL(_did, id), privateKey, storepass)
    }
    
   public func loadPrivateKey(_ did: DID, id: DIDURL) throws -> String {
        return try self._storage.loadPrivateKey(did, id)
    }
    
    public func containsPrivateKeys(_ did: DID) throws -> Bool {
        return try self._storage.containsPrivateKeys(did)
    }
    
    public func containsPrivateKeys(_ did: String) throws -> Bool {
        return try containsPrivateKeys(DID(did))
    }
    
    public func containsPrivateKey(_ did: DID,_ id: DIDURL) throws -> Bool {
        return try self._storage.containsPrivateKey(did, id)
    }
    
    public func containsPrivateKey(_ did: String,_ id: String) throws -> Bool {
        let _did: DID = try DID(did)
        return try containsPrivateKey(_did, DIDURL(_did, id))
    }
    
    public func deletePrivateKey(_ did: DID,_ id: DIDURL) throws -> Bool {
        return try self._storage.deletePrivateKey(did, id)
    }
    
    public func deletePrivateKey(_ did: String,_ id: String) throws -> Bool {
        let _did: DID = try DID(did)
        return try deletePrivateKey(_did, DIDURL(_did, id))
    }

    public func sign(_ did: DID, id: DIDURL? = nil, _ storePass: String, _ count: Int, _ inputs: [CVarArg]) throws -> String {

        guard !storePass.isEmpty else {
            throw DIDError.illegalArgument()
        }

        var useId: DIDURL
        if id == nil {
            do {
                let doc: DIDDocument? = try loadDid(did)
                guard let _ = doc else {
                    throw DIDError.didStoreError("Can not resolve DID document")
                }
                useId = doc!.defaultPublicKey!
            } catch {
                throw DIDError.didStoreError()
            }
        } else {
            useId = id!
        }

        // let binKey = DIDStore.decryptFromBase64(try loadPrivateKey(did, id: useId), storePass))
        // TODO:
    }
}
