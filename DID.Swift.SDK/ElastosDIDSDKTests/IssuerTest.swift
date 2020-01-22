
import XCTest
import ElastosDIDSDK

class IssuerTest: XCTestCase {
    
    func testnewIssuerTestWithSignKey() {
        do {
            let testData: TestData = TestData()
            let store = try testData.setupStore(true)
            
            let issuerDoc: DIDDocument = try testData.loadTestIssuer()
            let signKey: DIDURL = issuerDoc.getDefaultPublicKey()
            let issuer: Issuer = try Issuer(issuerDoc.subject!, signKey: signKey, store)
            XCTAssertEqual(issuerDoc.subject, issuer.getDid())
            XCTAssertEqual(signKey, issuer.signKey)
        } catch {
            XCTFail()
        }
    }
    
    func testnewIssuerTestWithoutSignKey() {
        do {
            let testData: TestData = TestData()
            let store = try testData.setupStore(true)
            
            let issuerDoc: DIDDocument = try testData.loadTestIssuer()
            let issuer: Issuer = try Issuer(issuerDoc.subject!, store)
            XCTAssertEqual(issuerDoc.subject, issuer.getDid())
            XCTAssertEqual(issuerDoc.getDefaultPublicKey(), issuer.signKey)
        } catch  {
            XCTFail()
        }
    }
    
    func testnewIssuerTestWithInvalidKey() {
        do {
            let testData: TestData = TestData()
            let store = try testData.setupStore(true)
            var issuerDoc: DIDDocument = try testData.loadTestIssuer()
            
            let key: DerivedKey = try TestData.generateKeypair()
            let signKey: DIDURL = try DIDURL(issuerDoc.subject!, "testKey")
            let db: DIDDocumentBuilder = issuerDoc.edit()
            _ = try db.addAuthenticationKey(signKey, try key.getPublicKeyBase58())
            
            issuerDoc = try db.seal(storepass: storePass)
            XCTAssertTrue(try issuerDoc.isValid())
            
            let issuer: Issuer = try Issuer(issuerDoc, signKey: signKey)
            XCTAssertEqual(issuer.getDid(), issuer.getDid())
        } catch {
            if error is DIDError {
                let err = error as! DIDError
                switch err {
                case  DIDError.failue("No private key."):
                    XCTAssertTrue(true)
                default:
                    XCTFail()
                }
            }
        }
    }
    
    func testnewIssuerTestWithInvalidKey2() {
        do {
            let testData: TestData = TestData()
            _ = try testData.setupStore(true)
            
            let issuerDoc: DIDDocument = try testData.loadTestIssuer()
            let signKey: DIDURL = try DIDURL(issuerDoc.subject!, "recovery")
            let issuer: Issuer = try Issuer(issuerDoc, signKey: signKey)
            XCTAssertEqual(issuer.getDid(), issuer.getDid())
        }
        catch {
            if error is DIDError {
                let err = error as! DIDError
                switch err {
                case  DIDError.failue("Invalid sign key id."):
                    XCTAssertTrue(true)
                default:
                    XCTFail()
                }
            }
        }
    }
    
    func testIssueKycCredentialTest() {
        do {
            let testData: TestData = TestData()
            _ = try testData.setupStore(true)
            
            let issuerDoc: DIDDocument = try testData.loadTestIssuer()
            let testDoc: DIDDocument = try testData.loadTestDocument()
            
            var props: Dictionary<String, String> = [: ]
            props["name"] = "John"
            props["gender"] = "Male"
            props["nation"] = "Singapore"
            props["language"] = "English"
            props["email"] = "john@example.com"
            props["twitter"] = "@john"
            
            let issuer: Issuer =  try Issuer(issuerDoc)
            let cb: CredentialBuilder = issuer.issueFor(did: testDoc.subject!)
            let vc: VerifiableCredential = try cb.idString("testCredential")
                .types(["BasicProfileCredential", "InternetAccountCredential"])
                .properties(props)
                .seal(storepass: storePass)
            let vcId: DIDURL = try DIDURL(testDoc.subject!, "testCredential")

            XCTAssertEqual(vcId, vc.id)
            XCTAssertTrue(vc.types.contains("BasicProfileCredential"))
            XCTAssertTrue(vc.types.contains("InternetAccountCredential"))
            XCTAssertFalse(vc.types.contains("SelfProclaimedCredential"))
            
            XCTAssertEqual(issuerDoc.subject, vc.issuer)
            XCTAssertEqual(testDoc.subject, vc.subject.id)
            
            XCTAssertEqual("John", vc.subject.getProperty("name"))
            XCTAssertEqual("Male", vc.subject.getProperty("gender"))
            XCTAssertEqual("Singapore", vc.subject.getProperty("nation"))
            XCTAssertEqual("English", vc.subject.getProperty("language"))
            XCTAssertEqual("john@example.com", vc.subject.getProperty("email"))
            XCTAssertEqual("@john", vc.subject.getProperty("twitter"))
            
            XCTAssertFalse(try vc.isExpired())
            XCTAssertTrue(try vc.isGenuine())
            XCTAssertTrue(try vc.isValid())
        }
        catch {
            XCTFail()
        }
    }
    
    func testIssueSelfProclaimedCredentialTest() {
        do {
            let testData: TestData = TestData()
            _ = try testData.setupStore(true)
            
            let issuerDoc: DIDDocument = try testData.loadTestIssuer()
            
            var props: Dictionary<String, String> = [: ]
            props["name"] = "Testing Issuer"
            props["nation"] = "Singapore"
            props["language"] = "English"
            props["email"] = "issuer@example.com"
            let issuer: Issuer =  try Issuer(issuerDoc)
            let cb: CredentialBuilder = issuer.issueFor(did: issuerDoc.subject!)
            let vc: VerifiableCredential = try cb.idString("myCredential")
                .types(["BasicProfileCredential", "SelfProclaimedCredential"])
                .properties(props)
                .seal(storepass: storePass)
            
            let vcId: DIDURL = try DIDURL(issuerDoc.subject!, "myCredential")
            XCTAssertEqual(vcId, vc.id)
            XCTAssertTrue(vc.types.contains("BasicProfileCredential"))
            XCTAssertTrue(vc.types.contains("SelfProclaimedCredential"))
            XCTAssertFalse(vc.types.contains("InternetAccountCredential"))
            
            XCTAssertEqual(issuerDoc.subject, vc.issuer)
            XCTAssertEqual(issuerDoc.subject, vc.subject.id)
            
            XCTAssertEqual("Testing Issuer", vc.subject.getProperty("name"))
            XCTAssertEqual("Singapore", vc.subject.getProperty("nation"))
            XCTAssertEqual("English", vc.subject.getProperty("language"))
            XCTAssertEqual("issuer@example.com", vc.subject.getProperty("email"))
            
            XCTAssertFalse(try vc.isExpired())
            XCTAssertTrue(try vc.isGenuine())
            XCTAssertTrue(try vc.isValid())
        }
        catch {
            XCTFail()
        }
    }
    
}
