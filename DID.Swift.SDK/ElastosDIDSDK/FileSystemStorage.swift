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
    private static let STORE_VERSION = 2
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

            var data = Data(capacity: 8)
            data.append(contentsOf: FileSystemStorage.STORE_MAGIC)

            let version = [UInt8]()
            // TODO: do with version
            data.append(contentsOf: version)

            let file = try openFileHandle(true, Constants.META_FILE)
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
            file = try openFileHandle(false, Constants.META_FILE)
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

        // TODO: STORE_VERSION

        // postChangePassword()
    }

    private func openFileHandle(_ forWrite: Bool, _ pathArgs: String...) throws -> FileHandle {
        var path: String = ""
        for item in pathArgs {
            path.append(item)
        }

        if !FileManager.default.fileExists(atPath: path) && forWrite {
            FileManager.default.createFile(atPath: path, contents: nil, attributes: nil)
        }

        let handle: FileHandle?
        if forWrite {
            handle = FileHandle(forWritingAtPath: path)
        } else {
            handle = FileHandle(forReadingAtPath: path)
        }

        guard let _ = handle else {
            throw DIDError.unknownFailure("opening file at \(path) error")
        }

        return handle!
    }

    private func openPrivateIdentityFile(_ forWrite: Bool) throws -> FileHandle {
        return try openFileHandle(forWrite, Constants.PRIVATE_DIR, Constants.HDKEY_FILE)
    }

    private func openPrivateIdentityFile() throws -> FileHandle {
        return try openPrivateIdentityFile(false)
    }

    func containsPrivateIdentity() -> Bool {
        do {
            let handle = try openPrivateIdentityFile()
            defer {
                handle.closeFile()
            }

            return handle.readDataToEndOfFile().count > 0
        } catch {
            return false
        }
    }

    func storePrivateIdentity(_ key: String) throws {
        do {
            let handle = try openPrivateIdentityFile(true)
            defer {
                handle.closeFile()
            }

            handle.write(key.data(using: .utf8)!)
        } catch {
            throw DIDError.didStoreError("store private key identity error")
        }
    }

    func loadPrivateIdentity() throws -> String {
        do {
            let handle = try openPrivateIdentityFile()
            defer {
                handle.closeFile()
            }

            let data = handle.readDataToEndOfFile()
            return String(data: data, encoding: .utf8)!
        } catch {
            throw DIDError.didStoreError("load private key identity error")
        }
    }

    private func openPrivateIdentityIndexFile(_ forWrite: Bool) throws -> FileHandle {
        return try openFileHandle(forWrite, Constants.PRIVATE_DIR, Constants.INDEX_FILE)
    }

    private func openPrivateIdentityIndexFile() throws -> FileHandle {
        return try openPrivateIdentityIndexFile(false)
    }

    func storePrivateIdentityIndex(_ index: Int) throws {
        do {
            let handle = try openPrivateIdentityIndexFile(true)
            defer {
                handle.closeFile()
            }
            let data = withUnsafeBytes(of: index) { ptr in
                Data(ptr)
            }
            handle.write(data)
        } catch {
            throw DIDError.didStoreError("store private identity index error")
        }
    }

    func loadPrivateIdentityIndex() throws -> Int {
        do {
            let handle = try openPrivateIdentityIndexFile()
            defer {
                handle.closeFile()
            }

            let data = handle.readDataToEndOfFile()
            return data.withUnsafeBytes { (pointer: UnsafePointer<Int32>) -> Int in
                return Int(pointer.pointee)
            }
        } catch {
            throw DIDError.didStoreError("load private identity index error")
        }
    }

    private func openMnemonicFile(_ forWrite: Bool) throws -> FileHandle {
        return try openFileHandle(forWrite, Constants.PRIVATE_DIR, Constants.MNEMONIC_FILE)
    }

    private func openMnemonicFile() throws -> FileHandle {
        return try openMnemonicFile(false)
    }

    func storeMnemonic(_ mnemonic: String) throws {
        do {
            let handle = try openMnemonicFile(true)
            defer {
                handle.closeFile()
            }

            handle.write(mnemonic.data(using: .utf8)!)
        } catch {
            throw DIDError.didStoreError("store mnemonic error")
        }
    }

    func loadMnemonic() throws -> String {
        do {
            let handle = try openMnemonicFile()
            defer {
                handle.closeFile()
            }

            let data = handle.readDataToEndOfFile()
            return String(data: data, encoding: .utf8)!
        } catch {
            throw DIDError.didStoreError("load mnemonic error")
        }
    }

    private func openDidMetaFile(_ did: DID, _ forWrite: Bool) throws -> FileHandle {
        return try openFileHandle(forWrite, Constants.DID_DIR, did.methodSpecificId, Constants.META_FILE)
    }

    private func openDidMetaFile(_ did: DID) throws -> FileHandle {
        return try openDidMetaFile(did, false)
    }

    func storeDidMeta(_ did: DID, _ meta: DIDMeta) throws {
        do {
            let handle = try openDidMetaFile(did, true)
            let metadata = meta.toString()
            defer {
                handle.closeFile()
            }

            if metadata.isEmpty {
                var path: String = ""
                path.append(Constants.DID_DIR)
                path.append(did.methodSpecificId)
                path.append(Constants.META_FILE)

                try FileManager.default.removeItem(atPath: path)
            } else {
                handle.write(metadata.data(using: .utf8)!)
            }
        } catch {
            throw DIDError.didStoreError("store DID metadata error")
        }
    }

    func loadDidMeta(_ did: DID) throws -> DIDMeta {
        do {
            let handle = try openDidMetaFile(did)
            defer {
                handle.closeFile()
            }

            let data = handle.readDataToEndOfFile()
            return try DIDMeta.fromJson(String(data: data, encoding: .utf8)!)
        } catch {
            throw DIDError.didStoreError("load DID metadata error")
        }
    }

    private func openDocumentFile(_ did: DID, _ forWrite: Bool) throws -> FileHandle {
        return try openFileHandle(forWrite, Constants.DID_DIR, did.methodSpecificId, Constants.DOCUMENT_FILE)
    }

    private func openDocumentFile(_ did: DID) throws -> FileHandle {
        return try openDocumentFile(did, false)
    }

    func storeDid(_ doc: DIDDocument) throws {
        do {
            let handle = try openDocumentFile(doc.subject, true)
            defer {
                handle.closeFile()
            }

            let data: Data = try doc.toJson(true, false)
            handle.write(data)
        } catch {
            throw DIDError.didStoreError("store DIDDocument error")
        }
    }

    func loadDid(_ did: DID) throws -> DIDDocument {
        do {
            let handle = try openDocumentFile(did)
            defer {
                handle.closeFile()
            }

            let data = handle.readDataToEndOfFile()
            return try DIDDocument.convertToDIDDocument(fromData: data)
        } catch {
            throw DIDError.didStoreError("load DIDDocument error")
        }
    }

    func containsDid(_ did: DID) -> Bool {
        do {
            let handle = try openDocumentFile(did)
            defer {
                handle.closeFile()
            }
            return true
        } catch {
            return false
        }
    }

    func deleteDid(_ did: DID) -> Bool {
        do {
            let path = Constants.DID_DIR + did.methodSpecificId
            try FileManager.default.removeItem(atPath: path)
            return true
        } catch {
            return false
        }
    }

    func listDids(_ filter: Int) throws -> Array<DID> {
        // TODO:
        return Array<DID>()
    }

    private func openCredentialMetaFile(_ did: DID, _ id: DIDURL, _ forWrite: Bool) throws -> FileHandle {
        return try openFileHandle(forWrite, Constants.DID_DIR, did.methodSpecificId,
                       Constants.CREDENTIALS_DIR, id.fragment!, Constants.META_FILE)
    }

    private func openCredentialMetaFile(_ did: DID, _ id: DIDURL) throws -> FileHandle {
        return try openCredentialMetaFile(did, id, false)
    }

    func storeCredentialMeta(_ did: DID, _ id: DIDURL, _ meta: CredentialMeta) throws {
        do {
            let handle = try openCredentialMetaFile(did, id, true)
            let metadata = meta.toJson().data(using: .utf8)!
            defer {
                handle.closeFile()
            }

            if metadata.isEmpty {
                var path: String = ""
                path.append(Constants.DID_DIR)
                path.append(did.methodSpecificId)
                path.append(Constants.CREDENTIALS_DIR)
                path.append(id.fragment!)
                path.append(Constants.META_FILE)

                try FileManager.default.removeItem(atPath: path)
            } else {
                handle.write(metadata)
            }
        } catch {
            throw DIDError.didStoreError("store credential meta error")
        }
    }

    func loadCredentialMeta(_ did: DID, _ id: DIDURL) throws -> CredentialMeta {
        do {
            let handle = try openCredentialMetaFile(did, id)
            defer {
                handle.closeFile()
            }

            let data = handle.readDataToEndOfFile()
            return try CredentialMeta.fromJson(String(data: data, encoding: .utf8)!)
        } catch {
            throw DIDError.didStoreError("load credential meta error")
        }
    }

    private func openCredentialFile(_ did: DID, _ id: DIDURL, _ forWrite: Bool) throws -> FileHandle {
        return try openFileHandle(forWrite, Constants.DID_DIR, did.methodSpecificId,
                       Constants.CREDENTIALS_DIR, id.fragment!, Constants.CREDENTIAL_FILE)
    }

    private func openCredentialFile(_ did: DID, _ id: DIDURL) throws -> FileHandle {
        return try openCredentialFile(did, id, false)
    }

    func storeCredential(_ credential: VerifiableCredential) throws {
        do {
            let handle = try openCredentialFile(credential.subject.did, credential.getId(), true)
            defer {
                handle.closeFile()
            }

            let data = credential.toJson(true, false)
            handle.write(data.data(using: .utf8)!)
        } catch {
            throw DIDError.didStoreError("store credential error")
        }
    }

    func loadCredential(_ did: DID, _ id: DIDURL) throws -> VerifiableCredential {
        do {
            let handle = try openCredentialFile(did, id)
            defer {
                handle.closeFile()
            }

            let data = handle.readDataToEndOfFile()
            return try VerifiableCredential.fromJson(data)
        } catch {
            throw DIDError.didStoreError("load credential error")
        }
    }

    func containsCredentials(_ did: DID) -> Bool {
        // TODO:
        return false
    }

    func containsCredential(_ did: DID, _ id: DIDURL) -> Bool {
        do {
            let handle = try openCredentialFile(did, id)
            defer {
                handle.closeFile()
            }
            return true
        } catch {
            return false
        }
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

    private func openPrivateKeyFile(_ did: DID, _ id: DIDURL, _ forWrite: Bool) throws -> FileHandle {
        return try openFileHandle(forWrite, Constants.DID_DIR, did.methodSpecificId,
                                  Constants.PRIVATEKEYS_DIR, id.fragment!)
    }

    private func openPrivateKeyFile(_ did: DID, _ id: DIDURL) throws -> FileHandle {
        return try openPrivateKeyFile(did, id, false)
    }

    func storePrivateKey(_ did: DID, _ id: DIDURL, _ privateKey: String) throws {
        do {
            let handle = try openPrivateKeyFile(did, id, true)
            defer {
                handle.closeFile()
            }

            handle.write(privateKey.data(using: .utf8)!)
        } catch {
            throw DIDError.didStoreError("store private key error.")
        }
    }

    func loadPrivateKey(_ did: DID, _ id: DIDURL) throws -> String {
        do {
            let handle = try openPrivateKeyFile(did, id)
            defer {
                handle.closeFile()
            }

            let data = handle.readDataToEndOfFile()
            return String(data: data, encoding: .utf8)!
        } catch {
            throw DIDError.didStoreError("load private key error.")
        }
    }

    func containsPrivateKeys(_ did: DID) -> Bool {
        // TODO:
        return false
    }

    func containsPrivateKey(_ did: DID, _ id: DIDURL) -> Bool {
        // TODO:
        return false
    }

    func deletePrivateKey(_ did: DID, _ id: DIDURL) -> Bool {
        // TODO:
        return false
    }

    func changePassword(_ callback: (String) throws -> String) throws {
        // TODO:
    }
}
