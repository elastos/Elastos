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

    private static let STORE_MAGIC: [UInt8] =  [0x00, 0x0D, 0x01, 0x0D]
    private static let STORE_VERSION: [UInt8] = [0x00, 0x00, 0x00, 0x01]
    private static let STORE_META_SIZE = 8

    private static let PRIVATE_DIR = "private"
    private static let HDKEY_FILE = "key"
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
    
    private var storeRootPath: String
    
    public init(_ dir: String) throws {
        if dir.isEmpty {
            throw DIDError.didStoreError(_desc: "Invalid DIDStore root directory.") 
        }
        self.storeRootPath = dir

        guard try exists(dir) else {
            try creatStore(dir)
            return
        }

        try checkStore()
    }
    
    // create store file
    private func creatStore(_ dir: String) throws {
        try FileManager.default.createDirectory(atPath: dir, withIntermediateDirectories: true, attributes: nil)

        let path = "\(dir)/\(FileSystemStorage.META_FILE)"
        let tagFilePathURL = URL(fileURLWithPath: path)
        
        var data: Data = Data(capacity: 8)
        data.append(contentsOf: FileSystemStorage.STORE_MAGIC)
        data.append(contentsOf: FileSystemStorage.STORE_VERSION)

        try data.write(to: tagFilePathURL)
    }

    private func checkStore() throws {
        // Check if the root directory is a file
        let fileManager = FileManager.default
        var isDir: ObjCBool = false
        if fileManager.fileExists(atPath: storeRootPath, isDirectory:&isDir) {
            guard isDir.boolValue else {
                throw DIDError.didStoreError(_desc: "Store root \(storeRootPath ) is a file.")
            }
        }
        
        // check.DIDStore file
        let tagFilePath: String = storeRootPath + "/" + FileSystemStorage.META_FILE
        var tagFilePathIsDir: ObjCBool = false
        let tagFilePathExists: Bool = fileManager.fileExists(atPath: tagFilePath, isDirectory:&tagFilePathIsDir)
        guard !tagFilePathIsDir.boolValue || tagFilePathExists else {
            throw DIDError.didStoreError(_desc: "Directory \(tagFilePath) is not a DIDStore.")
        }
        
        let localData = try Data(contentsOf: URL(fileURLWithPath: tagFilePath))
        let uInt8DataArray = [UInt8](localData)
        
        guard uInt8DataArray.count == FileSystemStorage.STORE_META_SIZE else {
            throw DIDError.didStoreError(_desc: "Directory \(tagFilePath) is not a DIDStore.")
        }
        
        let magicArray = localData[0...3]
        let versionArray = localData[4...7]
        
        guard magicArray.elementsEqual(FileSystemStorage.STORE_MAGIC) else {
            throw DIDError.didStoreError(_desc: "Directory \(tagFilePath) is not a DIDStore.")
        }
        
        guard versionArray.elementsEqual(FileSystemStorage.STORE_VERSION) else {
            throw DIDError.didStoreError(_desc: "Directory \(tagFilePath) unsupported version.") 
        }
    }

    public func containsPrivateIdentity() throws -> Bool {
        let path = try getHDPrivateKeyFile(false)
        return try exists(path)
    }
    
    public func storePrivateIdentity(_ key: String) throws {
        let path = try getHDPrivateKeyFile(true)
        try writeTextToPath(path, key)
    }
    
    public func loadPrivateIdentity() throws -> String {
        let path = storeRootPath + "/" + FileSystemStorage.PRIVATE_DIR + "/" + FileSystemStorage.HDKEY_FILE
        return try readTextFromPath(path)
    }
    
    public func storePrivateIdentityIndex(_ index: Int) throws {
        let targetPath = storeRootPath + "/" + FileSystemStorage.PRIVATE_DIR + "/" + FileSystemStorage.INDEX_FILE
        let path = try getFile(true, targetPath)
        try writeTextToPath(path, String(index))
        _ = try readTextFromPath(path)
    }
    
    public func loadPrivateIdentityIndex() throws -> Int {
        let targetPath = storeRootPath + "/" + FileSystemStorage.PRIVATE_DIR + "/" + FileSystemStorage.INDEX_FILE
        let index = try readTextFromPath(targetPath)
        return Int(index)!
    }
    public func storeMnemonic(_ mnemonic: String) throws {
        do {
            let p = storeRootPath + "/" + FileSystemStorage.PRIVATE_DIR + "/" + FileSystemStorage.MNEMONIC_FILE
            let path = try getFile(true, p)
            try writeTextToPath(path, mnemonic)
        } catch {
            throw DIDError.didStoreError(_desc: "Store mnemonic error.")
        }
    }
    
    public func loadMnemonic() throws -> String {
        do {
            let path = storeRootPath + "/" + FileSystemStorage.PRIVATE_DIR + "/" + FileSystemStorage.MNEMONIC_FILE
            return try readTextFromPath(path)
        } catch {
            throw DIDError.didStoreError(_desc: "Load mnemonic error.")
        }
    }

    public func storeDidMeta(_ did: DID, _ meta: DIDMeta?) throws {
        let path = storeRootPath + "/" + FileSystemStorage.DID_DIR + "/" + did.methodSpecificId + "/" + FileSystemStorage.META_FILE
        _ = try getFile(true, path)
        let metadata: String = meta != nil ? meta!.toJson() : ""
        if metadata == "" {
            _ = try deleteFile(path)
        }
        else {
            if metadata == "{}" {
                return 
            }
            try writeTextToPath(path, metadata)
        }
    }
    
    public func loadDidMeta(_ did: DID) throws -> DIDMeta {
        let path = storeRootPath + "/" + FileSystemStorage.DID_DIR + "/" + did.methodSpecificId + "/" + FileSystemStorage.META_FILE
        let meta = try readTextFromPath(path)
        return try DIDMeta.fromString(meta)
    }
    
    public func storeDid(_ doc: DIDDocument) throws {
        let path = storeRootPath + "/" + FileSystemStorage.DID_DIR + "/" + doc.subject!.methodSpecificId + "/" + FileSystemStorage.DOCUMENT_FILE
        _ = try getFile(true, path)
        _ = try exists(path)
        let dicString = doc.toJson(true, forSign: false)
        
        let data: Data = dicString.data(using: .utf8)!
        // & Write to local
        let dirPath: String = PathExtracter(path).dirNamePart()
        let fileM = FileManager.default
        let re = fileM.fileExists(atPath: dirPath)
        if !re {
            try fileM.createDirectory(atPath: dirPath, withIntermediateDirectories: true, attributes: nil)
        }
        let dre = fileM.fileExists(atPath: path)
        if !dre {
            fileM.createFile(atPath: path, contents: nil, attributes: nil)
        }
        let writeHandle = FileHandle(forWritingAtPath: path)
        writeHandle?.write(data)
    }
    
    public func loadDid(_ did: DID) throws -> DIDDocument {
        let path = storeRootPath + "/" + FileSystemStorage.DID_DIR + "/" + did.methodSpecificId + "/" + FileSystemStorage.DOCUMENT_FILE
        let exist = try exists(path)
        guard exist else {
            throw DIDError.didStoreError(_desc: "No did.")
        }
        return try DIDDocument.fromJson(path: path)
    }
    
    public func containsDid(_ did: DID) throws -> Bool {
        let path = FileSystemStorage.DID_DIR + did.methodSpecificId + FileSystemStorage.DOCUMENT_FILE
        return try exists(path)
    }
    
    public func deleteDid(_ did: DID) throws -> Bool {
        let path = storeRootPath + "/" + FileSystemStorage.DID_DIR + "/" + did.methodSpecificId
        let re = try deleteFile(path)
        return re
    }
    
    public func listDids(_ filter: Int) throws -> Array<DID>{
        var arr: Array<DID> = []
        let path = storeRootPath + "/" + FileSystemStorage.DID_DIR
        let re = try exists_dir(path)
        guard re else {
            return []
        }

        let fileManager = FileManager.default
        let enumerator = try fileManager.contentsOfDirectory(atPath: path)
        for element: String in enumerator {
            var hasPrivateKey: Bool = false
            hasPrivateKey = try containsPrivateKeys(element)
            
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
                arr.append(did)
            }
        }
        return arr
    }
    
    private func containsPrivateKeys(_ didstr: String) throws -> Bool {
        let did: DID = DID(DID.METHOD, didstr)
        return try containsPrivateKeys(did)
    }
    
    public func storeCredentialMeta(_ did: DID, _ id: DIDURL, _ meta: CredentialMeta?) throws {
        do {
            let path = storeRootPath + "/" + FileSystemStorage.DID_DIR + "/" + did.methodSpecificId + "/" + FileSystemStorage.CREDENTIALS_DIR + "/" + id.fragment + "/" + FileSystemStorage.META_FILE
            let metadata: String = meta != nil ? meta!.toJson() : ""
            
            if metadata == "" {
                _ = try deleteFile(path)
            }
            else {
                try writeTextToPath(path, metadata)
            }
        }
        catch {
            throw DIDError.didStoreError(_desc: "Write alias error.")
        }
    }

    public func loadCredentialMeta(_ did: DID, _ id: DIDURL) throws -> CredentialMeta {
        let path = storeRootPath + "/" + FileSystemStorage.DID_DIR + "/" + did.methodSpecificId + "/" + FileSystemStorage.CREDENTIALS_DIR + "/" + id.fragment + "/" + FileSystemStorage.META_FILE
        let meta = try readTextFromPath(path)
        return try CredentialMeta.fromString(meta)
    }
    
    public func storeCredential(_ credential: VerifiableCredential) throws {
        let targetPath = storeRootPath + "/" + FileSystemStorage.DID_DIR + "/" + credential.subject.id.methodSpecificId + "/" + FileSystemStorage.CREDENTIALS_DIR + "/" + credential.id.fragment + "/" + FileSystemStorage.CREDENTIAL_FILE

        let path = try getFile(true, targetPath)
        let storeDic = credential.toJson(credential.issuer, true, false)
        let storeJson = JsonHelper.creatJsonString(dic: storeDic)
        let data: Data = storeJson.data(using: .utf8)!
        let handle = FileHandle(forWritingAtPath:path)
        handle?.write(data)
    }
    
    public func loadCredential(_ did: DID, _ id: DIDURL) throws -> VerifiableCredential? {
        let path: String = storeRootPath + "/" + FileSystemStorage.DID_DIR + "/" + did.methodSpecificId + "/" + FileSystemStorage.CREDENTIALS_DIR + "/" + id.fragment + "/" + FileSystemStorage.CREDENTIAL_FILE
        guard try exists(path) else {
             return nil
        }
        return try VerifiableCredential.fromJsonInPath(path)
    }
    
    public func containsCredentials(_ did: DID) throws -> Bool {
        let targetPath = storeRootPath + "/" + FileSystemStorage.DID_DIR + "/" + did.methodSpecificId + "/" + FileSystemStorage.CREDENTIALS_DIR
        let exit = try exists(targetPath)
        guard exit else {
            return false
        }
        let arr = try listCredentials(did)
        guard arr.count > 0 else {
            return false
        }
        return true
    }
    
    public func containsCredential(_ did: DID, _ id: DIDURL) throws -> Bool {
        let targetPath = storeRootPath + "/" + FileSystemStorage.DID_DIR + "/" + did.methodSpecificId + "/" + FileSystemStorage.CREDENTIALS_DIR + "/" + id.fragment + "/" + FileSystemStorage.CREDENTIAL_FILE
        return try exists(targetPath)
    }
    
    func deleteCredential(_ did: DID, _ id: DIDURL) throws -> Bool {
        let targetPath = storeRootPath + "/" + FileSystemStorage.DID_DIR + "/" + did.methodSpecificId + "/" + FileSystemStorage.CREDENTIALS_DIR + "/" + id.fragment
        let path = try getFile(targetPath)
        if try exists_dir(path!) {
            return try deleteFile(path!)
        }
        return false
    }
    
    func listCredentials(_ did: DID) throws -> Array<DIDURL> {
        let dir: String = storeRootPath + "/" + FileSystemStorage.DID_DIR + "/" + did.methodSpecificId + "/" + FileSystemStorage.CREDENTIALS_DIR
        guard try exists_dir(dir) else {
            return []
        }
        
        let fileManager = FileManager.default
        let enumerator = try fileManager.contentsOfDirectory(atPath: dir)
        var arr: Array<DIDURL> = []
        for element: String in enumerator  {
            // if !element.hasSuffix(".meta")
            let didUrl: DIDURL = try DIDURL(did, element)
            arr.append(didUrl)
        }
        return arr
    }
    
    public func selectCredentials(_ did: DID, _ id: DIDURL, _ type: Array<Any>) throws -> Array<DIDURL> {
        // TODO: Auto-generated method stub
        return []
    }
    
    public func storePrivateKey(_ did: DID, _ id: DIDURL, _ privateKey: String) throws {
        let path: String = storeRootPath + "/" + FileSystemStorage.DID_DIR + "/" + did.methodSpecificId + "/" + FileSystemStorage.PRIVATEKEYS_DIR + "/" + id.fragment
        let privateKeyPath: String = try getFile(path) ?? ""
        
        // Delete before storing , Java no
//        try _ = deletePrivateKey(did, id)
        let fileManager: FileManager = FileManager.default
        let dir: String = PathExtracter(path).dirNamePart()
        try fileManager.createDirectory(atPath: dir, withIntermediateDirectories: true, attributes: nil)
        fileManager.createFile(atPath: privateKeyPath, contents:nil, attributes:nil)
        let handle = FileHandle(forWritingAtPath: privateKeyPath)
        handle?.write(privateKey.data(using: String.Encoding.utf8)!)
    }
    
    public func loadPrivateKey(_ did: DID, _ id: DIDURL) throws -> String {
        let path: String = storeRootPath + "/" + FileSystemStorage.DID_DIR + "/" + did.methodSpecificId + "/" + FileSystemStorage.PRIVATEKEYS_DIR + "/" + id.fragment
        let privateKeyPath = try getFile(path)
        return try! String(contentsOfFile:privateKeyPath!, encoding: String.Encoding.utf8)
    }
    
    public func containsPrivateKeys(_ did: DID) throws -> Bool  {
        let dir: String = storeRootPath + "/" + FileSystemStorage.DID_DIR + "/" + did.methodSpecificId + "/" + FileSystemStorage.PRIVATEKEYS_DIR
        let path: String = try getFile(dir)!
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
    
    public func containsPrivateKey(_ did: DID, _ id: DIDURL) throws -> Bool {
        let dir: String = storeRootPath + "/" + FileSystemStorage.DID_DIR + "/" + did.methodSpecificId + "/" + FileSystemStorage.PRIVATEKEYS_DIR + "/" + id.fragment
        let path: String = try getFile(dir)!
        return try exists(path)
    }
    
    public func deletePrivateKey(_ did: DID, _ id: DIDURL) throws -> Bool {
        let path: String = storeRootPath + "/" + FileSystemStorage.DID_DIR + "/" + did.methodSpecificId + "/" + FileSystemStorage.PRIVATEKEYS_DIR + "/" + id.fragment
        let privateKeyPath = try getFile(path)
        _ =  try exists(path)
        let fileExists = FileManager.default.fileExists(atPath: privateKeyPath!, isDirectory: nil)
        
        if fileExists {
            try FileManager.default.removeItem(atPath: privateKeyPath!)
            return true
        }
        return false
    }
    
    func exists(_ dirPath: String) throws -> Bool {
        let fileManager = FileManager.default
        var isDir : ObjCBool = false
        fileManager.fileExists(atPath: dirPath, isDirectory:&isDir)
        let readhandle = FileHandle.init(forReadingAtPath: dirPath)
        let data: Data = (readhandle?.readDataToEndOfFile()) ?? Data()
        let str: String = String(data: data, encoding: .utf8) ?? ""
        return str.count > 0 ? true : false
    }
    
    func exists_dir(_ dirPath: String) throws -> Bool {
        let fileManager = FileManager.default
        var isDir : ObjCBool = false
        let re = fileManager.fileExists(atPath: dirPath, isDirectory:&isDir)
        return re && isDir.boolValue
    }

    func getFile(_ create: Bool, _ path: String) throws -> String {
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
            let dirPath: String = PathExtracter(relPath).dirNamePart()
            if try !exists(dirPath) {
                try fileManager.createDirectory(atPath: dirPath, withIntermediateDirectories: true, attributes: nil)
            }
            fileManager.createFile(atPath: relPath, contents: nil, attributes: nil)
        }
        return relPath
    }
    
    func deleteFile(_ path: String) throws -> Bool {
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
    
    private func getFile(_ path: String) throws -> String? {
        return try getFile(false, path)
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
    
    private func readTextFromPath(_ path: String) throws -> String {
        guard try exists(path) else {
            return ""
        }
        return try String(contentsOfFile:path, encoding: String.Encoding.utf8)
    }
    
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
    
    private func needReencrypt(_ path: String) throws -> Bool {
        let patterns: Array<String> = [
            "(.+)\\" + "/" + FileSystemStorage.PRIVATE_DIR + "\\" + "/" + FileSystemStorage.HDKEY_FILE,
            "(.+)\\" + "/" + FileSystemStorage.PRIVATE_DIR + "\\" + "/" + FileSystemStorage.MNEMONIC_FILE,
            "(.+)\\" + "/" + FileSystemStorage.DID_DIR + "\\" + "/" + "(.+)" + "\\" + "/" + FileSystemStorage.PRIVATEKEYS_DIR + "\\" + "/" + "(.+)"]
        for pattern in patterns {
            let matcher: RegexHelper = try RegexHelper(pattern)
            
            if matcher.match(input: path)  { // if (path.matches(pattern))
                return true
            }
        }
        return false
    }
    
    private func copy(_ src: String, _ dest: String, _ reEncryptor: ReEncryptor) throws {
        if isDirectory(src) {
            _ = try getFile(true, dest) // dest create if not
            
            let fileManager = FileManager.default
            let enumerator = try fileManager.contentsOfDirectory(atPath: src)
            for element: String in enumerator  {
                // if !element.hasSuffix(".meta")
                let srcFile = src + element
                let destFile = dest + element
                try copy(srcFile, destFile, reEncryptor)
            }
        }
        else {
            if try needReencrypt(src) {
                let org: String = try readTextFromPath(src)
                try writeTextToPath(dest, reEncryptor(org))
            }
            else {
                let fileManager = FileManager.default
                try fileManager.copyItem(atPath: src, toPath: dest)
            }
        }
    }
    
    private func postChangePassword() throws {
        let privateDir: String = storeRootPath + "/" + FileSystemStorage.PRIVATE_DIR
        let privateDeprecated = storeRootPath + "/" + FileSystemStorage.PRIVATE_DIR + "/" + FileSystemStorage.JOURNAL_SUFFIX
        let privateJournal = storeRootPath + "/" + FileSystemStorage.PRIVATE_DIR + "/"  + FileSystemStorage.JOURNAL_SUFFIX
        
        let didDir = storeRootPath + "/" + FileSystemStorage.DID_DIR
        let didDeprecated = storeRootPath + "/" + FileSystemStorage.DID_DIR + "/" + FileSystemStorage.DEPRECATED_SUFFIX
        let didJournal = storeRootPath + "/" + FileSystemStorage.DID_DIR + "/" + FileSystemStorage.JOURNAL_SUFFIX
        let stageFile = storeRootPath +  "/postChangePassword"

        let fileManager = FileManager.default
        if try exists(stageFile) {
            if try exists(privateJournal) {
                if try exists(privateDir) {
                    try fileManager.moveItem(atPath: privateDir, toPath: privateDeprecated)
                }
                try fileManager.moveItem(atPath: privateJournal, toPath: privateDir)
            }
            if try exists(didJournal) {
                if try exists(didDir) {
                    try fileManager.moveItem(atPath: didDir, toPath: didDeprecated)
                }
                try fileManager.moveItem(atPath: didJournal, toPath: didDir)
            }
            
            _ = try deleteFile(privateDeprecated)
            _ = try deleteFile(didDeprecated)
            _ = try deleteFile(stageFile)
        }
        else {
            if try exists(privateJournal) {
                _ = try deleteFile(privateJournal)
            }
            if try exists(didJournal) {
                _ = try deleteFile(didJournal)
            }
        }
    }
    
    func changePassword(_ reEncryptor: (String) -> String) throws {
        let privateDir = storeRootPath + "/" + FileSystemStorage.PRIVATE_DIR
        let privateJournal = storeRootPath + "/" + FileSystemStorage.PRIVATE_DIR + "/" + FileSystemStorage.JOURNAL_SUFFIX

        let didDir = storeRootPath + "/" + FileSystemStorage.DID_DIR
        let didJournal = storeRootPath + "/" + FileSystemStorage.DID_DIR + "/" + FileSystemStorage.JOURNAL_SUFFIX
        do {
        try copy(privateDir, privateJournal, reEncryptor)
        try copy(didDir, didJournal, reEncryptor)
        }
        catch {
            throw DIDError.didStoreError(_desc: "Change store password failed.")
        }
        try postChangePassword()
    }
    
    func isDirectory(_ path: String) -> Bool {
        let fileManager = FileManager.default
        var isDir : ObjCBool = false
        _ = fileManager.fileExists(atPath: path, isDirectory:&isDir)
        return isDir.boolValue
    }
}

public class Entry<K, V> {
    public var key: K!
    public var value: V!
    
    init(_ key: K, _ value: V) {
        self.key = key
        self.value = value
    }
}
 
