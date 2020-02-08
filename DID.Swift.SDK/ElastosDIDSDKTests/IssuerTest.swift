
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
            
            XCTAssertEqual("John", vc.subject.getProperty("name") as! String)
            XCTAssertEqual("Male", vc.subject.getProperty("gender") as! String)
            XCTAssertEqual("Singapore", vc.subject.getProperty("nation") as! String)
            XCTAssertEqual("English", vc.subject.getProperty("language") as! String)
            XCTAssertEqual("john@example.com", vc.subject.getProperty("email") as! String)
            XCTAssertEqual("@john", vc.subject.getProperty("twitter") as! String)
            
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
            
            XCTAssertEqual("Testing Issuer", vc.subject.getProperty("name") as! String)
            XCTAssertEqual("Singapore", vc.subject.getProperty("nation") as! String)
            XCTAssertEqual("English", vc.subject.getProperty("language") as! String)
            XCTAssertEqual("issuer@example.com", vc.subject.getProperty("email") as! String)
            
            XCTAssertFalse(try vc.isExpired())
            XCTAssertTrue(try vc.isGenuine())
            XCTAssertTrue(try vc.isValid())
        }
        catch {
            XCTFail()
        }
    }
    
    func testIssueJsonPropsCredential() {
        do {
            let testData = TestData()
            _ = try testData.setupStore(true)
            let issuerDoc = try testData.loadTestIssuer()
            let props: String = "{\"name\":\"Jay Holtslander\",\"alternateName\":\"Jason Holtslander\",\"booleanValue\":true,\"numberValue\":1234,\"doubleValue\":9.5,\"nationality\":\"Canadian\",\"birthPlace\":{\"type\":\"Place\",\"address\":{\"type\":\"PostalAddress\",\"addressLocality\":\"Vancouver\",\"addressRegion\":\"BC\",\"addressCountry\":\"Canada\"}},\"affiliation\":[{\"type\":\"Organization\",\"name\":\"Futurpreneur\",\"sameAs\":[\"https://twitter.com/futurpreneur\",\"https://www.facebook.com/futurpreneur/\",\"https://www.linkedin.com/company-beta/100369/\",\"https://www.youtube.com/user/CYBF\"]}],\"alumniOf\":[{\"type\":\"CollegeOrUniversity\",\"name\":\"Vancouver Film School\",\"sameAs\":\"https://en.wikipedia.org/wiki/Vancouver_Film_School\",\"year\":2000},{\"type\":\"CollegeOrUniversity\",\"name\":\"CodeCore Bootcamp\"}],\"gender\":\"Male\",\"Description\":\"Technologist\",\"disambiguatingDescription\":\"Co-founder of CodeCore Bootcamp\",\"jobTitle\":\"Technical Director\",\"worksFor\":[{\"type\":\"Organization\",\"name\":\"Skunkworks Creative Group Inc.\",\"sameAs\":[\"https://twitter.com/skunkworks_ca\",\"https://www.facebook.com/skunkworks.ca\",\"https://www.linkedin.com/company/skunkworks-creative-group-inc-\",\"https://plus.google.com/+SkunkworksCa\"]}],\"url\":\"https://jay.holtslander.ca\",\"image\":\"https://s.gravatar.com/avatar/961997eb7fd5c22b3e12fb3c8ca14e11?s=512&r=g\",\"address\":{\"type\":\"PostalAddress\",\"addressLocality\":\"Vancouver\",\"addressRegion\":\"BC\",\"addressCountry\":\"Canada\"},\"sameAs\":[\"https://twitter.com/j_holtslander\",\"https://pinterest.com/j_holtslander\",\"https://instagram.com/j_holtslander\",\"https://www.facebook.com/jay.holtslander\",\"https://ca.linkedin.com/in/holtslander/en\",\"https://plus.google.com/+JayHoltslander\",\"https://www.youtube.com/user/jasonh1234\",\"https://github.com/JayHoltslander\",\"https://profiles.wordpress.org/jasonh1234\",\"https://angel.co/j_holtslander\",\"https://www.foursquare.com/user/184843\",\"https://jholtslander.yelp.ca\",\"https://codepen.io/j_holtslander/\",\"https://stackoverflow.com/users/751570/jay\",\"https://dribbble.com/j_holtslander\",\"http://jasonh1234.deviantart.com/\",\"https://www.behance.net/j_holtslander\",\"https://www.flickr.com/people/jasonh1234/\",\"https://medium.com/@j_holtslander\"]}"
            let issuer = try Issuer(issuerDoc)
            let cb = issuer.issueFor(did: issuerDoc.subject!)
            let vc = try cb.idString("myCredential")
                .types(["BasicProfileCredential", "SelfProclaimedCredential"])
                .properties(properties: props)
                .seal(storepass: storePass)
            let vcId = try DIDURL(issuerDoc.subject!, "myCredential")
            XCTAssertEqual(vcId, vc.id)
            XCTAssertTrue(vc.types.contains("BasicProfileCredential"))
            XCTAssertTrue(vc.types.contains("SelfProclaimedCredential"))
            XCTAssertFalse(vc.types.contains("InternetAccountCredential"))
            XCTAssertEqual(issuerDoc.subject, vc.issuer)
            XCTAssertEqual(issuerDoc.subject, vc.subject.id)

            XCTAssertEqual("Technologist", vc.subject.getProperty("Description") as! String)
            XCTAssertEqual("Jason Holtslander", vc.subject.getProperty("alternateName") as! String)
            XCTAssertEqual(1234, vc.subject.getProperty("numberValue") as! Int)
            XCTAssertEqual(9.5, vc.subject.getProperty("doubleValue") as! Float)

            XCTAssertNotNil(vc.subject.properties)
            XCTAssertFalse(try vc.isExpired())
            XCTAssertTrue(try vc.isGenuine())
            XCTAssertTrue(try vc.isValid())
        } catch {
            print(error)
            XCTFail()
        }
    }
}
