
import XCTest
@testable import ElastosDIDSDK
import SPVWrapper

class TestDataGenerator: XCTestCase {
    private var outputDir: String = ""
    private var adapter: DIDAdapter!
    private var issuer: DIDDocument!
    private var test: DIDDocument!
    private var store: DIDStore!
    
    func create(_ storeRoot: String) throws -> String {
        let cblock: PasswordCallback = ({(walletDir, walletId) -> String in return walletPassword})
        adapter = SPVAdaptor(walletDir, walletId, networkConfig, resolver, cblock)
        //        TestUtils.deleteFile(storeRoot)
        store = try DIDStore.open(atPath: storeRoot, withType: "filesystem", adapter: adapter)
        try DIDBackend.initializeInstance(resolver, TestData.getResolverCacheDir())

        let mnemonic: String = try Mnemonic.generate(Mnemonic.ENGLISH)
        try store.initializePrivateIdentity(using: Mnemonic.ENGLISH, mnemonic: mnemonic, passPhrase: passphrase, storePassword: storePass, true)
        outputDir = tempDir + "/" + "DIDTestFiles"
        
        return mnemonic
    }
    
    func createTestIssuer() throws {
        let doc: DIDDocument = try store.newDid(using: storePass)
        print("Generate issuer DID: \(doc.subject)...")
//        let selfIssuer = try Issuer(doc)
        let selfIssuer = try VerifiableCredentialIssuer(doc)
        var props: Dictionary<String, String> = [: ]
        props["name"] = "Test Issuer"
        props["nation"] = "Singapore"
        props["language"] = "English"
        props["email"] = "issuer@example.com"
        
        let cb = selfIssuer.editingVerifiableCredentialFor(did: doc.subject)
        let vc = try cb.withId("profile")
            .withTypes("BasicProfileCredential","SelfProclaimedCredential")
            .withProperties(props)
            .sealed(using: storePass)

        let db: DIDDocumentBuilder = doc.editing()
        _ = try db.appendCredential(with: vc)
        issuer = try db.sealed(using: storePass)
        try store.storeDid(using: issuer)
        try store.storeCredential(using: vc, with: "Profile")
        
        let id: DIDURL = issuer.defaultPublicKey
        let sk: String = try store.loadPrivateKey(for: issuer.subject, byId: id)
        let data: Data = try DIDStore.decryptFromBase64(sk, storePass)
        let binSk: [UInt8] = [UInt8](data)
        writeTo("issuer." + id.fragment! + ".sk", Base58.base58FromBytes(binSk))
        
        var json = issuer.toString(true)
        writeTo("issuer.normalized.json", json)
        
        json = try formatJson(json)
        writeTo("issuer.json", json);
        
        json = issuer.toString()
        writeTo("issuer.compact.json", json);
        print(issuer.isValid)
    }
    
    func createTestDocument() throws {
        let doc = try store.newDid(using: storePass)
        
        // Test document with two embedded credentials
        print("Generate test DID: \(doc.subject)...")
        let db: DIDDocumentBuilder = doc.editing()
        var temp = try TestData.generateKeypair()
        _ = try db.appendAuthenticationKey(with: "key2", keyBase58: temp.getPublicKeyBase58())
        //            writeTo("document.key2.sk", Base58.encode(temp.serialize()))
        
        temp = try TestData.generateKeypair()
        _ = try db.appendAuthenticationKey(with: "key3", keyBase58: temp.getPublicKeyBase58())
        //            writeTo("document.key3.sk", Base58.encode(temp.serialize()))
        
        temp = try TestData.generateKeypair()
        let controller  = HDKey.DerivedKey.getAddress(temp.getPublicKeyBytes())
        
        _ = try db.appendAuthorizationKey(with: "recovery", controller: DID(DID.METHOD, controller), keyBase58: temp.getPublicKeyBase58())
        _ = try db.appendService(with: "openid", type: "OpenIdConnectVersion1.0Service", endpoint: "https://openid.example.com/")
        _ = try db.appendService(with: "vcr", type: "CredentialRepositoryService", endpoint: "https://did.example.com/credentials")
        _ = try db.appendService(with: "carrier", type: "CarrierAddress", endpoint: "carrier://X2tDd1ZTErwnHNot8pTdhp7C7Y9FxMPGD8ppiasUT4UsHH2BpF1d")
        
        let selfIssuer = try VerifiableCredentialIssuer(doc)
        var props: Dictionary<String, String> = [: ]
        props["name"] = "John"
        props["gender"] = "Male"
        props["nation"] = "Singapore"
        props["language"] = "English"
        props["email"] = "john@example.com"
        props["twitter"] = "@john"
        var cb = selfIssuer.editingVerifiableCredentialFor(did: doc.subject)
        
        let vcProfile = try cb.withId("profile")
            .withTypes("BasicProfileCredential", "SelfProclaimedCredential")
            .withProperties(props)
            .sealed(using: storePass)
        
        let kycIssuer = try VerifiableCredentialIssuer(issuer)
        cb = kycIssuer.editingVerifiableCredentialFor(did: doc.subject)
        props = [: ]
        props["email"] = "john@example.com"
        let vcEmail: VerifiableCredential = try cb.withId("email")
            .withTypes("BasicProfileCredential", "InternetAccountCredential", "EmailCredential")
            .withProperties(props)
            .sealed(using: storePass)

        _ = try db.appendCredential(with: vcProfile)
        _ = try db.appendCredential(with: vcEmail)

        test = try db.sealed(using: storePass)
        try store.storeDid(using: test)
        
        var id = test.defaultPublicKey
        let sk = try store.loadPrivateKey(for: test.subject, byId: id)
        let data: Data = try DIDStore.decryptFromBase64(sk, storePass)
        let binSk = [UInt8](data)
        writeTo("document." + id.fragment! + ".sk", Base58.base58FromBytes(binSk))
        
        var json = test.toString(true)
        writeTo("document.normalized.json", json)
        
        json = try formatJson(json)
        writeTo("document.json", json);
        
        json = test.toString()
        writeTo("document.compact.json", json);
        print(test.isValid)
        
        // Profile credential
        print("Generate credential:  \(vcProfile.getId())...")
        json = vcProfile.toString(true)
        writeTo("vc-profile.normalized.json", json);
        
        json = try formatJson(json)
        writeTo("vc-profile.json", json);
        
        json = vcProfile.toString()
        writeTo("vc-profile.compact.json", json);
        print(vcProfile.isValid)
        
        // email credential
        print("Generate credential:  \(vcEmail.getId())...")
        json = vcEmail.toString(true)
        writeTo("vc-email.normalized.json", json);
        
        json = try formatJson(json)
        writeTo("vc-email.json", json);
        
        json = vcEmail.toString()
        writeTo("vc-email.compact.json", json);
        print(vcEmail.isValid)
        
        // Passport credential
        id = try DIDURL(test.subject, "passport")
        print("Generate credential:  \(id)...")

        cb = selfIssuer.editingVerifiableCredentialFor(did: doc.subject)
        props = [: ]
        props["nation"] = "Singapore"
        props["passport"] = "S653258Z07"

        let vcPassport = try cb.withId("passport")
            .withTypes("BasicProfileCredential", "SelfProclaimedCredential")
            .withProperties(props)
            .sealed(using: storePass)
        
        json = vcPassport.toString(true)
        writeTo("vc-passport.normalized.json", json)
        
        json = try formatJson(json)
        writeTo("vc-passport.json", json)
        
        json = vcPassport.toString()
        writeTo("vc-passport.compact.json", json)
        print(vcPassport.isValid)
        
        // Twitter credential
        id = try DIDURL(test.subject, "twitter")
        print("Generate credential:  \(id)...")

        cb = kycIssuer.editingVerifiableCredentialFor(did: doc.subject)
        props = [: ]
        props["twitter"] = "@john"
        let vcTwitter = try cb.withId("twitter")
            .withTypes("InternetAccountCredential", "TwitterCredential")
            .withProperties(props)
            .sealed(using: storePass)
        
        json = vcTwitter.toString(true)
        writeTo("vc-twitter.normalized.json", json)
        
        json = try formatJson(json)
        writeTo("vc-twitter.json", json)
        
        json = vcTwitter.toString()
        writeTo("vc-twitter.compact.json", json)
        
        print(vcPassport.isValid)
        //
        print("OK")
        
        // Json format credential
        id = try DIDURL(test.subject, "json")
        print("Generate credential: \(id)...")
        
        cb = kycIssuer.editingVerifiableCredentialFor(did: doc.subject)
        let jsonProps: String = "{\"name\":\"Jay Holtslander\",\"alternateName\":\"Jason Holtslander\",\"booleanValue\":true,\"numberValue\":1234,\"doubleValue\":9.5,\"nationality\":\"Canadian\",\"birthPlace\":{\"type\":\"Place\",\"address\":{\"type\":\"PostalAddress\",\"addressLocality\":\"Vancouver\",\"addressRegion\":\"BC\",\"addressCountry\":\"Canada\"}},\"affiliation\":[{\"type\":\"Organization\",\"name\":\"Futurpreneur\",\"sameAs\":[\"https://twitter.com/futurpreneur\",\"https://www.facebook.com/futurpreneur/\",\"https://www.linkedin.com/company-beta/100369/\",\"https://www.youtube.com/user/CYBF\"]}],\"alumniOf\":[{\"type\":\"CollegeOrUniversity\",\"name\":\"Vancouver Film School\",\"sameAs\":\"https://en.wikipedia.org/wiki/Vancouver_Film_School\",\"year\":2000},{\"type\":\"CollegeOrUniversity\",\"name\":\"CodeCore Bootcamp\"}],\"gender\":\"Male\",\"Description\":\"Technologist\",\"disambiguatingDescription\":\"Co-founder of CodeCore Bootcamp\",\"jobTitle\":\"Technical Director\",\"worksFor\":[{\"type\":\"Organization\",\"name\":\"Skunkworks Creative Group Inc.\",\"sameAs\":[\"https://twitter.com/skunkworks_ca\",\"https://www.facebook.com/skunkworks.ca\",\"https://www.linkedin.com/company/skunkworks-creative-group-inc-\",\"https://plus.google.com/+SkunkworksCa\"]}],\"url\":\"https://jay.holtslander.ca\",\"image\":\"https://s.gravatar.com/avatar/961997eb7fd5c22b3e12fb3c8ca14e11?s=512&r=g\",\"address\":{\"type\":\"PostalAddress\",\"addressLocality\":\"Vancouver\",\"addressRegion\":\"BC\",\"addressCountry\":\"Canada\"},\"sameAs\":[\"https://twitter.com/j_holtslander\",\"https://pinterest.com/j_holtslander\",\"https://instagram.com/j_holtslander\",\"https://www.facebook.com/jay.holtslander\",\"https://ca.linkedin.com/in/holtslander/en\",\"https://plus.google.com/+JayHoltslander\",\"https://www.youtube.com/user/jasonh1234\",\"https://github.com/JayHoltslander\",\"https://profiles.wordpress.org/jasonh1234\",\"https://angel.co/j_holtslander\",\"https://www.foursquare.com/user/184843\",\"https://jholtslander.yelp.ca\",\"https://codepen.io/j_holtslander/\",\"https://stackoverflow.com/users/751570/jay\",\"https://dribbble.com/j_holtslander\",\"http://jasonh1234.deviantart.com/\",\"https://www.behance.net/j_holtslander\",\"https://www.flickr.com/people/jasonh1234/\",\"https://medium.com/@j_holtslander\"]}"
        let vcJson = try cb.withId(id)
            .withTypes("InternetAccountCredential", "TwitterCredential")
            .withProperties(jsonProps)
            .sealed(using: storePass)

        try store.storeCredential(using: vcTwitter, with: "json")
        json = vcJson.toString(true)
        writeTo("vc-json.normalized.json", json)
        
        json = try formatJson(json)
        writeTo("vc-json.json", json)

        json = vcJson.toString()
        writeTo("vc-json.compact.json", json)
        
        print("OK")

        // Presentation with above credentials
        print("Generate presentation...")
        
        let pb = try VerifiablePresentation.editingVerifiablePresentation(for: test.subject, using: store)
        let vp = try pb.withCredentials(vcProfile, vcEmail, vcPassport, vcTwitter)
            .withRealm("https://example.com/")
            .withNonce("873172f58701a9ee686f0630204fee59")
            .sealed(using: storePass)

        json = vp.description
        writeTo("vp.normalized.json", json)
        
        json = try formatJson(json)
        writeTo("vp.json", json)
        
        print(vcPassport.isValid)
    }
    
    func writeTo(_ fileName: String, _ content: String) {
        let path = outputDir + "/" + fileName
        let fileManager = FileManager.default
        fileManager.createFile(atPath: path, contents:nil, attributes:nil)
        let handle = FileHandle(forWritingAtPath:path)
        handle?.write(content.data(using: String.Encoding.utf8)!)
    }
    
    func formatJson(_ json: String) throws -> String {
        return json
    }
    
    func testcreateTestFiles() throws {
        do {
            let path = tempDir + "/" + "DIDTestFiles" + "/" + "teststore"
            _ = try create(path)
            try createTestIssuer()
            try createTestDocument()
        }
        catch {
            XCTFail()
        }
    }
    
    func testcreateTestDidsForRestore() throws {
        do {
            print("Generate mnemonic for restore...")
            let mnemonic = try create(storeRoot)
            writeTo("mnemonic.restore", mnemonic)
            print("OK")
            var dids: String = ""
            print("Generate DIDs for restore......")
            let ad = adapter as! SPVAdaptor
            for _ in 0..<5 {
                print("******** Waiting for wallet available")
                while true {
                    if try ad.isAvailable() {
                        print("OK")
                        break
                    }
                    else {
                        print(".")
                    }
                    wait(interval: 30)
                }
                
                var doc = try store.newDid(using: storePass)
                let selfIssuer: VerifiableCredentialIssuer = try VerifiableCredentialIssuer(doc)
                
                let did: DID = doc.subject
                var properties: [String: String] = ["name": "John", "nation": "Singapore", "language": "English", "email": "john@example.com"]
                
                var cb = selfIssuer.editingVerifiableCredentialFor(did: did)
                let vcProfile: VerifiableCredential = try cb.withId("profile")
                    .withTypes("BasicProfileCredential", "SelfProclaimedCredential")
                    .withProperties(properties)
                    .sealed(using: storePass)
                
                cb = selfIssuer.editingVerifiableCredentialFor(did: doc.subject)
                properties.removeAll()
                properties = ["email": "john@gmail.com"]
                
                let vcEmail: VerifiableCredential = try cb.withId("email")
                    .withTypes("BasicProfileCredential", "InternetAccountCredential", "EmailCredential")
                    .withProperties(properties)
                    .sealed(using: storePass)
                
                cb = selfIssuer.editingVerifiableCredentialFor(did: doc.subject)
                properties.removeAll()
                properties = ["nation": "Singapore", "passport": "S653258Z07"]
                
                let vcPassport: VerifiableCredential = try cb.withId("passport")
                    .withTypes("BasicProfileCredential", "SelfProclaimedCredential")
                    .withProperties(properties)
                    .sealed(using: storePass)
                
                cb = selfIssuer.editingVerifiableCredentialFor(did: doc.subject)
                properties.removeAll()
                properties = ["twitter": "@john"]
                
                let vcTwitter: VerifiableCredential = try cb.withId("twitter")
                    .withTypes("InternetAccountCredential", "TwitterCredential")
                    .withProperties(properties)
                    .sealed(using: storePass)
                
                let db: DIDDocumentBuilder = doc.editing()
                doc = try db.appendCredential(with: vcProfile)
                    .appendCredential(with: vcEmail)
                    .appendCredential(with: vcPassport)
                    .appendCredential(with: vcTwitter)
                    .sealed(using: storePass)
                try store.storeDid(using: doc)
                
                print("******** Publishing DID:\(doc.subject)")
                _ = try store.publishDid(for: doc.subject, using: storePass)
                
                while true {
                    wait(interval: 30)
                    do {
                        let d = try doc.subject.resolve(true)
                        try store.storeDid(using: d)
                        print(" OK")
                        break
                    } catch {
                        print(".")
                    }
                }
                dids = "\(dids)\(doc.subject)\n"
            }
            writeTo("dids.restore", dids)
            print("Generate DIDs for restore......OK")
        }
        catch {
            XCTFail()
        }
    }
    
    func wait(interval: Double) {
        
        let lock = XCTestExpectation(description: "")
        DispatchQueue.main.asyncAfter(deadline: DispatchTime.now() + interval) {
            lock.fulfill()
        }
        wait(for: [lock], timeout: interval + 10)
    }
}
