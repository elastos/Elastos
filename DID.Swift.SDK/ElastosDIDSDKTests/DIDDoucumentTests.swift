

import XCTest
import ElastosDIDSDK


class DIDDoucumentTests: XCTestCase {

    override func setUp() {
        // Put setup code here. This method is called before the invocation of each test method in the class.
    }

    override func tearDown() {
        // Put teardown code here. This method is called after the invocation of each test method in the class.
    }

    func testParseDocument() {
        let document: DIDDocument = try! DIDDocument.fromJson("/Users/liaihong/Desktop/testdiddoc.json")
        XCTAssertEqual(3, document.getPublicKeyCount())
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
        let document: DIDDocument = try! DIDDocument.fromJson("/Users/liaihong/Desktop/testdiddoc.json")
        let jsonString: String = try! document.toExternalForm(true)
        let url = URL(fileURLWithPath: "/Users/liaihong/Desktop/compact.json")
        let jsonStr = try! String(contentsOf: url)
//        let data = jsonStr.data(using: .utf8)
//        let json = try JSONSerialization.jsonObject(with: data, options: .allowFragments) as? [String: Any]

        XCTAssertEqual(jsonString, jsonStr)
    }
    
    func  testNormalizedJson() {
        let document: DIDDocument = try! DIDDocument.fromJson("/Users/liaihong/Desktop/testdiddoc.json")
    }
    
    /*
     public void testNormalizedJson() throws DIDException, IOException {
         Reader input = new InputStreamReader(getClass()
                 .getClassLoader().getResourceAsStream("testdiddoc.json"));
         DIDDocument doc = DIDDocument.fromJson(input);
         input.close();

         String json = doc.toExternalForm(false);

         File file = new File(getClass().getClassLoader().getResource("normalized.json").getFile());
         char[] chars = new char[(int)file.length()];
         input = new InputStreamReader(new FileInputStream(file));
         input.read(chars);
         input.close();

         String expected = new String(chars);

         assertEquals(expected, json);
     }
   */
    func testPerformanceExample() {
        // This is an example of a performance test case.
        self.measure {
            // Put the code you want to measure the time of here.
        }
    }

}
