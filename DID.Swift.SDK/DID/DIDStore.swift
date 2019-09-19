

import Foundation
import BitcoinKit
//import CryptoSwift

class DIDStore: NSObject {

    public static let DID_HAS_PRIVATEKEY = 0
    public static let DID_NO_PRIVATEKEY = 1
    public static let DID_ALL = 2
    private static var instance: DIDStore!
    private var privateIdentity: HDKey!
    private var lastIndex: Int!

    override init() {
    }

    private init(_ type: String, _ location: String, _ passphase: String) throws {
        // TODO:
    }

    public static func creatInstance(_ type: String, location: String, passphase: String) throws {
        guard type == "filesystem" else {
            throw DIDStoreError.failue("Unsupported store type:\(type)")
        }
        if instance == nil {
            let store: DIDStore = try DIDStore(type, location, passphase)
            instance = store
        }
    }

    public static func shareInstance() -> DIDStore {
        return instance
    }

    public func hasPrivateIdentity() throws -> Bool { return false }

    func storePrivateIdentity(_ key: String) throws {}

    func loadPrivateIdentity() throws -> String { return "" }

    func storePrivateIdentityIndex(_ index: Int) throws {}

    func loadPrivateIdentityIndex() throws -> Int { return 0 }

   private func encryptToBase64(_ passphrase: String ,_ input: Data) throws -> String{
    // TODO:
        return ""
    }

    private func decryptFromBase64(_ passphrase: String ,_ input: String) throws -> Data{
        // TODO:
        return Data()
    }

    public func initPrivateIdentity(_ mnemonic: String ,_ passphrase: String, _ force: Bool ) throws {

        if (try hasPrivateIdentity() && !force) {
            // TODO: throws error
        }
        privateIdentity = try HDKey.fromMnemonic(mnemonic, passphrase)
        lastIndex = 0

        // Save seed instead of root private key,
        // keep compatible with Native SDK
        let encryptedIdentity = try encryptToBase64(passphrase, privateIdentity.getSeed())
        try storePrivateIdentity(encryptedIdentity)
        try storePrivateIdentityIndex(lastIndex)
    }

    func initPrivateIdentity(_ passphrase: String) throws -> Bool {
        if !(try hasPrivateIdentity()) {
            return false
        }
        let seed: Data = try decryptFromBase64(passphrase, try loadPrivateIdentity())
        privateIdentity = HDKey.fromSeed(seed)
        lastIndex = 0
        try lastIndex = loadPrivateIdentityIndex()
        return true
    }

    public func newDid(_ passphrase:String, _ hint:String?) throws -> DIDDocument {
        if privateIdentity == nil {
            // TODO: THROWS EROR
        }

//        let key: HDPrivateKey = try privateIdentity.derive(lastIndex++)
//        let did: DID = DID(DID.METHOD, key)
//        let pk: PublicKey = PublicKey(try DIDURL(did, "primary"), Constants.defaultPublicKeyType, did, key)
//        let doc: DIDDocument = DIDDocument()
//        doc.subject = did
//        doc.addPublicKey(pk)
//        doc.addAuthenticationKey(pk)
//        doc.readonly = true
//
//        try storeDid(doc, hint!)
//        let encryptedKey: String = encryptToBase64(passphrase, key.)
//


        return DIDDocument()
    }

    public func newDid(_ passphrase: String) throws -> DIDDocument {
        return try newDid(passphrase, nil)
    }

    public func publishDid(_ doc: DIDDocument,_ signKey: DIDURL ,_ passphrase :String) throws -> Bool {
        return try DIDBackend.create(doc, signKey, passphrase)
    }

    public func updateDid(_ doc: DIDDocument,_ signKey: DIDURL ,_ passphrase :String) throws -> Bool {
        try storeDid(doc)
        return try DIDBackend.update(doc, signKey, passphrase)
    }

    public func deactivateDid(_ did: DID,_ signKey: DIDURL ,_ passphrase :String) throws -> Bool {
        // TODO: how to handle locally?
        return try DIDBackend.deactivate(did, signKey, passphrase)
    }

    public func resolveDid(_ did: DID) throws -> DIDDocument{
        // TODO DIDBackend generate
        var doc = try DIDBackend.resolve(did)
        if doc !== nil {
            try storeDid(doc!)
        }
        else {
            doc = try loadDid(did)
        }
        return doc!
    }

    public func storeDid(_ doc: DIDDocument, _ hint: String?) throws {}
    public func storeDid(_ doc: DIDDocument) throws {
        try storeDid(doc, nil)
    }

    public func setDidHint(_ did: DID , _ hint:String ) throws {}

    public func getDidHint(_ did: DID) throws {}

    public func loadDid(_ did: DID) throws -> DIDDocument { return DIDDocument() }

    public func loadDid(_ did: String) throws -> DIDDocument {
        return try loadDid(DID(did))
    }

    public func containsDid(_ did: DID) throws -> Bool { return false }

    public func containsDid(_ did: String) throws -> Bool {
        return try containsDid(DID(did))
    }

    public func deleteDid(_ did: DID) throws -> Bool { return false }

    public func deleteDid(_ did: String) throws -> Bool {
        return try deleteDid(DID(did))
    }

    public func listDids(_ filter: Int) throws -> Array<Entry<DID, String>> { return [Entry]() }

    public func storeCredential(_ credential: VerifiableCredential, _ hint: String?) throws { }
    public func storeCredential(_ credential: VerifiableCredential ) throws {
        return try storeCredential(credential, nil)
    }

    public func setCredentialHint(_ did: DID, _ id: DIDURL, _ hint: String) throws {}
    public func getCredentialHint(_ did: DID, _ id: DIDURL) throws {}

//    public func loadCredential(_ did: DID, _ id: DIDURL) throws -> VerifiableCredential { return VerifiableCredential() }
//    public func loadCredential(_ did:String, _ id: String) throws -> VerifiableCredential { return VerifiableCredential() }

    public func containsCredentials(_ did:DID) throws -> Bool { return false }
    public func containsCredentials(_ did: String) throws -> Bool {
        return try containsCredentials(DID(did))
    }

    public func containsCredential(_ did: DID, _ id: DIDURL) throws -> Bool { return false }
    public func containsCredential(_ did: String, _ id: String) throws -> Bool {
        return try containsCredential(DID(did), DIDURL(id))
    }

    public func deleteCredential(_ did: DID , _ id: DIDURL) throws -> Bool{ return false }
    public func deleteCredential(_ did: String , _ id: String) throws -> Bool{
        return try deleteCredential(DID(did), DIDURL(id))
    }

    public func listCredentials(_ did: DID) throws -> Array<Entry<DIDURL, String>> { return [Entry]() }
    public func listCredentials(_ did: String) throws -> Array<Entry<DIDURL, String>> {
        return try listCredentials(DID(did))
    }

    public func selectCredentials(_ did: DID, _ id: DIDURL,_ type: Array<Any>) throws -> Array<Entry<DIDURL, String>> { return [Entry]() }
    public func selectCredentials(_ did: String, _ id: String,_ type: Array<Any>) throws -> Array<Entry<DIDURL, String>> {
        return try selectCredentials(DID(did), DIDURL(id), type)
    }

    public func containsPrivateKeys(_ did: DID) throws -> Bool { return false }
    public func containsPrivateKeys(_ did: String) throws -> Bool {
        return try containsPrivateKeys(DID(did))
    }

    public func containsPrivateKey(_ did: DID,_ id: DIDURL) throws -> Bool { return false }
    public func containsPrivateKey(_ did: String,_ id: String) throws -> Bool {
        return try containsPrivateKey(DID(did), DIDURL(id))
    }

    public func storePrivateKey(_ did: DID,_ id: DIDURL, _ privateKey:String) throws {}
    public func storePrivateKey(_ did: String,_ id: String, _ privateKey:String) throws {
       try storePrivateKey(DID(did), DIDURL(id), privateKey)
    }

    func loadPrivateKey(_ did: DID, id: DIDURL) -> String { return "" }

    public func deletePrivateKey(_ did: DID,_ id: DIDURL) throws -> Bool { return false }
    public func deletePrivateKey(_ did: String,_ id: String) throws -> Bool {
        return try deletePrivateKey(DID(did), DIDURL(id))
    }

//    public func sign(_ did: DID,_ id: DIDURL,_ passphrase: String,_ data: Array<Any>, _ nonce: Array<Any> ) throws -> String {
//        return ""
//    }
//    public func sign(_ did: DID,_ id: DIDURL,_ passphrase: String,_ data: Array<Any>) throws -> String {
//        return ""
//    }
//
//    public func sign(did: String, _ id: String, _ passphrase: String, _ data: Array<Any>) throws -> String {
//        return ""
//    }

}


extension Int {

    static prefix  func ++(num:inout Int) -> Int  {
        num += 1
        return num
    }

    static postfix  func ++(num:inout Int) -> Int  {
        let temp = num
        num += 1
        return temp
    }

    static prefix  func --(num:inout Int) -> Int  {
        num -= 1
        return num
    }

    static postfix  func --(num:inout Int) -> Int  {
        let temp = num
        num -= 1
        return temp
    }
}
