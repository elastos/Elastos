
import XCTest
import ElastosDIDSDK

class DIDStoreTests: XCTestCase {
    
    var store: DIDStore!
    static var ids: Dictionary<DID, String> = [: ]
    static var primaryDid: DID!
    var adapter: SPVAdaptor!

    override func setUp() {
        /*
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
         */
    }
    
    func testCreateEmptyStore() {
        do {
            let testData: TestData = TestData()
            testData.setupStore(true)
            
            let store: DIDStore = try DIDStore.shareInstance()!
            TestData.exists(storePath)
            
            let path = storeRoot + "/" + ".meta"
            TestData.existsFile(path)
        } catch {
            print("testCreateEmptyStore error: \(error)")
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
    func test10InitPrivateIdentity0() {
        do {
            let testData: TestData = TestData()
            testData.setupStore(true)
            let store: DIDStore = try DIDStore.shareInstance()!
            
            XCTAssertFalse(store.containsPrivateIdentity())
            
            testData.initIdentity()
            XCTAssertTrue(store.containsPrivateIdentity())
            XCTAssertFalse(store.containsPrivateIdentity())
            
            testData.initIdentity()
            XCTAssertTrue(store.containsPrivateIdentity())
            
            var path = storeRoot + "/" + "private"
                + "/" + "key"
            XCTAssertTrue(file.exists())
            XCTAssertTrue(file.isFile())
            path = storeRoot + "/" + "private"
                + "/" + "index"
            XCTAssertTrue(file.exists())
            XCTAssertTrue(file.isFile())
            
            DIDStore.creatInstance("filesystem", storeRoot, DummyAdapter())
            
            store = DIDStore.shareInstance()
            XCTAssertTrue(store.containsPrivateIdentity())
        } catch {
            print(error)
        }
    }
    
    func testCreateDIDWithAlias() {
        do {
            let testData: TestData = TestData()
            testData.setupStore(true)
            testData.initIdentity()
            
            let store: DIDStore = DIDStore.shareInstance()
            let alias: String = "my first did"
            
            let doc: DIDDocument = store.newDid(storePass, alias)
            XCTAssertTrue(doc.isValid())
            
            let resolved: DIDDocument = store.resolveDid(doc.getSubject(), true)
            XCTAssertNil(resolved)
            
            store.publishDid(doc, TestConfig.storePass)
            
            var path = storeRoot + "/" + "ids" + "/" + doc.subject.methodSpecificId + "/" + "document"
            XCTAssertTrue(file.exists())
            XCTAssertTrue(file.isFile())
            
            path = storeRoot + "/" + "ids" + "/" + doc.subject.methodSpecificId + "/" + ".meta"
            XCTAssertTrue(file.exists())
            XCTAssertTrue(file.isFile())
            
            resolved = store.resolveDid(doc.Subject, true)
            XCTAssertNotNull(resolved)
            XCTAssertEquals(alias, resolved.Alias)
            XCTAssertEquals(doc.subject, resolved.Subject)
            XCTAssertEquals(doc.proof.signature, resolved.proof.signature)
            
            XCTAssertTrue(resolved.isValid())
        } catch {
            print(error)
        }
    }
    
    func tesCreateDIDWithoutAlias() {
        let testData: TestData = TestData()
        testData.setupStore(true)
        testData.initIdentity()
        let store: DIDStore = DIDStore.shareInstance()
        
        let doc: DIDDocument = store.newDid(TestConfig.storePass)
        XCTAssertTrue(doc.isValid())
        
        let resolved: DIDDocument = store.resolveDid(doc.getSubject(), true)
        XCTAssertNull(resolved)
        
        store.publishDid(doc, storePass)
        
        var path = storeRoot + "/" + "ids" + "/" + doc.subject.methodSpecificId + "/" + "document"
        XCTAssertTrue(TestData.existsFile(path))
        
        file = storeRoot + "/" + "ids" + "/" + doc.subject.methodSpecificId + "/" + ".meta"
        assertFalse(file.exists());
        
        resolved = store.resolveDid(doc.getSubject(), true)
        XCTAssertNotNil(resolved)
        XCTAssertEqual(doc.subject, resolved.subject)
        XCTAssertEqual(doc.proof.signature, resolved.proof.signature)
        
        XCTAssertTrue(resolved.isValid())
    }
    
    func testBulkCreate() {
        let testData: TestData = TestData()
        testData.setupStore(true)
        testData.initIdentity()
        let store: DIDStore = DIDStore.shareInstance()
        
        for i in 0..<100 {
            let alias: String = "my did " + i
            let doc: DIDDocument = store.newDid(storePass, alias)
            XCTAssertTrue(doc.isValid())
            
            let resolved: DIDDocument = store.resolveDid(doc.subject, true)
            XCTAssertNil(resolved)
            
            store.publishDid(doc, storePass)
            
            var path = storeRoot + "/" + "ids" + "/" + doc.subject.methodSpecificId + "/" + "document"
            XCTAssertTrue(TestData.existsFile(path))
            
            path = storeRoot + "/" + "ids" + "/" + doc.subject.methodSpecificId + "/" + ".meta"
            XCTAssertTrue(TestData.existsFile(path))
            
            resolved = store.resolveDid(doc.subject, true)
            XCTAssertNotNil(resolved)
            XCTAssertEqual(alias, resolved.alias)
            XCTAssertEqual(doc.subject, resolved.subject)
            XCTAssertEqual(doc.proof.signature, resolved.proof.signature)
            XCTAssertTrue(resolved.isValid())
            
            let dids: Array<DID> = store.listDids(DIDStore.DID_ALL)
            XCTAssertEquals(100, dids.count)
            
            dids = store.listDids(DIDStore.DID_HAS_PRIVATEKEY)
            XCTAssertEqual(100, dids.count)
            
            dids = store.listDids(DIDStore.DID_NO_PRIVATEKEY)
            XCTAssertEquals(0, dids.count)
        }
    }
    
    func testDeleteDID() {
        let testData: TestData = TestData()
        testData.setupStore(true)
        testData.initIdentity()
        let store: DIDStore = DIDStore.shareInstance()
        // Create test DIDs
        var dids: Array<DID> = []
        for i in 0..<100 {
            let alias: String = "my did " + i
            let doc: DIDDocument = store.newDid(storePass, alias)
            store.publishDid(doc, storePass)
            dids.append(doc.subject)
        }
        
        for i in 0..<100 {
            if (i % 5 != 0){
                continue
            }
            
            let did: DID = dids[i]
            
            let deleted: Bool = store.deleteDid(did)
            XCTAssertTrue(deleted)
            
            var path = storeRoot + "/" + "ids" + "/" + did.methodSpecificId
            XCTAssertFalse(file.exists())
            
            deleted = store.deleteDid(did)
            XCTAssertFalse(deleted)
            
            var remains: Array<DID> = store.listDids(DIDStore.DID_ALL)
            XCTAssertEquals(80, remains.count)
            
            remains = store.listDids(DIDStore.DID_HAS_PRIVATEKEY)
            XCTAssertEquals(80, remains.count)
            
            remains = store.listDids(DIDStore.DID_NO_PRIVATEKEY)
            XCTAssertEquals(0, remains.count)
        }
    }
    
    func testStoreAndLoadDID() {
        let testData: TestData = TestData()
        testData.setupStore(true)
        testData.initIdentity()
        
        // Store test data into current store
        let issuer: DIDDocument = testData.loadTestDocument()
        let test: DIDDocument = testData.loadTestIssuer()
        
        let store: DIDStore = DIDStore.shareInstance()
        
        let doc: DIDDocument = store.loadDid(issuer.getSubject())
        XCTAssertEquals(issuer.subject, doc.subject)
        XCTAssertEquals(issuer.proof.signature, doc.proof.signature)
        XCTAssertTrue(doc.isValid())
        
        doc = store.loadDid(test.subject.description)
        XCTAssertEqual(test.subject, doc.subject)
        XCTAssertEqual(test.proof.signature, doc.proof.signature)
        XCTAssertTrue(doc.isValid())
        
        var dids: Array<DID> = store.listDids(DIDStore.DID_ALL)
        XCTAssertEquals(2, dids.count)
        
        dids = store.listDids(DIDStore.DID_HAS_PRIVATEKEY)
        XCTAssertEquals(2, dids.count)
        
        dids = store.listDids(DIDStore.DID_NO_PRIVATEKEY)
        XCTAssertEquals(0, dids.count)
    }
    
    func testLoadCredentials() {
        let testData: TestData = TestData()
        testData.setupStore(true)
        testData.initIdentity()
        
        // Store test data into current store
        testData.loadTestIssuer()
        let test: DIDDocument = testData.loadTestDocument()
        let vc: VerifiableCredential = testData.loadProfileCredential()
        vc.setAlias("MyProfile")
        vc = testData.loadEmailCredential()
        vc.setAlias("Email")
        vc = testData.loadTwitterCredential()
        vc.setAlias("Twitter")
        vc = testData.loadPassportCredential()
        vc.setAlias("Passport")
        
        let store: DIDStore = DIDStore.shareInstance()
        
        let id: DIDURL = DIDURL(test.subject, "profile")
        vc = store.loadCredential(test.subject, id)
        XCTAssertNotNil(vc)
        XCTAssertEqual("MyProfile", vc.alias)
        XCTAssertEquals(test.subject, vc.subject.id)
        XCTAssertEquals(id, vc.id)
        XCTAssertTrue(vc.isValid())
        
        // try with full id string
        vc = store.loadCredential(test.subject.description, id.description)
        XCTAssertNotNil(vc)
        XCTAssertEqual("MyProfile", vc.alias)
        XCTAssertEquals(test.subject, vc.subject.id)
        XCTAssertEquals(id, vc.id)
        XCTssertTrue(vc.isValid())
        
        id = DIDURL(test.subject, "twitter")
        vc = store.loadCredential(test.subject.description, "twitter")
        XCTAssertNotNil(vc)
        XCTAssertEqual("Twitter", vc.alias)
        XCTAssertEqual(test.subject, vc.subject.id)
        XCTAssertEquals(id, vc.id)
        XCTAssertTrue(vc.isValid())
        
        vc = store.loadCredential(test.subject.description, "notExist")
        XCTAssertNil(vc)
        
        id = DIDURL(test.subject, "twitter")
        XCTAssertTrue(store.containsCredential(test.subject, id))
        XCTAssertTrue(store.containsCredential(test.subject.description, "twitter"))
        XCTAssertFalse(store.containsCredential(test.subject.description, "notExist"))
    }
    
    func testListCredentials() {
        do {
            let testData: TestData = TestData()
            testData.setupStore(true)
            testData.initIdentity()
            
            // Store test data into current store
            testData.loadTestIssuer()
            let test: DIDDocument = testData.loadTestDocument()
            let vc: VerifiableCredential = testData.loadProfileCredential()
            vc.setAlias("MyProfile")
            vc = testData.loadEmailCredential()
            vc.setAlias("Email")
            vc = testData.loadTwitterCredential()
            vc.setAlias("Twitter")
            vc = testData.loadPassportCredential()
            vc.setAlias("Passport")
            let store: DIDStore = try DIDStore.shareInstance()
            
            var vcs: Array<DIDURL> = store.listCredentials(test.subject)
            XCTAssertEquals(4, vcs.count)
            for id in vcs {
                var re = id.fragment == "profile" || id.fragment == "email" || id.fragment == "twitter" || id.fragment == "passport"
                XCTAssertTrue(re)
                
                re = id.alias == "MyProfile" || id.alias == "email" || id.alias == "twitter" || id.alias == "passport"
                XCTAssertTrue(re)
            }
        } catch {
            print(error)
        }
    }
    
    func testDeleteCredential() {
        do {
            let testData: TestData = TestData()
            testData.setupStore(true)
            testData.initIdentity()
            
            // Store test data into current store
            testData.loadTestIssuer()
            let test: DIDDocument = testData.loadTestDocument()
            let vc: VerifiableCredential = testData.loadProfileCredential()
            vc.setAlias("MyProfile")
            vc = testData.loadEmailCredential()
            vc.setAlias("Email")
            vc = testData.loadTwitterCredential()
            vc.setAlias("Twitter")
            vc = testData.loadPassportCredential()
            vc.setAlias("Passport")
            
            let store = DIDStore.shareInstance()
            
            var path = storeRoot + "/" + "ids"
                + "/" + test.subject.methodSpecificId
                + "/" + "credentials" + "/" + "twitter"
                + "/" + "credential"
            XCTAssertTrue(TestData.exists(path))
            
            path = storeRoot + "/" + "ids" + "/" + test.subject.methodSpecificId + "/" + "credentials" + "/" + "twitter" + "/" + ".meta"
            XCTAssertTrue(TestData.exists(path))
            
            path = storeRoot + "/" + "ids"
                + "/" + test.subject.methodSpecificId
                + "/" + "credentials" + "/" + "passport"
                + "/" + "credential"
            XCTAssertTrue(TestData.exists(path))
            
            file = storeRoot + "/" + "ids"
                + "/" + test.subject.methodSpecificId
                + "/" + "credentials" + "/" + "passport"
                + "/" + ".meta"
            XCTAssertTrue(TestData.exists(path))
            
            var deleted: Bool = store.deleteCredential(test.subject, DIDURL(test.subject, "twitter"))
            XCTAssertTrue(deleted)
            
            deleted = store.deleteCredential(test.subject.description, "passport")
            XCTAssertTrue(deleted)
            
            deleted = store.deleteCredential(test.subject.description, "notExist")
            XCTAssertFalse(deleted)
            
            path = storeRoot + "/" + "ids"
                + "/" + test.subject.methodSpecificId
                + "/" + "credentials" + "/" + "twitter"
            XCTAssertFalse(TestData.exists(path))
            
            path = storeRoot + "/" + "ids"
                + "/" + test.subject.getMethodSpecificId
                + "/" + "credentials" + "/" + "passport"
            XCTAssertFalse(TestData.exists(path))
            
            XCTAssertTrue(store.containsCredential(test.subject.description, "email"))
            XCTAssertTrue(store.containsCredential(test.subject.description, "profile"))
            
            XCTAssertFalse(store.containsCredential(test.subject.description, "twitter"))
            XCTAssertFalse(store.containsCredential(test.subject.description, "passport"))
        } catch {
            print(error)
        }
    }
    
    func createDataForPerformanceTest() {
        do {
            let store: DIDStore = DIDStore.shareInstance()
            
            var props: Dictionary<String, String> = []
            props["name"] = "John"
            props["gender"] = "Male"
            props["nation"] = "Singapore"
            props["language"] = "English"
            props["email"] = "john@example.com"
            props["twitter"] = "@john"
            
            for i in 0..<10 {
                let alias: String = "my did " + i
                let doc: DIDDocument = store.newDid(storePass, alias)
                
                let issuer: Issuer = Issuer(doc)
                issuer.vc.types = ["BasicProfileCredential", "SelfProclaimedCredential"]
                issuer.vc.subject.properties = props
                issuer.vc.seal(storePass)
                store.storeCredential(vc)
            }
        } catch {
            print(error)
        }
    }
    
    func testStorePerformance() {
        do {
            let adapter: DIDAdapter = DummyAdapter()
            TestData.deleteFile(storeRoot)
            if (cached){
                DIDStore.creted("filesystem", storeRoot, adapter)
            }
            else {
                DIDStore.creted("filesystem", storeRoot, adapter, 0, 0)
            }
            
            let store: DIDStore = DIDStore.shareInstance()
            
            let mnemonic: String = HDKey.generateMnemonic(0)
            store.initPrivateIdentity(0, mnemonic, passphrase, storePass, true)
            
            createDataForPerformanceTest()
            let dids: Array<DID> = store.listDids(DIDStore.DID_ALL)
            XCTAssertEquals(10, dids.count)
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
        }
    }
    
    func testStoreWithCache() {
        testStorePerformance(true)
    }
    
    func testStoreWithoutCache() {
        testStorePerformance(false)
    }
}

