


import XCTest
import ElastosDIDSDK


class DIDStoreTests: XCTestCase {

    let storePath: String = "\(NSHomeDirectory())/Library/Caches/DIDStore"
    let passphrase: String = "secret"
    var store: DIDStore!
    var ids: Dictionary<DID, String> = [: ]
    var primaryDid: DID!
    
    override func setUp() {
        deleteFile(storePath)
        try! DIDStore.creatInstance("filesystem", location: storePath, passphase: passphrase)
        store = DIDStore.shareInstance()!
        let mnemonic: String = HDKey.generateMnemonic(0)
        try! store.initPrivateIdentity(mnemonic, passphrase, true)
    }

    override func tearDown() {
        // Put teardown code here. This method is called after the invocation of each test method in the class.
    }

    func test00CreateEmptyStore0() {
        deleteFile(storePath)
        try! DIDStore.creatInstance("filesystem", location: storePath, passphase: passphrase)
        let tempStore: DIDStore = try! DIDStore.shareInstance()!
        XCTAssertFalse(try! tempStore.hasPrivateIdentity())
        XCTAssertTrue(exists(storePath))
        let path: String = storePath + "/" + ".DIDStore"
        let filemanage: FileManager = FileManager.default
        filemanage.fileExists(atPath: path)
    }
    
    func test00CreateEmptyStore1() {
        try! DIDStore.creatInstance("filesystem", location: storePath, passphase: passphrase)
        let tempStore: DIDStore = DIDStore.shareInstance()!
        let doc: DIDDocument = try! tempStore.newDid(passphrase, "my first did")
        print(doc)
    }
    
    func test000InitPrivateIdentity0() {
        deleteFile(storePath)
        try! DIDStore.creatInstance("filesystem", location: storePath, passphase: passphrase)
        let tempStore: DIDStore = DIDStore.shareInstance()!
        XCTAssertFalse(try! tempStore.hasPrivateIdentity())
        
        let mnemonic: String = HDKey.generateMnemonic(0)
        try! tempStore.initPrivateIdentity(mnemonic, passphrase, true)
        let keypath: String = storePath + "/" + "private" + "/" + "key"
        XCTAssertTrue(existsFile(keypath))
        let indexPath: String = storePath + "/" + "private" + "/" + "index"
        XCTAssertTrue(existsFile(indexPath))
        XCTAssertTrue(try! tempStore.hasPrivateIdentity())
        
        try! DIDStore.creatInstance("filesystem", location: storePath, passphase: passphrase)
        let tempStore2: DIDStore = DIDStore.shareInstance()!
        XCTAssertTrue(try! tempStore2.hasPrivateIdentity())
    }
    
    func test01InitPrivateIdentity1() {
        try! DIDStore.creatInstance("filesystem", location: storePath, passphase: passphrase)
        let tempStore: DIDStore = DIDStore.shareInstance()!
        XCTAssert(try! tempStore.hasPrivateIdentity())
    }
    
    func test03CreateDID1() {
        let hint: String = "my first did"
        let doc: DIDDocument = try! store.newDid(passphrase, hint)
        primaryDid = doc.subject
        let id: String = doc.subject!.methodSpecificId!
        let path: String = storePath + "/" + "ids" + "/" + id + "/" + "document"
        XCTAssertTrue(existsFile(path))
        
        let path2: String = storePath + "/" + "ids" + "/" + "." + id + ".meta"
        XCTAssertTrue(existsFile(path2))
        ids[doc.subject!] = hint
    }
    
    func test03CreateDID2() {
        let doc: DIDDocument = try! store.newDid(passphrase, nil)
        let id: String = doc.subject!.methodSpecificId!
        let path: String = storePath + "/" + "ids" + "/" + id + "/document"
        XCTAssertTrue(existsFile(path))
        let path2: String = storePath + "/" + "ids" + "/" + "." + id + ".meta"
        XCTAssertFalse(existsFile(path2))
        ids[doc.subject!] = ""
    }
    
    func test03CreateDID3() {
        for i in 0...100 {
            var dic: Dictionary<String, String> = [: ]
            dic["12"] = String(i)
            
            let hint: String = "my did " + String(10)
            let doc: DIDDocument = try! store.newDid(passphrase, hint)
            let id: String = doc.subject!.methodSpecificId!
            let path: String = storePath + "/" + "ids" + "/" + id + "/" + "document"
            XCTAssertTrue(existsFile(path))
            
            let path2: String = storePath + "/" + "ids" + "/." + id + ".meta"
            XCTAssertTrue(existsFile(path2))
            ids[doc.subject!] = hint
            print(ids)
        }
    }
    
    func test04DeleteDID1() {
        let hint: String = "my did for deleted."
        let doc: DIDDocument = try! store.newDid(passphrase, hint)
        ids[doc.subject!] = hint
        primaryDid = doc.subject
        let dids: Array<DID> = Array(ids.keys)
        dids.forEach { did in
            if did.isEqual(primaryDid) {
                var deleted: Bool = try! store.deleteDid(did)
                XCTAssertTrue(deleted)
                var path: String = storePath + "/ids/" + did.methodSpecificId!
                XCTAssertFalse(exists(path))
                
                path = storePath + "/ids/." + did.methodSpecificId! + ".meta"
                XCTAssertFalse(exists(path))
                
                deleted = try! store.deleteDid(did)
                XCTAssertFalse(deleted)
            }
        }
    }
    
    func test04PublishDID() {
        let hint: String = "my did for deleted."
        let doc: DIDDocument = try! store.newDid(passphrase, hint)
        ids[doc.subject!] = hint
        primaryDid = doc.subject
        let dids: Array<DID> = Array(ids.keys)
        dids.forEach { did in
            do {
                let doc: DIDDocument = try store.loadDid(did)!
                try store.publishDid(doc, DIDURL(did, "primary"), passphrase)
            }catch {
                print(error)
            }
        }
    }
    
    func test05IssueSelfClaimCredential1() throws {
        let hint: String = "my did for test05IssueSelfClaimCredential1."
        let doc: DIDDocument = try! store.newDid(passphrase, hint)
        ids[doc.subject!] = hint
        primaryDid = doc.subject
        let issuer: Issuer = try! Issuer(primaryDid, nil)
        var props: OrderedDictionary<String, String> = OrderedDictionary()
        props["name"] = "Elastos"
        props["email"] = "contact@elastos.org"
        props["website"] = "https://www.elastos.org/"
        props["phone"] = "12345678900"
        
        issuer.target = primaryDid
        issuer.vc?.id = try DIDURL(primaryDid, "cred-1")
        issuer.vc?.types = ["SelfProclaimedCredential", "BasicProfileCredential"]
        issuer.vc?.expirationDate = Date()
        issuer.vc?.subject.properties = props
        issuer.sign(passphrase)
        
        var doc2: DIDDocument = try store.resolveDid(primaryDid)
        _ = doc2.modify()
        _ = doc2.addCredential(issuer.vc!)
        try store.storeDid(doc2)
        doc2 = try store.resolveDid(primaryDid)
        let vcId: DIDURL = try DIDURL(primaryDid, "cred-1")
        issuer.vc = try doc2.getCredential(vcId)
        
        XCTAssertNil(issuer.vc)
        XCTAssertEqual(vcId, issuer.vc?.id)
        XCTAssertEqual(primaryDid, issuer.vc?.subject.id)
    }

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
