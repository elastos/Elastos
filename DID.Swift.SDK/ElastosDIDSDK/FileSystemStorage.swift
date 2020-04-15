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
    typealias ReEncryptor = (String) throws -> String

    private static let STORE_MAGIC:   [UInt8] = [0x00, 0x0D, 0x01, 0x0D]
    private static let STORE_VERSION = 2
    private static let STORE_META_SIZE = 8
    private static let PRIVATE_DIR = "private"
    private static let HDKEY_FILE = "key"
    private static let HDPUBKEY_FILE = "key.pub"
    private static let INDEX_FILE = "index"
    private static let MNEMONIC_FILE = "mnemonic"

    private static let DID_DIR = "ids"
    private static let DOCUMENT_FILE = "document"
    private static let CREDENTIALS_DIR = "credentials"
    private static let CREDENTIAL_FILE = "credential"
    private static let PRIVATEKEYS_DIR = "privatekeys"

    private static let META_FILE = ".meta"

    private static let JOURNAL_SUFFIX = ".journal"
    private static let DEPRECATED_SUFFIX = ".deprecated"

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
            
            // TODO: do with version
            data.append(contentsOf: intToByteArray(i: FileSystemStorage.STORE_VERSION))

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
        guard data.count == FileSystemStorage.STORE_META_SIZE else {
            throw DIDError.didStoreError("Directory \(_rootPath) is not DIDStore directory")
        }

        // Check MAGIC & VERSION
        guard data[0...3].elementsEqual(FileSystemStorage.STORE_MAGIC) else {
            throw DIDError.didStoreError("Directory \(_rootPath) is not DIDStore file")
        }


        let array : [UInt8] = [UInt8](data[4...7])
        var value : UInt32 = 0
        let storeVersion = NSData(bytes: array, length: 4)
        storeVersion.getBytes(&value, length: 4)
        value = UInt32(bigEndian: value)

        guard value == FileSystemStorage.STORE_VERSION else {
            throw DIDError.didStoreError("Directory \(_rootPath) unsupported version.")
        }

        try postChangePassword()
    }
    
    private func postChangePassword() throws {
        let privateDir: String = _rootPath + "/" + FileSystemStorage.PRIVATE_DIR
        let privateDeprecated = _rootPath + "/" + FileSystemStorage.PRIVATE_DIR + FileSystemStorage.DEPRECATED_SUFFIX
        let privateJournal = _rootPath + "/" + FileSystemStorage.PRIVATE_DIR + FileSystemStorage.JOURNAL_SUFFIX
        
        let didDir = _rootPath + "/" + FileSystemStorage.DID_DIR
        let didDeprecated = _rootPath + "/" + FileSystemStorage.DID_DIR + FileSystemStorage.DEPRECATED_SUFFIX
        let didJournal = _rootPath + "/" + FileSystemStorage.DID_DIR + FileSystemStorage.JOURNAL_SUFFIX
        let stageFile = _rootPath +  "/postChangePassword"

        let fileManager = FileManager.default
        if fileManager.fileExists(atPath: stageFile) {
            if try dirExists(privateJournal) {
                if try dirExists(privateDir) {
                    try fileManager.moveItem(atPath: privateDir, toPath: privateDeprecated)
                }
                try fileManager.moveItem(atPath: privateJournal, toPath: privateDir)
            }
            if try dirExists(didJournal) {
                if try dirExists(didDir) {
                    try fileManager.moveItem(atPath: didDir, toPath: didDeprecated)
                }
                try fileManager.moveItem(atPath: didJournal, toPath: didDir)
            }
            
            _ = try deleteFile(privateDeprecated)
            _ = try deleteFile(didDeprecated)
            _ = try deleteFile(stageFile)
        }
        else {
            if try dirExists(privateJournal) {
                _ = try deleteFile(privateJournal)
            }
            if try dirExists(didJournal) {
                _ = try deleteFile(didJournal)
            }
        }
    }
    
    private func dirExists(_ dirPath: String) throws -> Bool {
        let fileManager = FileManager.default
        var isDir : ObjCBool = false
        let re = fileManager.fileExists(atPath: dirPath, isDirectory:&isDir)
        return re && isDir.boolValue
    }
    
    private func fileExists(_ dirPath: String) throws -> Bool {
        let fileManager = FileManager.default
        var isDir : ObjCBool = false
        fileManager.fileExists(atPath: dirPath, isDirectory:&isDir)
        let readhandle = FileHandle.init(forReadingAtPath: dirPath)
        let data = (readhandle?.readDataToEndOfFile()) ?? Data()
        let str = String(data: data, encoding: .utf8) ?? ""
        return str.count > 0 ? true : false
    }
    
    private func deleteFile(_ path: String) throws -> Bool {
        let fileManager = FileManager.default
        var isDir = ObjCBool.init(false)
        let fileExists = fileManager.fileExists(atPath: path, isDirectory: &isDir)
        // If path is a folder, traverse the subfiles under the folder and delete them
        let re: Bool = false
        guard fileExists else {
            return re
        }
        try fileManager.removeItem(atPath: path)
        return true
    }

    private func openFileHandle(_ forWrite: Bool, _ pathArgs: String...) throws -> FileHandle {
        var path: String = _rootPath
        for item in pathArgs {
            path.append("/")
            path.append(item)
        }

        if !FileManager.default.fileExists(atPath: path) && forWrite {
            let dirPath: String = PathExtracter(path).dirname()
            let fileM = FileManager.default
            let re = fileM.fileExists(atPath: dirPath)
            if !re {
                try fileM.createDirectory(atPath: dirPath, withIntermediateDirectories: true, attributes: nil)
            }
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

    private func openPublicIdentityFile() throws -> FileHandle {
        return try openPublicIdentityFile(false)
    }

    private func openMnemonicFile() throws -> FileHandle {
        return try openMnemonicFile(false)
    }

    private func openMnemonicFile(_ forWrite: Bool) throws -> FileHandle {
        return try openFileHandle(forWrite, Constants.PRIVATE_DIR, Constants.MNEMONIC_FILE)
    }

    private func openPublicIdentityFile(_ forWrite: Bool) throws -> FileHandle {
        return try openFileHandle(forWrite, Constants.PRIVATE_DIR, Constants.HDPUBKEY_FILE)
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

    func containsPublicIdentity() -> Bool {
        do {
            let handle = try openPublicIdentityFile()
            defer {
                handle.closeFile()
            }

            return handle.readDataToEndOfFile().count > 0
        } catch {
            return false
        }
    }

    func storePublicIdentity(_ key: String) throws {
        do {
            let handle = try openPublicIdentityFile(true)
            defer {
                handle.closeFile()
            }
            handle.write(key.data(using: .utf8)!)
        } catch {
            throw DIDError.didStoreError("store public key identity error")
        }
    }

    func loadPublicIdentity() throws -> String {
        do {
            let handle = try openPublicIdentityFile()
            defer {
                handle.closeFile()
            }
            let data = handle.readDataToEndOfFile()
            return String(data: data, encoding: .utf8)!
        } catch {
            throw DIDError.didStoreError("load public key identity error")
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

    func containMnemonic() -> Bool {
        do {
            let handle = try openMnemonicFile()
            defer {
                handle.closeFile()
            }

            return handle.readDataToEndOfFile().count > 0
        } catch {
            return false
        }
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

            if metadata.isEmpty || metadata == "{}" {
                var path: String = _rootPath
                path.append("/")
                path.append(Constants.DID_DIR)
                path.append("/")
                path.append(did.methodSpecificId)
                path.append("/")
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
        let handle = try openDidMetaFile(did)
        defer {
            handle.closeFile()
        }

        let data = handle.readDataToEndOfFile()
        return try DIDMeta.fromJson(String(data: data, encoding: .utf8)!)
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
            let path = _rootPath + "/" + Constants.DID_DIR + "/" + did.methodSpecificId
            try FileManager.default.removeItem(atPath: path)
            return true
        } catch {
            return false
        }
    }

    func listDids(_ filter: Int) throws -> Array<DID> {
        var dids: Array<DID> = []
        let path = _rootPath + "/" + FileSystemStorage.DID_DIR
        let re = try dirExists(path)
        guard re else {
            return []
        }

        let fileManager = FileManager.default
        let enumerator = try fileManager.contentsOfDirectory(atPath: path)
        for element: String in enumerator {
            var hasPrivateKey: Bool = false
            let did = DID(DID.METHOD, element)
            hasPrivateKey = containsPrivateKeys(did)
            
            if filter == DIDStore.DID_HAS_PRIVATEKEY {
                
            }
            else if filter == DIDStore.DID_NO_PRIVATEKEY {
                hasPrivateKey = !hasPrivateKey
            }
            else if filter == DIDStore.DID_ALL {
                hasPrivateKey = true
            }
         
            if hasPrivateKey {
                let did: DID = DID(DID.METHOD, element)
                dids.append(did)
            }
        }
        return dids
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
        do {
            let targetPath = _rootPath + "/" + FileSystemStorage.DID_DIR + "/" + did.methodSpecificId + "/" + FileSystemStorage.CREDENTIALS_DIR
            let exit = try dirExists(targetPath)
            guard exit else {
                return false
            }
            let arr = try listCredentials(did)
            guard arr.count > 0 else {
                return false
            }
            return true
        } catch  {
            return false
        }
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

    func deleteCredential(_ did: DID, _ id: DIDURL) -> Bool {
        do {
            let targetPath = _rootPath + "/" + FileSystemStorage.DID_DIR + "/" + did.methodSpecificId + "/" + FileSystemStorage.CREDENTIALS_DIR + "/" + id.fragment!
            let path = try getFile(targetPath)
            if try dirExists(path!) {
                _ = try deleteFile(path!)
                let dir = _rootPath + "/" + FileSystemStorage.DID_DIR + "/" + did.methodSpecificId + "/" + FileSystemStorage.CREDENTIALS_DIR
                
                let fileManager = FileManager.default
                let enumerator = try fileManager.contentsOfDirectory(atPath: dir)
                if enumerator.count == 0 {
                    try fileManager.removeItem(atPath: dir)
                }
                return true
            }
            return false
        } catch {
            return false
        }
    }
    
    private func getFile(_ create: Bool, _ path: String) throws -> String {
        let relPath = path
        let fileManager = FileManager.default
        if create {
            var isDirectory = ObjCBool.init(false)
            let fileExists = FileManager.default.fileExists(atPath: relPath, isDirectory: &isDirectory)
            if !isDirectory.boolValue && fileExists {
                _ = try deleteFile(relPath)
            }
        }
        if create {
            let dirPath: String = PathExtracter(relPath).dirname()
            if try !dirExists(dirPath) {
                try fileManager.createDirectory(atPath: dirPath, withIntermediateDirectories: true, attributes: nil)
            }
            fileManager.createFile(atPath: relPath, contents: nil, attributes: nil)
        }
        return relPath
    }
    
    private func getFile(_ path: String) throws -> String? {
        return try getFile(false, path)
    }

    func listCredentials(_ did: DID) throws -> Array<DIDURL> {
        let dir = _rootPath + "/" + FileSystemStorage.DID_DIR + "/" + did.methodSpecificId + "/" + FileSystemStorage.CREDENTIALS_DIR
        guard try dirExists(dir) else {
            return []
        }
        
        let fileManager = FileManager.default
        let enumerator = try fileManager.contentsOfDirectory(atPath: dir)
        var didurls: Array<DIDURL> = []
        for element: String in enumerator  {
            // if !element.hasSuffix(".meta")
            let didUrl: DIDURL = try DIDURL(did, element)
            didurls.append(didUrl)
        }
        return didurls
    }

    func selectCredentials(_ did: DID, _ id: DIDURL?, _ type: Array<Any>?) throws -> Array<DIDURL> {
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
        let dir = _rootPath + "/" + FileSystemStorage.DID_DIR + "/" + did.methodSpecificId + "/" + FileSystemStorage.PRIVATEKEYS_DIR
        var path = ""
        do {
            path = try getFile(dir)!
        } catch {
            return false
        }
        let fileManager: FileManager = FileManager.default
        var isDir = ObjCBool.init(false)
        _ = fileManager.fileExists(atPath: path, isDirectory: &isDir)
        guard isDir.boolValue else {
            return false
        }
        
        var keys: [String] = []
        if let dirContents = fileManager.enumerator(atPath: path) {
            // determine whether files are hidden or not
            while let url = dirContents.nextObject() as? String  {
                // Not hiding files
                if url.first!.description != "." {
                    keys.append(url)
                }
            }
        }
        return keys.count > 0
    }
    
    func containsPrivateKey(_ did: DID, _ id: DIDURL) -> Bool {
        let dir = _rootPath + "/" + FileSystemStorage.DID_DIR + "/" + did.methodSpecificId + "/" + FileSystemStorage.PRIVATEKEYS_DIR + "/" + id.fragment!
        do {
            let path = try getFile(dir)!
            return try fileExists(path)
        } catch {
            return false
        }
    }

    func deletePrivateKey(_ did: DID, _ id: DIDURL) -> Bool {
        do {
            let path = _rootPath + "/" + FileSystemStorage.DID_DIR + "/" + did.methodSpecificId + "/" + FileSystemStorage.PRIVATEKEYS_DIR + "/" + id.fragment!
            if try fileExists(path) {
                _ = try deleteFile(path)
                
                // Remove the privatekeys directory is no privatekey exists.
                let dir = _rootPath + "/" + FileSystemStorage.DID_DIR + "/" + did.methodSpecificId
                let fileManager = FileManager.default
                let enumerator = try fileManager.contentsOfDirectory(atPath: dir)
                if enumerator.count == 0 {
                    try fileManager.removeItem(atPath: dir)
                }
            }
            return false
        } catch {
            return false
        }
    }

    func changePassword(_ callback: (String) throws -> String) throws {
        let privateDir = _rootPath + "/" + FileSystemStorage.PRIVATE_DIR
        let privateJournal = _rootPath + "/" + FileSystemStorage.PRIVATE_DIR + FileSystemStorage.JOURNAL_SUFFIX

        let didDir = _rootPath + "/" + FileSystemStorage.DID_DIR
        let didJournal = _rootPath + "/" + FileSystemStorage.DID_DIR + FileSystemStorage.JOURNAL_SUFFIX
        do {
        try copy(privateDir, privateJournal, callback)
        try copy(didDir, didJournal, callback)
        }
        catch {
            throw DIDError.didStoreError("Change store password failed.")
        }
        _ = try getFile(true, "\(_rootPath)/postChangePassword")
        try postChangePassword()
    }
    
    private func copy(_ src: String, _ dest: String, _ callback: ReEncryptor) throws {
        if isDirectory(src) {
            try createDir(true, dest) // dest create if not
            
            let fileManager = FileManager.default
            let enumerator = try fileManager.contentsOfDirectory(atPath: src)
            for element: String in enumerator  {
                // if !element.hasSuffix(".meta")
                let srcFile = src + "/" + element
                let destFile = dest + "/" + element
                try copy(srcFile, destFile, callback)
            }
        }
        else {
            if try needReencrypt(src) {
                let org = try readTextFromPath(src)
                try writeTextToPath(dest, callback(org))
            }
            else {
                let fileManager = FileManager.default
                try fileManager.copyItem(atPath: src, toPath: dest)
            }
        }
    }
    
    private func isDirectory(_ path: String) -> Bool {
        let fileManager = FileManager.default
        var isDir : ObjCBool = false
        _ = fileManager.fileExists(atPath: path, isDirectory:&isDir)
        return isDir.boolValue
    }
    
    private func createDir(_ create: Bool, _ path: String) throws {
        let fileManager = FileManager.default
        if create {
            var isDirectory = ObjCBool.init(false)
            let fileExists = FileManager.default.fileExists(atPath: path, isDirectory: &isDirectory)
            if !fileExists {
                try fileManager.createDirectory(atPath: path, withIntermediateDirectories: true, attributes: nil)
            }
        }
    }
    
    private func needReencrypt(_ path: String) throws -> Bool {
        let patterns: Array<String> = [
            "(.+)\\" + "/" + FileSystemStorage.PRIVATE_DIR + "\\" + "/" + FileSystemStorage.HDKEY_FILE + "$",
            "(.+)\\" + "/" + FileSystemStorage.PRIVATE_DIR + "\\" + "/" + FileSystemStorage.MNEMONIC_FILE + "$",
            "(.+)\\" + "/" + FileSystemStorage.DID_DIR + "\\" + "/" + "(.+)" + "\\" + "/" + FileSystemStorage.PRIVATEKEYS_DIR + "\\" + "/" + "(.+)"]
        for pattern in patterns {
            let matcher: RegexHelper = try RegexHelper(pattern)
            
            if matcher.match(input: path)  { // if (path.matches(pattern))
                return true
            }
        }
        return false
    }
    
    private func readTextFromPath(_ path: String) throws -> String {
        guard try fileExists(path) else {
            return ""
        }
        return try String(contentsOfFile:path, encoding: String.Encoding.utf8)
    }
    
    private func writeTextToPath(_ path: String, _ text: String) throws {
        let writePath = try getFile(path)
        let fileManager = FileManager.default
        // Delete before writing
        _ = try deleteFile(writePath!)
        fileManager.createFile(atPath: path, contents:nil, attributes:nil)
        let handle = FileHandle(forWritingAtPath:path)
        handle?.write(text.data(using: String.Encoding.utf8)!)
    }
    
    func intToByteArray(i : Int) -> [UInt8] {
        var result: [UInt8] = []
        result.append(UInt8((i >> 24) & 0xFF))
        result.append(UInt8((i >> 16) & 0xFF))
        result.append(UInt8((i >> 8) & 0xFF))
        result.append(UInt8(i & 0xFF))
        return result
    }
}
