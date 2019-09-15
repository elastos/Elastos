

import Foundation

class DIDStore: NSObject {

    public static let DID_HAS_PRIVATEKEY = 0
    public static let DID_NO_PRIVATEKEY = 1
    public static let DID_ALL = 2
    private static var instance: DIDStore!
    private static var privateIdentity: HDKey!
    private static var lastIndex: HDKey!

    override init() {
    }

    private init(_ type: String, _ location: String, _ passphase: String) {
        // TODO
    }

    public static func creatInstance(_ type: String, location: String, passphase: String) throws {
        guard type == "filesystem" else {
            throw DIDStoreError.failue("Unsupported store type:\(type)")
        }
        if instance == nil {
            let store: DIDStore = DIDStore(type, location, passphase)
            instance = store
        }
    }

    public static func shareInstance() -> DIDStore {
        return instance
    }

    public func hasPrivateIdentity() throws -> Bool {
        // TODO:
        return false
    }

    func storePrivateIdentity(_ key: String) throws {}

    func loadPrivateIdentity() throws -> String { return "" }

    func storePrivateIdentityIndex(_ index: Int) throws {}

    func loadPrivateIdentityIndex() throws -> Int { return 0 }

   private func encryptToBase64(_ passphrase: String ,_ input: Array<Any>) throws -> String{
        // TODO:
        return ""
    }

    public func initPrivateIdentity(_ mnemonic: String ,_ passphrase: String, _ force: Bool ) throws {
        // TODO:
    }

    func initPrivateIdentity(_ passphrase: String) throws -> Bool {
        // TODO:
        return false
    }

    public func newDid(_ passphrase:String, _ hint:String ) throws -> DIDDocument {
        // TODO:
        return DIDDocument()
    }

    public func newDid(_ passphrase: String ) throws -> DIDDocument {
        // TODO:
        return DIDDocument()
    }

    public func publishDid(_ doc: DIDDocument,_ signKey: DIDURL ,_ passphrase :String) throws -> Bool {
        // TODO:
        return false
    }

    public func updateDid(_ doc: DIDDocument,_ signKey: DIDURL ,_ passphrase :String) throws -> Bool {
        // TODO:
        return false
    }

    public func deactivateDid(_ doc: DIDDocument,_ signKey: DIDURL ,_ passphrase :String) throws -> Bool {
        // TODO:
        return false
    }

    public func resolveDid(_ did: DID) throws -> DIDDocument{
        // TODO DIDBackend generate
        let doc: DIDDocument = DIDDocument()
        return doc
    }

    public func storeDid(_ doc: DIDDocument, _ hint: String) throws {}
    public func storeDid(_ doc: DIDDocument) throws {

    }

    public func setDidHint(_ did: DID , _ hint:String ) throws {}

    public func getDidHint(_ did: DID) throws {}

    public func loadDid(_ did: DID) throws -> DIDDocument { return DIDDocument() }

    public func loadDid(_ did: String) -> DIDDocument {
        // TODO:
        return DIDDocument()
    }

    public func containsDid(_ did: DID) throws -> Bool { return false }

    public func containsDid(_ did: String) -> Bool {
        // TODO:
        return false
    }

    public func deleteDid(_ did: DID) throws -> Bool { return false }

    public func deleteDid(_ did: String) throws -> Bool {
        // TODO:
        return false
    }

    public func listDids(_ filter: Int) throws -> Array<Entry<DID, String>> { return [Entry]() }

    public func storeCredential(_ credential: VerifiableCredential, _ hint: String) throws { }
    public func storeCredential(_ credential: VerifiableCredential ) throws {
        // TODO:
    }

    public func setCredentialHint(_ did: DID, _ id: DIDURL, _ hint: String) throws {}
    public func getCredentialHint(_ did: DID, _ id: DIDURL) throws {}

//    public func loadCredential(_ did: DID, _ id: DIDURL) throws -> VerifiableCredential { return VerifiableCredential() }
//    public func loadCredential(_ did:String, _ id: String) throws -> VerifiableCredential { return VerifiableCredential() }

    public func containsCredentials(_ did:String) throws -> Bool { return false }

    public func containsCredential(_ did: DID , _ id: DIDURL) throws -> Bool { return false }
    public func containsCredential(_ did: String , _ id: String) throws -> Bool { return false }
    public func deleteCredential(_ did: DID , _ id: DIDURL) throws -> Bool{ return false }
    public func deleteCredential(_ did: String , _ id: String) throws -> Bool{ return false }

    public func listCredentials(_ did: DID) throws -> Array<Entry<DIDURL, String>> { return [Entry]() }
    public func listCredentials(_ did: String) throws -> Array<Entry<DIDURL, String>> { return [Entry]() }

    public func selectCredentials(_ did: DID, _ id: DIDURL,_ type: Array<Any>) throws -> Array<Entry<DIDURL, String>> { return [Entry]() }
    public func selectCredentials(_ did: String, _ id: String,_ type: Array<Any>) throws -> Array<Entry<DIDURL, String>> {
        // TODO:
        return [Entry]()
    }

    public func containsPrivateKeys(_ did: DID) throws -> Bool { return false }
    public func containsPrivateKeys(_ did: String) throws -> Bool { return false }

    public func containsPrivateKey(_ did: DID,_ id: DIDURL) throws -> Bool { return false }
    public func containsPrivateKey(_ did: String,_ id: String) throws -> Bool { return false }

    public func storePrivateKey(_ did: DID,_ id: DIDURL, _ privateKey:String) throws {}
    public func storePrivateKey(_ did: String,_ id: String, _ privateKey:String) throws {}

    func loadPrivateKey(_ did: DID, id: DIDURL) -> String { return "" }

    public func deletePrivateKey(_ did: DID,_ id: DIDURL) throws -> Bool { return false }
    public func deletePrivateKey(_ did: String,_ id: String) throws -> Bool { return false }

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
