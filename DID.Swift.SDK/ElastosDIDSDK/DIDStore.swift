import Foundation
import PromiseKit

public typealias ConflictHandler = (_ chainCopy: DIDDocument, _ localCopy: DIDDocument) throws -> DIDDocument

public class DIDStore: NSObject {
    private static let TAG = "DIDStore"
    public static let CACHE_INITIAL_CAPACITY = 16
    public static let CACHE_MAX_CAPACITY = 32
    
    public static let DID_HAS_PRIVATEKEY = 0
    public static let DID_NO_PRIVATEKEY = 1
    public static let DID_ALL = 2

    private var documentCache: LRUCache<DID, DIDDocument>?
    private var credentialCache: LRUCache<DIDURL, VerifiableCredential>?
    private let DID_EXPORT = "did.elastos.export/1.0"

    private var storage: DIDStorage
    private var backend: DIDBackend

    private init(_ initialCapacity: Int, _ maxCapacity: Int, _ adapter: DIDAdapter, _ storage: DIDStorage) {
        if maxCapacity > 0 {
            documentCache = LRUCache<DID, DIDDocument>(initialCapacity, maxCapacity)
            credentialCache = LRUCache<DIDURL, VerifiableCredential>(initialCapacity, maxCapacity)
        }

        self.backend = DIDBackend.getInstance(adapter)
        self.storage = storage
    }

    private class func openStore(_ path: String,
                                 _ type: String,
                                 _ initialCacheCapacity: Int,
                                 _ maxCacheCapacity: Int,
                                 _ adapter: DIDAdapter) throws -> DIDStore {
        guard !type.isEmpty else {
            throw DIDError.illegalArgument("type is empty")
        }

        guard !path.isEmpty else {
            throw DIDError.illegalArgument("location is empty")
        }

        guard maxCacheCapacity >= initialCacheCapacity else {
            throw DIDError.illegalArgument()
        }

        guard type == "filesystem" else {
            throw DIDError.illegalArgument("Unsupported store type:\(type)")
        }

        let storage = try FileSystemStorage(path)
        return DIDStore(initialCacheCapacity, maxCacheCapacity, adapter, storage)
    }

    public class func open(atPath: String,
                         withType: String,
             initialCacheCapacity: Int,
                 maxCacheCapacity: Int,
                          adapter: DIDAdapter) throws -> DIDStore {

        return try openStore(atPath, withType, initialCacheCapacity, maxCacheCapacity, adapter)
    }

    public class func open(atPath: String,
                         withType: String,
                          adapter: DIDAdapter) throws -> DIDStore {

        return try openStore(atPath, withType, CACHE_INITIAL_CAPACITY, CACHE_MAX_CAPACITY, adapter)
    }

    public func containsPrivateIdentity() -> Bool {
        return storage.containsPrivateIdentity()
    }

    class func encryptToBase64(_ input: Data, _ storePassword: String) throws -> String {
        let cinput: UnsafePointer<UInt8> = input.withUnsafeBytes{ (by: UnsafePointer<UInt8>) -> UnsafePointer<UInt8> in
            return by
        }
        let capacity = input.count * 3
        let base64url: UnsafeMutablePointer<Int8> = UnsafeMutablePointer.allocate(capacity: capacity)
        let re = encrypt_to_base64(base64url, storePassword, cinput, input.count)
        guard re >= 0 else {
            throw DIDError.didStoreError("encryptToBase64 error.")
        }
        base64url[re] = 0
        return String(cString: base64url)
    }

    class func decryptFromBase64(_ input: String, _ storePassword: String) throws -> Data {
        let capacity = input.count * 3
        let plain: UnsafeMutablePointer<UInt8> = UnsafeMutablePointer<UInt8>.allocate(capacity: capacity)
        let re = decrypt_from_base64(plain, storePassword, input)
        guard re >= 0 else {
            throw DIDError.didStoreError("decryptFromBase64 error.")
        }
        let temp = UnsafeRawPointer(plain)
            .bindMemory(to: UInt8.self, capacity: re)
        
        let data = Data(bytes: temp, count: re)
        //        let intArray = [UInt8](data).map { Int8(bitPattern: $0) }
        return data
    }

    // Initialize & create new private identity and save it to DIDStore.
    private func initializePrivateIdentity(_ language: String,
                                           _ mnemonic: String,
                                           _ passphrase: String?,
                                           _ storePassword: String,
                                           _ force: Bool ) throws {
        guard try Mnemonic.isValid(language, mnemonic) else {
            throw DIDError.illegalArgument("Invalid mnemonic.")
        }

        guard !storePassword.isEmpty else {
            throw DIDError.illegalArgument("Invalid password.")
        }

        guard !containsPrivateIdentity() || force else {
            throw DIDError.didStoreError("Already has private indentity.")
        }

        var usedPhrase = passphrase
        if (usedPhrase == nil) {
            usedPhrase = ""
        }

        let privateIdentity = HDKey(mnemonic, usedPhrase!, language)
        try initializePrivateIdentity(privateIdentity, storePassword)

        // Save mnemonic
        let mnemonicData = mnemonic.data(using: .utf8)!
        let encryptedMnemonic = try DIDStore.encryptToBase64(mnemonicData, storePassword)
        try storage.storeMnemonic(encryptedMnemonic)
    }

    public func initializePrivateIdentity(using language: String,
                                                mnemonic: String,
                                              passphrase: String,
                                           storePassword: String,
                                                 _ force: Bool) throws {

        try initializePrivateIdentity(language, mnemonic, passphrase, storePassword, force)
    }

    public func initializePrivateIdentity(using language: String,
                                                mnemonic: String,
                                           storePassword: String,
                                                 _ force: Bool ) throws {
        try initializePrivateIdentity(language, mnemonic, nil, storePassword, false)
    }

    public func initializePrivateIdentity(using language: String,
                                                mnemonic: String,
                                              passPhrase: String,
                                           storePassword: String) throws {
        try initializePrivateIdentity(language, mnemonic, passPhrase, storePassword, false)
    }

    public func initializePrivateIdentity(using language: String,
                                                mnemonic: String,
                                           storePassword: String) throws {
        try initializePrivateIdentity(language, mnemonic, nil, storePassword, false)
    }

    private func initializePrivateIdentity(_ privateIdentity: HDKey,
                                           _ storePassword: String) throws {
        // Save extended root private key
        let encryptedIdentity = try DIDStore.encryptToBase64(privateIdentity.serialize(), storePassword)
        try storage.storePrivateIdentity(encryptedIdentity)

        // Save pre-derived public key
        let preDerivedKey = try privateIdentity.derive(HDKey.PRE_DERIVED_PUBLICKEY_PATH)
        try storage.storePublicIdentity(preDerivedKey.serializePublicKeyBase58())

        // Save index
        try storage.storePrivateIdentityIndex(0)
        preDerivedKey.wipe()
        privateIdentity.wipe()
    }

    private func initializePrivateIdentity(_ extendedPrivateKey: String,
                                           _ storePassword: String,
                                           _ force: Bool) throws {
        guard !extendedPrivateKey.isEmpty else {
            throw DIDError.illegalArgument()
        }

        guard !storePassword.isEmpty else {
            throw DIDError.illegalArgument()
        }

         guard !containsPrivateIdentity() || force else {
         throw DIDError.didStoreError("Already has private indentity.")
         }
         let privateIdentity = HDKey.deserializeBase58(extendedPrivateKey)
         try initializePrivateIdentity(privateIdentity, storePassword)
    }

    public func initializePrivateIdentity(using extendedPrivateKey: String,
                                                     storePassword: String,
                                                           _ force: Bool ) throws {
        return try initializePrivateIdentity(extendedPrivateKey, storePassword, force)
    }

    public func initializePrivateIdentity(using extentedPrivateKey: String,
                                               storePassword: String) throws {
        try initializePrivateIdentity(using: extentedPrivateKey, storePassword: storePassword, false)
    }

    public func exportMnemonic(using storePassword: String) throws -> String {
        guard !storePassword.isEmpty else {
            throw DIDError.illegalArgument("Invalid password.")
        }

        if storage.containsMnemonic() {
            let encryptedMnemonic = try storage.loadMnemonic()
            let decryptedMnemonic = try DIDStore.decryptFromBase64(encryptedMnemonic, storePassword)
            return String(data: decryptedMnemonic, encoding: .utf8)!
        }
        else {
            throw DIDError.didStoreError("DID store doesn't contain mnemonic.")
        }
    }

    // initialized from saved private identity from DIDStore.
    func loadPrivateIdentity(_ storePassword: String) throws -> HDKey {
        guard containsPrivateIdentity() else {
            throw DIDError.didStoreError("no private identity contained")
        }

        let privateIdentity: HDKey?
        var keyData = try DIDStore.decryptFromBase64(storage.loadPrivateIdentity(), storePassword)
        defer {
            keyData.removeAll()
        }

        if  keyData.count == HDKey.SEED_BYTES {
            // For backward compatible, convert to extended root private key
            // TODO: Should be remove in the future
            privateIdentity = HDKey(keyData)

            // convert to extended root private key.
            let encryptedIdentity = try DIDStore.encryptToBase64(privateIdentity!.serialize(), storePassword)
            try storage.storePrivateIdentity(encryptedIdentity)
        } else if keyData.count == HDKey.EXTENDED_PRIVATEKEY_BYTES {
            privateIdentity = HDKey.deserialize(keyData)
        } else {
            throw DIDError.didStoreError("invalid private identity")
        }

        // For backward compatible, create pre-derived public key if not exist.
        // TODO: Should be remove in the future
        if (!storage.containsPublicIdentity()) {
            let preDerivedKey = try privateIdentity!.derive(HDKey.PRE_DERIVED_PUBLICKEY_PATH)
            try storage.storePublicIdentity(preDerivedKey.serializePublicKeyBase58())
        }
        return privateIdentity!
    }

    func loadPublicIdentity() throws -> HDKey {
        guard containsPrivateIdentity() else {
            throw DIDError.didStoreError("no private identity contained")
        }

        let keyData = try storage.loadPublicIdentity()
        let publicIdentity = HDKey.deserializeBase58(keyData)

        return publicIdentity
    }

    private func synchronize(_ storePassword: String,
                             _ conflictHandler: ConflictHandler) throws {

        guard !storePassword.isEmpty else {
            throw DIDError.illegalArgument()
        }

        let nextIndex = try storage.loadPrivateIdentityIndex()
        let privateIdentity: HDKey?
        privateIdentity = try loadPrivateIdentity(storePassword)
        if privateIdentity == nil {
            throw DIDError.didStoreError("DID Store does not contains private identity.")
        }

        var blanks = 0
        var i = 0

        while i < nextIndex || blanks < 20 {
            let path = HDKey.DERIVE_PATH_PREFIX + "\(i)"
            let key: HDKey = try privateIdentity!.derive(path)
            i += 1
            let did = DID(Constants.METHOD, key.getAddress())

            Log.i(DIDStore.TAG, "Synchronize {}/{}... \(did.toString()) \(i)")

            let chainCopy: DIDDocument?
            do {
                chainCopy = try DIDBackend.resolve(did, true)
            } catch DIDError.didExpired {
                Log.d(DIDStore.TAG, "{} is {}, skip. \(did.toString()) expired")
                blanks = 0
                continue
            } catch DIDError.didDeactivated {
                Log.d(DIDStore.TAG, "{} is {}, skip. \(did.toString()) deactivated")
                blanks = 0
                continue
            }

            if let _ = chainCopy {
                Log.d(DIDStore.TAG, "{}\(did.toString()) exists, got the on-chain copy.")
                var finalCopy: DIDDocument? = chainCopy!
                var localCopy: DIDDocument?

                do {
                    localCopy = try loadDid(did)
                } catch {
                    localCopy = nil
                }

                if let _ = localCopy {
                    if  localCopy!.getMetadata().signature == nil ||
                        localCopy!.proof.signature != localCopy!.getMetadata().signature {

                        Log.d(DIDStore.TAG, "{}\(did.toString()) on-chain copy conflict with local copy.")
                        // local copy was modified.
                        do {
                            finalCopy = try conflictHandler(chainCopy!, localCopy!)
                        } catch {
                            finalCopy = nil
                        }

                        if finalCopy == nil || finalCopy?.subject != did {
                            Log.e(DIDStore.TAG, "Conflict handle merge the DIDDocument error.")
                            throw DIDError.didStoreError("deal with local modification error.")
                        }
                        else {
                            Log.d(DIDStore.TAG, "Conflict handle return the final copy.")
                        }
                    }
                }

                // save private key
                try storePrivateKey(for: did, id: finalCopy!.defaultPublicKey, privateKey: key.serialize(), using: storePassword)
                try storeDid(using: finalCopy!)

                if i >= nextIndex {
                    try storage.storePrivateIdentityIndex(i)
                }
                blanks = 0
            } else {
                if i >= nextIndex {
                    blanks += 1
                }
            }
            do {
                key.wipe()
            }
        }
        do {
            privateIdentity?.wipe()
        }
    }

    public func synchronize(using storePassword: String,
                                conflictHandler: ConflictHandler) throws {

        return try synchronize(storePassword, conflictHandler)
    }

    public func synchronize(using storePassword: String) throws {

            try synchronize(storePassword) { (c, l) throws -> DIDDocument in
            l.getMetadata().setPublished(c.getMetadata().getPublished()!)
            l.getMetadata().setSignature(c.getMetadata().signature)
            return l
        }
    }

    private func synchronizeAsync(_ storePassword: String,
                                  _ conflictHandler: ConflictHandler) -> Promise<Void> {
        return Promise<Void> { resolver in
            do {
                try synchronize(storePassword, conflictHandler)
                resolver.fulfill(())
            } catch let error  {
                resolver.reject(error)
            }
        }
    }

    public func synchornizeAsync(using storePassword: String,
                                     conflictHandler: ConflictHandler) -> Promise<Void> {

        return synchronizeAsync(storePassword, conflictHandler)
    }

    public func synchronizeAsync(using storePassword: String) throws -> Promise<Void> {

        guard !storePassword.isEmpty else {
            throw DIDError.illegalArgument("store password is empty")
        }
        return Promise<Void> { resolver in
            do {
                try synchronize(using: storePassword)
                resolver.fulfill(())
            } catch let error  {
                resolver.reject(error)
            }
        }
    }

    private func newDid(_ privateIdentityIndex: Int,
                        _ alias: String?,
                        _ storePassword: String) throws -> DIDDocument {
        guard privateIdentityIndex >= 0 else {
            throw DIDError.illegalArgument("invalid index")
        }

        guard !storePassword.isEmpty else {
            throw DIDError.illegalArgument("storePassword is empty ")
        }

        let privateIdentity = try loadPrivateIdentity(storePassword)
        let path = HDKey.DERIVE_PATH_PREFIX + "\(privateIdentityIndex)"
        let key = try privateIdentity.derive(path)

        defer {
            privateIdentity.wipe()
            key.wipe()
        }

        let did = DID(Constants.METHOD, key.getAddress())
        Log.i(DIDStore.TAG, "Creating new DID {\(did.toString())} with index {\(privateIdentityIndex)}...")
        var doc = try loadDid(did)
        if doc != nil {
            throw DIDError.didStoreError("DID already exists.")
        }

        let id  = try DIDURL(did, "primary")
        try storePrivateKey(for: did, id: id, privateKey: key.serialize(), using: storePassword)

        let builder = DIDDocumentBuilder(did, self)
        doc = try builder.appendAuthenticationKey(with: id, keyBase58: key.getPublicKeyBase58())
            .sealed(using: storePassword)
        doc?.getMetadata().setAlias(alias)
        try storeDid(using: doc!)

        return doc!
    }

    public func newDid(withPrivateIdentityIndex: Int,
                                          alias: String,
                            using storePassword: String) throws -> DIDDocument {
        return try newDid(withPrivateIdentityIndex, alias, storePassword)
    }

    public func newDid(withPrivateIdentityIndex: Int,
                            using storePassword: String) throws -> DIDDocument {

        return try newDid(withPrivateIdentityIndex, nil, storePassword)
    }

    private func newDid(_ alias: String?, _ storePassword: String) throws -> DIDDocument {
        var nextIndex = try storage.loadPrivateIdentityIndex()

        let doc = try newDid(nextIndex, alias, storePassword)
        nextIndex += 1
        try storage.storePrivateIdentityIndex(nextIndex)

        return doc
    }

    public func newDid(withAlias: String, using storePassword: String) throws -> DIDDocument {
        return try newDid(withAlias, storePassword)
    }

    public func newDid(using storePassword: String) throws -> DIDDocument {
        return try newDid(nil, storePassword)
    }

    public func getDid(byPrivateIdentityIndex: Int) throws -> DID {
        guard byPrivateIdentityIndex >= 0 else {
            throw DIDError.illegalArgument("invalid index.")
        }
        let publicIdentity = try loadPublicIdentity()

        let path = "0/\(byPrivateIdentityIndex)"
        let key = try publicIdentity.derive(path)
        let did = DID(Constants.METHOD, key.getAddress())

        return did
    }

    private func publishDid(_ did: DID,
                            _ signKey: DIDURL?,
                            _ storePassword: String,
                            _ force: Bool) throws {
        guard !storePassword.isEmpty else {
            throw DIDError.illegalArgument()
        }
        Log.i(DIDStore.TAG, "Publishing {}{}...", did.toString(), force ? " in force mode" : "" )
        let  doc = try loadDid(did)
        if doc == nil {
            Log.e(DIDStore.TAG, "No document for {}", did.toString())
            throw DIDError.didStoreError("Can not find the document for \(did)")
        }

        guard !doc!.isDeactivated else {
            Log.e(DIDStore.TAG, "\(did.toString()) already deactivated.")
            throw DIDError.didStoreError("DID already deactivated.")
        }

        if doc!.isExpired && !force {
            Log.e(DIDStore.TAG, "\(did.toString()) already expired, use force mode to publish anyway.")
        }
        var lastTxid: String? = nil
        var reolvedSignautre: String? = nil
        let resolvedDoc = try did.resolve(true)

        if resolvedDoc != nil {
            guard !resolvedDoc!.isDeactivated else {
                doc!.getMetadata().setDeactivated(true)
                try storage.storeDidMetadata(doc!.subject, doc!.getMetadata())
                Log.e(DIDStore.TAG, "\(did.toString()) already deactivated.")
                throw  DIDError.didStoreError("DID already deactivated")
            }
            reolvedSignautre = resolvedDoc?.proof.signature
            if !force {
                let localPrevSignature = doc!.getMetadata().previousSignature
                let localSignature = doc!.getMetadata().signature

                if localPrevSignature == nil && localSignature == nil {
                    Log.e(DIDStore.TAG ,"Missing signatures information, DID SDK dosen't know how to handle it, use force mode to ignore checks.")
                    throw DIDError.didStoreError("DID document not up-to-date")
                }

                else if localPrevSignature == nil || localSignature == nil {
                    let ls = localPrevSignature != nil ? localPrevSignature : localSignature
                    if ls != reolvedSignautre {
                        Log.e(DIDStore.TAG ,"Current copy not based on the lastest on-chain copy, txid mismatch.")
                        throw DIDError.didStoreError("DID document not up-to-date")
                    }
                }
                else {
                    if localSignature != reolvedSignautre && localPrevSignature != reolvedSignautre {
                        Log.e(DIDStore.TAG ,"Current copy not based on the lastest on-chain copy, txid mismatch.")
                        throw DIDError.didStoreError("DID document not up-to-date")
                    }
                }
            }

            lastTxid = resolvedDoc!.getMetadata().transactionId!
        }

        var usedSignKey = signKey
        if  usedSignKey == nil {
            usedSignKey = doc?.defaultPublicKey
        }

        if  lastTxid == nil || lastTxid!.isEmpty {
            Log.i(DIDStore.TAG ,"Try to publish[create] \(did.toString()...)")
            try backend.create(doc!, usedSignKey!, storePassword)
        } else {
            Log.i(DIDStore.TAG ,"Try to publish[update] \(did.toString()...)")
            try backend.update(doc!, lastTxid!, usedSignKey!, storePassword)
        }

        doc!.getMetadata().setPreviousSignature(reolvedSignautre)
        doc!.getMetadata().setSignature(doc!.getProof()!.signature)
        try storage.storeDidMetadata(doc!.subject, doc!.getMetadata())
    }

    public func publishDid(for did: DID,
                     using signKey: DIDURL,
                     storePassword: String,
                           _ force: Bool) throws {

        return try publishDid(did, signKey, storePassword, force)
    }

    public func publishDid(for did: DID,
                     using signKey: DIDURL,
                     storePassword: String) throws {

        return try publishDid(did, signKey, storePassword, false)
    }

    public func publishDid(for did: String,
                     using signKey: String,
                     storePassword: String,
                           _ force: Bool) throws {

        let _did = try DID(did)
        let _key = try DIDURL(_did, signKey)

        return try publishDid(_did, _key, storePassword, force)
    }

    public func publishDid(for did: String,
                     using signKey: String,
                     storePassword: String) throws {

        let _did = try DID(did)
        let _key = try DIDURL(_did, signKey)

        return try publishDid(_did, _key, storePassword, false)
    }

    public func publishDid(for did: DID,
               using storePassword: String) throws {

        do {
            return try publishDid(did, nil, storePassword, false)
        } catch {
            // Dead code.
            let e: DIDError = error as! DIDError
            switch e {
            case .invalidKeyError(_): break
            default:
                throw error
            }
        }
    }
    public func publishDid(for did: String,
               using storePassword: String) throws {

        return try publishDid(DID(did), nil, storePassword, false)
    }

    private func publishDidAsync(_ did: DID,
                                 _ signKey: DIDURL?,
                                 _ storePassword: String,
                                 _ force: Bool) -> Promise<Void> {
        return Promise<Void> { resolver in
            do {
                resolver.fulfill(try publishDid(did, signKey, storePassword, force))
            } catch let error  {
                resolver.reject(error)
            }
        }
    }

    public func publishDidAsync(for did: DID,
                          using signKey: DIDURL,
                          storePassword: String,
                                _ force: Bool) -> Promise<Void> {

        return publishDidAsync(did, signKey, storePassword, force)
    }

    public func publishDidAsync(for did: DID,
                          using signKey: DIDURL,
                          storePassword: String) -> Promise<Void> {

        return publishDidAsync(did, signKey, storePassword, false)
    }

    public func publishDidAsync(for did: String,
                          using signKey: String,
                          storePassword: String,
                                _ force: Bool) throws -> Promise<Void> {
        let _did = try DID(did)
        let _key = try DIDURL(_did, signKey)

        return publishDidAsync(_did, _key, storePassword, force)
    }

    public func publishDidAsync(for did: String,
                          using signKey: String,
                          storePassword: String) throws -> Promise<Void> {

        let _did = try DID(did)
        let _key = try DIDURL(_did, signKey)

        return publishDidAsync(_did, _key, storePassword, false)
    }

    public func publishDidAsync(for did: DID,
                    using storePassword: String) -> Promise<Void> {

        return publishDidAsync(did, nil, storePassword, false)
    }

    public func publishDidAsync(for did: String,
                    using storePassword: String) throws -> Promise<Void> {

        return publishDidAsync(try DID(did), nil, storePassword, false)
    }

    private func deactivateDid(_ did: DID,
                               _ signKey: DIDURL?,
                               _ storePassword: String) throws {
        guard !storePassword.isEmpty else {
            throw DIDError.didStoreError("storePassword is empty.")
        }

        // Document should use the IDChain's copy
        var localCopy = false
        var doc = try DIDBackend.resolve(did)

        if doc == nil {
            // Fail-back: try to load document from local store.
            doc = try loadDid(did)
            if doc == nil {
                throw DIDError.didNotFoundError("\(did.toString())")
            }
            localCopy = true
        } else {
            doc!.getMetadata().setStore(self)
        }

        var usedSignKey = signKey
        if  usedSignKey == nil {
            usedSignKey = doc!.defaultPublicKey
        }
        else {
            guard doc!.containsAuthenticationKey(forId: signKey!) else {
                throw DIDError.invalidKeyError("Not an authentication key.")
            }
        }

        try backend.deactivate(doc!, usedSignKey!, storePassword)

        // Save deactivated status to DID metadata
        if localCopy {
            doc!.getMetadata().setDeactivated(true)
            try storage.storeDidMetadata(did, doc!.getMetadata())
        }
    }

    public func deactivateDid(for target: DID,
                           using signKey: DIDURL,
                           storePassword: String) throws {

        return try deactivateDid(target, signKey, storePassword)
    }

    public func deactivateDid(for target: String,
                           using signKey: String,
                           storePassword: String) throws {

        let _did = try DID(target)
        let _key = try DIDURL(_did, signKey)

        return try deactivateDid(_did, _key, storePassword)
    }

    public func deactivateDid(for target: DID,
                     using storePassword: String) throws {

        return try deactivateDid(target, nil, storePassword)
    }

    public func deactivateDid(for target: String,
                     using storePassword: String) throws {

        return try deactivateDid(DID(target), nil, storePassword)
    }

    private func deactivateDidAsync(_ target: DID,
                                    _ signKey: DIDURL?,
                                    _ storePassword: String) -> Promise<Void> {
        return Promise<Void> { resolver in
            do {
                resolver.fulfill(try deactivateDid(target, signKey, storePassword))
            } catch let error  {
                resolver.reject(error)
            }
        }
    }

    public func deactivateDidAsync(for target: DID,
                                using signKey: DIDURL,
                                storePassword: String) throws -> Promise<Void> {

        return deactivateDidAsync(target, signKey, storePassword)
    }

    public func deactivateDidAsync(for target: String,
                                using signKey: String,
                                storePassword: String) throws -> Promise<Void> {
        let _did = try DID(target)
        let _key = try DIDURL(_did, signKey)

        return deactivateDidAsync(_did, _key, storePassword)
    }

    public func deactivateDidAsync(for target: DID,
                          using storePassword: String) throws -> Promise<Void> {

        return deactivateDidAsync(target, nil, storePassword)
    }

    public func deactivateDidAsync(for target: String,
                          using storePassword: String) throws -> Promise<Void> {

        return deactivateDidAsync(try DID(target), nil, storePassword)
    }

    // Deactivate target DID with authorization
    private func deactivateDid(_ target: DID,
                               _ did: DID,
                               _ signKey: DIDURL?,
                               _ storePassword: String) throws {
        guard !storePassword.isEmpty else {
            throw DIDError.illegalArgument("storePassword is empty.")
        }

        // All document should use the IDChain's copy
        var doc = try DIDBackend.resolve(did)

        if doc == nil {
            // Fail-back: try to load document from local store.
            doc = try loadDid(did)
            if doc == nil {
                throw DIDError.didNotFoundError("\(did.toString())")
            }
        } else {
            doc!.getMetadata().setStore(self)
        }

        var signPk: PublicKey? = nil
        if let _ = signKey {
            signPk = doc!.authenticationKey(ofId: signKey!)
            guard let _ = signPk else {
                throw DIDError.unknownFailure("Not an authentication key.")
            }
        }

        let targetDoc = try DIDBackend.resolve(target)
        guard let _ = targetDoc else {
            throw DIDError.didNotFoundError("\(target.toString())")
        }
        guard targetDoc!.authorizationKeyCount > 0 else {
            throw DIDError.illegalArgument("No matched authorization key.")
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
            throw DIDError.invalidKeyError("No matched authorization key.")
        }

        return try backend.deactivate(target, targetSignKey!, doc!, usedSignKey!, storePassword)
    }

    public func deactivateDid(for target: DID,
                    withAuthroizationDid: DID,
                           using signKey: DIDURL,
                           storePassword: String) throws {

        return try deactivateDid(target, withAuthroizationDid, signKey, storePassword)
    }

    public func deactivateDid(for target: String,
                    withAuthroizationDid: String,
                           using signKey: String,
                           storePassword: String) throws {

        let _did = try DID(withAuthroizationDid)
        let _key = try DIDURL(_did, signKey)

        return try deactivateDid(DID(target), _did, _key, storePassword)
    }

    public func deactivateDid(for target: DID,
                    withAuthroizationDid: DID,
                           storePassword: String) throws {

        return try deactivateDid(target, withAuthroizationDid, nil, storePassword)
    }

    public func deactivateDid(for target: String,
                    withAuthroizationDid: String,
                           storePassword: String) throws {

        return try deactivateDid(DID(target), DID(withAuthroizationDid), nil, storePassword)
    }

    private func deactivateDidAsync(_ target: DID,
                                    _ did: DID,
                                    _ signKey: DIDURL?,
                                    _ storePassword: String) -> Promise<Void> {
        return Promise<Void> { resolver in
            do {
                resolver.fulfill(try deactivateDid(target, did, signKey, storePassword))
            } catch let error  {
                resolver.reject(error)
            }
        }
    }

    public func deactivateDidAsync(for target: DID,
                         withAuthroizationDid: DID,
                                using signKey: DIDURL,
                                storePassword: String) -> Promise<Void> {

        return deactivateDidAsync(target, withAuthroizationDid, signKey, storePassword)
    }

    public func deactivateDidAsync(for target: String,
                         withAuthroizationDid: String,
                                using signKey: String,
                                storePassword: String) throws ->  Promise<Void> {

        let _did = try DID(withAuthroizationDid)
        let _key = try DIDURL(_did, signKey)

        return deactivateDidAsync(try DID(target), _did, _key, storePassword)
    }

    public func deactivateDidAsync(for target: DID,
                         withAuthroizationDid: DID,
                                storePassword: String) ->  Promise<Void> {

        return deactivateDidAsync(target, withAuthroizationDid, nil, storePassword)
    }

    public func deactivateDidAsync(for target: String,
                    withAuthroizationDid: String,
                           storePassword: String) throws ->  Promise<Void> {

        return try deactivateDidAsync(DID(target), DID(withAuthroizationDid), nil, storePassword)
    }

    public func storeDid(using doc: DIDDocument) throws {
        try storage.storeDid(doc)

        let metadata = try loadDidMetadata(doc.subject)
        try doc.getMetadata().merge(metadata)
        doc.getMetadata().setStore(self)
        try storage.storeDidMetadata(doc.subject, doc.getMetadata())

        for credential in doc.credentials() {
            try storeCredential(using: credential)
        }
        if (documentCache != nil) {
            documentCache!.setValue(doc, for: doc.subject)
        }
    }
    
    func storeDidMetadata(_  did: DID, _ metadata: DIDMeta) throws {
        try storage.storeDidMetadata(did, metadata)
        if documentCache != nil {
            let doc = documentCache?.getValue(for: did)
            if doc != nil {
                doc?.setMetadata(metadata)
            }
        }
    }

    func storeDidMetadata(_ did: String, _ metadata: DIDMeta) throws {
        let _did = try DID(did)
        try storeDidMetadata(_did, metadata)

    }
    
    func loadDidMetadata(_ did: DID) throws -> DIDMeta {
        var doc: DIDDocument? = nil
        var metadata: DIDMeta?
        if documentCache != nil {
            doc = documentCache!.getValue(for: did)
            if doc != nil {
                metadata = doc!.getMetadata()
                if !metadata!.isEmpty() {
                    return metadata!
                }
            }
        }
        metadata = try storage.loadDidMetadata(did)
        if doc != nil {
            doc!.setMetadata(metadata!)
        }
        return metadata!
    }

    func loadDidMetadata(_ did: String) throws -> DIDMeta {
        let _did = try DID(did)
        return try loadDidMetadata(_did)
    }

    public func loadDid(_ did: DID) throws -> DIDDocument? {
        var doc: DIDDocument?
        if documentCache != nil {
            doc = documentCache!.getValue(for: did)
            if doc != nil {
                return doc!
            }
        }

        doc = try storage.loadDid(did)

        if doc != nil {
            let metadata = try storage.loadDidMetadata(did)
            metadata.setStore(self)
            doc?.setMetadata(metadata)
        }

        if doc != nil && documentCache != nil {
            documentCache?.setValue(doc!, for: doc!.subject)
        }

        return doc
    }

    public func loadDid(_ did: String) throws -> DIDDocument? {
        return try loadDid(DID(did))
    }
    
    public func containsDid(_ did: DID) -> Bool {
        return storage.containsDid(did)
    }

    public func containsDid(_ did: String) throws -> Bool {
        return containsDid(try DID(did))
    }

    public func deleteDid(_ did: DID) -> Bool {
        documentCache?.removeValue(for: did)
        return storage.deleteDid(did)
    }

    public func deleteDid(_ did: String) throws -> Bool {
        return try deleteDid(DID(did))
    }

    public func listDids(using filter: Int) throws -> Array<DID> {
        let dids = try storage.listDids(filter)

        try dids.forEach { did in
            let metadata = try loadDidMetadata(did)
            metadata.setStore(self)
            did.setMetadata(metadata)
        }

        return dids
    }

    public func storeCredential(using credential: VerifiableCredential) throws {
        try storage.storeCredential(credential)

        let metadata = try loadCredentialMetadata(credential.subject.did, credential.getId())
        try credential.getMeta().merge(metadata)
        credential.getMeta().setStore(self)
        try storage.storeCredentialMetadata(credential.subject.did, credential.getId(), credential.getMeta())
        credentialCache?.setValue(credential, for: credential.getId())
    }

    func storeCredentialMetadata(_ did: DID, _ id: DIDURL, _ metadata: CredentialMeta) throws {
        try storage.storeCredentialMetadata(did, id, metadata)
        if credentialCache != nil {
            let vc = credentialCache?.getValue(for: id)
            if vc != nil {
                vc?.setMetadata(metadata)
            }
        }
    }
    
    func storeCredentialMetadata(_ did: String, _ id: String, _ metadata: CredentialMeta) throws {
        let _did = try DID(did)
        let _id = try DIDURL(_did, id)
        try storeCredentialMetadata(_did, _id, metadata)
    }
    
    func loadCredentialMetadata(_ did: DID, _ byId: DIDURL) throws -> CredentialMeta {
        var metadata: CredentialMeta?
        let vc: VerifiableCredential? = credentialCache?.getValue(for: byId)
        if vc != nil {
            metadata = vc!.getMetadata()
            if !metadata!.isEmpty() {
                return metadata!
            }
        }
        metadata = try? storage.loadCredentialMetadata(did, byId)
        if metadata == nil {
            metadata = CredentialMeta()
        }
        if vc != nil {
            vc!.setMetadata(metadata!)
        }
        return metadata!
    }

    func loadCredentialMetadata(_ did: String, _ byId: String) throws -> CredentialMeta? {
        let _did = try DID(did)
        let _id = try DIDURL(_did, byId)
        return try loadCredentialMetadata(_did, _id)
    }

    public func loadCredential(for did: DID, byId: DIDURL) throws -> VerifiableCredential? {
        var vc = credentialCache?.getValue(for: byId)
        if vc != nil {
            return vc
        }

        vc = try? storage.loadCredential(did, byId)
        if vc != nil {
            let metadata = try storage.loadCredentialMetadata(did, byId)
            metadata.setStore(self)
            vc!.setMetadata(metadata)
        }
        if vc != nil && credentialCache != nil {
            credentialCache?.setValue(vc!, for: vc!.getId())
        }

        return vc
    }
    
    public func loadCredential(for did: String, byId: String) throws -> VerifiableCredential? {
        let _did = try DID(did)
        let _key = try DIDURL(_did, byId)
        return try loadCredential(for: _did, byId: _key)
    }
    
    public func containsCredentials(_ did:DID) -> Bool {
        return storage.containsCredentials(did)
    }
    
    public func containsCredentials(_ did: String) -> Bool {
        do {
            return containsCredentials(try DID(did))
        } catch {
            return false
        }
    }
    
    public func containsCredential(_ did: DID, _ id: DIDURL) throws -> Bool {
        return storage.containsCredential(did, id)
    }
    
    public func containsCredential(_ did: String, _ id: String) throws -> Bool {
        let _did: DID = try DID(did)
        return try containsCredential(_did, DIDURL(_did, id))
    }
    
    public func deleteCredential(for did: DID, id: DIDURL) -> Bool{
        credentialCache?.removeValue(for: id)
        return storage.deleteCredential(did, id)
    }
    
    public func deleteCredential(for did: String, id: String) -> Bool{
        do {
            let _did = try DID(did)
            let _key = try DIDURL(_did, id)
            return deleteCredential(for: _did, id: _key)
        } catch {
            return false
        }
    }
    
    public func listCredentials(for did: DID) throws -> Array<DIDURL> {
        let ids = try storage.listCredentials(did)
        for id in ids {
            let metadata = try loadCredentialMetadata(did, id)
            metadata.setStore(self)
            id.setMetadata(metadata)
        }
        return ids
    }
    
    public func listCredentials(for did: String) throws -> Array<DIDURL> {
        return try listCredentials(for: DID(did))
    }

    public func selectCredentials(for did: DID,
                                  byId id: DIDURL?,
                             andType type: Array<String>?) throws -> Array<DIDURL> {
        return try storage.selectCredentials(did, id, type)
    }
    
    public func selectCredentials(for did: String,
                                  byId id: String?,
                             andType type: Array<String>?) throws -> Array<DIDURL> {
        let _did = try DID(did)
        let _key = id != nil ? try DIDURL(_did, id!) : nil

        return try selectCredentials(for: _did, byId: _key, andType: type)
    }
    
    public func storePrivateKey(for did: DID,
                                     id: DIDURL,
                             privateKey: Data,
                    using storePassword: String) throws {

        guard !storePassword.isEmpty else {
            throw DIDError.illegalArgument()
        }

        let encryptedKey = try DIDStore.encryptToBase64(privateKey, storePassword)
        try storage.storePrivateKey(did, id, encryptedKey)
    }

    public func storePrivateKey(for did: String,
                                     id: String,
                             privateKey: Data,
                    using storePassword: String) throws {

        let _did = try DID(did)
        let _key = try DIDURL(_did, id)

        return try storePrivateKey(for: _did, id: _key, privateKey: privateKey, using: storePassword)
    }
    
    func loadPrivateKey(_ did: DID, _ byId: DIDURL, _ storePassword: String) throws -> Data {
        let encryptedKey = try storage.loadPrivateKey(did, byId)
        let keyBytes = try DIDStore.decryptFromBase64(encryptedKey, storePassword)

        // For backward compatible, convert to extended private key
        // TODO: Should be remove in the future
        var extendedKeyBytes: Data?
        if keyBytes.count == HDKey.PRIVATEKEY_BYTES {
            let identity = try? loadPrivateIdentity(storePassword)
            if identity != nil {
                for i in 0..<100 {
                    let path = HDKey.DERIVE_PATH_PREFIX + "\(i)"
                    let child = try identity!.derive(path)
                    if child.getPrivateKeyData() == keyBytes {
                        extendedKeyBytes = try child.serialize()
                        break
                    }
                    child.wipe()
                }
                identity?.wipe()
            }
            if extendedKeyBytes == nil {
                extendedKeyBytes = HDKey.paddingToExtendedPrivateKey(keyBytes)
            }
            try storePrivateKey(for: did, id: byId, privateKey: extendedKeyBytes!, using: storePassword)
        }
        else {
            extendedKeyBytes = keyBytes
        }

        return extendedKeyBytes!
    }
    
    public func containsPrivateKeys(for did: DID) -> Bool {
        return storage.containsPrivateKeys(did)
    }
    
    public func containsPrivateKeys(for did: String) -> Bool {
        do {
            return containsPrivateKeys(for: try DID(did))
        } catch {
            return false
        }
    }
    
    public func containsPrivateKey(for did: DID, id: DIDURL) -> Bool {
        return storage.containsPrivateKey(did, id)
    }
    
    public func containsPrivateKey(for did: String, id: String) -> Bool {
        do {
            let _did = try DID(did)
            let _key = try DIDURL(_did, id)
            return containsPrivateKey(for: _did, id: _key)
        } catch {
            return false
        }
    }
    
    public func deletePrivateKey(for did: DID, id: DIDURL) -> Bool {
        return storage.deletePrivateKey(did, id)
    }
    
    public func deletePrivateKey(for did: String, id: String) -> Bool {
        do {
            let _did = try DID(did)
            let _key = try DIDURL(_did, id)
            
            return deletePrivateKey(for: _did, id: _key)
        } catch {
            return false
        }
    }

    func sign(_ did: DID, _ id: DIDURL?, _ storePassword: String, _ digest: Data, _ capacity: Int) throws -> String {
        guard !storePassword.isEmpty else {
            throw DIDError.illegalArgument("storePassword is empty.")
        }

        var usedId: DIDURL? = id
        if usedId == nil {
            let doc = try loadDid(did)
            if doc == nil {
                throw DIDError.didStoreError("Can not resolve DID document.")
            }
            usedId = doc?.defaultPublicKey
        }
        
        let key = try HDKey.deserialize(loadPrivateKey(did, usedId!, storePassword));
        let privatekeys = key.getPrivateKeyData()
        let toPPointer = privatekeys.toPointer()
        
        let cdigest = digest.toPointer()

        let csig = UnsafeMutablePointer<Int8>.allocate(capacity: capacity)
        let re = ecdsa_sign_base64(csig, UnsafeMutablePointer(mutating: toPPointer), UnsafeMutablePointer(mutating: cdigest), digest.count)

        guard re >= 0 else {
            throw DIDError.didStoreError("sign error.")
        }
        csig[re] = 0
        let sig = String(cString: csig)
        key.wipe()
        return sig
    }

    func sign(WithDid did: DID, using storePassword: String, for digest: Data, capacity: Int) throws -> String {

        return try sign(did, nil, storePassword, digest, capacity)
    }

    public func changePassword(_ oldPassword: String, _ newPassword: String) throws {
        let re: (String) throws -> String = { (data: String) -> String in
            let udata = try DIDStore.decryptFromBase64(data, oldPassword)
            let result = try DIDStore.encryptToBase64(udata, newPassword)
            return result
        }
        try storage.changePassword(re)
    }

    private func exportDid(_ did: DID,
                           _ generator: JsonGenerator,
                           _ password: String,
                           _ storePassword: String) throws {
        // All objects should load directly from storage,
        // avoid affects the cached objects.
        var doc: DIDDocument? = nil
        do {
            doc = try storage.loadDid(did)
        } catch {
            throw DIDError.didStoreError("Export DID \(did)failed.\(error)")
        }
        guard doc != nil else {
            throw DIDError.didStoreError("Export DID \(did) failed, not exist.")
        }

        Log.d(DIDStore.TAG, "Exporting \(did.toString())...")
        let sha256 = SHA256Helper()
        var bytes = [UInt8](password.data(using: .utf8)!)
        sha256.update(&bytes)

        generator.writeStartObject()
        
        // Type
        generator.writeStringField("type", DID_EXPORT)
        bytes = [UInt8](DID_EXPORT.data(using: .utf8)!)
        sha256.update(&bytes)
        
        // DID
        var value: String = did.description
        generator.writeStringField("id", value)
        bytes = [UInt8](value.data(using: .utf8)!)
        sha256.update(&bytes)
        
        // Create
        let now = DateFormatter.currentDate()
        value = DateFormatter.convertToUTCStringFromDate(now)
        generator.writeStringField("created", value)
        bytes = [UInt8](value.data(using: .utf8)!)
        sha256.update(&bytes)
        
        // Document
        generator.writeFieldName("document")
        generator.writeStartObject()
        generator.writeFieldName("content")
        try doc!.toJson(generator, false)
        value = doc!.toString(true)
        bytes = [UInt8](value.data(using: .utf8)!)
        sha256.update(&bytes)

        let didMetadata: DIDMeta? = try storage.loadDidMetadata(did)
        if !didMetadata!.isEmpty() {
            generator.writeFieldName("metadata")
            value = try didMetadata!.toString()
            generator.writeRawValue(value)
            bytes = [UInt8](value.data(using: .utf8)!)
            sha256.update(&bytes)
        }
        
        generator.writeEndObject()

        // Credential
        if storage.containsCredentials(did) {
            generator.writeFieldName("credential")
            generator.writeStartArray()
            var ids = try listCredentials(for: did)
            ids.sort { (a, b) -> Bool in
                let compareResult = a.toString().compare(b.toString())
                return compareResult == ComparisonResult.orderedAscending
            }
            for id in ids {
                var vc: VerifiableCredential? = nil
                do {
                    Log.i(DIDStore.TAG, "Exporting credential {}...\(id.toString())")

                    generator.writeStartObject()

                    generator.writeFieldName("content")

                    vc = try storage.loadCredential(did, id)
                } catch {
                    throw DIDError.didStoreError("Export DID \(did) failed.\(error)")
                }
                vc!.toJson(generator, false)
                value = vc!.toString(true)
                bytes = [UInt8](value.data(using: .utf8)!)
                sha256.update(&bytes)
                
                let metadata = try storage.loadCredentialMetadata(did, id)
                if !metadata.isEmpty() {
                    generator.writeFieldName("metadata")
                    value = try metadata.toString()
                    generator.writeRawValue(value)
                    bytes = [UInt8](value.data(using: .utf8)!)
                    sha256.update(&bytes)
                }
                generator.writeEndObject()
            }
            generator.writeEndArray()
        }

        // Private key
        if storage.containsPrivateKeys(did) {
            generator.writeFieldName("privatekey")
            generator.writeStartArray()
            
            var pks: Array<PublicKey> = doc!.publicKeys()
            pks.sort { (a, b) -> Bool in
                let compareResult = a.getId().toString().compare(b.getId().toString())
                return compareResult == ComparisonResult.orderedAscending
            }
            for pk in pks {
                let id = pk.getId()
                if storage.containsPrivateKey(did, id) {
                    Log.i(DIDStore.TAG, "Exporting private key {}...\(id.toString())")
                    var csk: String = try storage.loadPrivateKey(did, id)
                    let cskData: Data = try DIDStore.decryptFromBase64(csk, storePassword)
                    csk = try DIDStore.encryptToBase64(cskData, password)
                    
                    generator.writeStartObject()
                    value = id.toString()
                    generator.writeStringField("id", value)
                    bytes = [UInt8](value.data(using: .utf8)!)
                    sha256.update(&bytes)
                    
                    generator.writeStringField("key", csk)
                    bytes = [UInt8](csk.data(using: .utf8)!)
                    sha256.update(&bytes)
                    generator.writeEndObject()
                }
            }
            generator.writeEndArray()
        }

        // Fingerprint
        let result = sha256.finalize()

        let capacity = result.count * 3
        let cFing = UnsafeMutablePointer<Int8>.allocate(capacity: capacity)
        var dateFing = Data(bytes: result, count: result.count)
        let cFingerprint = dateFing.withUnsafeMutableBytes { fing -> UnsafeMutablePointer<UInt8> in
            return fing
        }
        let re = base64_url_encode(cFing, cFingerprint, dateFing.count)
        cFing[re] = 0
        let fingerprint = String(cString: cFing)

        generator.writeStringField("fingerprint", fingerprint)
        generator.writeEndObject()
    }

    public func exportDid(_ did: DID,
                      to output: OutputStream,
                 using password: String,
                  storePassword: String) throws {
        let generator = JsonGenerator()
        try exportDid(did, generator, password, storePassword)
        let exportStr = generator.toString()
        output.open()
        self.writeData(data: exportStr.data(using: .utf8)!, outputStream: output, maxLengthPerWrite: 1024)
        output.close()
    }

    public func exportDid(_ did: String,
                      to output: OutputStream,
                 using password: String,
                  storePassword: String) throws {
        try exportDid(DID(did), to: output, using: password, storePassword: storePassword)
    }

    public func exportDid(_ did: DID,
                  to fileHandle: FileHandle,
                 using password: String,
                  storePassword: String) throws {
        let generator = JsonGenerator()
        try exportDid(did, generator, password, storePassword)
        let exportStr = generator.toString()
        fileHandle.write(exportStr.data(using: .utf8)!)
    }

    public func exportDid(_ did: String,
                  to fileHandle: FileHandle,
                 using password: String,
                  storePassword: String) throws {
        try exportDid(DID(did), to: fileHandle, using: password, storePassword: storePassword)
    }

    private func importDid(_ root: JsonNode,
                           _ password: String,
                           _ storePassword: String) throws {
        let sha256 = SHA256Helper()
        var bytes = [UInt8](password.data(using: .utf8)!)
        sha256.update(&bytes)
        
        var value: String = ""
        
        // Type
        var serializer = JsonSerializer(root)
        var options: JsonSerializer.Options
        
        //bool ref hit
        options = JsonSerializer.Options()
            .withHint("export type")
        value = try serializer.getString("type", options)
        guard value == DID_EXPORT else {
            throw DIDError.didStoreError("Invalid export data, unknown type.")
        }
        bytes = [UInt8](value.data(using: .utf8)!)
        sha256.update(&bytes)
        
        // DID
        options = JsonSerializer.Options()
            .withHint("DID subject")
        let did = try serializer.getDID("id", options)
        value = did.description
        bytes = [UInt8](value.data(using: .utf8)!)
        sha256.update(&bytes)
        Log.d(DIDStore.TAG, "Importing {}...\(did.description)")

        // Created
        options = JsonSerializer.Options()
            .withOptional()
            .withHint("export date")
        let created = try serializer.getDate("created", options)
        value = DateFormatter.convertToUTCStringFromDate(created)
        bytes = [UInt8](value.data(using: .utf8)!)
        sha256.update(&bytes)
        
        // Document
        var node = root.get(forKey: "document")
        guard node?.toString() != nil else {
            Log.e(DIDStore.TAG, "Missing DID document.")
            throw DIDError.didStoreError("Missing DID document in the export data")
        }
        
        let docNode = node?.get(forKey: "content")
        guard docNode?.toString() != nil else {
            Log.e(DIDStore.TAG, "Missing DID document.")
            throw DIDError.didStoreError("Missing DID document in the export data")
        }
        var doc: DIDDocument? = nil
        do {
            doc = try DIDDocument.convertToDIDDocument(fromJson: docNode!)
        } catch  {
            throw DIDError.didStoreError("Invalid export data.\(error)")
        }
        guard doc!.subject == did || doc!.isGenuine else {
            throw DIDError.didStoreError("Invalid DID document in the export data.")
        }
        value = doc!.toString(true)
        bytes = [UInt8](value.data(using: .utf8)!)
        sha256.update(&bytes)
        
        let metaNode = node?.get(forKey: "metadata")
        if metaNode != nil {
            let metadata = DIDMeta()
            try metadata.load(metaNode!)
            metadata.setStore(self)
            doc?.setMetadata(metadata)
            value = try metadata.toString()

            bytes = [UInt8](value.data(using: .utf8)!)
            sha256.update(&bytes)
        }

        // Credential
        var vcs: [DIDURL: VerifiableCredential] = [: ]
        node = root.get(forKey: "credential")
        if node != nil {
            guard node!.asArray() != nil else {
                throw DIDError.didStoreError("Invalid export data, wrong credential data.")
            }
            for node in node!.asArray()! {
                let vcNode = node.get(forKey: "content")
                if (vcNode == nil) {
                    Log.e(DIDStore.TAG, "Missing credential " + " content")
                    throw DIDError.didStoreError("Invalid export data.")
                }
                var vc: VerifiableCredential? = nil
                do {
                    vc = try VerifiableCredential.fromJson(vcNode!, did)
                } catch {
                    Log.e(DIDStore.TAG, "Parse credential \(String(describing: vcNode)) error \(error)")
                    throw DIDError.didExpired("Invalid export data.\(error)")
                }
                guard vc?.subject.did == did else {
                    Log.e(DIDStore.TAG, "Credential {} not blongs to {} \(node) \(did.description)")
                    throw DIDError.didStoreError("Invalid credential in the export data.")
                }
                value = vc!.toString(true)
                bytes = [UInt8](value.data(using: .utf8)!)
                sha256.update(&bytes)

                let metaNode = node.get(forKey: "metadata")
                if metaNode != nil {
                    let metadata = CredentialMeta()
                    try metadata.load(metaNode!)
                    metadata.setStore(self)
                    vc?.setMetadata(metadata)

                    value = try metadata.toString()
                    bytes = [UInt8](value.data(using: .utf8)!)
                    sha256.update(&bytes)
                }

                vcs[vc!.getId()] = vc
            }
        }
        
        // Private key
        var sks: [DIDURL: String] = [: ]
        node = root.get(forKey: "privatekey")
        if node != nil {
            guard node!.asArray() != nil else {
                Log.e(DIDStore.TAG, "Privatekey should be an array.")
                throw DIDError.didStoreError("Invalid export data, wrong privatekey data.")
            }
            for dic in node!.asArray()! {
                serializer = JsonSerializer(dic)
                options = JsonSerializer.Options()
                    .withRef(did)
                    .withHint("privatekey id")
                let id = try serializer.getDIDURL("id", options)
                value = id!.toString()
                bytes = [UInt8](value.data(using: .utf8)!)
                sha256.update(&bytes)
                options = JsonSerializer.Options()
                    .withHint("privatekey")
                var csk = try serializer.getString("key", options)
                value = csk
                bytes = [UInt8](value.data(using: .utf8)!)
                sha256.update(&bytes)
                let sk = try DIDStore.decryptFromBase64(csk, password)
                csk = try DIDStore.encryptToBase64(sk, storePassword)
                sks[id!] = csk
            }
        }
        
        // Fingerprint
        node = root.get(forKey: "fingerprint")
        guard let _ = node else {
            Log.e(DIDStore.TAG, "Missing fingerprint")
            throw DIDError.didStoreError("Missing fingerprint in the export data")
        }
        let refFingerprint = node?.asString()
        let result = sha256.finalize()
        let capacity = result.count * 3
        let cFing = UnsafeMutablePointer<Int8>.allocate(capacity: capacity)
        var dateFing = Data(bytes: result, count: result.count)
        let cFingerprint = dateFing.withUnsafeMutableBytes { fing -> UnsafeMutablePointer<UInt8>  in
            return fing
        }
        let re = base64_url_encode(cFing, cFingerprint, dateFing.count)
        cFing[re] = 0
        let fingerprint = String(cString: cFing)
        guard fingerprint == refFingerprint else {
            throw DIDError.didStoreError("Invalid export data, the fingerprint mismatch.")
        }
        
        // Save
        // All objects should load directly from storage,
        // avoid affects the cached objects.
        Log.i(DIDStore.TAG, "Importing document...")
        try storage.storeDid(doc!)
        try storage.storeDidMetadata(doc!.subject, doc!.getMetadata())
        
        for vc in vcs.values {
            Log.i(DIDStore.TAG, "Importing credential {}...\(vc.getId().description)")
            try storage.storeCredential(vc)
            try storage.storeCredentialMetadata(did, vc.getId(), vc.getMetadata())
        }
        
        try sks.forEach { (key, value) in
            Log.i(DIDStore.TAG, "Importing private key {}...\(key.description)")
            try storage.storePrivateKey(did, key, value)
        }
    }

    public func importDid(from data: Data,
                     using password: String,
                      storePassword: String) throws {
        let dic = try JSONSerialization.jsonObject(with: data,options: JSONSerialization.ReadingOptions.mutableContainers) as? [String: Any]
        guard let _ = dic else {
            throw DIDError.notFoundError("data is not nil")
        }
        let jsonNode = JsonNode(dic!)
        try importDid(jsonNode, password, storePassword)
    }

    public func importDid(from input: InputStream,
                      using password: String,
                       storePassword: String) throws {
        let data = try readData(input: input)
        try importDid(from: data, using: password, storePassword: storePassword)
    }

    public func importDid(from handle: FileHandle,
                       using password: String,
                        storePassword: String) throws {
        handle.seekToEndOfFile()
        let data = handle.readDataToEndOfFile()
        try importDid(from: data, using: password, storePassword: storePassword)
    }

    private func exportPrivateIdentity(_ generator: JsonGenerator,
                                        _ password: String,
                                   _ storePassword: String) throws {
        var encryptedMnemonic = try storage.loadMnemonic()
        var plain = try DIDStore.decryptFromBase64(encryptedMnemonic, storePassword)
        encryptedMnemonic = try DIDStore.encryptToBase64(plain, password)
        var encryptedSeed = try storage.loadPrivateIdentity()
        
        plain = try DIDStore.decryptFromBase64(encryptedSeed, storePassword)
        encryptedSeed = try DIDStore.encryptToBase64(plain, password)
        
        let pubKey = storage.containsPublicIdentity() ? try storage.loadPublicIdentity() : nil

        let index = try storage.loadPrivateIdentityIndex()
        let sha256 = SHA256Helper()
        var bytes = [UInt8](password.data(using: .utf8)!)
        sha256.update(&bytes)
        
        generator.writeStartObject()
        
        // Type
        generator.writeStringField("type", DID_EXPORT)
        bytes = [UInt8](DID_EXPORT.data(using: .utf8)!)
        sha256.update(&bytes)
        
        // Mnemonic
        generator.writeStringField("mnemonic", encryptedMnemonic)
        bytes = [UInt8](encryptedMnemonic.data(using: .utf8)!)
        sha256.update(&bytes)
        
        // Key
        generator.writeStringField("key", encryptedSeed)
        bytes = [UInt8](encryptedSeed.data(using: .utf8)!)
        sha256.update(&bytes)
        
        // Key.pub
        if (pubKey != nil) {
            generator.writeStringField("key.pub", pubKey!)
            bytes = [UInt8](pubKey!.data(using: .utf8)!)
            sha256.update(&bytes)
        }

        // Index
        generator.writeNumberField("index", index)
        let indexStr = String(index)
        bytes = [UInt8](indexStr.data(using: .utf8)!)
        sha256.update(&bytes)
        
        // Fingerprint
        let result = sha256.finalize()
        let capacity = result.count * 3
        let cFing = UnsafeMutablePointer<Int8>.allocate(capacity: capacity)
        var dateFing = Data(bytes: result, count: result.count)
        let cFingerprint = dateFing.withUnsafeMutableBytes { fing -> UnsafeMutablePointer<UInt8> in
            return fing
        }
        let re = base64_url_encode(cFing, cFingerprint, dateFing.count)
        cFing[re] = 0
        let fingerprint = String(cString: cFing)
        generator.writeStringField("fingerprint", fingerprint)
        generator.writeEndObject()
    }

    public func exportPrivateIdentity(to output: OutputStream,
                                     _ password: String,
                                _ storePassword: String) throws {
        let generator = JsonGenerator()
        try exportPrivateIdentity(generator, password, storePassword)
        let exportStr = generator.toString()
        output.open()
        self.writeData(data: exportStr.data(using: .utf8)!, outputStream: output, maxLengthPerWrite: 1024)
        output.close()
    }

    public func exportPrivateIdentity(to handle: FileHandle,
                                     _ password: String,
                                _ storePassword: String) throws {
        let generator = JsonGenerator()
        try exportPrivateIdentity(generator, password, storePassword)
        let exportStr = generator.toString()
        handle.write(exportStr.data(using: .utf8)!)
    }

    public func exportPrivateIdentity(_ password: String,
                                 _ storePassword: String) throws -> Data {
        let generator = JsonGenerator()
        try exportPrivateIdentity(generator, password, storePassword)
        let exportStr = generator.toString()
        return exportStr.data(using: .utf8)!
    }
    
    private func importPrivateIdentity(_ root: JsonNode,
                                   _ password: String,
                              _ storePassword: String) throws {
        let sha256 = SHA256Helper()
        var bytes = [UInt8](password.data(using: .utf8)!)
        sha256.update(&bytes)
        
        let serializer = JsonSerializer(root)
        var options: JsonSerializer.Options
        options = JsonSerializer.Options()
            .withHint("export type")
        
        // Type
        let type = try serializer.getString("type", options)
        guard type == DID_EXPORT else {
            throw DIDError.didStoreError("Invalid export data, unknown type.")
        }
        bytes = [UInt8](type.data(using: .utf8)!)
        sha256.update(&bytes)
        
        // Mnemonic
        options = JsonSerializer.Options().withHint("mnemonic")
        var encryptedMnemonic = try serializer.getString("mnemonic", options)
        bytes = [UInt8](encryptedMnemonic.data(using: .utf8)!)
        sha256.update(&bytes)
        
        var plain = try DIDStore.decryptFromBase64(encryptedMnemonic, password)
        encryptedMnemonic = try DIDStore.encryptToBase64(plain, storePassword)
        
        // Key
        options = JsonSerializer.Options().withHint("key")
        var encryptedSeed = try serializer.getString("key", options)
        
        bytes = [UInt8](encryptedSeed.data(using: .utf8)!)
        sha256.update(&bytes)
        
        plain = try DIDStore.decryptFromBase64(encryptedSeed, password)
        encryptedSeed = try DIDStore.encryptToBase64(plain, storePassword)
        
        // Key.pub
        options = JsonSerializer.Options().withHint("key.pub")
        var pubKey: String?
        do {
            pubKey = try serializer.getString("key.pub", options)
            bytes = [UInt8](pubKey!.data(using: .utf8)!)
            sha256.update(&bytes)
        } catch {}

        // Index
        var node = root.get(forKey: "index")
        guard node != nil && node!.asInteger() != nil else {
            throw DIDError.didStoreError("Invalid export data, unknow index.")
        }
        let index: Int = node!.asInteger()!
        let indexStr = String(index)
        bytes = [UInt8](indexStr.data(using: .utf8)!)
        sha256.update(&bytes)
        
        // Fingerprint
        node = root.get(forKey: "fingerprint")
        guard node != nil && node!.asString() != nil else {
            throw DIDError.didStoreError("Missing fingerprint in the export data")
        }
        let result = sha256.finalize()
        let capacity = result.count * 3
        let cFing = UnsafeMutablePointer<Int8>.allocate(capacity: capacity)
        var dateFing = Data(bytes: result, count: result.count)
        let cFingerprint = dateFing.withUnsafeMutableBytes { fing -> UnsafeMutablePointer<UInt8> in
            return fing
        }
        let re = base64_url_encode(cFing, cFingerprint, dateFing.count)
        cFing[re] = 0
        let fingerprint = String(cString: cFing)
        guard fingerprint == node!.asString()! else {
            throw DIDError.didStoreError("Invalid export data, the fingerprint mismatch.")
        }
        
        // Save
        try storage.storeMnemonic(encryptedMnemonic)
        try storage.storePrivateIdentity(encryptedSeed)
        if pubKey != nil {
            try storage.storePublicIdentity(pubKey!)
        }
        try storage.storePrivateIdentityIndex(index)
    }

    public func importPrivateIdentity(from data: Data,
                                 using password: String,
                                  storePassword: String) throws {
        let dic = try JSONSerialization.jsonObject(with: data,options: JSONSerialization.ReadingOptions.mutableContainers) as? [String: Any]
        guard let _ = dic else {
            throw DIDError.notFoundError("data is not nil")
        }
        let jsonNode = JsonNode(dic!)
        try importPrivateIdentity(jsonNode, password, storePassword)
    }

    public func importPrivateIdentity(from input: InputStream,
                                  using password: String,
                                   storePassword: String) throws {
        let data = try readData(input: input)
        try importPrivateIdentity(from: data, using: password, storePassword: storePassword)
    }

    public func importPrivateIdentity(from handle: FileHandle,
                                   using password: String,
                                    storePassword: String) throws {
        let data = handle.readDataToEndOfFile()
        try importPrivateIdentity(from: data, using: password, storePassword: storePassword)
    }

    public func exportStore(to output: OutputStream,
                           _ password: String,
                      _ storePassword: String) throws {
        var exportStr = ""
        if containsPrivateIdentity() {
            let generator = JsonGenerator()
            try exportPrivateIdentity(generator, password, storePassword)
            exportStr = "{\"privateIdentity\":\"\(generator.toString())\"}"
        }
        let dids = try listDids(using: DIDStore.DID_ALL)
        for did in dids {
            let didstr = did.methodSpecificId
            let generator = JsonGenerator()
            try exportDid(did, generator, password, storePassword)
            exportStr = "{\"\(didstr)\":\"\(generator.toString())\"}"
        }
        output.open()
        self.writeData(data: exportStr.data(using: .utf8)!, outputStream: output, maxLengthPerWrite: 1024)
        output.close()
    }

    public func exportStore(to handle: FileHandle,
                           _ password: String,
                      _ storePassword: String) throws {
        var exportStr = ""
        if containsPrivateIdentity() {
            let generator = JsonGenerator()
            try exportPrivateIdentity(generator, password, storePassword)
            exportStr = "{\"privateIdentity\":\"\(generator.toString())\"}"
        }
        let dids = try listDids(using: DIDStore.DID_ALL)
        for did in dids {
            let didstr = did.methodSpecificId
            let generator = JsonGenerator()
            try exportDid(did, generator, password, storePassword)
            exportStr = "{\"\(didstr)\":\"\(generator.toString())\"}"
        }
        handle.write(exportStr.data(using: .utf8)!)
    }

    public func importStore(from input: InputStream,
                            _ password: String,
                       _ storePassword: String) throws {
        let data = try readData(input: input)
        let dic = try JSONSerialization.jsonObject(with: data,options: JSONSerialization.ReadingOptions.mutableContainers) as? [String: Any]
        guard let _ = dic else {
            throw DIDError.notFoundError("data is not nil")
        }
        
        let privateIdentity = dic!["privateIdentity"] as? [String: Any]
        if (privateIdentity != nil) && !(privateIdentity!.isEmpty) {
            let node = JsonNode(privateIdentity!)
            try importPrivateIdentity(node, password, storePassword)
        }
        try dic!.forEach{ key, value in
            if key == "privateIdentity" {
                let node = JsonNode(value)
                try importPrivateIdentity(node, password, storePassword)
            }
            else {
                let node = JsonNode(value)
                try importDid(node, password, storePassword)
            }
        }
    }

    public func importStore(from handle: FileHandle,
                             _ password: String,
                        _ storePassword: String) throws {
        let data = handle.readDataToEndOfFile()
        let dic = try JSONSerialization.jsonObject(with: data,options: JSONSerialization.ReadingOptions.mutableContainers) as? [String: Any]
        guard let _ = dic else {
            throw DIDError.notFoundError("data is not nil")
        }
        
        let privateIdentity = dic!["privateIdentity"] as? [String: Any]
        if (privateIdentity != nil) && !(privateIdentity!.isEmpty) {
            let node = JsonNode(privateIdentity!)
            try importPrivateIdentity(node, password, storePassword)
        }
        try dic!.forEach{ key, value in
            if key == "privateIdentity" {
                let node = JsonNode(dic as Any)
                try importPrivateIdentity(node, password, storePassword)
            }
            else {
                let node = JsonNode(dic as Any)
                try importDid(node, password, storePassword)
            }
        }
    }
    
    private func writeData(data: Data, outputStream: OutputStream, maxLengthPerWrite: Int) {
        let size = data.count
        data.withUnsafeBytes({(bytes: UnsafePointer<UInt8>) in
            var bytesWritten = 0
            while bytesWritten < size {
                var maxLength = maxLengthPerWrite
                if size - bytesWritten < maxLengthPerWrite {
                    maxLength = size - bytesWritten
                }
                let n = outputStream.write(bytes.advanced(by: bytesWritten), maxLength: maxLength)
                bytesWritten += n
            }
        })
    }
    
    private func readData(input: InputStream) throws -> Data {
        var data = Data()
        input.open()
        
        let bufferSize = 1024
        let buffer = UnsafeMutablePointer<UInt8>.allocate(capacity: bufferSize)
        
        while input.hasBytesAvailable {
            let read = input.read(buffer, maxLength: bufferSize)
            if read < 0 {
                //Stream error occured
                throw input.streamError!
            }
            else if read == 0 {
                //EOF
                break
            }
            data.append(buffer, count: read)
        }
        do{
            input.close()
        }
        do{
            buffer.deallocate()
        }
        return data
    }
}
