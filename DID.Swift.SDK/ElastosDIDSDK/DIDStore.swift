import Foundation

public typealias ConflictHanlder = (_ chainCopy: DIDDocument, _ localCopy: DIDDocument) -> DIDDocument

public class DIDStore: NSObject {
    public static let CACHE_INITIAL_CAPACITY = 16
    public static let CACHE_MAX_CAPACITY = 32
    
    public static let DID_HAS_PRIVATEKEY = 0
    public static let DID_NO_PRIVATEKEY = 1
    public static let DID_ALL = 2

    private var storage: DIDStorage
    private var backend: DIDBackend

    private init(_ initialCacheCapacity: Int,  _ maxCacheCapacity: Int,
                 _ adapter: DIDAdapter, _ storage: DIDStorage) {
        self.backend = DIDBackend.getInstance(adapter)
        self.storage = storage
    }
    
    public class func open(_ type: String,
                           _ location: String,
                           _ initialCacheCapacity: Int,
                           _ maxCacheCapacity: Int,
                           _ adapter: DIDAdapter) throws -> DIDStore {
        guard !location.isEmpty else {
            throw DIDError.illegalArgument()
        }
        guard maxCacheCapacity >= initialCacheCapacity else {
            throw DIDError.illegalArgument()
        }
        guard type == "filesystem" else {
            throw DIDError.illegalArgument("Unsupported store type:\(type)")
        }

        return DIDStore(initialCacheCapacity, maxCacheCapacity, adapter, try FileSystemStorage(location))
    }
    
    public class func open(_ type: String,
                           _ location: String,
                           _ adapter: DIDAdapter) throws -> DIDStore {
        return try open(type, location, CACHE_INITIAL_CAPACITY, CACHE_MAX_CAPACITY, adapter)
    }
    
    public func containsPrivateIdentity() throws -> Bool {
        return storage.containsPrivateIdentity()
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
    public func initializePrivateIdentity(_ language: String,
                                          _ mnemonic: String,
                                          _ passPhrase: String?,
                                          _ storePassword: String,
                                          _ force: Bool ) throws {
        if !(try Mnemonic.isValid(language, mnemonic)) {
            throw DIDError.illegalArgument("Invalid mnemonic.")
        }

        guard !storePassword.isEmpty else {
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
        try initializePrivateIdentity(privateIdentity, storePassword)


        // Save mnemonic
        let mnemonicData = mnemonic.data(using: .utf8)!
        let encryptedMnemonic = try DIDStore.encryptToBase64(mnemonicData, storePassword)
        try storage.storeMnemonic(encryptedMnemonic)
    }
    
    public func initializePrivateIdentity(_ language: String,
                                          _ mnemonic: String,
                                          _ passPhrase: String?,
                                          _ storePassword: String) throws {
        try initializePrivateIdentity(language, mnemonic, passPhrase, storePassword, false)
    }

    public func initializePrivateIdentity(_ extendedPrivateKey: String,
                                          _ storePassword: String,
                                          _ force: Bool) throws {
        // TODO:
    }

    public func initializePrivateIdentity(_ extendedPrivateKey: String,
                                          _ storePassword: String) throws {
        // TODO:
    }

    private func initializePrivateIdentity(_ privateIdentity: HDKey,
                                          _ storePassword: String) throws {
        // TODO:
    }
    
    public func exportMnemonic(using storePassword: String) throws -> String {
        guard !storePassword.isEmpty else {
            throw DIDError.illegalArgument()
        }

        let encryptedMnemonic = try storage.loadMnemonic()
        let decryptedMnemonic = try DIDStore.decryptFromBase64(encryptedMnemonic, storePassword)
        return String(data: decryptedMnemonic, encoding: .utf8)!
    }

    // initialized from saved private identity in DIDStore.
    func loadPrivateIdentity(_ storePassword: String) throws -> HDKey {
        guard try containsPrivateIdentity() else {
            throw DIDError.didStoreError("no private identity contained.")
        }

        let privityIdentity = try storage.loadPrivateIdentity()
        let seed = try DIDStore.decryptFromBase64(privityIdentity, storePassword)
        return HDKey(seed)
    }

    public func synchronize(using storePassword: String, _ mergeHandler: ConflictHanlder) throws {
        guard !storePassword.isEmpty else {
            throw DIDError.illegalArgument()
        }

        let privityIdentity  = try loadPrivateIdentity(storePassword)
        let nextIndex = try storage.loadPrivateIdentityIndex()
        var blanks = 0
        var index = 0

        while index < nextIndex || blanks < 10 {
            let key: DerivedKey = try privityIdentity.derive(index++)
            let did = DID(Constants.METHOD, key.getAddress())
            let doc: DIDDocument?

            do {
                doc = try DIDBackend.resolve(did, true)
            } catch DIDError.didExpired {
                continue
            } catch DIDError.didDeactivated {
                continue
            }

            if doc != nil {
                //save private key.
                try storePrivateKey(did, doc!.defaultPublicKey, key.serialize(), storePassword)
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

    public func synchronize(_ storePassword: String) throws {
        // TODO:
    }

    /*
    public func synchronizeAsync(_ storePassword: String, _ mergeHandler: ConflictHandler) -> Promise<Void> {
        // TODO:
    }

    public func synchronizeAsync(_ storePassword: String) throws -> Promise<Void> {
        // TODO:
    }
     */

    public func newDid(_ index: Int, _ alias: String, using storePassword: String) throws -> DIDDocument {
        guard !storePassword.isEmpty else {
            throw DIDError.illegalArgument()
        }

        let privateIdentity = try loadPrivateIdentity(storePassword)
        var nextIndex = try storage.loadPrivateIdentityIndex()
        let key = try privateIdentity.derive(nextIndex++)
        let did = DID(Constants.METHOD, key.getAddress())
        let id  = try DIDURL(did, "primary")

        try storePrivateKey(did, id, key.serialize(), storePassword)

        let builder = DIDDocumentBuilder(did, self)
        let doc = try builder.appendAuthenticationKey(id, try key.getPublicKeyBase58())
                             .sealed(using: storePassword)
        doc.getMeta().setAlias(alias)
        try storeDid(doc)

        try storage.storePrivateIdentityIndex(nextIndex)
        privateIdentity.wipe()
        key.wipe()

        return doc
    }
    
    public func newDid(_ index: Int, using storePassword: String) throws -> DIDDocument {
        return try newDid(index, "", using: storePassword)
    }

    public func newDid(_ alias: String, using storePassword: String) throws -> DIDDocument {
        var nextIndex = try storage.loadPrivateIdentityIndex()
        let doc = try newDid(nextIndex++, alias, using: storePassword)
        try storage.storePrivateIdentityIndex(nextIndex)

        return doc
    }

    public func newDid(using storePassword: String) throws -> DIDDocument {
        return try newDid("", using: storePassword)
    }

    public func getDid(_ index: Int, using storePassword: String) throws -> DID {
        guard !storePassword.isEmpty else {
            throw DIDError.illegalArgument()
        }

        let privateIdentity = try loadPrivateIdentity(storePassword)
        let key = try privateIdentity.derive(index)
        let did = DID(Constants.METHOD, key.getAddress())

        privateIdentity.wipe()
        key.wipe()
        return did
    }

    public func publishDid(_ did: DID, _ confirms: Int,
                   using signKey: DIDURL?,
                         _ force: Bool,
                   storePassword: String) throws -> String {
        guard !storePassword.isEmpty else {
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

        let resolvedDoc = did.resolve()
        if let _ = resolvedDoc {
            guard !resolvedDoc!.isDeactivated else {
                throw  DIDError.didStoreError("DID already deactivated")
            }

            /*
              TODO:
             */
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
            lastTransactionId = try backend.create(doc, usedSignKey!, storePassword)
        } else {
            lastTransactionId = try backend.update(doc, lastTransactionId!, usedSignKey!, storePassword)
        }

        if let _ = lastTransactionId {
            doc.getMeta().setTransactionId(lastTransactionId!)
            try storage.storeDidMeta(doc.subject, doc.getMeta())
        }

        return lastTransactionId ?? ""
    }

    public func publishDid(_ did: DID, _ confirms: Int,
                   using signKey: DIDURL?,
                   storePassword: String) throws -> String {
        return try publishDid(did, confirms, using: signKey, false, storePassword: storePassword)
    }

    public func publishDid(_ did: String, _ confirms: Int,
                   using signKey: String?,
                         _ force: Bool,
                   storePassword: String) throws -> String {
        let didObj = try DID(did)
        var signKeyObj: DIDURL? = nil
        if let _ = signKey {
            signKeyObj = try DIDURL(didObj, signKey!)
        }

        return try publishDid(DID(did), confirms, using: signKeyObj, force, storePassword: storePassword)
    }

    public func publishDid(_ did: String, _ confirms: Int,
                   using signKey: String?,
                   storePassword: String) throws -> String {
        return try publishDid(did, confirms, using: signKey, false, storePassword: storePassword)
    }

    public func publishDid(_ did: DID,
                   using signKey: DIDURL?,
                   storePassword: String) throws -> String {
        return try publishDid(did, 0, using: signKey, storePassword: storePassword)
    }

    public func publishDid(_ did: String,
                   using signKey: String?,
                   storePassword: String) throws -> String {
        return try publishDid(did, 0, using: signKey, storePassword: storePassword)
    }

    public func publishDid(_ did: DID, _ confirms: Int,
             using storePassword: String) throws -> String {
        return try publishDid(did, confirms, using: nil, storePassword: storePassword)
    }

    public func publishDid(_ did: String, _ confirms: Int,
             using storePassword: String) throws -> String {
        return try publishDid(did, confirms, using: nil, storePassword: storePassword)
    }

    public func publishDid(_ did: DID,
             using storePassword: String) throws -> String {
        return try publishDid(did, 0, using: storePassword)
    }

    public func publishDid(_ did: String,
             using storePassword: String) throws -> String {
        return try publishDid(did, 0, using: storePassword)
    }

    /*
    public func publishDidAsync(_ did: DID, _ confirms: Int,
                        using signKey: DIDURL?,
                              _ force: Bool,
                        storePassword: String) -> Promise<String> {
        // TODO:
    }

    public func publishDidAsync(_ did: String, _ confirms: Int,
                        using signKey: String?,
                              _ force: Bool,
                        storePassword: String) -> Promise<String> {
        // TODO:
    }

    public func publishDidAsync(_ did: DID, _ confirms: Int,
                        using signKey: DIDURL?,
                        storePassword: String) -> Promise<String> {
        // TODO:
    }

    public func publishDidAsync(_ did: String, _ confirms: Int,
                        using signKey: String?,
                        storePassword: String) -> Promise<String> {
        // TODO:
    }

    public func publishDidAsync(_ did: DID,
                        using signKey: DIDURL?,
                        storePassword: String) -> Promise<String> {
        // TODO:
    }

    public func publishDidAsync(_ did: String,
                        using signKey: String?,
                        storePassword: String) -> Promise<String> {
        // TODO:
    }

    public func publishDidAsync(_ did: DID, _ confirms: Int,
                  using storePassword: String) -> Promise<String> {
        // TODO:
    }

    public func publishDidAsync(_ did: String, _ confirms: Int,
                  using storePassword: String) -> Promise<String> {
        // TODO:
    }

    public func publishDidAsync(_ did: DID,
                  using storePassword: String) -> Promise<String> {
        // TODO:
    }

    public func publishDidAsync(_ did: String,
                  using storePassword: String) -> Promise<String> {
        // TODO:
    }
    */

    // Deactivate self use authentication keys
    public func deactivateDid(_ did: DID, _ confirms: Int,
                      using signKey: DIDURL?,
                      storePassword: String) throws -> String {
        guard !storePassword.isEmpty else {
            throw DIDError.didStoreError("Invalid storePass")
        }

        // Document should use the IDChain's copy
        var localCopy = false
        var doc: DIDDocument?
        do {
            doc = try DIDBackend.resolve(did)
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

        let transactionId = try backend.deactivate(doc!, usedSignKey!, storePassword)

        // Save deactivated status to DID metadata
        if localCopy {
            doc!.getMeta().setDeactivated(true)
            try storage.storeDidMeta(did, doc!.getMeta())
        }

        return transactionId
    }
    
    public func deactivateDid(_ did: String, _ confirms: Int,
                      using signKey: String?,
                      storePassword: String) throws -> String {
        let didObj = try DID(did)
        var signKeyObj: DIDURL? = nil
        if let _ = signKey {
            signKeyObj = try DIDURL(didObj, signKey!)
        }

        return try deactivateDid(didObj, confirms, using: signKeyObj, storePassword: storePassword)
    }

    public func deactivateDid(_ did: DID,
                      using signKey: DIDURL?,
                      storePassword: String) throws -> String {
        return try deactivateDid(did, 0, using: signKey, storePassword: storePassword)
    }

    public func deactivateDid(_ did: String,
                      using signKey: String?,
                      storePassword: String) throws -> String {
        return try deactivateDid(did, 0, using: signKey, storePassword: storePassword)
    }

    public func deactivateDid(_ did: DID, _ confirms: Int,
                using storePassword: String) throws -> String {
        return try deactivateDid(did, confirms, using: nil, storePassword: storePassword)
    }

    public func deactivateDid(_ did: String, _confirms: Int,
                using storePassword: String) throws -> String {
        return try deactivateDid(did, _confirms, using: nil, storePassword: storePassword)
    }

    public func deactivateDid(_ did: DID,
                using storePassword: String) throws -> String {
        return try deactivateDid(did, using: nil, storePassword: storePassword)
    }

    public func deactivateDid(_ did: String,
                using storePassword: String) throws -> String {
        return try deactivateDid(DID(did), using: nil, storePassword: storePassword)
    }

    /*
    public func deactivateDidAsync(_ did: DID, _ confirms: Int,
                           using signKey: DIDURL?,
                           storePassword: String) -> Promise<String> {
        // TODO:
    }

    public func deactivateDidAsync(_ did: String, _ confirms: Int,
                           using signKey: String?,
                           storePassword: String) -> Promise<String> {
        // TODO:
    }

    public func deactivateDidAsync(_ did: DID,
                           using signKey: DIDURL?,
                           storePassword: String) -> Promise<String> {
        // TODO:
    }

    public func deactivateDidAsync(_ did: String,
                           using signKey: String?,
                           storePassword: String) -> Promise<String> {
        // TODO:
    }

    public func deactivateDidAsync(_ did: DID, _ confirms: Int,
                     using storePassword: String) -> Promise<String> {
        // TODO:
    }

    public func deactivateDidAsync(_ did: String, _ confirms: Int,
                     using storePassword: String) -> Promise<String> {
        // TODO:
    }

    public func deactivateDidAsync(_ did: DID,
                     using storePassword: String) -> Promise<String> {
        // TODO:
    }

    public func deactivateDidAsync(_ did: String,
                     using storePassword: String) -> Promise<String> {
        // TODO:
    }

    */
    
    // Deactivate target DID with authorization
    public func deactivateDid(_ target: DID,
                              with did: DID,
                            _ confirms: Int,
                         using signKey: DIDURL?,
                         storePassword: String) throws -> String {
        guard !storePassword.isEmpty else {
            throw DIDError.didStoreError()
        }

        // All document should use the IDChain's copy
        var doc: DIDDocument?
        do {
            doc = try DIDBackend.resolve(did)
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

        let targetDoc = try DIDBackend.resolve(target)
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

        return try backend.deactivate(target, targetSignKey!, doc!, usedSignKey!, storePassword)
    }
    
    public func deactivateDid(_ target: String,
                              with did: String,
                            _ confirms: Int,
                         using signKey: String?,
                         storePassword: String) throws -> String {
        let didObj = try DID(did)
        var signKeyObj: DIDURL? = nil
        if let _ = signKey {
            signKeyObj = try DIDURL(didObj, signKey!)
        }

        return try deactivateDid(DID(target), with: didObj, confirms, using: signKeyObj!,
                                 storePassword: storePassword)
    }

    public func deactivateDid(_ target: DID,
                              with did: DID,
                         using signKey: DIDURL?,
                         storePassword: String) throws -> String {
        return try deactivateDid(target, with: did, 0, using: signKey, storePassword: storePassword)
    }

    public func deactivateDid(_ target: String,
                              with did: String,
                         using signKey: String?,
                         storePassword: String) throws -> String {
        return try deactivateDid(target, with: did, 0, using: signKey, storePassword: storePassword)
    }

    public func deactivateDid(_ target: DID,
                              with did: DID,
                            _ confirms: Int,
                   using storePassword: String) throws -> String {
        return try deactivateDid(target, with: did, confirms, using: nil, storePassword: storePassword)
    }

    public func deactivateDid(_ target: String,
                              with did: String,
                            _ confirms: Int,
                   using storePassword: String) throws -> String {
        return try deactivateDid(target, with: did, confirms, using: nil, storePassword: storePassword)
    }

    public func deactivateDid(_ target: DID,
                              with did: DID,
                   using storePassword: String) throws -> String {
        return try deactivateDid(target, with: did, 0, using: nil, storePassword: storePassword)
    }

    public func deactivateDid(_ target: String,
                              with did: String,
                   using storePassword: String) throws -> String {
        return try deactivateDid(target, with: did, 0, using: nil, storePassword: storePassword)
    }

    /*
    public func deactivateDidAsync(_ target: DID,
                                   with did: DID,
                                 _ confirms: Int,
                              using signKey: DIDURL?,
                              storePassword: String) -> Promise<String> {
        // TODO:
    }

    public func deactivateDidAsync(_ target: String,
                                   with did: String,
                                 _ confirms: Int,
                              using signKey: String?,
                              storePassword: String) -> Promise<String> {
        // TODO:
    }

    public func deactivateDidAsync(_ target: DID,
                                   with did: DID,
                              using signKey: DIDURL?,
                              storePassword: String) -> Promise<String> {
        // TODO:
    }

    public func deactivateDidAsync(_ target: String,
                                   with did: String,
                              using signKey: String?,
                              storePassword: String) -> Promise<String> {
        // TODO:
    }

    public func deactivateDidAsync(_ target: DID,
                                   with did: DID,
                                 _ confirms: Int,
                        using storePassword: String) -> Promise<String> {
        // TODO:
    }

    public func deactivateDidAsync(_ target: String,
                                   with did: String,
                                 _ confirms: Int,
                        using storePassword: String) -> Promise<String> {
        // TODO:
    }

    public func deactivateDidAsync(_ target: DID,
                                   with did: DID,
                        using storePassword: String) -> Promise<String> {
        // TODO:
    }

    public func deactivateDidAsync(_ target: String,
                                   with did: String,
                        using storePassword: String) -> Promise<String> {
        // TODO:
    }
    */

    public func storeDid(_ doc: DIDDocument, with alias: String) throws {
        doc.getMeta().setAlias(alias)
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

    func makeSignWithIdentity(_ did: DID, _ id: DIDURL?, _ storePassword: String, _ data: [Data]) throws -> String {
        guard !storePassword.isEmpty else {
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

        let binKey = try DIDStore.decryptFromBase64(loadPrivateKey(did, id: usedId!), storePassword)
        let key = DerivedKey.deserialize(binKey)!

        // TODO:
        let signature: Data? = nil
        key.wipe()

        return signature?.base64EncodedString() ?? ""
    }

    public func makeSignWithIdentity(did: DID, id: DIDURL?, using storePassword: String, data: Data...) throws -> String {
        return try makeSignWithIdentity(did, id, storePassword, data)
    }

    public func makeSignWithIdentity(did: DID, using storePassword: String, _ data: Data...) throws -> String {
        return try makeSignWithIdentity(did, nil, storePassword, data)
    }

    private func exportDid(_ did: DID,
                    _ generator: JsonGenerator,
                     _ password: String,
                _ storePassword: String) throws {
        // TODO:
    }

    public func exportDid(_ did: DID,
                      to output: OutputStream,
                 using password: String,
                  storePassword: String) throws {
        // TODO:
    }

    public func exportDid(_ did: String,
                      to output: OutputStream,
                 using password: String,
                  storePassword: String) throws {
        // TODO:
    }

    public func exportDid(_ did: DID,
                  to fileHandle: FileHandle,
                 using password: String,
                  storePassword: String) throws {
        // TODO:
    }

    public func exportDid(_ did: String,
                  to fileHandle: FileHandle,
                 using password: String,
                  storePassword: String) throws {
        // TODO:
    }

    private func importDid(_ root: JsonNode,
                       _ password: String,
                  _ storePassword: String) throws {
        // TODO:
    }

    public func importDid(from data: Data,
                     using password: String,
                      storePassword: String) throws {
        // TODO:
    }

    public func importDid(from input: InputStream,
                      using password: String,
                       storePassword: String) throws {
        // TODO:
    }

    public func importDid(from handle: FileHandle,
                      using password: String,
                       storePassword: String) throws {
        // TODO:
    }

    private func exportPrivateIdentity(_ generator: JsonGenerator,
                                        _ password: String,
                                   _ storePassword: String) throws {
        // TODO:
    }

    public func exportPrivateIdentity(to output: OutputStream,
                                     _ password: String,
                                _ storePassword: String) throws {
        // TODO:
    }

    public func exportPrivateIdentity(to handle: FileHandle,
                                     _ password: String,
                                _ storePassword: String) throws {
        // TODO:
    }

    public func exportPrivateIdentity(to data: Data,
                                   _ password: String,
                              _ storePassword: String) throws {
        // TODO:
    }

    private func importPrivateIdentity(_ root: JsonNode,
                                   _ password: String,
                              _ storePassword: String) throws {
        // TODO:
    }

    public func importPrivateIdentity(from data: Data,
                                 using password: String,
                                  storePassword: String) throws {
        // TODO:
    }

    public func importPrivateIdentity(from input: InputStream,
                                  using password: String,
                                   storePassword: String) throws {
        // TODO:
    }

    public func importPrivateIdentity(from handle: FileHandle,
                                   using password: String,
                                    storePassword: String) throws {
        // TODO:
    }

    public func exportStore(to output: OutputStream,
                           _ password: String,
                      _ storePassword: String) throws {
        // TODO:
    }

    public func exportStore(to handle: FileHandle,
                           _ password: String,
                      _ storePassword: String) throws {
        // TODO:
    }

    public func importStore(from input: InputStream,
                            _ password: String,
                       _ storePassword: String) throws {
        // TODO:
    }

    public func importStore(from handle: FileHandle,
                             _ password: String,
                        _ storePassword: String) throws {
        // TODO:
    }
}
