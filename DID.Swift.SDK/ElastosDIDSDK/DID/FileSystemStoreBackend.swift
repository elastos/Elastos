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
public class FileSystemStoreBackend: DIDStoreBackend {
    
    private static let STORE_MAGIC: [UInt8] =  [0x00, 0x0D, 0x01, 0x0D]
    private static let STORE_VERSION: [UInt8] = [0x00, 0x00, 0x00, 0x01]
    private static let STORE_META_SIZE = 8
    private static let PRIVATE_DIR: String = "private"
    private static let HDKEY_FILE: String = "key"
    private static let INDEX_FILE: String = "index"
    
    private static let DID_DIR: String = "ids"
    private static let DOCUMENT_FILE: String = "document"
    private static let CREDENTIALS_DIR: String = "credentials"
    private static let CREDENTIAL_FILE: String = "credential"
    private static let PRIVATEKEYS_DIR: String = "privatekeys"
    private static let META_FILE: String = ".meta"
    
    private static let DEFAULT_CHARSET: String = "UTF-8"
    
    private var storeRootPath: String!
    
    public init(_ dir: String) throws {
        super.init()
        if dir.isEmpty {
            throw DIDStoreError.failue("Invalid DIDStore root directory.")
        }
        self.storeRootPath = dir
        if try exists(dir) {
            try checkStore()
        }
        else {
            try creatStore(dir)
        }
    }
    
    // create store file
    private func creatStore(_ dir: String) throws {
        let fileManager = FileManager.default
        try fileManager.createDirectory(atPath: dir, withIntermediateDirectories: true, attributes: nil)
        let filePath = "\(dir)/\(FileSystemStoreBackend.META_FILE)"
        
        var didStoreData: Data = Data(capacity: 8)
        didStoreData.append(Data(bytes: FileSystemStoreBackend.STORE_MAGIC, count: 4))
        didStoreData.append(Data(bytes: FileSystemStoreBackend.STORE_VERSION, count: 4))
        let tagFilePathURL: URL = URL(fileURLWithPath: filePath)
        try didStoreData.write(to: tagFilePathURL)
    }

    private func checkStore() throws {
        // Check if the root directory is a file
        let fileManager = FileManager.default
        var isDir: ObjCBool = false
        if fileManager.fileExists(atPath: storeRootPath, isDirectory:&isDir) {
            guard isDir.boolValue else {
                throw DIDStoreError.failue("Store root \(storeRootPath ?? "") is a file.")
            }
        }
        
        // check.DIDStore file
        let tagFilePath: String = storeRootPath + "/" + FileSystemStoreBackend.META_FILE
        var tagFilePathIsDir: ObjCBool = false
        let tagFilePathExists: Bool = fileManager.fileExists(atPath: tagFilePath, isDirectory:&tagFilePathIsDir)
        guard !tagFilePathIsDir.boolValue || tagFilePathExists else {
            throw DIDStoreError.failue("Directory \(tagFilePath) is not a DIDStore.")
        }
        
        let localData = try Data(contentsOf: URL(fileURLWithPath: tagFilePath))
        let uInt8DataArray = [UInt8](localData)
        
        guard uInt8DataArray.count == FileSystemStoreBackend.STORE_META_SIZE else {
            throw DIDStoreError.failue("Directory \(tagFilePath) is not a DIDStore.")
        }
        
        let magicArray = localData[0...3]
        let versionArray = localData[4...7]
        
        guard magicArray.elementsEqual(FileSystemStoreBackend.STORE_MAGIC) else {
            throw DIDStoreError.failue("Directory \(tagFilePath) is not a DIDStore.")
        }
        
        guard versionArray.elementsEqual(FileSystemStoreBackend.STORE_VERSION) else {
            throw DIDStoreError.failue("Directory \(tagFilePath) unsupported version.")
        }
    }
    
    public override func storePrivateIdentity(_ key: String) throws {
        let path = try getHDPrivateKeyFile(true)
        try writeTextToPath(path, key)
    }
    
    public override func loadPrivateIdentity() throws -> String {
        let path = storeRootPath + "/" + FileSystemStoreBackend.PRIVATE_DIR + "/" + FileSystemStoreBackend.HDKEY_FILE
        return try readTextFromPath(path)
    }
    
    public override func storePrivateIdentityIndex(_ index: Int) throws {
        let targetPath = storeRootPath + "/" + FileSystemStoreBackend.PRIVATE_DIR + "/" + FileSystemStoreBackend.INDEX_FILE
        let path = try getFile(true, targetPath)
        try writeTextToPath(path, String(index))
        _ = try readTextFromPath(path)
    }
    
    public override func loadPrivateIdentityIndex() throws -> Int {
        let targetPath = storeRootPath + "/" + FileSystemStoreBackend.PRIVATE_DIR + "/" + FileSystemStoreBackend.INDEX_FILE
        let index = try readTextFromPath(targetPath)
        return Int(index)!
    }
    
    public override func storeDidAlias(_ did: DID, _ alias: String?) throws {
        let path = storeRootPath + "/" + FileSystemStoreBackend.DID_DIR + "/" + did.methodSpecificId + "/" + FileSystemStoreBackend.META_FILE
        if alias == nil {
            try deleteFile(path)
        }
        else {
            try writeTextToPath(path, alias!)
        }
    }
    
    public override func loadDidAlias(_ did: DID) throws -> String {
        let path = storeRootPath + "/" + FileSystemStoreBackend.DID_DIR + "/" + did.methodSpecificId + "/" + FileSystemStoreBackend.META_FILE
        return try readTextFromPath(path)
    }
    
    public override func storeDid(_ doc: DIDDocument) throws {
        let path = storeRootPath + "/" + FileSystemStoreBackend.DID_DIR + "/" + doc.subject!.methodSpecificId + "/" + FileSystemStoreBackend.DOCUMENT_FILE
        _ = try getFile(true, path)
        _ = try exists(path)
        _ = try doc.toJson(path, true, false)
    }
    
    public override func loadDid(_ did: DID) throws -> DIDDocument {
        let path = storeRootPath + "/" + FileSystemStoreBackend.DID_DIR + "/" + did.methodSpecificId + "/" + FileSystemStoreBackend.DOCUMENT_FILE
        let exist = try exists(path)
        guard exist else {
            throw DIDStoreError.failue("No did.")
        }
        return try DIDDocument.fromJson(path)
    }
    
    public override func containsDid(_ did: DID) throws -> Bool {
        let path = FileSystemStoreBackend.DID_DIR + did.methodSpecificId + FileSystemStoreBackend.DOCUMENT_FILE
        return try exists(path)
    }
    
    public override func deleteDid(_ did: DID) throws -> Bool {
        let path = storeRootPath + "/" + FileSystemStoreBackend.DID_DIR + "/" + did.methodSpecificId
        let re = try deleteFile(path)
        return re
    }
    
    public override func listDids(_ filter: Int) throws -> Array<DID>{
        var arr: Array<DID> = []
        let path = storeRootPath + "/" + FileSystemStoreBackend.DID_DIR
        _ = try exists(path)

        let fileManager = FileManager.default
        let enumerator = try! fileManager.contentsOfDirectory(atPath: path)
        for element: String in enumerator {
            if !element.hasSuffix(".meta") {
                let did: DID = DID(DID.METHOD, element)
                arr.append(did)
            }
        }
        return arr
    }
    
    public override func storeCredentialAlias(_ did: DID, _ id: DIDURL, _ alias: String?) throws {
        let path = storeRootPath + "/" + FileSystemStoreBackend.DID_DIR + "/" + did.methodSpecificId + "/" + FileSystemStoreBackend.CREDENTIALS_DIR + "/" + id.fragment + "/" + FileSystemStoreBackend.META_FILE
        if alias == nil {
           _ = try deleteFile(path)
        }
        else {
            try writeTextToPath(path, alias!)
        }
    }
    
    public override func loadCredentialAlias(_ did: DID, _ id: DIDURL) throws -> String {
        let path = storeRootPath + "/" + FileSystemStoreBackend.DID_DIR + "/" + did.methodSpecificId + "/" + FileSystemStoreBackend.CREDENTIALS_DIR + "/" + id.fragment + "/" + FileSystemStoreBackend.META_FILE
        return try readTextFromPath(path)
    }
    
    public override func storeCredential(_ credential: VerifiableCredential) throws {
        let targetPath = storeRootPath + "/" + FileSystemStoreBackend.DID_DIR + "/" + credential.subject.id.methodSpecificId + "/" + FileSystemStoreBackend.CREDENTIALS_DIR + "/" + credential.id.fragment + "/" + FileSystemStoreBackend.CREDENTIAL_FILE

        let path = try getFile(true, targetPath)
        let storeDic = credential.toJson(credential.issuer, true, false)
        let storeJson = JsonHelper.creatJsonString(dic: storeDic)
        let data: Data = storeJson.data(using: .utf8)!
        let handle = FileHandle(forWritingAtPath:path)
        handle?.write(data)
    }
    
    public override func loadCredential(_ did: DID, _ id: DIDURL) throws -> VerifiableCredential {
        let path: String = storeRootPath + "/" + FileSystemStoreBackend.DID_DIR + "/" + did.methodSpecificId + "/" + FileSystemStoreBackend.CREDENTIALS_DIR + "/" + id.fragment + "/" + FileSystemStoreBackend.CREDENTIAL_FILE

        guard try exists(path) else {
             throw DIDStoreError.failue("No credential.")
        }
        return try VerifiableCredential.fromJsonInPath(path)
    }
    
    public override func containsCredentials(_ did: DID) throws -> Bool {
        let targetPath = storeRootPath + "/" + FileSystemStoreBackend.DID_DIR + "/" + did.methodSpecificId + "/" + FileSystemStoreBackend.CREDENTIALS_DIR
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
    
    public override func containsCredential(_ did: DID, _ id: DIDURL) throws -> Bool {
        let targetPath = storeRootPath + "/" + FileSystemStoreBackend.DID_DIR + "/" + did.methodSpecificId + "/" + FileSystemStoreBackend.CREDENTIALS_DIR + "/" + id.fragment + "/" + FileSystemStoreBackend.CREDENTIAL_FILE
        return try exists(targetPath)
    }
    
    override public func deleteCredential(_ did: DID, _ id: DIDURL) throws -> Bool {
        let targetPath = storeRootPath + "/" + FileSystemStoreBackend.DID_DIR + "/" + did.methodSpecificId + "/" + FileSystemStoreBackend.CREDENTIALS_DIR + "/." + id.fragment
        let path = try getFile(targetPath)
        if try exists(path!) {
            _ = try deleteFile(path!)
        }
        return false
    }
    
    override public func listCredentials(_ did: DID) throws -> Array<DIDURL> {
        let dir: String = storeRootPath + "/" + FileSystemStoreBackend.DID_DIR + "/" + did.methodSpecificId + "/" + FileSystemStoreBackend.CREDENTIALS_DIR
        guard try exists_dir(dir) else {
            return []
        }
        
        let fileManager = FileManager.default
        let enumerator = try fileManager.contentsOfDirectory(atPath: dir)
        var arr: Array<DIDURL> = []
        for element: String in enumerator  {
            if !element.hasSuffix(".meta") {
                let didUrl: DIDURL = try DIDURL(did, element)
                arr.append(didUrl)
            }
        }
        return arr
    }
    
    public override func selectCredentials(_ did: DID, _ id: DIDURL, _ type: Array<Any>) throws -> Array<DIDURL> {
        // TODO: Auto-generated method stub
        return []
    }
    
    public override func storePrivateKey(_ did: DID, _ id: DIDURL, _ privateKey: String) throws {
        let path: String = storeRootPath + "/" + FileSystemStoreBackend.DID_DIR + "/" + did.methodSpecificId + "/" + FileSystemStoreBackend.PRIVATEKEYS_DIR + "/" + id.fragment
        let privateKeyPath: String = try getFile(path) ?? ""
        
        // Delete before storing , Java no
        try _ = deletePrivateKey(did, id)
        let fileManager: FileManager = FileManager.default
        let dir: String = PathExtracter(path).dirNamePart()
        try fileManager.createDirectory(atPath: dir, withIntermediateDirectories: true, attributes: nil)
        fileManager.createFile(atPath: privateKeyPath, contents:nil, attributes:nil)
        let handle = FileHandle(forWritingAtPath: privateKeyPath)
        handle?.write(privateKey.data(using: String.Encoding.utf8)!)
    }
    
    public override func loadPrivateKey(_ did: DID, _ id: DIDURL) throws -> String {
        let path: String = storeRootPath + "/" + FileSystemStoreBackend.DID_DIR + "/" + did.methodSpecificId + "/" + FileSystemStoreBackend.PRIVATEKEYS_DIR + "/" + id.fragment
        let privateKeyPath = try getFile(path)
        return try! String(contentsOfFile:privateKeyPath!, encoding: String.Encoding.utf8)
    }
    
    public override func containsPrivateKeys(_ did: DID) throws -> Bool  {
        let dir: String = storeRootPath + "/" + FileSystemStoreBackend.DID_DIR + "/" + did.methodSpecificId + "/" + FileSystemStoreBackend.PRIVATEKEYS_DIR
        let path: String = try getFile(dir)!
        let fileManager: FileManager = FileManager.default
        var isDir = ObjCBool.init(false)
        _ = fileManager.fileExists(atPath: path, isDirectory: &isDir)
        guard isDir.boolValue else {
            return false
        }
        
        var keys: [URL] = []
        if let dirContents = fileManager.enumerator(atPath: path) {
            // determine whether files are hidden or not
            for case let url as URL in dirContents {
                // Not hiding files
                if url.absoluteString.first?.description != "." {
                    keys.append(url)
                }
            }
        }
        return keys.count > 0
    }
    
    public override func containsPrivateKey(_ did: DID, _ id: DIDURL) throws -> Bool {
        let dir: String = storeRootPath + "/" + FileSystemStoreBackend.DID_DIR + "/" + did.methodSpecificId + "/" + FileSystemStoreBackend.PRIVATEKEYS_DIR + "/" + id.fragment
        let path: String = try getFile(dir)!
        return try exists(path)
    }
    
    public override func deletePrivateKey(_ did: DID, _ id: DIDURL) throws -> Bool {
        let path: String = storeRootPath + "/" + FileSystemStoreBackend.DID_DIR + "/" + did.methodSpecificId + "/" + FileSystemStoreBackend.PRIVATEKEYS_DIR + "/" + id.fragment
        let privateKeyPath = try getFile(path)
        let fileExists = FileManager.default.fileExists(atPath: privateKeyPath!, isDirectory: nil)
        
        if fileExists {
            try FileManager.default.removeItem(atPath: privateKeyPath!)
            return true
        }
        return false
    }
    
    // -----------------------
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
        var re: Bool = false
        if fileExists && isDir.boolValue {
            if let dirContents = fileManager.enumerator(atPath: path) {
                
                for case let url as URL in dirContents {
                    re = try deleteFile(url.absoluteString)
                }
            }
        }
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
        let path = storeRootPath + "/" + FileSystemStoreBackend.PRIVATE_DIR + "/" + FileSystemStoreBackend.HDKEY_FILE
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
}

public class Entry<K, V> {
    public var key: K!
    public var value: V!
    
    init(_ key: K, _ value: V) {
        self.key = key
        self.value = value
    }
}
