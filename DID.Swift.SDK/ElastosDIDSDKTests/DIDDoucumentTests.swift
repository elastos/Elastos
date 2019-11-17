

import XCTest
import ElastosDIDSDK


class DIDDoucumentTests: XCTestCase {
    
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
    
    /*
    func testSignAndVerify() {
        do {
            /*
             Util.deleteFile(new File(TestConfig.storeRoot));
             DIDStore.initialize("filesystem", TestConfig.storeRoot,
                     TestConfig.storePass, new FakeConsoleAdapter());
             store = DIDStore.getInstance();
             String mnemonic = Mnemonic.generate(Mnemonic.ENGLISH);
             store.initPrivateIdentity(mnemonic, TestConfig.passphrase,
                     TestConfig.storePass, true);

             LinkedHashMap<DID, String> ids = new LinkedHashMap<DID, String>(128);
             for (int i = 0; i < 100; i++) {
                 String hint = "my did " + i;
                 DIDDocument doc = store.newDid(TestConfig.storePass, hint);

                 File file = new File(TestConfig.storeRoot + File.separator + "ids"
                         + File.separator + doc.getSubject().getMethodSpecificId()
                         + File.separator + "document");
                 assertTrue(file.exists());
                 assertTrue(file.isFile());

                 file = new File(TestConfig.storeRoot + File.separator + "ids"
                         + File.separator + "."
                         + doc.getSubject().getMethodSpecificId() + ".meta");
                 assertTrue(file.exists());
                 assertTrue(file.isFile());

                 ids.put(doc.getSubject(), hint);
             }
             */
            
            // add ids
//            TestUtils.deleteFile(tempStoreRoot)
            DIDStore.creatInstance(<#T##type: String##String#>, location: <#T##String#>, storepass: <#T##String#>)
            let hint: String = "my first did"
            let doc: DIDDocument = try! store.newDid(storePass, hint)
            primaryDid = doc.subject
            let id: String = doc.subject!.methodSpecificId!
            let path: String = storePath + "/" + "ids" + "/" + id + "/" + "document"
            XCTAssertTrue(TestUtils.existsFile(path))
            
            let path2: String = storePath + "/" + "ids" + "/" + "." + id + ".meta"
            XCTAssertTrue(TestUtils.existsFile(path2))
            ids[doc.subject!] = hint
            
            try ids.keys.forEach { did in
                let doc: DIDDocument = try store.loadDid(did)!
                let json: String = try doc.toExternalForm(false)
                let pkid: DIDURL = try DIDURL(did, "primary")
                let inputs: [CVarArg] = [json, json.count]
                let sig: String = try doc.sign(pkid, storePass, inputs)
                var re: Bool = try doc.verify(pkid, sig, inputs)
                XCTAssertTrue(re)
            }
        } catch {
            print(error)
        }
    }
    */
    func testPerformanceExample() {
        // This is an example of a performance test case.
        self.measure {
            // Put the code you want to measure the time of here.
        }
    }

}
