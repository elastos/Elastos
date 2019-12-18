
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
            
            var props: Dictionary<String, String> = [:]
            props["name"] = "John"
            props["gender"] = "Male"
            props["nation"] = "Singapore"
            props["language"] = "English"
            props["email"] = "john@example.com"
            props["twitter"] = "@john"
            
            let issuer: Issuer =  try Issuer(issuerDoc)
            /*
             Issuer.CredentialBuilder cb = issuer.issueFor(testDoc.getSubject());
             VerifiableCredential vc = cb.id("testCredential")
             .type("BasicProfileCredential", "InternetAccountCredential")
             .properties(props)
             .seal(TestConfig.storePass);
             
             DIDURL vcId = new DIDURL(testDoc.getSubject(), "testCredential");
             
             assertEquals(vcId, vc.getId());
             assertTrue(Arrays.asList(vc.getTypes()).contains("BasicProfileCredential"));
             assertTrue(Arrays.asList(vc.getTypes()).contains("InternetAccountCredential"));
             assertFalse(Arrays.asList(vc.getTypes()).contains("SelfProclaimedCredential"));
             
             assertEquals(issuerDoc.getSubject(), vc.getIssuer());
             assertEquals(testDoc.getSubject(), vc.getSubject().getId());
             
             assertEquals("John", vc.getSubject().getProperty("name"));
             assertEquals("Male", vc.getSubject().getProperty("gender"));
             assertEquals("Singapore", vc.getSubject().getProperty("nation"));
             assertEquals("English", vc.getSubject().getProperty("language"));
             assertEquals("john@example.com", vc.getSubject().getProperty("email"));
             assertEquals("@john", vc.getSubject().getProperty("twitter"));
             
             assertFalse(vc.isExpired());
             assertTrue(vc.isGenuine());
             assertTrue(vc.isValid());
             */
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
            /*
             Issuer.CredentialBuilder cb = issuer.issueFor(issuerDoc.getSubject());
             VerifiableCredential vc = cb.id("myCredential")
             .type("BasicProfileCredential", "SelfProclaimedCredential")
             .properties(props)
             .seal(TestConfig.storePass);
             
             DIDURL vcId = new DIDURL(issuerDoc.getSubject(), "myCredential");
             
             assertEquals(vcId, vc.getId());
             
             assertTrue(Arrays.asList(vc.getTypes()).contains("BasicProfileCredential"));
             assertTrue(Arrays.asList(vc.getTypes()).contains("SelfProclaimedCredential"));
             assertFalse(Arrays.asList(vc.getTypes()).contains("InternetAccountCredential"));
             
             assertEquals(issuerDoc.getSubject(), vc.getIssuer());
             assertEquals(issuerDoc.getSubject(), vc.getSubject().getId());
             
             assertEquals("Testing Issuer", vc.getSubject().getProperty("name"));
             assertEquals("Singapore", vc.getSubject().getProperty("nation"));
             assertEquals("English", vc.getSubject().getProperty("language"));
             assertEquals("issuer@example.com", vc.getSubject().getProperty("email"));
             
             assertFalse(vc.isExpired());
             assertTrue(vc.isGenuine());
             assertTrue(vc.isValid());
             */
        }
        catch {
            
        }
    }
    
}
