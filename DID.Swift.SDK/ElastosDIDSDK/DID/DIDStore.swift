import Foundation

public class DIDStore: NSObject {
    public static let CACHE_INITIAL_CAPACITY = 16
    public static let CACHE_MAX_CAPACITY = 32
    
    public static let DID_HAS_PRIVATEKEY = 0
    public static let DID_NO_PRIVATEKEY = 1
    public static let DID_ALL = 2
    
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
            throw DIDStoreError.failue("Unsupported store type:\(type)")
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

    func encryptToBase64(_ passwd: String ,_ input: Data) throws -> String {
        let cinput: UnsafePointer<UInt8> = input.withUnsafeBytes{ (by: UnsafePointer<UInt8>) -> UnsafePointer<UInt8> in
            return by
        }
        let base64url: UnsafeMutablePointer<Int8> = UnsafeMutablePointer.allocate(capacity: 4096)
          let re = encrypt_to_base64(base64url, passwd, cinput, input.count)
        guard re >= 0 else {
            throw DIDStoreError.failue("encryptToBase64 error.")
        }
        var json: String = String(cString: base64url)
        let endIndex = json.index(json.startIndex, offsetBy: re)
        json = String(json[json.startIndex..<endIndex])
        return json
    }
    
    public func decryptFromBase64(_ passwd: String ,_ input: String) throws -> [Int8] {
        let plain: UnsafeMutablePointer<UInt8> = UnsafeMutablePointer<UInt8>.allocate(capacity: 4096)
        let re = decrypt_from_base64(plain, passwd, input)
        guard re >= 0 else {
            throw DIDStoreError.failue("decryptFromBase64 error.")
        }
        let temp = UnsafeRawPointer(plain)
        .bindMemory(to: UInt8.self, capacity: re)
        
        let data = Data(bytes: temp, count: re)
        let intArray = [UInt8](data).map { Int8(bitPattern: $0) }
        print(intArray)
        return intArray
    }
    
    public func decryptFromBase64(_ passwd: String ,_ input: String) throws -> Data {
        let plain: UnsafeMutablePointer<UInt8> = UnsafeMutablePointer<UInt8>.allocate(capacity: 4096)
        let re = decrypt_from_base64(plain, passwd, input)
        guard re >= 0 else {
            throw DIDStoreError.failue("decryptFromBase64 error.")
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
            throw DIDStoreError.failue("Already has private indentity.")
        }
        let privateIdentity: HDKey = try HDKey.fromMnemonic(mnemonic, passphrase)

        // Save seed instead of root private key,
        // keep compatible with Native SDK
        let seedData = privateIdentity.getSeed()
        let encryptedIdentity = try encryptToBase64(storepass, seedData)
        try storage!.storePrivateIdentity(encryptedIdentity)
        try storage!.storePrivateIdentityIndex(0)
        
        // Save mnemonic
        let mnemData = mnemonic.data(using: .utf8)
        let encryptedMnemonic: String = try encryptToBase64(storepass, mnemData!)
        try storage.storeMnemonic(encryptedMnemonic)
    }
    
    func initPrivateIdentity(_ language: Int, _ mnemonic: String, _ passphrase: String, _ storepass: String) throws -> Void {
        try initPrivateIdentity(language, mnemonic, passphrase, storepass, false)
    }
    
    public func exportMnemonic(_ storepass: String) throws -> String {
        let encryptedMnemonic: String = try storage.loadMnemonic()
        let re: Data = try decryptFromBase64(storepass, encryptedMnemonic)
        return String(data: re, encoding: .utf8)!
    }
    
    func loadPrivateIdentity(_ storepass: String) throws -> HDKey {
        guard try containsPrivateIdentity() else {
            throw DIDStoreError.failue("DID Store not contains private identity.")
        }
        let seed: Data = try decryptFromBase64(storepass, storage.loadPrivateIdentity())
        return HDKey.fromSeed(seed)
    }

    public func synchronize(_ storepass: String) throws {
        let privateIdentity: HDKey = try loadPrivateIdentity(storepass)
        let nextIndex: Int = try storage.loadPrivateIdentityIndex()
        var blanks: Int = 0
        var i: Int = 0
        
        while i < nextIndex || blanks < 10 {
            let key: DerivedKey = try privateIdentity.derive(i++)
            let pks: [UInt8] = try key.getPublicKeyBytes()
            let methodIdString: String = DerivedKey.getIdString(pks)
            let did: DID = DID(DID.METHOD, methodIdString)
            var doc: DIDDocument?
            do {
                doc = try DIDBackend.shareInstance().resolve(did, true)
            } catch {
                if error is DIDResolveError {
                    blanks++
                }
                continue
            }
            
            if (doc != nil) {
                // TODO: check local conflict
                try storeDid(doc!)
                
                // Save private key
                let privatekeyData: Data = try key.getPrivateKeyData()
                let encryptedKey: String = try encryptToBase64(storepass, privatekeyData)
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
    
    public func newDid(_ storepass:String, _ alias: String?) throws -> DIDDocument {
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
        var doc: DIDDocument = DIDDocument(did)
        
        _ = try doc.addAuthenticationKey(id, try key.getPublicKeyBase58())
        
        doc = try doc.seal(self, storepass)
        doc.meta.alias = alias ?? ""
        try storeDid(doc)
        try storage.storePrivateIdentityIndex(nextIndex)
        
        return doc
    }
    
    public func newDid(_ storepass: String) throws -> DIDDocument {
        return try newDid(storepass, nil)
    }
    
    public func publishDid(_ did: DID, _ signKey: DIDURL?, _ storepass: String) throws -> String? {
        let doc = try loadDid(did)
        guard doc != nil else {
            throw DIDStoreError.failue("Can not find the document for \(did)")
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
            lastTxid = try DIDBackend.shareInstance().create(doc!, sigk!, storepass)
        }
        else {
            lastTxid = try DIDBackend.shareInstance().update(doc!, lastTxid, sigk!, storepass)
        }
        if lastTxid != nil {
            doc?.meta.transactionId = lastTxid!
        }
        return lastTxid
    }

    public func publishDid(_ did: String, _ signKey: String?, _ storepass: String) throws -> String? {
        let _did = try DID(did)
        let _signKey = signKey == nil ? nil : try DIDURL(_did, signKey!)
        return try publishDid(_did, _signKey, storepass)
    }
    
    public func publishDid(_ did: DID, _ storepass :String) throws -> String? {
        return try publishDid(did, nil, storepass)
    }
    
    public func publishDid(_ did: String, _ storepass :String) throws -> String? {
        return try publishDid(did, nil, storepass)
    }
    
    // Deactivate self use authentication keys
    public func deactivateDid(_ did: DID, _ signKey: DIDURL?, _ storepass: String) throws -> String {
        // Document should use the IDChain's copy
        var localCopy = false
        var doc: DIDDocument?
        var sigk = signKey
        doc = try DIDBackend.shareInstance().resolve(did)
        if doc == nil {
            // Fail-back: try to load document from local store
            doc = try loadDid(did)
            if doc == nil {
                throw DIDStoreError.failue("Can not resolve DID document.")
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
        let txid = try DIDBackend.shareInstance().deactivate(doc!, sigk!, storepass)
        // Save deactivated status to DID metadata
        if localCopy {
            doc?.meta.deactivated = true
            try storage.storeDidMeta(did, doc!.meta)
        }
        return txid
    }
    
    public func deactivateDid(_ did: String,_ signKey: String?, _ storepass :String) throws -> String? {
        let _did = try DID(did)
        let _signKey = signKey == nil ? nil : try DIDURL(_did, signKey!)
        return try deactivateDid(_did, _signKey, storepass)
    }

    public func deactivateDid(_ did: DID, _ storepass :String) throws -> String? {
        return try deactivateDid(did, nil, storepass)
    }

    public func deactivateDid(_ did: String, _ storepass :String) throws -> String? {
        return try deactivateDid(did, nil, storepass)
    }
    
    // Deactivate target DID with authorization
    public func deactivateDid(_ target: DID, _ did: DID, _ signKey: DIDURL?, _ storepass: String) throws -> String {
         // All documents should use the IDChain's copy
        var doc: DIDDocument?
        var sigk = signKey
        doc = try DIDBackend.shareInstance().resolve(did)
        if doc == nil {
           // Fail-back: try to load document from local store
            doc = try loadDid(did)
            if doc == nil {
                throw DIDStoreError.failue("Can not resolve DID document.")
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
        
        let targetDoc = try DIDBackend.shareInstance().resolve(target)
        if targetDoc == nil {
            throw DIDResolveError.failue("DID \(target) not exist.")
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
                    if targetPk.keyBase58 == signPk?.keyBase58 {
                        targetSignKey = targetPk.id
                    }
                    break
                }
                else {
                    let pks: Array<DIDPublicKey> = doc!.getAuthenticationKeys()
                    pks.forEach { pk in
                        if pk.keyBase58 == targetPk.keyBase58 {
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
        return try DIDBackend.shareInstance().deactivate(target, targetSignKey!, doc!, sigk!, storepass)!
    }
    
    public func deactivateDid(_ target: String, _ did: String, _ signKey: String?, _ storepass: String) throws -> String? {
        let _did = try DID(did)
        let _signKey = signKey == nil ? nil : try DIDURL(_did, signKey!)
        return try deactivateDid(DID(target), _did, _signKey, storepass)
    }
    
    public func deactivateDid(_ target: DID, _ did: DID, _ storepass: String) throws -> String? {
        return try deactivateDid(target, did, nil, storepass)
    }
    
    public func resolveDid(_ did: DID, _ force: Bool) throws -> DIDDocument? {
        var doc = try DIDBackend.shareInstance().resolve(did)
        if doc !== nil {
            try storeDid(doc!)
        }
        if (doc == nil && !force){
                doc = try loadDid(did)
            }
        return doc
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

        var ids: Array<DIDURL> = try storage.listCredentials(did)

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
        let encryptedKey = try encryptToBase64(storepass, privateKey)
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

    public func sign(_ did: DID, _ id: DIDURL? = nil, _ storepass: String, _ count: Int, _ inputs: [CVarArg]) throws -> String {
        let sig: UnsafeMutablePointer<Int8> = UnsafeMutablePointer<Int8>.allocate(capacity: 4096)
        var privatekeys: Data
        if id == nil {
            let doc = try loadDid(did)
            if doc == nil {
                throw DIDStoreError.failue("Can not resolve DID document.")
            }
            let id_1 = doc!.getDefaultPublicKey()
            privatekeys = try decryptFromBase64(storepass,try loadPrivateKey(did, id: id_1))
        }
        else {
            privatekeys = try decryptFromBase64(storepass,try loadPrivateKey(did, id: id!))
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
            throw DIDStoreError.failue("sign error.")
        }
        let jsonStr: String = String(cString: sig)
        let endIndex = jsonStr.index(jsonStr.startIndex, offsetBy: re)
        let sig_ = String(jsonStr[jsonStr.startIndex..<endIndex])
        return sig_
    }
}

