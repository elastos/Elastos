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
public class FileSystemStore: DIDStore {
    
    private static let TAG_FILE: String = ".DIDStore"
    private static let TAG_MAGIC: [UInt8] =  [0x00, 0x0D, 0x01, 0x0D]
    private static let TAG_VERSION: [UInt8] = [0x00, 0x00, 0x00, 0x01]
    private static let TAG_SIZE = 8
    private static let PRIVATE_DIR: String = "private"
    private static let HDKEY_FILE: String = "key"
    private static let INDEX_FILE: String = "index"
    
    private static let DID_DIR: String = "ids"
    private static let DOCUMENT_FILE: String = "document"
    private static let CREDENTIALS_DIR: String = "credentials"
    private static let PRIVATEKEYS_DIR: String = "privatekeys"
    private static let META_EXT: String = ".meta"
    
    private static let DEFAULT_CHARSET: String = "UTF-8"
    
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
        let filePath = "\(dir)/\(FileSystemStore.TAG_FILE)"
        
        var didStoreData: Data = Data(capacity: 8)
        didStoreData.append(Data(bytes: FileSystemStore.TAG_MAGIC, count: 4))
        didStoreData.append(Data(bytes: FileSystemStore.TAG_VERSION, count: 4))
        let tagFilePathURL: URL = URL(fileURLWithPath: filePath)
        try didStoreData.write(to: tagFilePathURL)
    }
    
    private func checkStore() throws {
        // 检查根目录是否是文件
        let fileManager = FileManager.default
        var isDir: ObjCBool = false
        if fileManager.fileExists(atPath: storeRootPath, isDirectory:&isDir) {
            guard isDir.boolValue else {
                throw DIDStoreError.failue("Store root \(storeRootPath ?? "") is a file.")
            }
        }
        
        // 检查.DIDStore文件
        let tagFilePath: String = storeRootPath + "/" + FileSystemStore.TAG_FILE
        var tagFilePathIsDir: ObjCBool = false
        let tagFilePathExists: Bool = fileManager.fileExists(atPath: tagFilePath, isDirectory:&tagFilePathIsDir)
        guard !tagFilePathIsDir.boolValue || tagFilePathExists else {
            throw DIDStoreError.failue("Directory \(tagFilePath) is not a DIDStore.")
        }
        
        let localData = try Data(contentsOf: URL(fileURLWithPath: tagFilePath))
        let uInt8DataArray = [UInt8](localData)
        
        /*
         对应JAVA
         if (file.length() != TAG_SIZE)
         throw new DIDStoreException("Directory \""
         + storeRoot.getAbsolutePath() + "\" is not a DIDStore.");
         */
        guard uInt8DataArray.count == FileSystemStore.TAG_SIZE else {
            throw DIDStoreError.failue("Directory \(tagFilePath) is not a DIDStore.")
        }
        
        let magicArray = localData[0...3]
        let versionArray = localData[4...7]
        
        guard magicArray.elementsEqual(FileSystemStore.TAG_MAGIC) else {
            throw DIDStoreError.failue("Directory \(tagFilePath) is not a DIDStore.")
        }
        
        guard versionArray.elementsEqual(FileSystemStore.TAG_VERSION) else {
            throw DIDStoreError.failue("Directory \(tagFilePath) unsupported version.")
        }
    }
    
    public func exists(_ dirPath: String) throws -> Bool {
        let fileManager = FileManager.default
        var isDir : ObjCBool = false
        fileManager.fileExists(atPath: dirPath, isDirectory:&isDir)
        let readhandle = FileHandle.init(forReadingAtPath: dirPath)
        let data: Data = (readhandle?.readDataToEndOfFile()) ?? Data()
        let str: String = String(data: data, encoding: .utf8) ?? ""
        return str.count > 0 ? true : false
    }
    
    public func getFile(_ create: Bool, _ path: String) throws -> String {
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
    
    public func deleteFile(_ path: String) throws -> Bool {
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
    
    override public func hasPrivateIdentity() throws -> Bool {
        return try getHDPrivateKeyFile(FileSystemStore.PRIVATE_DIR, FileSystemStore.HDKEY_FILE, false)
    }
    
    override public func storePrivateIdentity(_ key: String) throws {
        let path = try getHDPrivateKeyFile(true)
        try writeTextToPath(path, key)
    }
    
    override public func loadPrivateIdentity() throws -> String {
        let path = storeRootPath + "/" + FileSystemStore.PRIVATE_DIR + "/" + FileSystemStore.HDKEY_FILE
        return try readTextFromPath(path)
    }
    
    override public func storePrivateIdentityIndex(_ index: Int) throws {
        let targetPath = storeRootPath + "/" + FileSystemStore.PRIVATE_DIR + "/" + FileSystemStore.INDEX_FILE
        let path = try getFile(true, targetPath)
        try writeTextToPath(path, String(index))
        _ = try readTextFromPath(path)
    }
    
    override func loadPrivateIdentityIndex() throws -> Int {
        let targetPath = storeRootPath + "/" + FileSystemStore.PRIVATE_DIR + "/" + FileSystemStore.INDEX_FILE
        let index = try readTextFromPath(targetPath)
        return Int(index)!
    }
    
    override public func setDidHint(_ did: DID,_ hint: String?) throws {
        let path = storeRootPath + "/" + FileSystemStore.DID_DIR + "/" + "." + did.methodSpecificId + FileSystemStore.META_EXT
        if hint == nil {
            let fileManager = FileManager.default
            guard fileManager.fileExists(atPath: path) else {
                return
            }
            try fileManager.removeItem(atPath: path)
        }
        else {
            let dir: String = PathExtracter(path).dirNamePart()
            let fm = FileManager.default
            try fm.createDirectory(atPath: dir, withIntermediateDirectories: true, attributes: nil)
            fm.createFile(atPath: path, contents: nil, attributes: nil)
            let writeHandle = FileHandle(forWritingAtPath: path)
            writeHandle?.write(hint!.data(using: .utf8)!)
        }
    }
    
    override public func getDidHint(_ did: DID) throws -> String? {
        let path = storeRootPath + "/" + FileSystemStore.DID_DIR + "." + did.methodSpecificId + FileSystemStore.META_EXT
        let readHandle = FileHandle(forReadingAtPath: path)
        let data = readHandle?.readDataToEndOfFile()
        guard data != nil else {
            return nil
        }
        let hint = String(data: data!, encoding: .utf8)
        return hint
    }
    
    override public func storeDid(_ doc: DIDDocument ,_ hint: String?) throws {
        let path = storeRootPath + "/" + FileSystemStore.DID_DIR + "/" + doc.subject!.methodSpecificId + "/" + FileSystemStore.DOCUMENT_FILE
        _ = try getFile(true, path)
        let exist = try exists(path)
        _ = try doc.toJson(path, true)
        if !exist || (hint != nil) {
            try setDidHint(doc.subject!, hint)
        }
    }
    
    override public func loadDid(_ did: DID) throws -> DIDDocument? {
        let path = storeRootPath + "/" + FileSystemStore.DID_DIR + "/" + did.methodSpecificId + "/" + FileSystemStore.DOCUMENT_FILE
        let exist = try exists(path)
        guard exist else {
            return nil
        }
        return try DIDDocument.fromJson(path)
    }
    
    override public func containsDid(_ did: DID) throws -> Bool {
        let path = FileSystemStore.DID_DIR + did.methodSpecificId + FileSystemStore.DOCUMENT_FILE
        return try exists(path)
    }
    
    override public func deleteDid(_ did: DID) throws -> Bool {
        var path = storeRootPath + "/" + FileSystemStore.DID_DIR + "/." + did.methodSpecificId + FileSystemStore.META_EXT
        let re1 = try deleteFile(path)
        path = storeRootPath + "/" + FileSystemStore.DID_DIR + "/" + did.methodSpecificId
        let re2 = try deleteFile(path)
        return re1 && re2
    }
    
    override public func listDids(_ filter: Int) throws -> Array<Entry<DID, String>> {
        var arr: Array<Entry<DID, String>> = [Entry]()
        let path = try getDir(FileSystemStore.DID_DIR)
        let exist = try exists(path)
        if exist {
        }
        let fileManager = FileManager.default
        let enumerator = try! fileManager.contentsOfDirectory(atPath: path)
        for element: String in enumerator {
            if !element.hasSuffix(".meta") {
                let did: DID = DID(DID.METHOD, element)
                let hint = try getDidHint(did)
                let dic: Entry<DID, String> = Entry(did, hint ?? "")
                arr.append(dic)
            }
        }
        return arr
    }
    
    override public func setCredentialHint(_ did: DID, _ id: DIDURL, _ hint: String) throws {
        let targetPath = storeRootPath + "/" + FileSystemStore.DID_DIR + "/" + did.methodSpecificId + "/" + FileSystemStore.CREDENTIALS_DIR + "/." + id.fragment + FileSystemStore.META_EXT
        let path: String = try getFile(true, targetPath)
        if hint.isEmpty {
            _ = try deleteFile(path)
        }
        else {
            try writeTextToPath(path, hint)
        }
    }
    
    override public func getCredentialHint(_ did: DID, _ id: DIDURL) throws -> String{
        let targetPath = storeRootPath + "/" + FileSystemStore.DID_DIR + "/" + did.methodSpecificId + "/" + FileSystemStore.CREDENTIALS_DIR + "." + "/" + id.fragment + "/" + FileSystemStore.META_EXT
        let path = try getFile(targetPath)
        return try readTextFromPath(path!)
    }
    
    override public func loadCredential(_ did: DID, _ id: DIDURL) throws -> VerifiableCredential? {
        let path: String = storeRootPath + "/" + FileSystemStore.DID_DIR + "/" + did.methodSpecificId + "/" + FileSystemStore.CREDENTIALS_DIR + "/" + id.fragment
        guard try exists(path) else { return nil }
        return try VerifiableCredential.fromJsonInPath(path)
    }
    
    override public func storeCredential(_ credential: VerifiableCredential , _ hint: String?) throws {
        let targetPath = storeRootPath + "/" + FileSystemStore.DID_DIR + "/" + credential.subject.id.methodSpecificId + "/" + FileSystemStore.CREDENTIALS_DIR + "/" + credential.id.fragment
        let path = try getFile(true, targetPath)
        let storeDic = credential.toJson(credential.issuer, true, false)
        let storeJson = JsonHelper.creatJsonString(dic: storeDic)
        let data: Data = storeJson.data(using: .utf8)!
        let handle = FileHandle(forWritingAtPath:path)
        handle?.write(data)
        
        if hint != nil && !hint!.isEmpty {
            try setCredentialHint(credential.subject.id, credential.id, hint!)
        }
    }
    
    override public func containsCredentials(_ did: DID) throws -> Bool {
        let targetPath = storeRootPath + "/" + FileSystemStore.DID_DIR + "/" + did.methodSpecificId + "/" + FileSystemStore.CREDENTIALS_DIR
        let path = try getDir(targetPath)
        let exit = try exists(path)
        guard exit else {
            return false
        }
        let arr = try listCredentials(did)
        guard arr.count > 0 else {
            return false
        }
        return true
    }
    
    override public func containsCredential(_ did: DID, _ id: DIDURL) throws -> Bool {
        let targetPath = storeRootPath + "/" + FileSystemStore.DID_DIR + "/" + did.methodSpecificId + "/" + FileSystemStore.CREDENTIALS_DIR + "/" + id.fragment
        let path = try getFile(targetPath)
        return try exists(path!)
    }
    
    override public func deleteCredential(_ did: DID, _ id: DIDURL) throws -> Bool {
        var targetPath = storeRootPath + "/" + FileSystemStore.DID_DIR + "/" + did.methodSpecificId + "/" + FileSystemStore.CREDENTIALS_DIR + "/." + id.fragment + FileSystemStore.META_EXT
        var path = try getFile(targetPath)
        if try exists(path!) {
            _ = try deleteFile(path!)
        }
        targetPath = storeRootPath + "/" + FileSystemStore.DID_DIR + "/" + did.methodSpecificId + "/" + FileSystemStore.CREDENTIALS_DIR + "/" + id.fragment
        path = try getFile(targetPath)
        
        if try exists(path!) {
            _ = try deleteFile(path!)
            return true
        }
        return false
    }
    
    override public func listCredentials(_ did: DID) throws -> Array<Entry<DIDURL, String>> {
        let dir: String = storeRootPath + "/" + FileSystemStore.DID_DIR + "/" + did.methodSpecificId + "/" + FileSystemStore.CREDENTIALS_DIR
        guard try exists(dir) else {
            return [Entry]()
        }
        
        let fileManager = FileManager.default
        let enumerater = fileManager.enumerator(atPath: dir)
        var arr: Array<Entry<DIDURL, String>> = []
        while let file = enumerater?.nextObject() as? String {
            if !file.hasSuffix(".meta") {
                let didUrl: DIDURL = try DIDURL(did, file)
                var hint: String = ""
                hint = try getCredentialHint(did, didUrl)
                let dic: Entry<DIDURL, String> = Entry(didUrl, hint)
                arr.append(dic)
            }
        }
        
        return arr
    }
    
    override public func selectCredentials(_ did: DID, _ id: DIDURL, _ type: Array<Any>) throws -> Array<Entry<DIDURL, String>> {
        // TODO: Auto-generated method stub
        return [Entry]()
    }
    
    override public func containsPrivateKeys(_ did: DID) throws -> Bool  {
        let dir: String = storeRootPath + "/" + FileSystemStore.DID_DIR + "/" + did.methodSpecificId + "/" + FileSystemStore.PRIVATEKEYS_DIR
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
    
    override public func containsPrivateKey(_ did: DID, _ id: DIDURL) throws -> Bool {
        let dir: String = storeRootPath + "/" + FileSystemStore.DID_DIR + "/" + did.methodSpecificId + "/" + FileSystemStore.PRIVATEKEYS_DIR + "/" + id.fragment
        let path: String = try getFile(dir)!
        return try exists(path)
    }
    
    override public func storePrivateKey(_ did: DID, _ id: DIDURL, _ privateKey: String) throws {
        let path: String = storeRootPath + "/" + FileSystemStore.DID_DIR + "/" + did.methodSpecificId + "/" + FileSystemStore.PRIVATEKEYS_DIR + "/" + id.fragment
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
    
    override func loadPrivateKey(_ did: DID, id: DIDURL) throws -> String {
        let path: String = storeRootPath + "/" + FileSystemStore.DID_DIR + "/" + did.methodSpecificId + "/" + FileSystemStore.PRIVATEKEYS_DIR + "/" + id.fragment
        let privateKeyPath = try getFile(path)
        return try! String(contentsOfFile:privateKeyPath!, encoding: String.Encoding.utf8)
    }
    
    override public func deletePrivateKey(_ did: DID, _ id: DIDURL) throws -> Bool {
        let path: String = storeRootPath + "/" + FileSystemStore.DID_DIR + "/" + did.methodSpecificId + "/" + FileSystemStore.PRIVATEKEYS_DIR + "/" + id.fragment
        let privateKeyPath = try getFile(path)
        let fileExists = FileManager.default.fileExists(atPath: privateKeyPath!, isDirectory: nil)
        
        if fileExists {
            try FileManager.default.removeItem(atPath: privateKeyPath!)
            return true
        }
        return false
    }
    
    private func getFile(_ path: String) throws -> String? {
        return try getFile(false, path)
    }
    
    private func getDir(_ path: String) throws -> String {
        var dirPath = storeRootPath
        let paths = path.split{$0 == "/"}.map(String.init)
        
        for (index, path) in paths.enumerated() {
            if index == paths.endIndex - 1 {
                dirPath?.append("/")
                dirPath?.append(path)
            }
        }
        return dirPath!
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
        let readPath = try getFile(path)
        return try! String(contentsOfFile:readPath!, encoding: String.Encoding.utf8)
    }
    
    private func getHDPrivateKeyFile(_ create: Bool) throws -> String{
        let path = storeRootPath + "/" + FileSystemStore.PRIVATE_DIR + "/" + FileSystemStore.HDKEY_FILE
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
