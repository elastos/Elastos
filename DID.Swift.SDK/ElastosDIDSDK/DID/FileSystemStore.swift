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

    private var storeRoot: String!

    init(_ dir: String) throws {
        super.init()
        storeRoot = dir
        guard !(dir.isEmpty) else {
            // Throws error
            return
        }
        if try exists(dir) {
           try checkStore(dir)
        }
        else {
           try creatStore(dir)
        }
    }

    private func creatStore(_ dir: String) throws {
        let fileManager = FileManager.default
        try fileManager.createDirectory(atPath: dir, withIntermediateDirectories: true, attributes: nil)
        let filePath = "\(dir)/\(FileSystemStore.TAG_FILE)"
        fileManager.createFile(atPath: filePath, contents: nil, attributes: nil)
        let data1 = Data(bytes: FileSystemStore.TAG_MAGIC)
        let data2 = Data(bytes: FileSystemStore.TAG_VERSION)
        let writeHandle = FileHandle(forWritingAtPath: filePath)
        writeHandle?.write(data1)
        writeHandle?.seekToEndOfFile()
        writeHandle?.write(data2)
    }

    private func checkStore(_ dir: String) throws {
        let filePath = "\(dir)/\(FileSystemStore.TAG_FILE)"
        guard try exists(filePath) else {
            // Throws error
            return
        }

        let readHandler = FileHandle(forReadingAtPath: filePath)
        let fileManager = FileManager.default
        let auttributes = try fileManager.attributesOfItem(atPath: filePath)
        let fileSize = auttributes[FileAttributeKey.size] as! Int
        guard fileSize == FileSystemStore.TAG_SIZE  else {
            // Throws error
            return
        }
        let data1 = Data(bytes: FileSystemStore.TAG_MAGIC)
        let data2 = Data(bytes: FileSystemStore.TAG_VERSION)
        let magig = readHandler?.readData(ofLength: 4)
        let seek = UInt64(magig!.count)
        readHandler?.seek(toFileOffset: seek)
        let version = readHandler?.readDataToEndOfFile()
        guard (magig?.count == 4) && (version?.count == 4 ) && magig == data1 && version == data2 else  {
            // TODO: throws error
            return
        }
    }

    private func exists(_ dir: String) throws -> Bool {
        let fileManager = FileManager.default
        try fileManager.createDirectory(atPath: dir, withIntermediateDirectories: true, attributes: nil)
        let exist: Bool = fileManager.fileExists(atPath: dir)
        return exist
    }

    private func getFile(_ create: Bool, _ path: String) throws -> String {
        var relPath = storeRoot
        let fileManager = FileManager.default
        let paths = path.split{$0 == "/"}.map(String.init)

        for (index, path) in paths.enumerated() {
            if index == paths.endIndex - 1 {
                relPath?.append("/")
                relPath?.append(path)

                if create {
                    var isDirectory = ObjCBool.init(false)
                    let fileExists = FileManager.default.fileExists(atPath: relPath!, isDirectory: &isDirectory)
                    if !isDirectory.boolValue && fileExists {
                       try deleteFile(relPath!)
                    }
                }
            }
        }

        if create {
            fileManager.createFile(atPath: relPath!, contents: nil, attributes: nil)
        }
        return relPath!
    }

    private func deleteFile(_ path: String) throws {

        let fileManager = FileManager.default
        var isDir = ObjCBool.init(false)
        let fileExists = fileManager.fileExists(atPath: path, isDirectory: &isDir)
        // If path is a folder, traverse the subfiles under the folder and delete them
        if fileExists && isDir.boolValue {
            if let dirContents = fileManager.enumerator(atPath: path) {

                for case let url as URL in dirContents {
                   try deleteFile(url.absoluteString)
                }
            }
        }
        try fileManager.removeItem(atPath: path)
    }

    override public func hasPrivateIdentity() throws -> Bool {
        return try getHDPrivateKeyFile(FileSystemStore.PRIVATE_DIR, FileSystemStore.HDKEY_FILE, false)
    }

    override public func storePrivateIdentity(_ key: String) throws {
        let path = try getHDPrivateKeyFile(true)
        try writeTextToPath(path, key)
    }

    override public func loadPrivateIdentity() throws -> String {
        let path = FileSystemStore.PRIVATE_DIR + "/" + FileSystemStore.INDEX_FILE
        return try readTextFromPath(path)
    }

    override public func storePrivateIdentityIndex(_ index: Int) throws {
        let targetPath = FileSystemStore.PRIVATE_DIR + "/" + FileSystemStore.INDEX_FILE
        let path = try getFile(true, targetPath)
        try writeTextToPath(path, String(index))
        _ = try readTextFromPath(path)
    }

    override func loadPrivateIdentityIndex() throws -> Int {
        let targetPath = FileSystemStore.PRIVATE_DIR + "/" + FileSystemStore.INDEX_FILE
        let path = try getFile(true, targetPath)
        let index = try readTextFromPath(path)
        return Int(index)!
    }

    override public func setDidHint(_ did: DID,_ hint: String) throws {
        let path = FileSystemStore.DID_DIR + "." + did.methodSpecificId + FileSystemStore.META_EXT
        if hint.isEmpty {
            let fileManager = FileManager.default
            try fileManager.removeItem(atPath: path)
        }
        else {
            let writeHandle = FileHandle(forWritingAtPath: path)
            writeHandle?.write(hint.data(using: .utf8)!)
        }
    }

    override public func getDidHint(_ did: DID) throws -> String {
        let path = FileSystemStore.DID_DIR + "." + did.methodSpecificId + FileSystemStore.META_EXT
        let readHandle = FileHandle(forReadingAtPath: path)
        let hint = String(data: (readHandle?.readDataToEndOfFile())!, encoding: .utf8)
        return hint!
    }

    override public func storeDid(_ doc: DIDDocument ,_ hint: String?) throws {
        let path = FileSystemStore.DID_DIR + doc.subject!.methodSpecificId + FileSystemStore.DOCUMENT_FILE
        let exist = try exists(path)
        // TOOD: 如果存在呢，是否删除？java no
        try doc.toJson(path, true)
        if !exist || (hint != nil && hint!.isEmpty) {
           try setDidHint(doc.subject!, hint!)
        }
    }

    override func loadDid(_ did: DID) throws -> DIDDocument? {
        let path = FileSystemStore.DID_DIR + did.methodSpecificId + FileSystemStore.DOCUMENT_FILE
        let exist = try exists(path)
        if !exist {
            return nil
        }
        return try DIDDocument.fromJson(path)
    }

    override public func containsDid(_ did: DID) throws -> Bool {
        let path = FileSystemStore.DID_DIR + did.methodSpecificId + FileSystemStore.DOCUMENT_FILE
        return try exists(path)
    }

    override public func deleteDid(_ did: DID) throws -> Bool {
        let path = FileSystemStore.DID_DIR + "." + did.methodSpecificId + FileSystemStore.META_EXT
         try deleteFile(path)
        return false
    }

    override func listDids(_ filter: Int) throws -> Array<Entry<DID, String>> {
        let path = try getDir(FileSystemStore.DID_DIR)
        let exist = try exists(path)
        if exist { return [Entry]() }
        
        return [Entry]()
    }

    override func setCredentialHint(_ did: DID, _ id: DIDURL, _ hint: String) throws {
        let targetPath = FileSystemStore.DID_DIR + "/" + did.methodSpecificId + "/" + FileSystemStore.CREDENTIALS_DIR + "." + "/" + id.fragment + "/" + FileSystemStore.META_EXT
        let path: String = try getFile(true, targetPath)
        if hint.isEmpty {
            try deleteFile(path)
        }
        else {
            try writeTextToPath(path, hint)
        }
    }

    override func getCredentialHint(_ did: DID, _ id: DIDURL) throws -> String{
        let targetPath = FileSystemStore.DID_DIR + "/" + did.methodSpecificId + "/" + FileSystemStore.CREDENTIALS_DIR + "." + "/" + id.fragment + "/" + FileSystemStore.META_EXT
        let path = try getFile(targetPath)
        return try readTextFromPath(path!)
    }

    override public func storeCredential(_ credential: VerifiableCredential , _ hint: String?) throws {
        let targetPath = FileSystemStore.DID_DIR + "/" + credential.subject.id.methodSpecificId + "/" + FileSystemStore.CREDENTIALS_DIR + credential.id.fragment
        let path = try getFile(true, targetPath)
        let storeDic =  credential.toJson(credential.issuer, true, false)
        let handle = FileHandle(forWritingAtPath:path)
        let storeData: Data = try JSONSerialization.data(withJSONObject: storeDic, options: [])
        handle?.write(storeData)
        
        if hint != nil && !hint!.isEmpty {
            try setCredentialHint(credential.subject.id, credential.id, hint!)
        }
    }

    // TODO: override loadCredential
    override func containsCredentials(_ did: DID) throws -> Bool {
        let targetPath = FileSystemStore.DID_DIR + "/" + did.methodSpecificId + "/" + FileSystemStore.CREDENTIALS_DIR
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

    override func containsCredential(_ did: DID, _ id: DIDURL) throws -> Bool {
        let targetPath = FileSystemStore.DID_DIR + "/" + did.methodSpecificId + "/" + FileSystemStore.CREDENTIALS_DIR + "/" + id.fragment
        let path = try getFile(targetPath)
        return try exists(path!)
    }

    override func deleteCredential(_ did: DID, _ id: DIDURL) throws -> Bool {
        var targetPath = FileSystemStore.DID_DIR + "/" + did.methodSpecificId + "/" + FileSystemStore.CREDENTIALS_DIR + "." + id.fragment + "/" + FileSystemStore.META_EXT
        var path = try getFile(targetPath)
        if try exists(path!) {
            try deleteFile(path!)
        }
        targetPath = FileSystemStore.DID_DIR + "/" + did.methodSpecificId + "/" + FileSystemStore.CREDENTIALS_DIR + "/" + id.fragment
        path = try getFile(targetPath)
        
        if try exists(path!) {
           try deleteFile(path!)
            return true
        }
        return false
    }

    override public func listCredentials(_ did: DID) throws -> Array<Entry<DIDURL, String>> {
        return [Entry]()
    }

    override func selectCredentials(_ did: DID, _ id: DIDURL, _ type: Array<Any>) throws -> Array<Entry<DIDURL, String>> {
        return [Entry]()
    }

    override func containsPrivateKeys(_ did: DID) throws -> Bool  {
        let dir: String = FileSystemStore.DID_DIR + "/" + did.methodSpecificId + "/" + FileSystemStore.PRIVATEKEYS_DIR
        let path: String = try getFile(dir)!
        let fileManager: FileManager = FileManager.default
        var isDir = ObjCBool.init(false)
        _ = fileManager.fileExists(atPath: path, isDirectory: &isDir)
        if !isDir.boolValue {
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

    override func containsPrivateKey(_ did: DID, _ id: DIDURL) throws -> Bool {
        let dir: String = FileSystemStore.DID_DIR + "/" + did.methodSpecificId + "/" + FileSystemStore.PRIVATEKEYS_DIR + "/" + id.fragment
        let path: String = try getFile(dir)!
        return try exists(path)
    }

    override func storePrivateKey(_ did: DID, _ id: DIDURL, _ privateKey: String) throws {
        let path: String = FileSystemStore.DID_DIR + "/" + did.methodSpecificId + "/" + FileSystemStore.PRIVATEKEYS_DIR + id.fragment
        let privateKeyPath: String = try getFile(path) ?? ""

        // Delete before storing , Java no
        try _ = deletePrivateKey(did, id)
        let fileManager: FileManager = FileManager.default
        fileManager.createFile(atPath: privateKeyPath, contents:nil, attributes:nil)
        let handle = FileHandle(forWritingAtPath:privateKeyPath)
        handle?.write(privateKey.data(using: String.Encoding.utf8)!)
    }

    override func loadPrivateKey(_ did: DID, id: DIDURL) throws -> String {
        let path: String = FileSystemStore.DID_DIR + "/" + did.methodSpecificId + "/" + FileSystemStore.PRIVATEKEYS_DIR + id.fragment
        let privateKeyPath = try getFile(path)
        return try! String(contentsOfFile:privateKeyPath!, encoding: String.Encoding.utf8)
    }

    override func deletePrivateKey(_ did: DID, _ id: DIDURL) throws -> Bool {
        let path: String = FileSystemStore.DID_DIR + "/" + did.methodSpecificId + "/" + FileSystemStore.PRIVATEKEYS_DIR + id.fragment
        let privateKeyPath = try getFile(path)
        let fileExists = FileManager.default.fileExists(atPath: privateKeyPath!, isDirectory: nil)

        if fileExists {
            try FileManager.default.removeItem(atPath: privateKeyPath!)
            return true
        }
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

    private func getDir(_ path: String) throws -> String {
        var dirPath = storeRoot
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
        try deleteFile(writePath!)
        fileManager.createFile(atPath: path, contents:nil, attributes:nil)
        let handle = FileHandle(forWritingAtPath:path)
        handle?.write(text.data(using: String.Encoding.utf8)!)
    }

    private func readTextFromPath(_ path: String) throws -> String {
        let readPath = try getFile(path)
        return try! String(contentsOfFile:readPath!, encoding: String.Encoding.utf8)
    }

    private func getHDPrivateKeyFile(_ create: Bool) throws -> String{
        let path = FileSystemStore.PRIVATE_DIR + "/" + FileSystemStore.HDKEY_FILE
        return try getFile(create, path)
    }

    private func getHDPrivateKeyFile(_ dir: String, _ hdKey: String, _ create: Bool) throws -> Bool {
        let keyPath = "\(dir)/\(hdKey)/key"
        guard try exists(keyPath) else {
            // TODO: THROWS error
            return false
        }
        let readHandler = FileHandle(forReadingAtPath: keyPath)
        let key = readHandler?.readDataToEndOfFile() ?? Data()
        guard key.count != 0 else {
            // TODO: throws error
            return false
        }
        return true
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
