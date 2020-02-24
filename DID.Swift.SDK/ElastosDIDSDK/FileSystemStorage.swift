import Foundation

/*
 * FileSystem DID Store: storage layout
 *
 *  + DIDStore root
 *    - .meta                            [Store meta file, include magic and version]
 *    + private                            [Personal root private key for HD identity]
 *      - key                            [HD root private key]
 *      - index                            [Last derive index]
 *    + ids
 *      + ixxxxxxxxxxxxxxx0             [DID root, named by id specific string]
 *        - .meta                        [Meta for DID, alias only, OPTIONAL]
 *        - document                    [DID document, json format]
 *        + credentials                    [Credentials root, OPTIONAL]
 *          + credential-id-0           [Credential root, named by id' fragment]
 *            - .meta                    [Meta for credential, alias only, OPTONAL]
 *            - credential                [Credential, json format]
 *          + ...
 *          + credential-id-N
 *            - .meta
 *            - credential
 *        + privatekeys                    [Private keys root, OPTIONAL]
 *          - privatekey-id-0            [Encrypted private key, named by pk' id]
 *          - ...
 *          - privatekey-id-N
 *
 *      ......
 *
 *      + ixxxxxxxxxxxxxxxN
 *
 */
public class FileSystemStorage: DIDStorage {
    private static let STORE_MAGIC:   [UInt8] = [0x00, 0x0D, 0x01, 0x0D]
    private static let STORE_VERSION: [UInt8] = [0x00, 0x00, 0x00, 0x01]
    private static let STORE_META_SIZE = 8

    private static let DEFAULT_CHARSET = "UTF-8"
    
    private var _rootPath: String
    
    init(_ dir: String) throws {
        guard !dir.isEmpty else {
            throw DIDError.didStoreError("Invalid DIDStore root directory.")
        }

        self._rootPath = dir
        if FileManager.default.fileExists(atPath: dir) {
            try checkStore()
        } else {
            try initializeStore()
        }
    }

    private func initializeStore() throws {
        do {
            try FileManager.default.createDirectory(atPath: self._rootPath,
                               withIntermediateDirectories: true,
                                                attributes: nil)

            var data: Data = Data(capacity: 8)
            data.append(contentsOf: FileSystemStorage.STORE_MAGIC)
            data.append(contentsOf: FileSystemStorage.STORE_VERSION)

            let file = try getFileHandle(false, Constants.META_FILE)
            file.write(data)
            file.closeFile()
        } catch {
            throw DIDError.didStoreError("Initialize DIDStore \(_rootPath) error")
        }
    }

    private func checkStore() throws {
        var isDir: ObjCBool = false

        // Further to check the '_rootPath' is not a file path.
        guard FileManager.default.fileExists(atPath: self._rootPath, isDirectory: &isDir) && isDir.boolValue else {
            throw DIDError.didStoreError("Store root \(_rootPath) is a file path")
        }

        // Check 'META_FILE' file exists or not.
        let file: FileHandle
        do {
            file = try getFileHandle(false, Constants.META_FILE)
        } catch {
            throw DIDError.didStoreError("Directory \(_rootPath) is not DIDStore directory")
        }

        // Check 'META_FILE' file size is equal to STORE_META_FILE.
        let data = file.readDataToEndOfFile()
        guard data.count != FileSystemStorage.STORE_META_SIZE else {
            throw DIDError.didStoreError("Directory \(_rootPath) is not DIDStore directory")
        }

        // Check MAGIC & VERSION
        guard data[0...3].elementsEqual(FileSystemStorage.STORE_MAGIC) else {
            throw DIDError.didStoreError("Directory \(_rootPath) is not DIDStore file")
        }
        guard data[4...7].elementsEqual(FileSystemStorage.STORE_VERSION) else {
            throw DIDError.didStoreError("DIDStore \(_rootPath) unsupported version.")
        }
    }

    private func getFileHandle(_ create: Bool, _ path: String...) throws -> FileHandle {
        // TODO:
    }

    /*
    private func getFile(_ path: String, _ needCreate: Bool) throws -> FileHandle {
        var realPath = ""
        // var file: FileHandle?

        realPath.append(self._rootPath)
        // TODO:

        return FileHandle(forReadingAtPath: realPath)!
    }*/

    /*

    private func getHDPrivateKeyFile(_ create: Bool) throws -> String{
        let path = storeRootPath + "/" + FileSystemStorage.PRIVATE_DIR + "/" + FileSystemStorage.HDKEY_FILE
        return try getFile(create, path)
    }

    private func getHDPrivateKeyFile(_ dir: String, _ hdKey: String, _ create: Bool) throws -> Bool {
        let keyPath = storeRootPath + "/" + "\(dir)/\(hdKey)"
        guard try exists(keyPath) else {
            return false
        }
        let readHandler = FileHandle(forReadingAtPath: keyPath)
        let key = readHandler?.readDataToEndOfFile() ?? Data()
        guard key.count != 0 else {
            return false
        }
        return true
    }

    public func containsPrivateIdentity() throws -> Bool {
        let path = try getHDPrivateKeyFile(false)
        return try exists(path)
    }
     */

    func containsPrivateIdentity() -> Bool {
        do {
            let handle = try getFileHandle(false, Constants.PRIVATE_DIR, Constants.HDKEY_FILE)
            defer { handle.closeFile() }
            return handle.readDataToEndOfFile().count > 0
        } catch {
            return false
        }
    }

    func storePrivateIdentity(_ key: String) throws {
        do {
            let handle = try getFileHandle(true, Constants.PRIVATE_DIR, Constants.HDKEY_FILE)
            defer { handle.closeFile() }
            handle.write(key.data(using: .utf8)!)
        } catch {
            throw DIDError.didStoreError("Store private key identity error")
        }
    }

    func loadPrivateIdentity() throws -> String {
        do {
            let handle = try getFileHandle(false, Constants.PRIVATE_DIR, Constants.HDKEY_FILE)
            handle.closeFile()
            // TODO
        } catch {
            throw DIDError.didStoreError("Load private key identity error")
        }

        return "TODO"
    }

    func storePrivateIdentityIndex(_ index: Int) throws {
        do {
            let handle = try getFileHandle(true, Constants.PRIVATE_DIR, Constants.INDEX_FILE)
            handle.closeFile()
            // TODO
        } catch {
            throw DIDError.didStoreError("Store private identity index error")
        }
    }

    func loadPrivateIdentityIndex() throws -> Int {
        do {
            let handle = try getFileHandle(false, Constants.PRIVATE_DIR, Constants.INDEX_FILE)
            handle.closeFile()
            // TODO
        } catch {
            throw DIDError.didStoreError("Load private identity index error")
        }
    }

    func storeMnemonic(_ mnemonic: String) throws {
        do {
            let handle = try getFileHandle(true, Constants.PRIVATE_DIR, Constants.MNEMONIC_FILE)
            defer { handle.closeFile() }
            handle.write(mnemonic.data(using: .utf8)!)
        } catch {
            throw DIDError.didStoreError("Store mnemonic error")
        }
    }

    func loadMnemonic() throws -> String {
        do {
            let handle = try getFileHandle(false, Constants.PRIVATE_DIR, Constants.MNEMONIC_FILE)
            handle.closeFile()
            // TODO
        } catch {
            throw DIDError.didStoreError("Load mnemonic error")
        }

        // TODO:
        return ""
    }

    func storeDidMeta(_ did: DID, _ alias: DIDMeta?) throws {
        do {
            let handle = try getFileHandle(true, Constants.DID_DIR,
                                           did.methodSpecificId, Constants.META_FILE)
            handle.closeFile()
            // TODO
        } catch {
            throw DIDError.didStoreError("Store alias name error")
        }
    }

    func loadDidMeta(_ did: DID) throws -> DIDMeta {
        do {
            let handle = try getFileHandle(false, Constants.DID_DIR,
                                           did.methodSpecificId, Constants.META_FILE)
            handle.closeFile()
            // TODO
        } catch {
            throw DIDError.didStoreError("Load alias name error")
        }
    }

    func storeDid(_ doc: DIDDocument) throws {
        do {
            let handle = try getFileHandle(true, Constants.DID_DIR,
                                           doc.subject.methodSpecificId, Constants.DOCUMENT_FILE)
            handle.closeFile()
            // TODO
        } catch {
            throw DIDError.didStoreError("Store DIDDocument error")
        }
    }

    func loadDid(_ did: DID) throws -> DIDDocument {
        do {
            let handle = try getFileHandle(false, Constants.DID_DIR,
                                           did.methodSpecificId, Constants.DOCUMENT_FILE)
            handle.closeFile()
            // TODO
        } catch {
            throw DIDError.didStoreError("Load DIDDocument error")
        }
    }

    func containsDid(_ did: DID) -> Bool {
        do {
            let handle = try getFileHandle(true, Constants.DID_DIR,
                                           did.methodSpecificId, Constants.DOCUMENT_FILE)
            handle.closeFile()
            // TODO
            return true
        } catch {
            return false
        }
    }

    func deleteDid(_ did: DID) throws {
        // TODO:
    }

    func listDids(_ filter: Int) throws -> Array<DID> {
        // TODO:
        return Array<DID>()
    }

    func storeCredentialMeta(_ did: DID, _ id: DIDURL, _ meta: CredentialMeta?) throws {
        do {
            let handle = try getFileHandle(true, Constants.DID_DIR,
                                          did.methodSpecificId, Constants.CREDENTIALS_DIR,
                                          id.fragment!, Constants.META_FILE)
            defer { handle.closeFile()}
            let metadata = (meta != nil && !meta!.isEmpty()) ? meta!.toString() : nil
            if  metadata?.isEmpty ?? false {
                // TODO: handle.delete()
            } else {
                // TODO
            }

        } catch {
            throw DIDError.didStoreError("Write credential meta error")
        }
    }

    func loadCredentialMeta(_ did: DID, _ id: DIDURL) throws -> CredentialMeta {
        // TODO
    }

    func storeCredential(_ credential: VerifiableCredential) throws {
        // TODO:
    }

    func loadCredential(_ did: DID, _ id: DIDURL) throws -> VerifiableCredential? {
        // TODO:
        return nil
    }

    func containsCredentials(_ did: DID) throws -> Bool {
        // TODO:
        return false
    }

    func containsCredential(_ did: DID, _ id: DIDURL) throws -> Bool {
        // TODO:
        return false
    }

    func deleteCredential(_ did: DID, _ id: DIDURL) throws -> Bool {
        // TODO:
        return false
    }

    func listCredentials(_ did: DID) throws -> Array<DIDURL> {
        // TODO:
        return Array<DIDURL>()
    }

    func selectCredentials(_ did: DID, _ id: DIDURL, _ type: Array<Any>) throws -> Array<DIDURL> {
        // TODO:
        return Array<DIDURL>()
    }

    func storePrivateKey(_ did: DID, _ id: DIDURL, _ privateKey: String) throws {
        // TODO:
    }

    func loadPrivateKey(_ did: DID, _ id: DIDURL) throws -> String {
        // TODO:
        return ""
    }

    func containsPrivateKeys(_ did: DID) throws -> Bool {
        // TODO:
        return false
    }

    func containsPrivateKey(_ did: DID, _ id: DIDURL) throws -> Bool {
        // TODO:
        return false
    }

    func deletePrivateKey(_ did: DID, _ id: DIDURL) throws -> Bool {
        // TODO:
        return false
    }

    func changePassword(_ callback: (String) throws -> String) throws {
        // TODO:
    }
}
