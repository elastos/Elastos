
import Foundation

/*
 * FileSystem DID Store: storage layout
 *
 *  + DIDStore root
 *    - .DIDStore                        [Store tag file, include magic and version]
 *    + private                            [Personal root private key for HD identity]
 *      - key                            [HD root private key]
 *      - index                            [Last derive index]
 *    + ids
 *      - .ixxxxxxxxxxxxxxx0.meta        [Meta for DID, alias only, OPTIONAL]
 *      + ixxxxxxxxxxxxxxx0             [DID root, named by id specific string]
 *        - document                    [DID document, json format]
 *          + credentials                [Credentials root, OPTIONAL]
 *            - credential-id-0            [Credential, json format, named by id' fragment]
 *            - .credential-id-0.meta    [Meta for credential, alias only, OPTONAL]
 *            - ...
 *            - credential-id-N
 *            - .credential-id-N.meta
 *          + privatekeys                [Private keys root, OPTIONAL]
 *            - privatekey-id-0            [Encrypted private key, named by pk' id]
 *            - ...
 *            - privatekey-id-N
 *
 *      ......
 *
 *      - .ixxxxxxxxxxxxxxxN.meta
 *      + ixxxxxxxxxxxxxxxN
 *
 */
class FileSystemStore: DIDStore {

    private static let TAG_FILE: String = ".DIDStore"

    private static let PRIVATE_DIR: String = "private"
    private static let HDKEY_FILE: String = "key"
    private static let INDEX_FILE: String = "index"

    private static let DID_DIR: String = "ids"
    private static let DOCUMENT_FILE: String = "document"
    private static let CREDENTIALS_DIR: String = "credentials"
    private static let PRIVATEKEYS_DIR: String = "privatekeys"
    private static let META_EXT: String = ".meta"

    private static let DEFAULT_CHARSET: String = "UTF-8"

    private var storeRoot: String!

    init(_ dir: String) throws {
        super.init()
        if dir.isEmpty {
            // TODO ERROR
        }

        let fileManager = FileManager.default
        let exist: Bool = fileManager.fileExists(atPath: dir)

        if exist {
            try checkStore(dir)
        } else {
            try createStore(dir)
        }
    }

    private func createStore(_ dir: String) throws {

        let fileManager = FileManager.default
        do {
            try fileManager.createDirectory(atPath: dir, withIntermediateDirectories: true, attributes: nil)
        }
        catch{
            // TODO:
        }
        //        let cachePath: String = "\(dir)/\(path)"

    }

    private func checkStore(_ filePath: String) throws {
        // TODO
    }

    override public func hasPrivateIdentity() -> Bool {

        let fileManager = FileManager.default
        return fileManager.fileExists(atPath: getHDPrivateKeyFile())
    }

    override public func storePrivateIdentity(_ key: String) throws {

        do {
            let path = try getHDPrivateKeyFile(true)
            try writeTextToPath(path, key)
        }
        catch{
            // TODO throw error
        }
    }

    override public func loadPrivateIdentity() throws -> String {

        do {
            let path = FileSystemStore.PRIVATE_DIR + "/" + FileSystemStore.INDEX_FILE

            try readTextFromPath(path)
        }
        catch{
            // TODO throw error
        }
        return ""
    }

    override public func storePrivateIdentityIndex(_ index: Int) throws {

        do {
            let targetPath = FileSystemStore.PRIVATE_DIR + "/" + FileSystemStore.INDEX_FILE
            let path = try getFile(true, targetPath)
            try writeTextToPath(path, String(index))
            _ = try readTextFromPath(path)
        }
        catch{
            // TODO throw error
        }
    }

    override func loadPrivateIdentityIndex() throws -> Int {
        return 0
    }

    override public func setDidHint(_ did: DID,_ hint: String) throws {

    }

    override public func getDidHint(_ did: DID) throws {

    }

    override public func storeDid(_ doc: DIDDocument ,_ hint: String ) throws {

    }

    override func loadDid(_ did: String) -> DIDDocument {
        return DIDDocument()
    }

    override public func containsDid(_ did: DID) throws -> Bool {
        return false
    }

    override public func deleteDid(_ did: DID) throws -> Bool {
        return false
    }

    override func listDids(_ filter: Int) throws -> Array<Entry<DID, String>> {
        return [Entry]()
    }

    override func setCredentialHint(_ did: DID, _ id: DIDURL, _ hint: String) throws {

    }

    override func getCredentialHint(_ did: DID, _ id: DIDURL) throws {

    }


    override public func storeCredential(_ credential: VerifiableCredential , _ hint: String) throws {

    }

    // TODO: override loadCredential

    override func containsCredentials(_ did: String) throws -> Bool {
        return false
    }

    override func containsCredential(_ did: DID, _ id: DIDURL) throws -> Bool {
        return false
    }

    override func containsCredential(_ did: String, _ id: String) throws -> Bool {
        return false
    }

    override func deleteCredential(_ did: DID, _ id: DIDURL) throws -> Bool {
        return false
    }

    override public func listCredentials(_ did: DID) throws -> Array<Entry<DIDURL, String>> {
        return [Entry]()
    }

    override func listCredentials(_ did: String) throws -> Array<Entry<DIDURL, String>> {
        return [Entry]()
    }

    override func selectCredentials(_ did: DID, _ id: DIDURL, _ type: Array<Any>) throws -> Array<Entry<DIDURL, String>> {
        return [Entry]()
    }

    override func selectCredentials(_ did: String, _ id: String, _ type: Array<Any>) throws -> Array<Entry<DIDURL, String>> {
        return [Entry]()
    }

    override func containsPrivateKeys(_ did: DID) throws -> Bool  {
        // TODO
        return false
    }

    override func containsPrivateKeys(_ did: String) throws -> Bool {
        return false
    }

    override func containsPrivateKey(_ did: DID, _ id: DIDURL) throws -> Bool {
        return false
    }

    override func containsPrivateKey(_ did: String, _ id: String) throws -> Bool {
        return false
    }

    override func storePrivateKey(_ did: DID, _ id: DIDURL, _ privateKey: String) throws {
        // TODO
    }

    override func storePrivateKey(_ did: String, _ id: String, _ privateKey: String) throws {
        // TODO
    }

    override func loadPrivateKey(_ did: DID, id: DIDURL) -> String {
        return ""
    }

    override func deletePrivateKey(_ did: DID, _ id: DIDURL) throws -> Bool {
        // TODO
        return false
    }


    private func getFile(_ path: String) throws -> String? {

        do {
            return try getFile(false, path)
        }
        catch{
            return nil
        }
    }

    private func getFile(_ create: Bool, _ path: String) throws -> String {

        let store: String = "\(NSHomeDirectory())/Library/Caches/temp"

        // TODO
        return ""
    }

    private func getDir(_ path: String) throws -> String {
        // TODO
        return ""
    }

    private func writeTextToPath(_ path: String, _ text: String) throws {
        // TODO
    }

    private func readTextFromPath(_ path: String) throws -> String {
        // TODO
        return ""
    }

    private func getHDPrivateKeyFile(_ create: Bool) throws -> String{

        let path = FileSystemStore.PRIVATE_DIR + "/" + FileSystemStore.HDKEY_FILE
        return try getFile(create, path)
    }

    private func getHDPrivateKeyFile() -> String {
        // TODO
        return ""
    }

}


public class Entry<K, V> {
    var key: K!
    var value: V!

    init(_ key: K, _ value: V) {
        self.key = key
        self.value = value
    }
}
