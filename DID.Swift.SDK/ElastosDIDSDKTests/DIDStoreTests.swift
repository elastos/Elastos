
import XCTest
import ElastosDIDSDK

class DIDStoreTests: XCTestCase {
    
    var store: DIDStore!
    static var ids: Dictionary<DID, String> = [: ]
    static var primaryDid: DID!
    var adapter: SPVAdaptor!
    
    func testCreateEmptyStore() {
        do {
            let testData: TestData = TestData()
            try testData.setupStore(true)
            
            let _: DIDStore = try DIDStore.shareInstance()!
            _ = testData.exists(storePath)
            
            let path = storePath + "/" + ".meta"
            _ = testData.existsFile(path)
        } catch {
            print("testCreateEmptyStore error: \(error)")
            XCTFail()
        }
    }
    /*
     @Test(expected = DIDStoreException.class)
     public void testCreateDidInEmptyStore() throws DIDStoreException {
     TestData testData = new TestData();
     testData.setupStore(true);
     
     DIDStore store = DIDStore.getInstance();
     store.newDid(TestConfig.storePass, "this will be fail");
     }
     */
    func testInitPrivateIdentity0() {
        do {
            let testData: TestData = TestData()
            try testData.setupStore(true)
            var store: DIDStore = try DIDStore.shareInstance()!
            
            XCTAssertFalse(try store.containsPrivateIdentity())
            
            _ = try testData.initIdentity()
            XCTAssertTrue(try store.containsPrivateIdentity())
                        
            var path = storePath + "/" + "private" + "/" + "key"
            XCTAssertTrue(testData.existsFile(path))
            path = storePath + "/" + "private" + "/" + "index"
            XCTAssertTrue(testData.existsFile(path))
            
            try DIDStore.creatInstance("filesystem", storePath, DummyAdapter())
            
            store = try DIDStore.shareInstance()!
            XCTAssertTrue(try store.containsPrivateIdentity())
        } catch {
            print(error)
            XCTFail()
        }
    }
    
    func testCreateDIDWithAlias() {
        do {
            let testData: TestData = TestData()
            try testData.setupStore(true)
            _ = try testData.initIdentity()
            
            let store: DIDStore = try DIDStore.shareInstance()!
            let alias: String = "my first did"
            
            let doc: DIDDocument = try store.newDid(storePass, alias)
            XCTAssertTrue(try doc.isValid())
            
            var resolved = try store.resolveDid(doc.subject!, true)
            XCTAssertNil(resolved)
            
            _ = try store.publishDid(doc, storePass)
            var path = storePath
            
            path = storePath + "/ids/" + doc.subject!.methodSpecificId + "/document"
            XCTAssertTrue(testData.existsFile(path))
            
            
            path = storePath + "/ids/" + doc.subject!.methodSpecificId + "/.meta"
            XCTAssertTrue(testData.existsFile(path))
            
            resolved = try store.resolveDid(doc.subject!, true)!
            
            XCTAssertNotNil(resolved)
            XCTAssertEqual(alias, resolved!.alias)
            XCTAssertEqual(doc.subject, resolved!.subject)
            XCTAssertEqual(doc.proof.signature, resolved!.proof.signature)
            
            XCTAssertTrue(try resolved!.isValid())
        } catch {
            print(error)
            XCTFail()
        }
    }
    
    func tesCreateDIDWithoutAlias() {
        do {
            let testData: TestData = TestData()
            try testData.setupStore(true)
            try testData.initIdentity()
            let store: DIDStore = try DIDStore.shareInstance()!
            
            let doc: DIDDocument = try store.newDid(storePass)
            XCTAssertTrue(try doc.isValid())
            
            var resolved: DIDDocument = try store.resolveDid(doc.subject!, true)!
            XCTAssertNil(resolved)
            
            try store.publishDid(doc, storePass)
            
            var path = storePath + "/ids/" + doc.subject!.methodSpecificId + "/document"
            XCTAssertTrue(testData.existsFile(path))
            
            path = storePath + "/ids/" + doc.subject!.methodSpecificId + "/.meta"
            XCTAssertFalse(testData.existsFile(path))
            
            resolved = try store.resolveDid(doc.subject!, true)!
            XCTAssertNotNil(resolved)
            XCTAssertEqual(doc.subject, resolved.subject)
            XCTAssertEqual(doc.proof.signature, resolved.proof.signature)
            
            XCTAssertTrue(try resolved.isValid())
        } catch {
            XCTFail()
        }
    }
    
    func testBulkCreate() {
        do {
            let testData: TestData = TestData()
            try testData.setupStore(true)
            try testData.initIdentity()
            let store: DIDStore = try DIDStore.shareInstance()!
            
            for i in 0..<100 {
                let alias: String = "my did \(i)"
                let doc: DIDDocument = try store.newDid(storePass, alias)
                XCTAssertTrue(try doc.isValid())
                
                var resolved = try store.resolveDid(doc.subject!, true)
                XCTAssertNil(resolved)
                
                try store.publishDid(doc, storePass)
                
                var path = storePath + "/ids/" + doc.subject!.methodSpecificId + "/document"
                XCTAssertTrue(testData.existsFile(path))
                
                path = storePath + "/ids/" + doc.subject!.methodSpecificId + "/.meta"
                XCTAssertTrue(testData.existsFile(path))
                
                resolved = try store.resolveDid(doc.subject!, true)
                XCTAssertNotNil(resolved)
                XCTAssertEqual(alias, resolved!.alias)
                XCTAssertEqual(doc.subject, resolved!.subject)
                XCTAssertEqual(doc.proof.signature, resolved!.proof.signature)
                XCTAssertTrue(try resolved!.isValid())
                
                var dids: Array<DID> = try store.listDids(DIDStore.DID_ALL)
                XCTAssertEqual(100, dids.count)
                
                dids = try store.listDids(DIDStore.DID_HAS_PRIVATEKEY)
                XCTAssertEqual(100, dids.count)
                
                dids = try store.listDids(DIDStore.DID_NO_PRIVATEKEY)
                XCTAssertEqual(0, dids.count)
            }
        } catch {
            XCTFail()
        }
    }
    
    func testDeleteDID() {
        do {
            let testData: TestData = TestData()
            try testData.setupStore(true)
            try testData.initIdentity()
            let store: DIDStore = try DIDStore.shareInstance()!
            // Create test DIDs
            var dids: Array<DID> = []
            for i in 0..<100 {
                let alias: String = "my did \(i)"
                let doc: DIDDocument = try store.newDid(storePass, alias)
                try store.publishDid(doc, storePass)
                dids.append(doc.subject!)
            }
            
            for i in 0..<100 {
                if (i % 5 != 0){
                    continue
                }
                
                let did: DID = dids[i]
                
                var deleted: Bool = try store.deleteDid(did)
                XCTAssertTrue(deleted)
                
                let path = storePath + "/ids/" + did.methodSpecificId
                XCTAssertFalse(testData.exists(path))
                
                deleted = try store.deleteDid(did)
                XCTAssertFalse(deleted)
            }
            var remains: Array<DID> = try store.listDids(DIDStore.DID_ALL)
            XCTAssertEqual(80, remains.count)
            
            remains = try store.listDids(DIDStore.DID_HAS_PRIVATEKEY)
            XCTAssertEqual(80, remains.count)
            
            remains = try store.listDids(DIDStore.DID_NO_PRIVATEKEY)
            XCTAssertEqual(0, remains.count)
        } catch  {
            XCTFail()
        }
    }
    
    func testStoreAndLoadDID() {
        do {
            let testData: TestData = TestData()
            try testData.setupStore(true)
            try testData.initIdentity()
            
            // Store test data into current store
            let issuer: DIDDocument = try testData.loadTestDocument()
            let test: DIDDocument = try testData.loadTestIssuer()
            
            let store: DIDStore = try DIDStore.shareInstance()!
            
            var doc: DIDDocument = try  store.loadDid(issuer.subject!)
            XCTAssertEqual(issuer.subject, doc.subject)
            XCTAssertEqual(issuer.proof.signature, doc.proof.signature)
            XCTAssertTrue(try doc.isValid())
            
            doc = try store.loadDid(test.subject!.description)
            XCTAssertEqual(test.subject, doc.subject)
            XCTAssertEqual(test.proof.signature, doc.proof.signature)
            XCTAssertTrue(try doc.isValid())
            
            var dids: Array<DID> = try store.listDids(DIDStore.DID_ALL)
            XCTAssertEqual(2, dids.count)
            
            dids = try store.listDids(DIDStore.DID_HAS_PRIVATEKEY)
            XCTAssertEqual(2, dids.count)
            
            dids = try store.listDids(DIDStore.DID_NO_PRIVATEKEY)
            XCTAssertEqual(0, dids.count)
        }
        catch {
            XCTFail()
        }
    }
    
    func testLoadCredentials() {
        do {
            let testData: TestData = TestData()
            try testData.setupStore(true)
            try testData.initIdentity()
            
            // Store test data into current store
            try testData.loadTestIssuer()
            let test: DIDDocument = try testData.loadTestDocument()
            var vc = try testData.loadProfileCredential()
            try vc!.setAlias("MyProfile")
            vc = try testData.loadEmailCredential()
            try vc!.setAlias("Email")
            vc = try testData.loadTwitterCredential()
            try vc!.setAlias("Twitter")
            vc = try testData.loadPassportCredential()
            try vc!.setAlias("Passport")
            
            let store: DIDStore = try! DIDStore.shareInstance()!
            
            var id: DIDURL = try DIDURL(test.subject!, "profile")
            vc = try store.loadCredential(test.subject!, id)
            XCTAssertNotNil(vc)
            XCTAssertEqual("MyProfile", try vc!.getAlias())
            XCTAssertEqual(test.subject, vc!.subject.id)
            XCTAssertEqual(id, vc!.id)
            XCTAssertTrue(try vc!.isValid())
            
            // try with full id string
            vc = try store.loadCredential(test.subject!.description, id.description)
            XCTAssertNotNil(vc)
            XCTAssertEqual("MyProfile", try vc!.getAlias())
            XCTAssertEqual(test.subject, vc!.subject.id)
            XCTAssertEqual(id, vc!.id)
            XCTAssertTrue(try vc!.isValid())
            
            id = try DIDURL(test.subject!, "twitter")
            vc = try store.loadCredential(test.subject!.description, "twitter")
            XCTAssertNotNil(vc)
            XCTAssertEqual("Twitter", try vc!.getAlias())
            XCTAssertEqual(test.subject, vc!.subject.id)
            XCTAssertEqual(id, vc!.id)
            XCTAssertTrue(try vc!.isValid())
            
            vc = try  store.loadCredential(test.subject!.description, "notExist")
            XCTAssertNil(vc)
            
            id = try DIDURL(test.subject!, "twitter")
            XCTAssertTrue(try store.containsCredential(test.subject!, id))
            XCTAssertTrue(try store.containsCredential(test.subject!.description, "twitter"))
            XCTAssertFalse(try store.containsCredential(test.subject!.description, "notExist"))
        }
        catch {
            XCTFail()
        }
    }
    
    func testListCredentials() {
        do {
            let testData: TestData = TestData()
            try testData.setupStore(true)
            try testData.initIdentity()
            
            // Store test data into current store
            try testData.loadTestIssuer()
            let test: DIDDocument = try testData.loadTestDocument()
            var vc = try testData.loadProfileCredential()
            try vc!.setAlias("MyProfile")
            vc = try testData.loadEmailCredential()
            try vc!.setAlias("Email")
            vc = try testData.loadTwitterCredential()
            try vc!.setAlias("Twitter")
            vc = try testData.loadPassportCredential()
            try vc!.setAlias("Passport")
            let store: DIDStore = try DIDStore.shareInstance()!
            
            var vcs: Array<DIDURL> = try store.listCredentials(test.subject!)
            XCTAssertEqual(4, vcs.count)
            for id in vcs {
                var re = id.fragment == "profile" || id.fragment == "email" || id.fragment == "twitter" || id.fragment == "passport"
                XCTAssertTrue(re)
                
                re = id.alias == "MyProfile" || id.alias == "Email" || id.alias == "Twitter" || id.alias == "Passport"
                XCTAssertTrue(re)
            }
        } catch {
            print(error)
            XCTFail()
        }
    }
    
    func testDeleteCredential() {
        do {
            let testData: TestData = TestData()
            try testData.setupStore(true)
            try testData.initIdentity()
            
            // Store test data into current store
            try testData.loadTestIssuer()
            let test: DIDDocument = try testData.loadTestDocument()
            var vc = try testData.loadProfileCredential()
            try vc!.setAlias("MyProfile")
            vc = try testData.loadEmailCredential()
            try vc!.setAlias("Email")
            vc = try testData.loadTwitterCredential()
            try vc!.setAlias("Twitter")
            vc = try testData.loadPassportCredential()
            try vc!.setAlias("Passport")
            
            let store = try DIDStore.shareInstance()!
            var path = storePath + "/ids/" + test.subject!.methodSpecificId + "/credentials/twitter/credential"
            XCTAssertTrue(testData.existsFile(path))
            
            path = storePath + "/" + "ids" + "/" + test.subject!.methodSpecificId + "/" + "credentials" + "/" + "twitter" + "/" + ".meta"
            XCTAssertTrue(testData.existsFile(path))
            
            path = storePath + "/" + "ids" + "/" + test.subject!.methodSpecificId + "/" + "credentials" + "/" + "passport" + "/" + "credential"
            XCTAssertTrue(testData.existsFile(path))
            
            path = storePath + "/" + "ids" + "/" + test.subject!.methodSpecificId
                + "/" + "credentials" + "/" + "passport" + "/" + ".meta"
            XCTAssertTrue(testData.existsFile(path))
            
            var deleted: Bool = try store.deleteCredential(test.subject!, DIDURL(test.subject!, "twitter"))
            XCTAssertTrue(deleted)
            
            deleted = try store.deleteCredential(test.subject!.description, "passport")
            XCTAssertTrue(deleted)
            
            deleted = try store.deleteCredential(test.subject!.description, "notExist")
            XCTAssertFalse(deleted)
            
            path = storePath + "/" + "ids"
                + "/" + test.subject!.methodSpecificId
                + "/" + "credentials" + "/" + "twitter"
            XCTAssertFalse(testData.existsFile(path))
            
            path = storePath + "/" + "ids"
                + "/" + test.subject!.methodSpecificId
                + "/" + "credentials" + "/" + "passport"
            XCTAssertFalse(testData.existsFile(path))
            
            XCTAssertTrue(try store.containsCredential(test.subject!.description, "email"))
            XCTAssertTrue(try store.containsCredential(test.subject!.description, "profile"))
            
            XCTAssertFalse(try store.containsCredential(test.subject!.description, "twitter"))
            XCTAssertFalse(try store.containsCredential(test.subject!.description, "passport"))
        } catch {
            print(error)
            XCTFail()
        }
    }
    
    func createDataForPerformanceTest() {
        do {
            let store: DIDStore = try  DIDStore.shareInstance()!
            
            var props: Dictionary<String, String> = [: ]
            props["name"] = "John"
            props["gender"] = "Male"
            props["nation"] = "Singapore"
            props["language"] = "English"
            props["email"] = "john@example.com"
            props["twitter"] = "@john"
            
            for i in 0..<10 {
                let alias: String = "my did \(i)"
                let doc: DIDDocument = try store.newDid(storePass, alias)
                
                let issuer: Issuer = try Issuer(doc)
                let vc: VerifiableCredential = try issuer.seal(for: doc.subject!, "cred-1", ["BasicProfileCredential", "SelfProclaimedCredential"], props, storePass)
                try store.storeCredential(vc)
            }
        } catch {
            print(error)
            XCTFail()
        }
    }
    
    func testStorePerformance(_ cached: Bool) {
        do {
            let adapter: DIDAdapter = DummyAdapter()
            let testData: TestData = TestData()
            TestData.deleteFile(storePath)
            if (cached){
                try DIDStore.creatInstance("filesystem", storePath, adapter)
            }
            else {
                try DIDStore.creatInstance("filesystem", storePath, adapter, 0, 0)
            }
            
            let store: DIDStore = try DIDStore.shareInstance()!
            
            let mnemonic: String = HDKey.generateMnemonic(0)
            try store.initPrivateIdentity(0, mnemonic, passphrase, storePass, true)
            
            createDataForPerformanceTest()
            let dids: Array<DID> = try store.listDids(DIDStore.DID_ALL)
            XCTAssertEqual(10, dids.count)
            // TODO: TimeMillis
            /*
             long start = System.currentTimeMillis()
             private void testStorePerformance(boolean cached) throws DIDException {
             
             for (int i = 0; i < 1000; i++) {
             for (DID did : dids) {
             DIDDocument doc = store.loadDid(did);
             assertEquals(did, doc.getSubject());
             
             DIDURL id = new DIDURL(did, "cred-1");
             VerifiableCredential vc = store.loadCredential(did, id);
             assertEquals(id, vc.getId());
             }
             }
             
             long end = System.currentTimeMillis();
             
             System.out.println("Store " + (cached ? "with " : "without ") +
             "cache took " + (end - start) + " milliseconds.");
             }
             */
            
        } catch {
            print(error)
            XCTFail()
        }
    }
    
    func testStoreWithCache() {
        testStorePerformance(true)
    }
    
    func testStoreWithoutCache() {
        testStorePerformance(false)
    }
}

