
import XCTest
import ElastosDIDSDK

class DIDStoreTests: XCTestCase {
    
    var store: DIDStore!
    static var ids: Dictionary<DID, String> = [: ]
    static var primaryDid: DID!
    var adapter: SPVAdaptor!

    override func setUp() {
        do {
            let cblock: PasswordCallback = ({(walletDir, walletId) -> String in return "test111111"})
            adapter = SPVAdaptor(walletDir, walletId, networkConfig, resolver, cblock)
//            TestUtils.deleteFile(storePath)
            try DIDStore.creatInstance("filesystem", storePath, adapter)
            store = try DIDStore.shareInstance()!
            let mnemonic: String = HDKey.generateMnemonic(0)
            try store.initPrivateIdentity(0, mnemonic, passphrase, storePass, true)
        } catch {
            print(error)
        }
    }
    
    override func tearDown() {
        // Put teardown code here. This method is called after the invocation of each test method in the class.
    }
    
    func test00CreateEmptyStore1() {
        do {
            try DIDStore.creatInstance("filesystem", storePath, adapter)
            let tempStore: DIDStore = try DIDStore.shareInstance()!
            _ = try tempStore.newDid(storePass, "my first did")
        } catch {
            print("test00CreateEmptyStore1 error: \(error)")
        }
    }
    
    func test10InitPrivateIdentity0() {
        do {
            TestUtils.deleteFile(storePath)
            try DIDStore.creatInstance("filesystem", storePath, adapter)
            let tempStore: DIDStore = try DIDStore.shareInstance()!
            XCTAssertFalse(try! tempStore.hasPrivateIdentity())
            
            let mnemonic: String = HDKey.generateMnemonic(0)
            try! tempStore.initPrivateIdentity(0, mnemonic, passphrase, storePass, true)
            let keypath: String = storePath + "/" + "private" + "/" + "key"
            XCTAssertTrue(TestUtils.existsFile(keypath))
            let indexPath: String = storePath + "/" + "private" + "/" + "index"
            XCTAssertTrue(TestUtils.existsFile(indexPath))
            XCTAssertTrue(try! tempStore.hasPrivateIdentity())
            
            try DIDStore.creatInstance("filesystem", storePath, adapter)
            let tempStore2: DIDStore = try DIDStore.shareInstance()!
            XCTAssertTrue(try! tempStore2.hasPrivateIdentity())
        } catch {
            print(error)
        }
    }
    
    func test10InitPrivateIdentity1() {
        do {
            try DIDStore.creatInstance("filesystem", storePath, adapter)
            let tempStore: DIDStore = try DIDStore.shareInstance()!
            XCTAssert(try tempStore.hasPrivateIdentity())
        } catch {
            print(error)
        }
    }
    
    func test30CreateDID1() {
        let hint: String = "my first did"
        let doc: DIDDocument = try! store.newDid(storePass, hint)
        DIDStoreTests.primaryDid = doc.subject
        let id: String = doc.subject!.methodSpecificId!
        let path: String = storePath + "/" + "ids" + "/" + id + "/" + "document"
        XCTAssertTrue(TestUtils.existsFile(path))
        
        let path2: String = storePath + "/" + "ids" + "/" + "." + id + ".meta"
        XCTAssertTrue(TestUtils.existsFile(path2))
        DIDStoreTests.ids[doc.subject!] = hint
    }
    
    func test30CreateDID2() {
        let doc: DIDDocument = try! store.newDid(storePass, nil)
        let id: String = doc.subject!.methodSpecificId!
        let path: String = storePath + "/" + "ids" + "/" + id + "/document"
        XCTAssertTrue(TestUtils.existsFile(path))
        let path2: String = storePath + "/" + "ids" + "/" + "." + id + ".meta"
        XCTAssertFalse(TestUtils.existsFile(path2))
        DIDStoreTests.ids[doc.subject!] = ""
    }
    
    func test30CreateDID3() {
        for i in 0...100 {
            var dic: Dictionary<String, String> = [: ]
            dic["12"] = String(i)
            
            let hint: String = "my did " + String(10)
            let doc: DIDDocument = try! store.newDid(storePass, hint)
            let id: String = doc.subject!.methodSpecificId!
            let path: String = storePath + "/" + "ids" + "/" + id + "/" + "document"
            XCTAssertTrue(TestUtils.existsFile(path))
            
            let path2: String = storePath + "/" + "ids" + "/." + id + ".meta"
            XCTAssertTrue(TestUtils.existsFile(path2))
            DIDStoreTests.ids[doc.subject!] = hint
            print(DIDStoreTests.ids)
        }
    }
    
    func test40DeleteDID1() {
        let dids: Array<DID> = Array(DIDStoreTests.ids.keys)
        dids.forEach { did in
            if did != DIDStoreTests.primaryDid {
                var deleted: Bool = try! store.deleteDid(did)
                XCTAssertTrue(deleted)
                var path: String = storePath + "/ids/" + did.methodSpecificId!
                XCTAssertFalse(TestUtils.exists(path))
                DIDStoreTests.ids.removeValue(forKey: did)
                path = storePath + "/ids/." + did.methodSpecificId! + ".meta"
                XCTAssertFalse(TestUtils.exists(path))
                
                deleted = try! store.deleteDid(did)
                XCTAssertFalse(deleted)
            }
        }
    }
    
    func test40PublishDID() {
        let dids: Array<DID> = Array(DIDStoreTests.ids.keys)
        dids.forEach { did in
            do {
                let doc: DIDDocument = try store.loadDid(did)!
                _ = try store.publishDid(doc, DIDURL(did, "primary"), storePass)
            } catch {
                print(error)
            }
        }
    }
    
    func test50IssueCredential1() {
        do {
            // SelfClaim
            var props: OrderedDictionary<String, String> = OrderedDictionary()
            props["name"] = "Elastos"
            props["email"] = "contact@elastos.org"
            props["website"] = "https://www.elastos.org/"
            props["phone"] = "12345678900"
            
            let expire = TestUtils.currentDateToWantDate(1)
            let issuer = try! Issuer(DIDStoreTests.primaryDid, nil)
            issuer.target = DIDStoreTests.primaryDid
            issuer.vc.id = try! DIDURL(DIDStoreTests.primaryDid, "cred-1")
            issuer.vc.types = ["SelfProclaimedCredential", "BasicProfileCredential"]
            issuer.vc.expirationDate = expire
            issuer.vc.subject.properties = props
            _ = try! issuer.sign(storePass)
            
            var doc = try! store.resolveDid(DIDStoreTests.primaryDid)
            _ = doc!.modify()
            _ = doc!.addCredential(issuer.vc)
            try store.storeDid(doc!)
            
            doc = try store.resolveDid(DIDStoreTests.primaryDid)
            let vcId: DIDURL = try DIDURL(DIDStoreTests.primaryDid, "cred-1")
            issuer.vc = (try doc?.getCredential(vcId))!
            XCTAssertNotNil(issuer.vc)
            XCTAssertEqual(vcId, issuer.vc.id)
            XCTAssertEqual(DIDStoreTests.primaryDid, issuer.vc.subject.id)
        } catch {
            print(error)
        }
    }
    
    func test50IssueCredential2() {
        do {
            let issuerDid: DID = DIDStoreTests.primaryDid
            var issuer: Issuer = try Issuer(issuerDid)
            
            try DIDStoreTests.ids.keys.forEach { did in
                var props: OrderedDictionary<String, String> = OrderedDictionary()
                props["name"] = "Elastos-" + did.methodSpecificId
                props["email"] = "contact@elastos.org"
                props["website"] = "https://www.elastos.org/"
                props["phone"] = did.methodSpecificId
                
                let expire = TestUtils.currentDateToWantDate(1)
                issuer = try Issuer(DIDStoreTests.primaryDid, nil)
                issuer.target = issuerDid
                issuer.vc.id = try DIDURL(DIDStoreTests.primaryDid, "cred-1")
                issuer.vc.types = ["BasicProfileCredential"]
                issuer.vc.expirationDate = expire
                issuer.vc.subject.properties = props
                _ = try issuer.sign(storePass)
                
                try store.storeCredential(issuer.vc, "default")
                
                props.removeAll(keepCapacity: 0)
                props["name"] = "CyberRepublic-" + did.methodSpecificId
                props["email"] = "contact@CyberRepublic.org"
                props["website"] = "https://www.CyberRepublic.org/"
                props["phone"] = did.methodSpecificId
                
                issuer = try Issuer(did, nil)
                issuer.target = did
                issuer.vc.id = try DIDURL(DIDStoreTests.primaryDid, "cred-2")
                issuer.vc.types = ["BasicProfileCredential"]
                issuer.vc.expirationDate = expire
                issuer.vc.subject.properties = props
                _ = try issuer.sign(storePass)
                
                try store.storeCredential(issuer.vc)
                
                var keypath: String = storePath + "/" + "ids/" + did.methodSpecificId +  "/" + "credentials" + "/cred-1"
                XCTAssertTrue(TestUtils.existsFile(keypath))
                
                keypath = storePath + "/" + "ids/" + did.methodSpecificId +  "/" + "credentials" + ".cred-1.meta"
                XCTAssertTrue(TestUtils.existsFile(keypath))
                
                keypath = storePath + "/" + "ids/" + did.methodSpecificId +  "/" + "credentials" + "cred-2"
                XCTAssertTrue(TestUtils.existsFile(keypath))
                
                keypath = storePath + "/" + "ids/" + did.methodSpecificId +  "/" + "credentials" + ".cred-2.meta"
                XCTAssertFalse(TestUtils.existsFile(keypath))
            }
        } catch {
            print(error)
        }
    }
    
    func test60DeleteCredential1() {
        do {
            var deleted: Bool = try store.deleteCredential(DIDStoreTests.primaryDid, DIDURL(DIDStoreTests.primaryDid, "cred-1"))
            XCTAssertTrue(deleted)
            
            deleted = try store.deleteCredential(DIDStoreTests.primaryDid, DIDURL(DIDStoreTests.primaryDid, "cred-2"))
            XCTAssertTrue(deleted)
            
            deleted = try store.deleteCredential(DIDStoreTests.primaryDid, DIDURL(DIDStoreTests.primaryDid, "cred-3"))
            XCTAssertFalse(deleted)
            
            var path: String = storePath + "/ids" + "/" + DIDStoreTests.primaryDid.methodSpecificId + "/credentials" + "/cred-1"
            XCTAssertFalse(TestUtils.existsFile(path))
              
            path = storePath + "/ids" + "/" + DIDStoreTests.primaryDid.methodSpecificId + "/credentials/" + ".cred-1.meta"
            XCTAssertFalse(TestUtils.existsFile(path))

            path = storePath + "/ids" + "/" + DIDStoreTests.primaryDid.methodSpecificId + "/credentials/" + "cred-2"
            XCTAssertFalse(TestUtils.existsFile(path))

            path = storePath + "/ids" + "/" + DIDStoreTests.primaryDid.methodSpecificId + "/credentials/" + ".cred-2.meta"
            XCTAssertFalse(TestUtils.existsFile(path))
        } catch {
            print(error)
        }
    }
    
    func test60ListCredential1() {
        do {
            try DIDStoreTests.ids.keys.forEach { did in
                let creds = try store.listCredentials(did)
                
                if (did == DIDStoreTests.primaryDid) {
                    XCTAssertEqual(0, creds.count)
                }
                else{
                    XCTAssertEqual(2, creds.count)
                }
                
                creds.forEach { cred in
                    if cred.key.fragment == "cred-1" {
                        XCTAssertEqual("default", cred.value)
                    }
                    else if (cred.key.fragment == "cred-2") {
                        XCTAssertNil(cred.value)
                    }
                    else{
                        XCTFail("Unexpected credential id '\(cred.value)'.")
                    }
                }
            }
        } catch {
            print(error)
        }
    }

    func testPerformanceExample() {
        // This is an example of a performance test case.
        self.measure {
            // Put the code you want to measure the time of here.
        }
    }
}

