
import XCTest
import ElastosDIDSDK

class IssuerTest: XCTestCase {
    
    func newIssuerTestWithSignKey() {
        do {
            let testData: TestData = TestData()
            try testData.setupStore(true)
            
            let issuerDoc: DIDDocument = try testData.loadTestIssuer()
            
            let signKey: DIDURL = issuerDoc.getDefaultPublicKey()
            
            let issuer: Issuer = try Issuer(issuerDoc.subject!, signKey)
            
            XCTAssertEqual(issuer.getDid(), issuer.getDid())
            XCTAssertEqual(signKey, issuer.signKey)
        } catch {
            
        }
    }
    
    func newIssuerTestWithoutSignKey() {
        do {
            let testData: TestData = TestData()
            try testData.setupStore(true)
            
            let issuerDoc: DIDDocument = try testData.loadTestIssuer()
            
            let issuer: Issuer = try Issuer(issuerDoc.subject!)
            
            XCTAssertEqual(issuerDoc.subject, issuer.getDid())
            XCTAssertEqual(issuerDoc.getDefaultPublicKey(), issuer.signKey)
            
        } catch  {
        }
    }
    
    func newIssuerTestWithInvalidKey() {
        do {
            let testData: TestData = TestData()
            try testData.setupStore(true)
            
            var issuerDoc: DIDDocument = try testData.loadTestIssuer()
            
            let key: DerivedKey = try TestData.generateKeypair()
            let signKey: DIDURL = try DIDURL(issuerDoc.subject!, "testKey")
            try issuerDoc.addAuthenticationKey(signKey, try key.getPublicKeyBase58())
            
            issuerDoc = try issuerDoc.seal(storePass)
            XCTAssertTrue(try issuerDoc.isValid())
            
            let issuer: Issuer = try Issuer(issuerDoc, signKey)
            
            // Dead code.
            XCTAssertEqual(issuer.getDid(), issuer.getDid())
        } catch {
            
        }
    }
    
    func newIssuerTestWithInvalidKey2() {
        do {
            let testData: TestData = TestData()
            try testData.setupStore(true)
            
            let issuerDoc: DIDDocument = try testData.loadTestIssuer()
            let signKey: DIDURL = try DIDURL(issuerDoc.subject!, "recovery")
            let issuer: Issuer = try Issuer(issuerDoc, signKey)
            
            // Dead code.
            XCTAssertEqual(issuer.getDid(), issuer.getDid())
        }
        catch {  
        }
    }
    
    func IssueKycCredentialTest() {
        do {
            let testData: TestData = TestData()
            try testData.setupStore(true)
            
            let issuerDoc: DIDDocument = try testData.loadTestIssuer()
            let testDoc: DIDDocument = try testData.loadTestDocument()
            
            var props: OrderedDictionary<String, String> = OrderedDictionary()
            props["name"] = "John"
            props["gender"] = "Male"
            props["nation"] = "Singapore"
            props["language"] = "English"
            props["email"] = "john@example.com"
            props["twitter"] = "@john"
            
            let issuer: Issuer =  try Issuer(issuerDoc)
            let vc: VerifiableCredential = issuer.seal("testCredential", ["BasicProfileCredential", "InternetAccountCredential"], props, storePass)
            
            let vcId: DIDURL = DIDURL(testDoc.subject, "testCredential")
            
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
            
            XCTAssertFalse(vc.isExpired())
            XCTAssertTrue(vc.isGenuine())
            XCTAssertTrue(vc.isValid())
        }
        catch {
            
        }
    }
    
    func IssueSelfProclaimedCredentialTest() {
        do {
            let testData: TestData = TestData()
            try testData.setupStore(true)
            
            let issuerDoc: DIDDocument = try testData.loadTestIssuer()
            
            var props: Dictionary<String, String> = [:]
            props["name"] = "Testing Issuer"
            props["nation"] = "Singapore"
            props["language"] = "English"
            props["email"] = "issuer@example.com"
            let issuer: Issuer =  try Issuer(issuerDoc)
            let vc: VerifiableCredential = issuer.seal("myCredential", ["BasicProfileCredential", "SelfProclaimedCredential"], props, storePass)
            
            let vcId: DIDURL = DIDURL(testDoc.subject, "myCredential")
            XCTAssertEqual(vcId, vc.id)
            XCTAssertTrue(vc.types.contains("BasicProfileCredential"))
            XCTAssertTrue(vc.types.contains("SelfProclaimedCredential"))
            XCTAssertFalse(vc.types.contains("InternetAccountCredential"))
            
            XCTAssertEqual(issuerDoc.subject, vc.issuer)
            XCTAssertEqual(testDoc.subject, vc.subject.id)
            
            XCTAssertEqual("Testing Issuer", vc.subject.getProperty("name"))
            XCTAssertEqual("Singapore", vc.subject.getProperty("nation"))
            XCTAssertEqual("English", vc.subject.getProperty("language"))
            XCTAssertEqual("issuer@example.com", vc.subject.getProperty("email"))
            
            XCTAssertFalse(vc.isExpired())
            XCTAssertTrue(vc.isGenuine())
            XCTAssertTrue(vc.isValid())
        }
        catch {
            
        }
    }
    
}
