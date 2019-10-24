


import XCTest
import ElastosDIDSDK


class DIDStoreTests: XCTestCase {
    
    let storePath: String = "\(NSHomeDirectory())/Library/Caches/DIDStore"
    let passphrase: String = "secret"
    let storePass: String = "passwd"
    var store: DIDStore!
    var ids: Dictionary<DID, String> = [: ]
    var primaryDid: DID!
    
    override func setUp() {
        do {
            TestUtils.deleteFile(storePath)
            try DIDStore.creatInstance("filesystem", location: storePath, passphase: passphrase)
            store = try DIDStore.shareInstance()!
            let mnemonic: String = HDKey.generateMnemonic(0)
            try store.initPrivateIdentity(mnemonic, passphrase, true)
        } catch {
            print(error)
        }
    }
    
    override func tearDown() {
        // Put teardown code here. This method is called after the invocation of each test method in the class.
    }
    
    func test00CreateEmptyStore1() {
        do {
            try DIDStore.creatInstance("filesystem", location: storePath, passphase: passphrase)
            let tempStore: DIDStore = try DIDStore.shareInstance()!
            _ = try tempStore.newDid(passphrase, "my first did")
        } catch {
            print("test00CreateEmptyStore1 error: \(error)")
        }
    }
    
    func test00InitPrivateIdentity0() {
        do {
            TestUtils.deleteFile(storePath)
            try! DIDStore.creatInstance("filesystem", location: storePath, passphase: passphrase)
            let tempStore: DIDStore = try DIDStore.shareInstance()!
            XCTAssertFalse(try! tempStore.hasPrivateIdentity())
            
            let mnemonic: String = HDKey.generateMnemonic(0)
            try! tempStore.initPrivateIdentity(mnemonic, passphrase, true)
            let keypath: String = storePath + "/" + "private" + "/" + "key"
            XCTAssertTrue(TestUtils.existsFile(keypath))
            let indexPath: String = storePath + "/" + "private" + "/" + "index"
            XCTAssertTrue(TestUtils.existsFile(indexPath))
            XCTAssertTrue(try! tempStore.hasPrivateIdentity())
            
            try! DIDStore.creatInstance("filesystem", location: storePath, passphase: passphrase)
            let tempStore2: DIDStore = try DIDStore.shareInstance()!
            XCTAssertTrue(try! tempStore2.hasPrivateIdentity())
        } catch {
            print(error)
        }
    }
    
    func test01InitPrivateIdentity1() {
        do {
            try DIDStore.creatInstance("filesystem", location: storePath, passphase: passphrase)
            let tempStore: DIDStore = try DIDStore.shareInstance()!
            XCTAssert(try tempStore.hasPrivateIdentity())
        } catch {
            print(error)
        }
    }
    
    func test03CreateDID1() {
        let hint: String = "my first did"
        let doc: DIDDocument = try! store.newDid(passphrase, hint)
        primaryDid = doc.subject
        let id: String = doc.subject!.methodSpecificId!
        let path: String = storePath + "/" + "ids" + "/" + id + "/" + "document"
        XCTAssertTrue(TestUtils.existsFile(path))
        
        let path2: String = storePath + "/" + "ids" + "/" + "." + id + ".meta"
        XCTAssertTrue(TestUtils.existsFile(path2))
        ids[doc.subject!] = hint
    }
    
    func test03CreateDID2() {
        let doc: DIDDocument = try! store.newDid(passphrase, nil)
        let id: String = doc.subject!.methodSpecificId!
        let path: String = storePath + "/" + "ids" + "/" + id + "/document"
        XCTAssertTrue(TestUtils.existsFile(path))
        let path2: String = storePath + "/" + "ids" + "/" + "." + id + ".meta"
        XCTAssertFalse(TestUtils.existsFile(path2))
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
            XCTAssertTrue(TestUtils.existsFile(path))
            
            let path2: String = storePath + "/" + "ids" + "/." + id + ".meta"
            XCTAssertTrue(TestUtils.existsFile(path2))
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
                XCTAssertFalse(TestUtils.exists(path))
                
                path = storePath + "/ids/." + did.methodSpecificId! + ".meta"
                XCTAssertFalse(TestUtils.exists(path))
                
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

    func test30CreateDID1() {
        do {
            let hint: String = "my first did"
            let doc: DIDDocument = try store.newDid(storePass, hint)
        } catch {
            print(error)
        }
    }
    /*
    public void test30CreateDID1() throws DIDStoreException {
        String hint = "my first did";

        DIDDocument doc = store.newDid(storePass, hint);
        primaryDid = doc.getSubject();

        File file = new File(storeRoot + File.separator + "ids"
                + File.separator + doc.getSubject().getMethodSpecificId()
                + File.separator + "document");
        assertTrue(file.exists());
        assertTrue(file.isFile());

        file = new File(storeRoot + File.separator + "ids"
                + File.separator + "."
                + doc.getSubject().getMethodSpecificId() + ".meta");
        assertTrue(file.exists());
        assertTrue(file.isFile());

        ids.put(doc.getSubject(), hint);
    }
    */
    func testPerformanceExample() {
        // This is an example of a performance test case.
        self.measure {
            // Put the code you want to measure the time of here.
        }
    }

}
