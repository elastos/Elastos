


import XCTest
import ElastosDIDSDK


class DIDStoreTests: XCTestCase {

    let store: String = "\(NSHomeDirectory())/Library/Caches/DIDStore"
    let passphrase: String = "secret"
    
    override func setUp() {
        // Put setup code here. This method is called before the invocation of each test method in the class.
    }

    override func tearDown() {
        // Put teardown code here. This method is called after the invocation of each test method in the class.
    }

    func test00CreateEmptyStore0() {
        deleteFile(store)
        try! DIDStore.creatInstance("filesystem", location: store, passphase: passphrase)
        let tempStore: DIDStore = try! DIDStore.shareInstance()!
        XCTAssertFalse(try! tempStore.hasPrivateIdentity())
        XCTAssertTrue(exists(store))
        let path: String = store + "/" + ".DIDStore"
        let filemanage: FileManager = FileManager.default
        filemanage.fileExists(atPath: path)
    }
    
    func test00CreateEmptyStore1() {
        try! DIDStore.creatInstance("filesystem", location: store, passphase: passphrase)
        let tempStore: DIDStore = DIDStore.shareInstance()!
        let doc: DIDDocument = try! tempStore.newDid(passphrase, "my first did")
        print(doc)
    }
    
    func test000InitPrivateIdentity0() {
        deleteFile(store)
        try! DIDStore.creatInstance("filesystem", location: store, passphase: passphrase)
        let tempStore: DIDStore = DIDStore.shareInstance()!
        XCTAssertFalse(try! tempStore.hasPrivateIdentity())
        
        let mnemonic: String = HDKey.generateMnemonic(0)
        try! tempStore.initPrivateIdentity(mnemonic, passphrase, true)
        let keypath: String = store + "/" + "private" + "/" + "key"
        XCTAssertTrue(existsFile(keypath))
        let indexPath: String = store + "/" + "private" + "/" + "index"
        XCTAssertTrue(existsFile(indexPath))
        XCTAssertTrue(try! tempStore.hasPrivateIdentity())
        
        try! DIDStore.creatInstance("filesystem", location: store, passphase: passphrase)
        let tempStore2: DIDStore = DIDStore.shareInstance()!
        XCTAssertTrue(try! tempStore.hasPrivateIdentity())
    }
    
    /*
     */
    func deleteFile(_ path: String) {
        let filemanager: FileManager = FileManager.default
        var isdir = ObjCBool.init(false)
        let fileExists = filemanager.fileExists(atPath: path, isDirectory: &isdir)
        if fileExists && isdir.boolValue {
            if let dircontents = filemanager.enumerator(atPath: path) {
                for case let url as URL in dircontents {
                    try! deleteFile(url.absoluteString)
                }
            }
        }
        guard fileExists else {
            return
        }
        try! filemanager.removeItem(atPath: path)
    }
    
     func exists(_ dirPath: String) -> Bool {
        let fileManager = FileManager.default
        var isDir : ObjCBool = false
        if fileManager.fileExists(atPath: dirPath, isDirectory:&isDir) {
            if isDir.boolValue {
                return true
            }
        }
        return false
    }
    
    func existsFile(_ path: String) -> Bool {
        var isDirectory = ObjCBool.init(false)
        let fileExists = FileManager.default.fileExists(atPath: path, isDirectory: &isDirectory)
        return !isDirectory.boolValue && fileExists
    }

    func testPerformanceExample() {
        // This is an example of a performance test case.
        self.measure {
            // Put the code you want to measure the time of here.
        }
    }

}
