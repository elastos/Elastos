import Foundation

public class DIDStore: NSObject {
    public static let CACHE_INITIAL_CAPACITY = 16
    public static let CACHE_MAX_CAPACITY = 32
    
    public static let DID_HAS_PRIVATEKEY = 0
    public static let DID_NO_PRIVATEKEY = 1
    public static let DID_ALL = 2

    private var storage: DIDStorage

    init(_ storage: DIDStorage){
        self.storage = storage
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
        return self.storage.containsPrivateIdentity()
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

        guard !storePass.isEmpty else {
            throw DIDError.illegalArgument()
        }
        guard !(try containsPrivateIdentity()) || force else {
            throw DIDError.didStoreError("Already has private indentity.")
        }

        var usedPhrase = passPhrase
        if (usedPhrase == nil) {
            usedPhrase = ""
        }

        let privateIdentity = try HDKey.fromMnemonic(mnemonic, usedPhrase!)

        // Save seed instead of root private key,
        // keep compatible with Native SDK
        let encryptedIdentity = try DIDStore.encryptToBase64(privateIdentity.seed, storePass)
        try storage.storePrivateIdentity(encryptedIdentity)
        
        // Save mnemonic
        let mnemonicData = mnemonic.data(using: .utf8)!
        let encryptedMnemonic = try DIDStore.encryptToBase64(mnemonicData, storePass)
        try storage.storeMnemonic(encryptedMnemonic)

        // Save index
        try storage.storePrivateIdentityIndex(0)
        privateIdentity.wipe()
    }
    
    public func initPrivateIdentity(_ language: Int,
                                    _ mnemonic: String,
                                    _ passPhrase: String?,
                                    _ storePass: String) throws {
        try initPrivateIdentity(language, mnemonic, passPhrase, storePass, false)
    }
    
    public func exportMnemonic(using storePass: String) throws -> String {
        guard !storePass.isEmpty else {
            throw DIDError.illegalArgument()
        }

        let encryptedMnemonic = try storage.loadMnemonic()
        let decryptedMnemonic = try DIDStore.decryptFromBase64(encryptedMnemonic, storePass)
        return String(data: decryptedMnemonic, encoding: .utf8)!
    }

    // initialized from saved private identity in DIDStore.
    func loadPrivateIdentity(_ storePass: String) throws -> HDKey {
        guard try containsPrivateIdentity() else {
            throw DIDError.didStoreError("no private identity contained.")
        }

        let privityIdentity = try storage.loadPrivateIdentity()
        let seed = try DIDStore.decryptFromBase64(privityIdentity, storePass)
        return HDKey(seed)
    }

    public func synchronize(using storePass: String) throws {
        guard !storePass.isEmpty else {
            throw DIDError.illegalArgument()
        }

        let privityIdentity  = try loadPrivateIdentity(storePass)
        let nextIndex = try storage.loadPrivateIdentityIndex()
        var blanks = 0
        var index = 0

        while index < nextIndex || blanks < 10 {
            let key: DerivedKey = try privityIdentity.derive(index++)
            let did = DID(Constants.METHOD, key.getAddress())
            let doc: DIDDocument?

            do {
                doc = try DIDBackend.shareInstance()?.resolve(did, true)
            } catch DIDError.didExpired {
                continue
            } catch DIDError.didDeactivated {
                continue
            }

            if doc != nil {
                //save private key.
                try storePrivateKey(did, doc!.defaultPublicKey, key.serialize(), storePass)
                try storeDid(doc!)

                if index >= nextIndex {
                    try storage.storePrivateIdentityIndex(index)
                }
                blanks = 0
            } else {
                blanks += 1
            }
        }
    }

    public func newDid(_ alias: String?, using storePass: String) throws -> DIDDocument {
        guard !storePass.isEmpty else {
            throw DIDError.illegalArgument()
        }

        let privateIdentity = try loadPrivateIdentity(storePass)
        var nextIndex = try storage.loadPrivateIdentityIndex()
        let key = try privateIdentity.derive(nextIndex++)
        let did = DID(Constants.METHOD, key.getAddress())
        let id  = try DIDURL(did, "primary")

        try storePrivateKey(did, id, key.serialize(), storePass)

        let builder = DIDDocumentBuilder(did, self)
        let doc = try builder.appendAuthenticationKey(id, try key.getPublicKeyBase58())
                             .seal(using: storePass)
        doc.getMeta().setAlias(alias)
        try storeDid(doc)

        try storage.storePrivateIdentityIndex(nextIndex)
        privateIdentity.wipe()
        key.wipe()

        return doc
    }
    
    public func newDid(using storePass: String) throws -> DIDDocument {
        return try newDid(nil, using: storePass)
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

        var usedSignKey = signKey
        if  usedSignKey == nil {
            usedSignKey = doc.defaultPublicKey
        }

        var lastTransactionId = doc.transactionId
        if  lastTransactionId?.isEmpty ?? true {
            lastTransactionId = try DIDBackend.shareInstance()!.create(doc, usedSignKey!, storePass)
        } else {
            lastTransactionId = try DIDBackend.shareInstance()!.update(doc, lastTransactionId!, usedSignKey!, storePass)
        }

        if let _ = lastTransactionId {
            doc.getMeta().setTransactionId(lastTransactionId!)
            try storage.storeDidMeta(doc.subject, doc.getMeta())
        }

        return lastTransactionId ?? ""
    }

    public func publishDid(_ did: String, using signKey: String?, storePass: String) throws -> String {
        let didObj = try DID(did)
        var signKeyObj: DIDURL? = nil
        if let _ = signKey {
            signKeyObj = try DIDURL(didObj, signKey!)
        }

        return try publishDid(DID(did), using: signKeyObj, storePass: storePass)
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

        var usedSignKey = signKey
        if  usedSignKey == nil {
            usedSignKey = doc!.defaultPublicKey
        }

        let transactionId = try DIDBackend.shareInstance()?.deactivate(doc!, usedSignKey!, storePass)

        // Save deactivated status to DID metadata
        if localCopy {
            doc!.getMeta().setDeactivated(true)
            try storage.storeDidMeta(did, doc!.getMeta())
        }

        return transactionId ?? ""
    }
    
    public func deactivateDid(_ did: String, using signKey: String?, storePass :String) throws -> String {
        let didObj = try DID(did)
        var signKeyObj: DIDURL? = nil
        if let _ = signKey {
            signKeyObj = try DIDURL(didObj, signKey!)
        }

        return try deactivateDid(didObj, using: signKeyObj, storePass: storePass)
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
            throw DIDError.didStoreError()
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

        var signPk: PublicKey? = nil
        if let _ = signKey {
            signPk = doc!.authenticationKey(ofId: signKey!)
            guard let _ = signPk else {
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

        // The authorization key id in the target doc
        var targetSignKey: DIDURL? = nil
        var usedSignKey: DIDURL? = signKey
        matchLoop: for targetKey in targetDoc!.authorizationKeys() {
            if targetKey.controller != did {
                continue
            }
            if let _ = signPk {
                if targetKey.publicKeyBase58 != signPk!.publicKeyBase58 {
                    continue
                }

                targetSignKey = targetKey.getId()
                break
            } else {
                for pk in doc!.authenticationKeys() {
                    if pk.publicKeyBase58 == targetKey.publicKeyBase58 {
                        signPk = pk
                        usedSignKey = signPk?.getId()
                        targetSignKey = targetKey.getId()
                    }
                }
            }
        }

        guard let _ = targetSignKey else {
            throw DIDError.didStoreError("no matched authorization key found.")
        }

        return try DIDBackend.shareInstance()?.deactivate(target, targetSignKey!, doc!, usedSignKey!, storePass) ?? ""
    }
    
    public func deactivateDid(_ target: String, with did: String, using signKey: String?, storePass: String) throws -> String {
        let didObj = try DID(did)
        var signKeyObj: DIDURL? = nil
        if let _ = signKey {
            signKeyObj = try DIDURL(didObj, signKey!)
        }

        return try deactivateDid(DID(target), with: didObj, using: signKeyObj!, storePass: storePass)
    }

    public func deactivateDid(_ target: DID, with did: DID, using storePass: String) throws -> String {
        return try deactivateDid(target, with: did, using: nil, storePass: storePass)
    }

    public func storeDid(_ doc: DIDDocument, withAlias: String?) throws {
        doc.getMeta().setAlias(withAlias)
        try storeDid(doc)
    }
    
    public func storeDid(_ doc: DIDDocument) throws {
        try storage.storeDid(doc)

        let meta = try loadDidMeta(doc.subject)
        try meta.merge(doc.getMeta())
        meta.setStore(self)
        doc.setMeta(meta)

        try storage.storeDidMeta(doc.subject, meta)

        for credential in doc.credentials {
            try storeCredential(credential)
        }
    }
    
    func storeDidMeta(_ meta: DIDMeta, for did: DID) throws {
        try storage.storeDidMeta(did, meta)
    }
    
    func storeDidMeta(_ meta: DIDMeta, for did: String) throws {
        try storeDidMeta(meta, for: try DID(did))
    }
    
    func loadDidMeta(_ did: DID) throws -> DIDMeta {
        return try storage.loadDidMeta(did)
    }

    func loadDidMeta(_ did: String) throws -> DIDMeta {
        return try loadDidMeta(DID(did))
    }
    
    public func loadDid(_ did: DID) throws -> DIDDocument {
        let doc = try storage.loadDid(did)
        doc.setMeta(try storage.loadDidMeta(did))
        doc.getMeta().setStore(self)
        return doc
    }

    public func loadDid(_ did: String) throws -> DIDDocument {
        return try loadDid(DID(did))
    }
    
    public func containsDid(_ did: DID) -> Bool {
        return storage.containsDid(did)
    }

    public func containsDid(_ did: String) throws -> Bool {
        return containsDid(try DID(did))
    }

    public func deleteDid(_ did: DID) -> Bool {
        try? storage.deleteDid(did)
        return true
    }

    public func deleteDid(_ did: String) throws -> Bool {
        return try deleteDid(DID(did))
    }

    public func listDids(using filter: Int) throws -> Array<DID> {
        let dids = try storage.listDids(filter)

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
        try storage.storeCredential(credential)

        let meta = try loadCredentialMeta(credential.subject.did, credential.getId())
        try meta.merge(credential.getMeta())
        meta.setStore(self)
        credential.setMeta(meta)
        credential.getMeta().setStore(self)
        try storage.storeCredentialMeta(credential.subject.did, credential.getId(), meta)
    }

    func storeCredentialMeta(_ did: DID, _ id: DIDURL, _ meta: CredentialMeta) throws {
        try storage.storeCredentialMeta(did, id, meta)
    }
    
    func storeCredentialMeta(_ did: String, _ id: String, _ meta: CredentialMeta) throws {
        let didObj = try DID(did)
        try storeCredentialMeta(didObj, try DIDURL(didObj, id), meta)
    }
    
    func loadCredentialMeta(_ did: DID, _ id: DIDURL) throws -> CredentialMeta {
        return try storage.loadCredentialMeta(did, id)
    }

    func loadCredentialMeta(_ did: String, _ id: String) throws -> CredentialMeta? {
        let _did = try DID(did)
        return try loadCredentialMeta(_did, DIDURL(_did, id))
    }

    public func loadCredential(_ did: DID, _ id: DIDURL) throws -> VerifiableCredential? {
        return try storage.loadCredential(did, id)
    }
    
    public func loadCredential(_ did: String, _ id: String) throws -> VerifiableCredential? {
        let _did = try DID(did)
        return try loadCredential(_did, DIDURL(_did, id))
    }
    
    public func containsCredentials(_ did:DID) throws -> Bool {
        return try storage.containsCredentials(did)
    }
    
    public func containsCredentials(_ did: String) throws -> Bool {
        return try containsCredentials(DID(did))
    }
    
    public func containsCredential(_ did: DID, _ id: DIDURL) throws -> Bool {
        return try storage.containsCredential(did, id)
    }
    
    public func containsCredential(_ did: String, _ id: String) throws -> Bool {
        let _did: DID = try DID(did)
        return try containsCredential(_did, DIDURL(_did, id))
    }
    
    public func deleteCredential(_ did: DID , _ id: DIDURL) throws -> Bool{
        return try storage.deleteCredential(did, id)
    }
    
    public func deleteCredential(_ did: String , _ id: String) throws -> Bool{
        let _did: DID = try DID(did)
        return try deleteCredential(_did, DIDURL(_did, id))
    }
    
    public func listCredentials(_ did: DID) throws -> Array<DIDURL> {
        let ids = try storage.listCredentials(did)
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
        return try storage.selectCredentials(did, id, type)
    }
    
    public func selectCredentials(_ did: String, _ id: String,_ type: Array<Any>) throws -> Array<DIDURL> {
        let didObj = try DID(did)
        return try selectCredentials(didObj, DIDURL(didObj, id), type)
    }
    
    public func storePrivateKey(_ did: DID,_ id: DIDURL, _ privateKey: Data, _ storePass: String) throws {
        guard !storePass.isEmpty else {
            throw DIDError.illegalArgument()
        }

        let encryptedKey = try DIDStore.encryptToBase64(privateKey, storePass)
        try storage.storePrivateKey(did, id, encryptedKey)
    }
    
    public func storePrivateKey(_ did: String,_ id: String, _ privateKey: Data, _ storepass: String) throws {
        let didObj: DID = try DID(did)
       try storePrivateKey(didObj, DIDURL(didObj, id), privateKey, storepass)
    }
    
   public func loadPrivateKey(_ did: DID, id: DIDURL) throws -> String {
        return try storage.loadPrivateKey(did, id)
    }
    
    public func containsPrivateKeys(_ did: DID) throws -> Bool {
        return try storage.containsPrivateKeys(did)
    }
    
    public func containsPrivateKeys(_ did: String) throws -> Bool {
        return try containsPrivateKeys(DID(did))
    }
    
    public func containsPrivateKey(_ did: DID,_ id: DIDURL) throws -> Bool {
        return try storage.containsPrivateKey(did, id)
    }
    
    public func containsPrivateKey(_ did: String,_ id: String) throws -> Bool {
        let didObj: DID = try DID(did)
        return try containsPrivateKey(didObj, DIDURL(didObj, id))
    }
    
    public func deletePrivateKey(_ did: DID,_ id: DIDURL) throws -> Bool {
        return try storage.deletePrivateKey(did, id)
    }
    
    public func deletePrivateKey(_ did: String,_ id: String) throws -> Bool {
        let _did: DID = try DID(did)
        return try deletePrivateKey(_did, DIDURL(_did, id))
    }

    private func signEx(_ did: DID, _ id: DIDURL?, _ storePass: String, _ data: [Data]) throws -> String {
        guard !storePass.isEmpty else {
            throw DIDError.illegalArgument()
        }

        var usedId: DIDURL? = id
        if  usedId == nil {
            do {
                let doc = try loadDid(did)
                usedId = doc.defaultPublicKey
            } catch {
                throw DIDError.didStoreError("Can not resolve DID document")
            }
        }

        let binKey = try DIDStore.decryptFromBase64(loadPrivateKey(did, id: usedId!), storePass)
        let key = DerivedKey.deserialize(binKey)!

        // TODO:
        let signature: Data? = nil
        key.wipe()

        return signature?.base64EncodedString() ?? ""
    }

    public func sign(_ did: DID, _ id: DIDURL?, _ storePass: String, _ data: Data...) throws -> String {
        return try signEx(did, id, storePass, data)
    }

    public func sign(_ did: DID, _ storePass: String, _ data: Data...) throws -> String {
        return try signEx(did, nil, storePass, data)
    }
}
