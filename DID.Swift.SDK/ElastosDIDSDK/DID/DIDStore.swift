import Foundation

public class DIDStore: NSObject {
    public static let CACHE_INITIAL_CAPACITY = 16
    public static let CACHE_MAX_CAPACITY = 32
    
    public static let DID_HAS_PRIVATEKEY = 0
    public static let DID_NO_PRIVATEKEY = 1
    public static let DID_ALL = 2
    private static let DID_EXPORT = "did.elastos.export/1.0"
    private var didCache: LRUCache?
    private var vcCache: LRUCache?
    
    private var storage: DIDStorage!

    init(_ initialCacheCapacity: Int, _ maxCacheCapacity: Int, _ storage: DIDStorage) {
        if (maxCacheCapacity > 0) {
            self.didCache = LRUCache(maxCacheCapacity)
            self.vcCache = LRUCache(maxCacheCapacity)
        }

        self.storage = storage
    }
    
    public class func open(_ type: String, _ location: String, _ initialCacheCapacity: Int, _ maxCacheCapacity: Int) throws -> DIDStore {
        guard type == "filesystem" else {
            throw DIDError.didStoreError(_desc: "Unsupported store type:\(type)")
        }
        let storage = try FileSystemStorage(location)
        return DIDStore(initialCacheCapacity, maxCacheCapacity, storage)
    }
    
    public class func open(_ type: String, _ location: String) throws -> DIDStore {
        return try open(type, location, CACHE_INITIAL_CAPACITY, CACHE_MAX_CAPACITY)
    }
    
    public func containsPrivateIdentity() throws -> Bool {
        return try storage!.containsPrivateIdentity()
    }

   class func encryptToBase64(_ passwd: String ,_ input: Data) throws -> String {
        let cinput: UnsafePointer<UInt8> = input.withUnsafeBytes{ (by: UnsafePointer<UInt8>) -> UnsafePointer<UInt8> in
            return by
        }
        let base64url: UnsafeMutablePointer<Int8> = UnsafeMutablePointer.allocate(capacity: 4096)
          let re = encrypt_to_base64(base64url, passwd, cinput, input.count)
        guard re >= 0 else {
            throw DIDError.didStoreError(_desc: "encryptToBase64 error.")
        }
        var json: String = String(cString: base64url)
        let endIndex = json.index(json.startIndex, offsetBy: re)
        json = String(json[json.startIndex..<endIndex])
        return json
    }
    
   public class func decryptFromBase64(_ passwd: String ,_ input: String) throws -> [Int8] {
        let plain: UnsafeMutablePointer<UInt8> = UnsafeMutablePointer<UInt8>.allocate(capacity: 4096)
        let re = decrypt_from_base64(plain, passwd, input)
        guard re >= 0 else {
            throw DIDError.didStoreError(_desc: "decryptFromBase64 error.")
        }
        let temp = UnsafeRawPointer(plain)
        .bindMemory(to: UInt8.self, capacity: re)
        
        let data = Data(bytes: temp, count: re)
        let intArray = [UInt8](data).map { Int8(bitPattern: $0) }
        print(intArray)
        return intArray
    }
    
   public class func decryptFromBase64(_ passwd: String ,_ input: String) throws -> Data {
        let plain: UnsafeMutablePointer<UInt8> = UnsafeMutablePointer<UInt8>.allocate(capacity: 4096)
        let re = decrypt_from_base64(plain, passwd, input)
        guard re >= 0 else {
            throw DIDError.didStoreError(_desc: "decryptFromBase64 error.")
        }
        let temp = UnsafeRawPointer(plain)
        .bindMemory(to: UInt8.self, capacity: re)
        
        let data = Data(bytes: temp, count: re)
        let intArray = [UInt8](data).map { Int8(bitPattern: $0) }
        print(intArray)
        return data
    }
    // Initialize & create new private identity and save it to DIDStore.
    public func initPrivateIdentity(_ language: Int,_ mnemonic: String ,_ passphrase: String, _ storepass: String, _ force: Bool ) throws {
        // TODO: CHECK mnemonic isValid
        
        if (try containsPrivateIdentity() && !force) {
            throw DIDError.didStoreError(_desc: "Already has private indentity.")
        }
        let privateIdentity: HDKey = try HDKey.fromMnemonic(mnemonic, passphrase)

        // Save seed instead of root private key,
        // keep compatible with Native SDK
        let seedData = privateIdentity.getSeed()
        let encryptedIdentity = try DIDStore.encryptToBase64(storepass, seedData)
        try storage!.storePrivateIdentity(encryptedIdentity)
        
        // Save mnemonic
        let mnemData = mnemonic.data(using: .utf8)
        let encryptedMnemonic: String = try DIDStore.encryptToBase64(storepass, mnemData!)
        try storage.storeMnemonic(encryptedMnemonic)
        // Save index
        try storage.storePrivateIdentityIndex(0)
    }
    
    func initPrivateIdentity(_ language: Int, _ mnemonic: String, _ passphrase: String, _ storepass: String) throws -> Void {
        try initPrivateIdentity(language, mnemonic, passphrase, storepass, false)
    }
    
    public func exportMnemonic(_ storepass: String) throws -> String {
        let encryptedMnemonic: String = try storage.loadMnemonic()
        let re: Data = try DIDStore.decryptFromBase64(storepass, encryptedMnemonic)
        return String(data: re, encoding: .utf8)!
    }
    
    // initialized from saved private identity from DIDStore.
    func loadPrivateIdentity(_ storepass: String) throws -> HDKey {
        guard try containsPrivateIdentity() else {
            throw DIDError.didStoreError(_desc: "DID Store not contains private identity.")
        }
        let seed: Data = try DIDStore.decryptFromBase64(storepass, storage.loadPrivateIdentity())
        let privateIdentity: HDKey = HDKey.fromSeed(seed)
        
        return privateIdentity
    }

    public func synchronize(_ storepass: String) throws {
        let nextIndex = try storage.loadPrivateIdentityIndex()
        let privateIdentity: HDKey = try loadPrivateIdentity(storepass)
        var blanks: Int = 0
        var i: Int = 0
        
        while i < nextIndex || blanks < 10 {
            let key: DerivedKey = try privateIdentity.derive(i++)
            let pks: [UInt8] = try key.getPublicKeyBytes()
            let methodIdString: String = DerivedKey.getIdString(pks)
            let did: DID = DID(DID.METHOD, methodIdString)
            var doc: DIDDocument?
            do {
                doc = try DIDBackend.shareInstance()!.resolve(did, true)
            } catch {
                if error is DIDError {
                    switch error as! DIDError {
                    case .didResolveError: do{
                        blanks = blanks++
                        }
                    default:
                        break
                    }
                }
                continue
            }
            
            if (doc != nil) {
                // TODO: check local conflict
                try storeDid(doc!)
                
                // Save private key
                let privatekeyData: Data = try key.getPrivateKeyData()
                let encryptedKey: String = try DIDStore.encryptToBase64(storepass, privatekeyData)
                let data = encryptedKey.data(using: .utf8)
                try storePrivateKey(did, doc!.getDefaultPublicKey(), data!, storepass)
                if (i >= nextIndex){
                    try storage.storePrivateIdentityIndex(i)
                }
                blanks = 0
            } else {
                blanks = blanks + 1
            }
        }
    }
    
    public func newDid(storepass: String, alias: String? = nil) throws -> DIDDocument {
        let privateIdentity =  try loadPrivateIdentity(storepass)
        var nextIndex: Int = try storage.loadPrivateIdentityIndex()

        let key: DerivedKey = try! privateIdentity.derive(nextIndex++)
        let pks: [UInt8] = try key.getPublicKeyBytes()
        let methodIdString: String = DerivedKey.getIdString(pks)
        let did: DID = DID(DID.METHOD, methodIdString)
        let id: DIDURL = try DIDURL(did, "primary")
        
        let privatekeyData: Data = try key.getPrivateKeyData()
        // TODO: get real private key bytes
        try storePrivateKey(did, id, privatekeyData, storepass)
        
        let db: DIDDocumentBuilder = DIDDocumentBuilder(did: did, store: self)
        _ = try db.addAuthenticationKey(id, try key.getPublicKeyBase58())
        let doc: DIDDocument = try db.seal(storepass: storepass)
        doc.meta.alias = alias ?? ""
        try storeDid(doc)
        try storage.storePrivateIdentityIndex(nextIndex)

        return doc
    }
    
    public func newDid(_ storepass: String) throws -> DIDDocument {
        return try newDid(storepass: storepass)
    }
    
    public func publishDid(_ did: DID, signKey: DIDURL? = nil, _ storepass: String) throws -> String? {
        let doc = try loadDid(did)
        guard doc != nil else {
            throw DIDError.didStoreError(_desc: "Can not find the document for \(did)")
        }
        var sigk = signKey
        if sigk == nil {
            sigk = doc?.getDefaultPublicKey()
        }
        var lastTxid = doc?.getTransactionId()
        if lastTxid == nil || lastTxid == "" {
            // TODO: check me
            let resolved = try did.resolve()
            if resolved != nil {
                lastTxid = resolved?.getTransactionId()
            }
        }
        if lastTxid == nil || lastTxid == "" {
            lastTxid = try DIDBackend.shareInstance()?.create(doc!, sigk!, storepass)
        }
        else {
            lastTxid = try DIDBackend.shareInstance()?.update(doc!, previousTxid: lastTxid, sigk!, storepass)
        }
        if lastTxid != nil {
            doc?.meta.transactionId = lastTxid!
        }
        return lastTxid
    }

    public func publishDid(_ did: String, signKey: String? = nil, _ storepass: String) throws -> String? {
        let _did = try DID(did)
        let _signKey = signKey == nil ? nil : try DIDURL(_did, signKey!)
        return try publishDid(_did, signKey: _signKey, storepass)
    }
    
    // Deactivate self use authentication keys
    public func deactivateDid(_ did: DID, signKey: DIDURL? = nil, _ storepass: String) throws -> String {
        // Document should use the IDChain's copy
        var localCopy = false
        var doc: DIDDocument?
        var sigk = signKey
        doc = try DIDBackend.shareInstance()?.resolve(did)
        if doc == nil {
            // Fail-back: try to load document from local store
            doc = try loadDid(did)
            if doc == nil {
                throw DIDError.didStoreError(_desc: "Can not resolve DID document.")
            }
            else {
                localCopy = true
            }
        }
        else {
            doc!.meta.store = self
        }
        
        if sigk == nil {
            sigk = doc?.getDefaultPublicKey()
        }
        let txid = try DIDBackend.shareInstance()!.deactivate(doc!, sigk!, storepass)
        // Save deactivated status to DID metadata
        if localCopy {
            doc?.meta.deactivated = true
            try storage.storeDidMeta(did, doc!.meta)
        }
        return txid
    }
    
    public func deactivateDid(_ did: String, signKey: String? = nil, _ storepass :String) throws -> String? {
        let _did = try DID(did)
        let _signKey = signKey == nil ? nil : try DIDURL(_did, signKey!)
        return try deactivateDid(_did, signKey: _signKey, storepass)
    }
    
    // Deactivate target DID with authorization
    public func deactivateDid(_ target: DID, _ did: DID, signKey: DIDURL? = nil, _ storepass: String) throws -> String {
         // All documents should use the IDChain's copy
        var doc: DIDDocument?
        var sigk = signKey
        doc = try DIDBackend.shareInstance()?.resolve(did)
        if doc == nil {
           // Fail-back: try to load document from local store
            doc = try loadDid(did)
            if doc == nil {
                throw DIDError.didStoreError(_desc: "Can not resolve DID document.")
            }
        }
        else {
            doc?.meta.store = self
        }
        
        var signPk: DIDPublicKey? = nil
        if sigk != nil {
            signPk = try doc?.getAuthenticationKey(signKey!)
            if signPk == nil {
                throw DIDError.failue("Not authentication key.")
            }
        }
        
        let targetDoc = try DIDBackend.shareInstance()?.resolve(target)
        if targetDoc == nil {
            throw DIDError.didResolveError(_desc: "DID \(target) not exist.")
        }
        
        if targetDoc?.getAuthorizationKeyCount() == 0 {
            throw DIDError.failue("No authorization.")
        }
        // The authorization key id in the target doc
        
        var targetSignKey: DIDURL?
        let authorizationKeys: Array<DIDPublicKey> = (targetDoc?.getAuthorizationKeys())!
        for targetPk in authorizationKeys {
            if targetPk.controller == did {
                if signPk != nil {
                    if targetPk.publicKeyBase58 == signPk?.publicKeyBase58 {
                        targetSignKey = targetPk.id
                    }
                    break
                }
                else {
                    let pks: Array<DIDPublicKey> = doc!.getAuthenticationKeys()
                    pks.forEach { pk in
                        if pk.publicKeyBase58 == targetPk.publicKeyBase58 {
                            signPk = pk
                            sigk = signPk?.id
                            targetSignKey = targetPk.id
                        }
                    }
                }
            }
        }
        if (targetSignKey == nil) {
            throw DIDError.failue("No matched authorization key.")
        }
        return try DIDBackend.shareInstance()!.deactivate(target, targetSignKey!, doc!, sigk!, storepass)
    }
    
    public func deactivateDid(_ target: String, _ did: String, signKey: String? = nil, _ storepass: String) throws -> String? {
        let _did = try DID(did)
        let _signKey = signKey == nil ? nil : try DIDURL(_did, signKey!)
        return try deactivateDid(DID(target), _did, signKey: _signKey, storepass)
    }
    
    public func resolveDid(_ did: DID, _ force: Bool) throws -> DIDDocument? {
        var doc = try DIDBackend.shareInstance()?.resolve(did)
        if doc !== nil {
            try storeDid(doc!)
        }
        if (doc == nil && !force){
                doc = try loadDid(did)
            }
        return doc
    }
    
    public func storeDid(_ doc: DIDDocument, _ alias: String) throws {
        doc.meta.alias = alias
        try storeDid(doc)
    }
    
    public func storeDid(_ doc: DIDDocument) throws {
        try storage.storeDid(doc)
        // TODO: Check me!!!
        let meta = try loadDidMeta(doc.subject!)
        try meta.merge(doc.meta)
        meta.store = self
        doc.meta = meta
        try storage.storeDidMeta(doc.subject!, meta)
        for vc in doc.getCredentials() {
            try storeCredential(vc)
        }
        
        if didCache != nil {
            didCache!.put(doc.subject!, data: doc)
        }
    }
    
   public func storeDidMeta(_ did: DID, _ meta: DIDMeta) throws {
        try storage.storeDidMeta(did, meta)
        if (didCache != nil) {
            let d = didCache!.get(did)
            if (d != nil) {
                let doc: DIDDocument = d as! DIDDocument
                doc.meta = meta
            }
        }
    }
    
    public func storeDidMeta(_ did: String, _ meta: DIDMeta) throws {
        try storeDidMeta(try DID(did), meta)
    }
    
    func loadDidMeta(_ did: DID) throws -> DIDMeta {
        var meta: DIDMeta?
        var doc: DIDDocument?
        
        if (didCache != nil) {
            doc = didCache!.get(did) as? DIDDocument
            if (doc != nil) {
                meta = doc!.meta
                if (meta != nil) {
                    return meta!
                }
            }
        }
        
        meta = try storage.loadDidMeta(did)
        if (doc != nil) {
            doc!.meta = meta!
        }
        
        return meta!
    }
    
    func loadDidMeta(_ did: String) throws -> DIDMeta {
        return try loadDidMeta(DID(did))
    }
    
    public func loadDid(_ did: DID) throws -> DIDDocument? {
        var doc = didCache!.get(did)
        if doc != nil {
            let d = doc as! DIDDocument
            return d
        }
        
        doc = try storage.loadDid(did)
        if (doc != nil) {
            let d = doc as! DIDDocument
            d.meta = try storage.loadDidMeta(did)
            d.meta.store = self
            didCache!.put(d.subject!, data: d)
        }
        let d = doc as? DIDDocument
        return d
    }

    public func loadDid(_ did: String) throws -> DIDDocument? {
        return try loadDid(DID(did))
    }
    
    public func containsDid(_ did: DID) throws -> Bool {
        return try storage.containsDid(did)
    }

    public func containsDid(_ did: String) throws -> Bool {
        return try containsDid(DID(did))
    }

    public func deleteDid(_ did: DID) throws -> Bool {
        return try storage.deleteDid(did)
    }

    public func deleteDid(_ did: String) throws -> Bool {
        return try deleteDid(DID(did))
    }

    public func listDids(_ filter: Int) throws -> Array<DID> {
        let dids = try storage.listDids(filter)
        for did in dids {
            let meta: DIDMeta = try loadDidMeta(did)
            meta.store = self
            did.meta = meta
        }

        return dids
    }
    
    public func storeCredential(_ credential: VerifiableCredential, _ alias: String) throws {
        credential.meta.alias = alias
        try storeCredential(credential)
    }
    
    public func storeCredential(_ credential: VerifiableCredential ) throws {
        try storage.storeCredential(credential)

        // TODO: Check me!!!
        let meta: CredentialMeta = try loadCredentialMeta(credential.subject.id, credential.id)!
        try meta.merge(credential.meta)
        meta.store = self
        credential.meta = meta

        credential.meta.store = self
        try storage.storeCredentialMeta(credential.subject.id, credential.id, meta)

        if (vcCache != nil) {
            vcCache!.put(credential.id!, data: credential)
        }
    }

    public func storeCredentialMeta(_ did: DID, _ id: DIDURL, _ meta: CredentialMeta) throws {
        try storage.storeCredentialMeta(did, id, meta)
        
        if (vcCache != nil) {
            let v = vcCache!.get(id)
            if (v != nil) {
                let vc: VerifiableCredential = v as! VerifiableCredential
                vc.meta = meta
            }
        }
    }
    
    public func storeCredentialMeta(_ did: String, _ id: String, _ meta: CredentialMeta) throws {
        let _did = try DID(did)
        try storeCredentialMeta(_did, try DIDURL(_did, id), meta)
    }
    
    public func loadCredentialMeta(_ did: DID, _ id: DIDURL) throws -> CredentialMeta? {
        
        var meta: CredentialMeta?
        var vc: VerifiableCredential?

        if (vcCache != nil) {
            vc = vcCache!.get(id) as? VerifiableCredential
            if (vc != nil) {
                meta = vc?.meta
                if (meta != nil) {
                    return meta!
                }
            }
        }
        meta = try storage.loadCredentialMeta(did, id)
        if (vc != nil) {
            vc!.meta = meta!
        }
        
        return meta
    }

    public func loadCredentialMeta(_ did: String, _ id: String) throws -> CredentialMeta? {
        let _did = try DID(did)
        return try loadCredentialMeta(_did, DIDURL(_did, id))
    }
    
    
    public func loadCredential(_ did: DID, _ id: DIDURL) throws -> VerifiableCredential? {
        var vc = vcCache!.get(id)
        let count = vcCache!.getCount()

        if count != 0 {
            if vc != nil {
                let v = vc as! VerifiableCredential
                return v
            }
        }
        vc = try storage.loadCredential(did, id)
        if (vc != nil) {
            let v = vc as! VerifiableCredential
            vcCache!.put(v.id!, data: v)
        }

        return vc as? VerifiableCredential
    }
    
    public func loadCredential(_ did: String, _ id: String) throws -> VerifiableCredential? {
        let _did: DID = try DID(did)
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

        let ids: Array<DIDURL> = try storage.listCredentials(did)

        for id in ids {
            let meta = try loadCredentialMeta(did, id)
            meta!.store = self
            id.meta = meta!
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
        let _did: DID = try DID(did)
        return try selectCredentials(_did, DIDURL(_did, id), type)
    }
    
    public func storePrivateKey(_ did: DID,_ id: DIDURL, _ privateKey: Data, _ storepass: String) throws {
        let encryptedKey = try DIDStore.encryptToBase64(storepass, privateKey)
        try storage.storePrivateKey(did, id, encryptedKey)
    }
    
    public func storePrivateKey(_ did: String,_ id: String, _ privateKey: Data, _ storepass: String) throws {
        let _did: DID = try DID(did)
       try storePrivateKey(_did, DIDURL(_did, id), privateKey, storepass)
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
        let _did: DID = try DID(did)
        return try containsPrivateKey(_did, DIDURL(_did, id))
    }
    
    public func deletePrivateKey(_ did: DID,_ id: DIDURL) throws -> Bool {
        return try storage.deletePrivateKey(did, id)
    }
    
    public func deletePrivateKey(_ did: String,_ id: String) throws -> Bool {
        let _did: DID = try DID(did)
        return try deletePrivateKey(_did, DIDURL(_did, id))
    }
    
    public func sign(_ did: DID, id: DIDURL? = nil, _ storepass: String, _ count: Int, _ inputs: [CVarArg]) throws -> String {
        let sig: UnsafeMutablePointer<Int8> = UnsafeMutablePointer<Int8>.allocate(capacity: 4096)
        var privatekeys: Data
        if id == nil {
            let doc = try loadDid(did)
            if doc == nil {
                throw DIDError.didStoreError(_desc: "Can not resolve DID document.") 
            }
            let id_1 = doc!.getDefaultPublicKey()
            privatekeys = try DIDStore.decryptFromBase64(storepass,try loadPrivateKey(did, id: id_1))
        }
        else {
            privatekeys = try DIDStore.decryptFromBase64(storepass,try loadPrivateKey(did, id: id!))
        }
        
        var cinputs: [CVarArg] = []
        for i in 0..<inputs.count {
            
            if (i % 2 == 0) {
                let json: String = inputs[i] as! String
                let cjson = json.toUnsafePointerInt8()!
                cinputs.append(cjson)
            }
            else {
                let count = inputs[i]
                cinputs.append(count)
            }
        }
        
        let toPPointer = privatekeys.toPointer()
        
        let c_inputs = getVaList(cinputs)
        // UnsafeMutablePointer(mutating: toPPointer)
        let re = ecdsa_sign_base64v(sig, UnsafeMutablePointer(mutating: toPPointer), Int32(count), c_inputs)
        guard re >= 0 else {
            throw DIDError.didStoreError(_desc: "sign error.")
        }
        let jsonStr: String = String(cString: sig)
        let endIndex = jsonStr.index(jsonStr.startIndex, offsetBy: re)
        let sig_ = String(jsonStr[jsonStr.startIndex..<endIndex])
        return sig_
    }
    
    public func changePassword(_ oldPassword: String, _ newPassword: String) throws {
        
        let ree: ReEncryptor = { (data: String) -> String in
            let udata: Data = try DIDStore.decryptFromBase64(oldPassword, data)
            let result: String = try DIDStore.encryptToBase64(newPassword, udata)
            return result
            } as! ReEncryptor
        try storage.changePassword(ree)
    }
    
    private func exportDid(did: DID, _ password: String, _ storepass: String) throws -> String {
        // All objects should load directly from storage,
        // avoid affects the cached objects.
        let sha256 = SHA256Helper()
        var bytes = [UInt8](password.data(using: .utf8)!)
        sha256.update(&bytes)
        
        var dic: OrderedDictionary<String, Any> = OrderedDictionary()
        var doc: DIDDocument? = nil
        do {
            doc = try storage.loadDid(did)
        } catch {
            throw DIDError.didStoreError(_desc: "Export DID \(did)failed.\(error)")
        }
        guard doc != nil else {
            throw DIDError.didStoreError(_desc: "Export DID \(did) failed, not exist.")
        }
        
        // Type
        dic["type"] = DIDStore.DID_EXPORT
        bytes = [UInt8](DIDStore.DID_EXPORT.data(using: .utf8)!)
        sha256.update(&bytes)
        
        // DID
        var value: String = did.description
        dic["id"] = value
        bytes = [UInt8](value.data(using: .utf8)!)
        sha256.update(&bytes)
        
        // Create
        let now = DateFormater.currentDate()
        value = DateFormater.format(now)
        dic["created"] = value
        bytes = [UInt8](value.data(using: .utf8)!)
        sha256.update(&bytes)
        
        // Document
        let dstr = doc?.toJson(false)
        let ddic = JsonHelper.handleString(dstr!)
        dic["document"] = ddic
        bytes = [UInt8](dstr!.data(using: .utf8)!)
        sha256.update(&bytes)
        
        var didMeta: DIDMeta? = try storage.loadDidMeta(did)
        if didMeta!.isEmpty() {
            didMeta = nil
        }
        
        // Credential
        var vcarr: Array<OrderedDictionary<String, Any>> = []
        var vcMetas: OrderedDictionary<DIDURL, CredentialMeta> = OrderedDictionary()
        if try storage.containsCredentials(did) {
            let ids = try listCredentials(did)
            for id in ids {
                var vc: VerifiableCredential? = nil
                do {
                    vc = try storage.loadCredential(did, id)
                } catch {
                    throw DIDError.didStoreError(_desc: "Export DID \(did) failed.\(error)")
                }
                let vcdic = vc!.toJson(false)
                let vcString = JsonHelper.creatJsonString(dic: vcdic)
                bytes = [UInt8](vcString.data(using: .utf8)!)
                sha256.update(&bytes)
                
                vcarr.append(vcdic)
                let meta = try storage.loadCredentialMeta(did, id)
                if !meta.isEmpty() {
                    vcMetas[id] = meta
                }
            }
        }
        dic["credential"] = vcarr
        
        // Private key
        vcarr.remove(at: 0)
        if try storage.containsPrivateKeys(did) {
            let list: Array<DIDPublicKey> = doc!.getPublicKeys()
            for pk in list {
                let id = pk.id
                if try storage.containsPrivateKey(did, id!) {
                    var csk: String = try storage.loadPrivateKey(did, id!)
                    
                    let cskData: Data = try DIDStore.decryptFromBase64(storepass, csk)
                    csk = try DIDStore.encryptToBase64(password, cskData)
                    
                    var dic: OrderedDictionary<String, Any> = OrderedDictionary()
                    let value = id!.description
                    dic["id"] = value
                    bytes = [UInt8](value.data(using: .utf8)!)
                    sha256.update(&bytes)
                    
                    dic["key"] = csk
                    bytes = [UInt8](csk.data(using: .utf8)!)
                    sha256.update(&bytes)
                    vcarr.append(dic)
                }
            }
            dic["privatekey"] = vcarr
        }
        
        // Metadata
        if didMeta != nil || vcMetas.count != 0 {
            var metaDic: OrderedDictionary<String, Any> = OrderedDictionary()
            
            if didMeta != nil {
                value = didMeta!.description
                metaDic["document"] = value
                bytes = [UInt8](value.description.data(using: .utf8)!)
                sha256.update(&bytes)
            }
            
            if vcMetas.count != 0 {
                var arr: Array<OrderedDictionary<String, Any>> = []
                
                vcMetas.forEach { (k, v) in
                    value = k.description
                    bytes = [UInt8](value.description.data(using: .utf8)!)
                    sha256.update(&bytes)
                    
                    var dic: OrderedDictionary<String, Any> = OrderedDictionary()
                    dic["id"] = v
                    value = v.description
                    dic["metadata"] = value
                    bytes = [UInt8](value.description.data(using: .utf8)!)
                    sha256.update(&bytes)
                    
                    arr.append(dic)
                }
                metaDic["credential"] = arr
            }
            dic["metadata"] = metaDic
        }
        
        // Fingerprint
        let result = sha256.finalize()
        let c_fing = UnsafeMutablePointer<Int8>.allocate(capacity: 4096)
        var re_fing: Data = Data(bytes: result, count: result.count)
        let c_fingerprint: UnsafeMutablePointer<UInt8> = re_fing.withUnsafeMutableBytes { (fing: UnsafeMutablePointer<UInt8>) -> UnsafeMutablePointer<UInt8> in
            return fing
        }
        let re = base64_url_encode(c_fing, c_fingerprint, re_fing.count)
        let jsonStr: String = String(cString: c_fing)
        let endIndex = jsonStr.index(jsonStr.startIndex, offsetBy: re)
        let fingerprint = String(jsonStr[jsonStr.startIndex..<endIndex])
        
        dic["fingerprint"] = fingerprint
        let str = JsonHelper.creatJsonString(dic: dic)
        return str
    }
    
    public func exportDid(_ did: DID, _ password: String, _ storepass: String) throws -> String {
        return try exportDid(did: did, password, storepass)
    }
    
    public func exportDid(_ did: String, _ password: String, _ storepass: String) throws -> String {
        return try exportDid(did: DID(did), password, storepass)
    }
    
    private func importDid(_ json: Dictionary<String, Any>, _ password: String, _ storepass: String)  throws {
        let sha256 = SHA256Helper()
        var bytes = [UInt8](password.data(using: .utf8)!)
        sha256.update(&bytes)
        var value: String = ""
        
        // Type
        let type = try JsonHelper.getString(json, "type", false, "export type")
        guard type == DIDStore.DID_EXPORT else {
            throw DIDError.didStoreError(_desc: "Invalid export data, unknown type.")
        }
        bytes = [UInt8](type.data(using: .utf8)!)
        sha256.update(&bytes)
        
        // DID
        let did = try JsonHelper.getDid(json, "id", false, "DID subject")
        value = did!.description
        bytes = [UInt8](value.data(using: .utf8)!)
        sha256.update(&bytes)
        
        // Created
        let created = try JsonHelper.getDate(json, "created", true, "export date")
        value = DateFormater.format(created!)
        bytes = [UInt8](value.data(using: .utf8)!)
        sha256.update(&bytes)
        
        // Document
        let jsonDoc: String? = json["document"] as? String
        guard jsonDoc != nil else {
            throw DIDError.didStoreError(_desc: "Missing DID document in the export data")
        }
        
        var doc: DIDDocument? = nil
        do {
            doc = try DIDDocument.fromJson(jsonDoc!)
        } catch  {
            throw DIDError.didStoreError(_desc: "Invalid export data.\(error)")
        }
        guard doc!.subject == did else {
            throw DIDError.didStoreError(_desc: "Invalid DID document in the export data.")
        }
        value = doc!.description
        bytes = [UInt8](value.data(using: .utf8)!)
        sha256.update(&bytes)
        
        // Credential
        var vcs: Dictionary<DIDURL, VerifiableCredential> = [: ]
        var re = json["credential"]
        if re != nil {
            guard re is Array<Any> else {
                throw DIDError.didStoreError(_desc: "Invalid export data, wrong credential data.")
            }
            for dic in re as! Array<Dictionary<String, Any>> {
                var vc: VerifiableCredential? = nil
                do {
                    vc = try VerifiableCredential.fromJson(dic, did!)
                } catch {
                    throw DIDError.didExpiredError(_desc: "Invalid export data.\(error)")
                }
                guard vc?.subject.id == did else {
                    throw DIDError.didStoreError(_desc: "Invalid credential in the export data.")
                }
                value = vc!.description(true)
                bytes = [UInt8](value.data(using: .utf8)!)
                sha256.update(&bytes)
                vcs[vc!.id] = vc
            }
        }
        
        // Private key
        var sks: Dictionary<DIDURL, String> = [: ]
        re = json["privatekey"]
        if re != nil {
            guard re is Array<Any> else {
                throw DIDError.didStoreError(_desc: "Invalid export data, wrong privatekey data.")
            }
            for dic in re as! Array<Dictionary<String, Any>> {
                let id = try JsonHelper.getDidUrl(dic, "id", ref: did, "privatekey id")
                value = id!.description
                bytes = [UInt8](value.data(using: .utf8)!)
                sha256.update(&bytes)
                
                var csk = try JsonHelper.getString(dic, "key", false, "privatekey")
                value = csk.description
                bytes = [UInt8](value.data(using: .utf8)!)
                sha256.update(&bytes)
                
                let sk: Data = try DIDStore.decryptFromBase64(password, csk)
                csk = try DIDStore.encryptToBase64(storepass, sk)
                sks[id!] = csk
            }
        }
        
        // Metadata
        re = json["metadata"]
        if re != nil {
            let redic = re as! Dictionary<String, Any>
            var m = redic["document"]
            if m != nil {
                let meta: DIDMeta = try DIDMeta.fromString(m as! String, DIDMeta.self)
                doc?.meta = meta
                value = meta.description
                bytes = [UInt8](value.data(using: .utf8)!)
                sha256.update(&bytes)
            }
            
            m = redic["credential"] as! String
            if re != nil {
                if m != nil {
                    guard m is Array<Any> else {
                        throw DIDError.didStoreError(_desc: "Invalid export data, wrong metadata.")
                    }
                    for dic in m as! Array<Dictionary<String, Any>>{
                        let id = try JsonHelper.getDidUrl(dic, "id", false, "credential id")
                        value = id!.description
                        bytes = [UInt8](value.data(using: .utf8)!)
                        sha256.update(&bytes)
                        
                        let meta: CredentialMeta = try CredentialMeta.fromString(dic["metadata"] as! String, CredentialMeta.self)
                        value = meta.description
                        bytes = [UInt8](value.data(using: .utf8)!)
                        sha256.update(&bytes)
                        
                        let vc: VerifiableCredential? = vcs[id!]
                        if vc != nil {
                            vc?.meta = meta
                        }
                    }
                }
            }
        }
        
        // Fingerprint
        re = json["fingerprint"]
        guard re != nil else {
            throw DIDError.didStoreError(_desc: "Missing fingerprint in the export data")
        }
        let refFingerprint = re as! String
        let result = sha256.finalize()
        
        let c_fing = UnsafeMutablePointer<Int8>.allocate(capacity: 4096)
        var re_fing: Data = Data(bytes: result, count: result.count)
        let c_fingerprint: UnsafeMutablePointer<UInt8> = re_fing.withUnsafeMutableBytes { (fing: UnsafeMutablePointer<UInt8>) -> UnsafeMutablePointer<UInt8> in
            return fing
        }
        let re_finger = base64_url_encode(c_fing, c_fingerprint, re_fing.count)
        let jsonStr: String = String(cString: c_fing)
        let endIndex = jsonStr.index(jsonStr.startIndex, offsetBy: re_finger)
        let fingerprint = String(jsonStr[jsonStr.startIndex..<endIndex])
        
        guard fingerprint == refFingerprint else {
            throw DIDError.didStoreError(_desc: "Invalid export data, the fingerprint mismatch.")
        }
        
        // All objects should load directly from storage,
        // avoid affects the cached objects.
        try storage.storeDid(doc!)
        try storage.storeDidMeta(doc!.subject!, doc?.meta)
        
        for vc in vcs.values {
            try storage.storeCredential(vc)
            try storage.storeCredentialMeta(did!, vc.id, vc.meta)
        }
        
        try sks.forEach { (key, value) in
            try storage.storePrivateKey(did!, key, value)
        }
    }
    
    public func importDid(file: String, _ password: String, _ storepass: String) throws {
        let dic: Dictionary<String, Any> = JsonHelper.handleString(jsonString: file) as! Dictionary<String, Any>
       try importDid(dic, password, storepass)
    }
    
    public func importDid(json: Dictionary<String, Any>, _ password: String, _ storepass: String) throws {
       try importDid(json, password, storepass)
    }
    
    private func exportPrivateIdentity(password: String, storepass: String) throws -> String {
        var encryptedMnemonic = try storage.loadMnemonic()
        var plain: Data = try DIDStore.decryptFromBase64(storepass, encryptedMnemonic)
        encryptedMnemonic = try DIDStore.encryptToBase64(password, plain)
        var encryptedSeed = try storage.loadPrivateIdentity()
        
        plain = try DIDStore.decryptFromBase64(storepass, encryptedSeed)
        encryptedSeed = try DIDStore.encryptToBase64(password, plain)
        
        let index = try storage.loadPrivateIdentityIndex()
        let sha256 = SHA256Helper()
        var bytes = [UInt8](password.data(using: .utf8)!)
        sha256.update(&bytes)
        
        // Type
        var dic: OrderedDictionary<String, Any> = OrderedDictionary()
        dic["type"] = DIDStore.DID_EXPORT
        bytes = [UInt8](DIDStore.DID_EXPORT.data(using: .utf8)!)
        sha256.update(&bytes)
        
        // Mnemonic
        dic["mnemonic"] = encryptedMnemonic
        bytes = [UInt8](encryptedMnemonic.data(using: .utf8)!)
        sha256.update(&bytes)
        
        // Key
        dic["key"] = encryptedSeed
        bytes = [UInt8](encryptedSeed.data(using: .utf8)!)
        sha256.update(&bytes)
        
        // Index
        dic["index"] = index
        let indexStr = String(index)
        bytes = [UInt8](indexStr.data(using: .utf8)!)
        sha256.update(&bytes)
        
        // Fingerprint
        let result = sha256.finalize()
        let c_fing = UnsafeMutablePointer<Int8>.allocate(capacity: 4096)
        var re_fing: Data = Data(bytes: result, count: result.count)
        let c_fingerprint: UnsafeMutablePointer<UInt8> = re_fing.withUnsafeMutableBytes { (fing: UnsafeMutablePointer<UInt8>) -> UnsafeMutablePointer<UInt8> in
            return fing
        }
        let re = base64_url_encode(c_fing, c_fingerprint, re_fing.count)
        let jsonStr: String = String(cString: c_fing)
        let endIndex = jsonStr.index(jsonStr.startIndex, offsetBy: re)
        let fingerprint = String(jsonStr[jsonStr.startIndex..<endIndex])
        
        dic["fingerprint"] = fingerprint
        let str = JsonHelper.creatJsonString(dic: dic)
        
        return str
    }
    
    public func exportPrivateIdentity(_ password: String, _ storepass: String) throws -> String {
       return try exportPrivateIdentity(password: password, storepass: storepass)
    }
    
    private func importPrivateIdentity(json: Dictionary<String, Any>, password: String
        , _ storepass: String) throws {
        let sha256 = SHA256Helper()
        var bytes = [UInt8](password.data(using: .utf8)!)
        sha256.update(&bytes)
        
        // Type
        let type = try JsonHelper.getString(json, "type", false, "export type")
        guard type == DIDStore.DID_EXPORT else {
            throw DIDError.didStoreError(_desc: "Invalid export data, unknown type.")
        }
        bytes = [UInt8](type.data(using: .utf8)!)
        sha256.update(&bytes)
        
        // Mnemonic
        var encryptedMnemonic = try JsonHelper.getString(json, "mnemonic", false, "mnemonic")
        bytes = [UInt8](encryptedMnemonic.data(using: .utf8)!)
        sha256.update(&bytes)
        
        var plain: Data = try DIDStore.decryptFromBase64(password, encryptedMnemonic)
        encryptedMnemonic = try DIDStore.encryptToBase64(storepass, plain)
     
        var encryptedSeed = try JsonHelper.getString(json, "key", false, "key")
        bytes = [UInt8](encryptedSeed.data(using: .utf8)!)
        sha256.update(&bytes)
        
        plain = try DIDStore.decryptFromBase64(password, encryptedSeed)
        encryptedSeed = try DIDStore.encryptToBase64(storepass, plain)
        
        let inde = json["index"]
        guard inde is Int else {
            throw DIDError.didStoreError(_desc: "Invalid export data, unknow index.")
        }
        let index: Int = inde as! Int
        let indexStr = String(index)
        bytes = [UInt8](indexStr.data(using: .utf8)!)
        sha256.update(&bytes)
        
        // Fingerprint
        let refFingerprint: String? = json["fingerprint"] as? String
        guard refFingerprint != nil else {
            throw DIDError.didStoreError(_desc: "Missing fingerprint in the export data")
        }
        let result = sha256.finalize()
        
        let c_fing = UnsafeMutablePointer<Int8>.allocate(capacity: 4096)
        var re_fing: Data = Data(bytes: result, count: result.count)
        let c_fingerprint: UnsafeMutablePointer<UInt8> = re_fing.withUnsafeMutableBytes { (fing: UnsafeMutablePointer<UInt8>) -> UnsafeMutablePointer<UInt8> in
            return fing
        }
        let re_finger = base64_url_encode(c_fing, c_fingerprint, re_fing.count)
        let jsonStr: String = String(cString: c_fing)
        let endIndex = jsonStr.index(jsonStr.startIndex, offsetBy: re_finger)
        let fingerprint = String(jsonStr[jsonStr.startIndex..<endIndex])
        
        guard fingerprint == refFingerprint else {
            throw DIDError.didStoreError(_desc: "Invalid export data, the fingerprint mismatch.")
        }
        
        // Save
        try storage.storeMnemonic(encryptedMnemonic)
        try storage.storePrivateIdentity(encryptedSeed)
        try storage.storePrivateIdentityIndex(index)
    }
    
    public func importPrivateIdentity(_ json: Dictionary<String, Any>, _ password: String, _ storepass: String) throws {
        try importPrivateIdentity(json: json, password: password, storepass)
    }

}


