
import XCTest
import ElastosDIDSDK

class DIDDoucumentTests: XCTestCase {
    
    var store: DIDStore!
    
    var compactPath: String!
    var documentPath: String!
    var normalizedPath: String!

    override func setUp() {
        super.setUp()
        
        let bundle = Bundle(for: type(of: self))
        compactPath = bundle.path(forResource: "compact", ofType: "json")!
        documentPath = bundle.path(forResource: "testdiddoc", ofType: "json")!
        normalizedPath = bundle.path(forResource: "normalized", ofType: "json")!
    }

    override func tearDown() {
        compactPath = nil
        documentPath = nil
        normalizedPath = nil
        super.tearDown()
    }
    
    func loadTestDocument() throws -> DIDDocument {
        let bundle = Bundle(for: type(of: self))
        let path = bundle.path(forResource: "testdiddoc", ofType: "json")!
        let doc = try DIDDocument.fromJson(path)
        return doc
    }
    
    func updateForTesting(_ doc: DIDDocument) throws -> DIDDocument {
       _ = doc.modify()
        var id: DIDURL
        let keyBase58: String = ""
        var success: Bool
        
        for i in 0...5 {
            id = try DIDURL(doc.subject!, "test-pk-\(i)")
            let data = Data()
            let udata = [UInt8](data)
            let keyBase58: String = Base58.base58FromBytes(udata)
            success = try doc.addPublicKey(id, doc.subject!, keyBase58)
            XCTAssertTrue(success)
        }
        
        for i in 0...5 {
            id = try DIDURL(doc.subject!, "test-auth-\(i)")
            let data = Data()
            let udata = [UInt8](data)
            let keyBase58: String = Base58.base58FromBytes(udata)
            success = try doc.addAuthenticationKey(id, keyBase58)
            XCTAssertTrue(success)
        }
        
        let controller: DID = try DID("did:elastos:ip7ntDo2metGnU8wGP4FnyKCUdbHm4BPDh")
        for i in 0...5 {
            id = try DIDURL(doc.subject!, "test-autho-\(i)")
            let data = Data()
            let udata = [UInt8](data)
            let keyBase58: String = Base58.base58FromBytes(udata)
            success = try doc.addAuthorizationKey(id, controller, keyBase58)
            XCTAssertTrue(success)
        }
        doc.readonly = true
        return doc
    }

    
    func testGetPublicKey() {
        do {
            let doc: DIDDocument = try loadTestDocument()
            XCTAssertNil(doc)
            // Count and list.
            XCTAssertEqual(4, doc.getPublicKeyCount())

            var pks = doc.getPublicKeys()
            XCTAssertEqual(4, pks.count)

            pks.forEach { pk in
                XCTAssertEqual(doc.subject!, pk.id.did)
                XCTAssertEqual(Constants.defaultPublicKeyType, pk.type)
                if (pk.id.fragment == "recovery") {
                    XCTAssertNotEqual(doc.subject!, pk.controller!)
                }
                else
                {
                    XCTAssertNotEqual(doc.subject!, pk.controller)
                }
                let re = pk.id.fragment == "default" || pk.id.fragment == "key2" || pk.id.fragment == "key3" || pk.id.fragment == "recovery"
                XCTAssertTrue(re)
            }
            
             // PublicKey getter.
            var pk: DIDPublicKey = try doc.getPublicKey("default")
            XCTAssertNotNil(pk)
            XCTAssertEqual(try DIDURL(doc.subject!, "default"),  pk.id)
            
            var id: DIDURL = try DIDURL(doc.subject!, "key3")
            pk = try doc.getPublicKey(id)!
            XCTAssertNotNil(pk)
            XCTAssertEqual(id,  pk.id)
            
            id = doc.getDefaultPublicKey()!
            XCTAssertNotNil(pk)
            XCTAssertEqual(try DIDURL(doc.subject!, "default"), id)
            
            // Key not exist, should fail.
            pk = try doc.getPublicKey("notExist")
            XCTAssertNil(pk)
            
            id = try DIDURL(doc.subject!, "notExist")
            pk = try doc.getPublicKey(id)!
            XCTAssertNil(pk)
            
            // Selector
            id = doc.getDefaultPublicKey()!
            pks = try doc.selectPublicKeys(id, Constants.defaultPublicKeyType)
            XCTAssertEqual(1, pks.count)
            XCTAssertEqual(try DIDURL(doc.subject!, "default"), pks[0].id)
            
            pks = try doc.selectPublicKeys(id, nil)
            XCTAssertEqual(1, pks.count)
            XCTAssertEqual(try DIDURL(doc.subject!, "default"), pks[0].id)

            pks = try doc.selectPublicKeys(nil, Constants.defaultPublicKeyType)
            XCTAssertEqual(4, pks.count)
            
            pks = try doc.selectPublicKeys("key2", Constants.defaultPublicKeyType)
            XCTAssertEqual(1, pks.count)
            XCTAssertEqual(try DIDURL(doc.subject!, "key2"), pks[0].id)

            pks = try doc.selectPublicKeys("key2", nil)
            XCTAssertEqual(1, pks.count)
            XCTAssertEqual(try DIDURL(doc.subject!, "key2"), pks[0].id)
        } catch {
            print(error)
        }
    }
    
    func testAddPublicKey() {
        do {
            let doc: DIDDocument = try loadTestDocument()
            XCTAssertNil(doc)
            
            let data = Data()
            let udata = [UInt8](data)
            var keyBase58: String = Base58.base58FromBytes(udata)
            
            // Read only mode, should fail.
            var id: DIDURL = try DIDURL(doc.subject!, "test0")
            var success: Bool = try doc.addPublicKey(id, doc.subject!, keyBase58)
            XCTAssertFalse(success)
            
            success = try doc.addPublicKey("test0", doc.subject!.description, keyBase58)
            XCTAssertFalse(success)
            
            _ = doc.modify()

            // Modification mode, should success.
            id = try DIDURL(doc.subject!, "test0")
            success = try doc.addPublicKey(id, doc.subject!, keyBase58)
            XCTAssertTrue(success)
            
            keyBase58 = Base58.base58FromBytes(udata)
            success = try doc.addPublicKey("test1", doc.subject!.description, keyBase58)
            XCTAssertTrue(success)
            
            var pk: DIDPublicKey = try doc.getPublicKey("test0")
            XCTAssertNotNil(pk);
            XCTAssertEqual(try DIDURL(doc.subject!, "test0"), pk.id)

            pk = try doc.getPublicKey("test1");
            XCTAssertNotNil(pk);
            XCTAssertEqual(try DIDURL(doc.subject!, "test1"), pk.id)

            // Check the final count.
            XCTAssertEqual(6, doc.getPublicKeyCount())
            
        } catch  {
        print(error)
        }
    }
    
    func testRemovePublicKey() {
        do {
            let doc: DIDDocument = try loadTestDocument()
            XCTAssertNotNil(doc)

            try _ = updateForTesting(doc)

            // Read only mode, should fail.
            var id: DIDURL = try DIDURL(doc.subject!, "test-pk-0")
            var success: Bool = try doc.removePublicKey(id, true)
            XCTAssertFalse(success)

            success = try doc.removePublicKey("test-pk-1", true)
            XCTAssertFalse(success)
            _ = doc.modify()
            
            // Modification mode, should success.
            id = try DIDURL(doc.subject!, "test-auth-0")
            success = try doc.removePublicKey(id, true)
            XCTAssertTrue(success)

            success = try doc.removePublicKey("test-pk-0")
            XCTAssertTrue(success)

            success = try doc.removePublicKey("test-autho-0", true)
            XCTAssertTrue(success)

            var pk = try doc.getPublicKey("test-auth-0")
            XCTAssertNil(pk)
            
            pk = try doc.getPublicKey("test-pk-0")
            XCTAssertNil(pk)

            pk = try doc.getPublicKey("test-autho-0")
            XCTAssertNil(pk)
            
            // PublicKey used by authentication, can not remove directly, should fail.
            id = try DIDURL(doc.subject!, "test-auth-0")
            success = try doc.removePublicKey(id)
            XCTAssertFalse(success)

            // Key not exist, should fail.
            success = try doc.removePublicKey("notExistKey", true)
            XCTAssertFalse(success)
            
            // Can not remove default publickey, should fail.
            success = try doc.removePublicKey(doc.getDefaultPublicKey()!, true)
            XCTAssertFalse(success)

            // Check the final count.
            XCTAssertEqual(16, doc.getPublicKeyCount())
            XCTAssertEqual(7, doc.getAuthenticationKeyCount())
            XCTAssertEqual(5, doc.getAuthorizationKeyCount())
        } catch {
            print(error)
        }
        
    }
    
    func testGetAuthenticationKey() {
        do {
            let doc: DIDDocument = try loadTestDocument()
            XCTAssertNil(doc)

            // Count and list.
            XCTAssertEqual(3, doc.getAuthenticationKeyCount())

            var pks: Array<DIDPublicKey> = doc.getAuthenticationKeys()
            XCTAssertEqual(3, pks.count)
            
            pks.forEach { pk in
                XCTAssertEqual(doc.subject!, pk.id.did)
                XCTAssertEqual(Constants.defaultPublicKeyType, pk.type)

                XCTAssertEqual(doc.subject!, pk.controller!)
                let re = pk.id.fragment == "default" || pk.id.fragment == "key2" || pk.id.fragment == "key3"
                XCTAssertTrue(re)
            }

            // AuthenticationKey getter
            var pk = try doc.getAuthenticationKey("default")
            XCTAssertNotNil(pk)
            XCTAssertEqual(try DIDURL(doc.subject!, "default"), pk!.id)

            var id: DIDURL = try DIDURL(doc.subject!, "key3")
            pk = doc.getAuthenticationKey(id)!
            XCTAssertNotNil(pk)
            XCTAssertEqual(id, pk!.id)
            
            // Key not exist, should fail.
            pk = try doc.getAuthenticationKey("notExist")
            XCTAssertNil(pk)

            id = try DIDURL(doc.subject!, "notExist")
            pk = doc.getAuthenticationKey(id)!
            XCTAssertNil(pk)
            
            // selector
            id = try DIDURL(doc.subject!, "key3")
            pks = try doc.selectAuthenticationKeys(id, type: Constants.defaultPublicKeyType)
            XCTAssertEqual(1, pks.count)
            XCTAssertEqual(id, pks[0].id)

            pks = try doc.selectAuthenticationKeys(id, type: nil)
            XCTAssertEqual(1, pks.count)
            XCTAssertEqual(id, pks[0].id)

            pks = try doc.selectAuthenticationKeys(nil, type: Constants.defaultPublicKeyType)
            XCTAssertEqual(3, pks.count)

            pks = try doc.selectAuthenticationKeys("key2", Constants.defaultPublicKeyType)
            XCTAssertEqual(1, pks.count)
            XCTAssertEqual(try DIDURL(doc.subject!, "key2"), pks[0].id)

            pks = try doc.selectAuthenticationKeys("key2", nil)
            XCTAssertEqual(1, pks.count)
            XCTAssertEqual(try DIDURL(doc.subject!, "key2"), pks[0].id)
        } catch {
            print(error)
        }
        
    }
   /*
    func testRemovePublicKey() {
        do {
            
        } catch {
            print(error)
        }
        
    }
 */
    
    func testAddAuthenticationKey() {
        do {
            
        } catch {
            print(error)
        }
        
    }
    /*
     @Test
     public void testAddAuthenticationKey() throws DIDException {
         DIDDocument doc = loadTestDocument();
         assertNotNull(doc);

         // Add the keys for testing.
         doc.modify();

         DIDURL id = new DIDURL(doc.getSubject(), "test1");
         byte[] keyBytes = new byte[33];
         Arrays.fill(keyBytes, (byte)1);
         String keyBase58 = Base58.encode(keyBytes);
         boolean success = doc.addPublicKey(id, doc.getSubject(), keyBase58);
         assertTrue(success);

         id = new DIDURL(doc.getSubject(), "test2");
         Arrays.fill(keyBytes, (byte)2);
         keyBase58 = Base58.encode(keyBytes);
         success = doc.addPublicKey(id, doc.getSubject(), keyBase58);
         assertTrue(success);

         doc.setReadonly(true);

         // Read only mode, shoud fail.
         id = new DIDURL(doc.getSubject(), "test1");
         success = doc.addAuthenticationKey(id);
         assertFalse(success);

         success = doc.addAuthenticationKey("test2");
         assertFalse(success);

         Arrays.fill(keyBytes, (byte)3);
         keyBase58 = Base58.encode(keyBytes);
         success = doc.addAuthenticationKey("test3", keyBase58);
         assertFalse(success);

         Arrays.fill(keyBytes, (byte)4);
         keyBase58 = Base58.encode(keyBytes);
         success = doc.addAuthenticationKey(new DIDURL(doc.getSubject(), "test4"), keyBase58);
         assertFalse(success);

         doc.modify();

         // Modification mode, should success.
         success = doc.addAuthenticationKey(new DIDURL(doc.getSubject(), "test1"));
         assertTrue(success);

         success = doc.addAuthenticationKey("test2");
         assertTrue(success);

         Arrays.fill(keyBytes, (byte)3);
         keyBase58 = Base58.encode(keyBytes);

         success = doc.addAuthenticationKey(new DIDURL(doc.getSubject(), "test3"), keyBase58);
         assertTrue(success);

         Arrays.fill(keyBytes, (byte)4);
         keyBase58 = Base58.encode(keyBytes);

         success = doc.addAuthenticationKey("test4", keyBase58);
         assertTrue(success);

         PublicKey pk = doc.getAuthenticationKey("test1");
         assertNotNull(pk);
         assertEquals(new DIDURL(doc.getSubject(), "test1"), pk.getId());

         pk = doc.getAuthenticationKey("test2");
         assertNotNull(pk);
         assertEquals(new DIDURL(doc.getSubject(), "test2"), pk.getId());

         pk = doc.getAuthenticationKey("test3");
         assertNotNull(pk);
         assertEquals(new DIDURL(doc.getSubject(), "test3"), pk.getId());

         pk = doc.getAuthenticationKey("test4");
         assertNotNull(pk);
         assertEquals(new DIDURL(doc.getSubject(), "test4"), pk.getId());

         // Try to add a non existing key, should fail.
         success = doc.addAuthenticationKey("test0");
         assertFalse(success);

         // Try to add a key not owned by self, should fail.
         success = doc.addAuthenticationKey("recovery");
         assertFalse(success);

         // Check the final count.
         assertEquals(8, doc.getPublicKeyCount());
         assertEquals(7, doc.getAuthenticationKeyCount());

     }
*/
    func testParseDocument() {
        let document: DIDDocument = try! DIDDocument.fromJson(documentPath)
        XCTAssertEqual(4, document.getPublicKeyCount())
        let pks: Array<DIDPublicKey> = document.getPublicKeys()
        pks.forEach { pk in
            let result: Bool = pk.id.fragment == "default" || pk.id.fragment == "key2" || pk.id.fragment == "keys3" || pk.id.fragment == "recovery"
            XCTAssert(result)
            print(pk.id.fragment as Any)
            if pk.id.fragment == "recovery"{
                XCTAssertNotEqual(document.subject, pk.controller)
            }
            else {
                XCTAssertEqual(document.subject, pk.controller)
            }
        }
        XCTAssertEqual(3, document.getAuthenticationKeyCount())
        XCTAssertEqual(1, document.getAuthorizationKeyCount())
        XCTAssertEqual(2, document.getCredentialCount())
        XCTAssertEqual(3, document.getServiceCount())
    }
    
    func testCompactJson() {
        
        let document: DIDDocument = try! DIDDocument.fromJson(documentPath)
        let jsonString: String = try! document.toExternalForm(true)
        let url = URL(fileURLWithPath:compactPath)
        let jsonStr = try! String(contentsOf: url)
        XCTAssertEqual(jsonString, jsonStr)
    }
    
    func  testNormalizedJson() {
        
        let document: DIDDocument = try! DIDDocument.fromJson(documentPath)
        let str: String = try! document.toExternalForm(false)
        let url = URL(fileURLWithPath: normalizedPath)
        let jsonStr = try! String(contentsOf: url)
        XCTAssertEqual(str, jsonStr)
    }
    
    func testSignAndVerify() {
        do {
            // add ids
            try DIDStore.creatInstance("filesystem", storePath, FakeConsoleAdaptor())
            store = try DIDStore.shareInstance()
            let mnemonic: String = HDKey.generateMnemonic(0)
            try store.initPrivateIdentity(0, mnemonic, passphrase, storePass, true)

            var ids: Dictionary<DID, String> = [: ]
            for i in 0..<100 {
                let hint: String = "my did \(i)"
                let doc: DIDDocument = try! store.newDid(storePass, hint)
                let id: String = doc.subject!.methodSpecificId!
                let path: String = storePath + "/" + "ids" + "/" + id + "/" + "document"
                XCTAssertTrue(TestUtils.existsFile(path))
                
                let path2: String = storePath + "/" + "ids" + "/" + "." + id + ".meta"
                XCTAssertTrue(TestUtils.existsFile(path2))
                ids[doc.subject!] = hint
            }
        
            try ids.keys.forEach { did in
                let doc: DIDDocument = try store.loadDid(did)!
                var json: String = try doc.toExternalForm(false)
                let pkid: DIDURL = try DIDURL(did, "primary")
                var inputs: [CVarArg] = [json, json.count]
                var count: Int = inputs.count / 2
                var sig: String = try doc.sign(pkid, storePass, count, inputs)
                var re: Bool = try doc.verify(pkid, sig, count, inputs)
                XCTAssertTrue(re)
                
                sig = try doc.sign(storePass, count,inputs)
                re = try doc.verify(sig, count,inputs)
                XCTAssertTrue(re)
                
                json = String(json.suffix(json.count - 1))
                inputs = [json, json.count]
                count = inputs.count / 2
                re = try doc.verify(pkid, sig, count, inputs)
                XCTAssertFalse(re)
                re = try doc.verify(sig, count,inputs)
                XCTAssertFalse(re)
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
