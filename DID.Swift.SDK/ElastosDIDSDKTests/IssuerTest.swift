

import XCTest

class IssuerTest: XCTestCase {

    func newIssuerTestWithSignKey() {
        let testData: TestData = TestData()
        testData.setupStore(true)

        let issuerDoc:DIDDocument = testData.loadTestIssuer()

        let signKey: DIDURL = issuerDoc.getDefaultPublicKey()

        let issuer: Issuer = Issuer(issuerDoc.subject, signKey)

        XCTAssertEqual(issuer.did, issuer.did)
        XCTAssertEqual(signKey, issuer.signKey)
    }
    func newIssuerTestWithoutSignKey() {
        let testData: TestData = TestData()
        testData.setupStore(true)

        let issuerDoc: DIDDocument = testData.loadTestIssuer()

        let issuer: Issuer = Issuer(issuerDoc.subject)

        XCTAssertEqual(issuerDoc.subject, issuer.did)
        XCTAssertEqual(issuerDoc.getDefaultPublicKey(), issuer.signKey)
    }
    
    func newIssuerTestWithInvalidKey() {

        let testData: TestData = TestData()
        testData.setupStore(true)

        let issuerDoc: DIDDocument = testData.loadTestIssuer()

        let key: DerivedKey = TestData.generateKeypair()
        let signKey: DIDURL = DIDURL(issuerDoc.getSubject(), "testKey")
        issuerDoc.addAuthenticationKey(signKey, key.getPublicKeyBase58())

        issuerDoc = issuerDoc.seal(storePass)
        XCTAssertTrue(issuerDoc.isValid())

        let issuer: Issuer = Issuer(issuerDoc, signKey)

        // Dead code.
        XCTAssertEqual(issuer.did, issuer.did)
    }
    
    func newIssuerTestWithInvalidKey2() {
        let testData: TestData = TestData()
        testData.setupStore(true)

        let issuerDoc: DIDDocument = testData.loadTestIssuer()
        let signKey: DIDURL = DIDURL(issuerDoc.subject, "recovery")
        let issuer: Issuer = Issuer(issuerDoc, signKey)

        // Dead code.
        XCTAssertEqual(issuer.did, issuer.did)
    }
    
    func IssueKycCredentialTest() {
        let testData: TestData = TestData()
        testData.setupStore(true)

        let issuerDoc: DIDDocument = testData.loadTestIssuer()
        let testDoc: DIDDocument = testData.loadTestDocument()

        var props: Dictionary<String, String> = []
        props["name"] = "John"
        props["gender"] = "Male"
        props["nation"] = "Singapore"
        props["language"] = "English"
        props["email"] = "john@example.com"
        props["twitter"] = "@john"

        let issuer: Issuer =  Issuer(issuerDoc)
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
  
    func IssueSelfProclaimedCredentialTest() {
        let testData: TestData = TestData()
        testData.setupStore(true)

        let issuerDoc: DIDDocument = testData.loadTestIssuer()
        
        var props: Dictionary<String, String> = []
        props["name"] = "Testing Issuer"
        props["nation"] = "Singapore"
        props["language"] = "English"
        props["email"] = "issuer@example.com"
        let issuer: Issuer =  Issuer(issuerDoc)
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
}
