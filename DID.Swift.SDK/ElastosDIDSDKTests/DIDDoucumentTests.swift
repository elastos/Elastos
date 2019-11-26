
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
            try DIDStore.creatInstance("filesystem", location: storePath, storepass: storePass, FakeConsoleAdaptor())
            store = try DIDStore.shareInstance()
            let mnemonic: String = HDKey.generateMnemonic(0)
            try store.initPrivateIdentity(mnemonic, passphrase, storePass, true)

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
