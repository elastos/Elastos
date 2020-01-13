
import XCTest
import ElastosDIDSDK
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
            store = try DIDStore.open("filesystem", storeRoot)
            DIDBackend.creatInstance(adapter)
            TestData.deleteFile(storeRoot)
            store = try DIDStore.open("filesystem", storeRoot)
            
            let mnemonic: String = HDKey.generateMnemonic(0)
            try store!.initPrivateIdentity(0, mnemonic, passphrase, storePass, true)
            outputDir = tempDir + "/" + "DIDTestFiles"
            
            return mnemonic
    }
    
    func createTestIssuer() throws {
            let doc: DIDDocument = try store.newDid(storePass)
            print("Generate issuer DID: \(doc.subject!)...")
            
            let selfIssuer = try Issuer(doc)
            var props: OrderedDictionary<String, String> = OrderedDictionary()
            props["name"] = "Test Issuer"
            props["nation"] = "Singapore"
            props["language"] = "English"
            props["email"] = "issuer@example.com"
            
            let vc: VerifiableCredential = try selfIssuer.seal(for: doc.subject!,"profile", ["BasicProfileCredential", "SelfProclaimedCredential"], props, storePass)
            _ = doc.addCredential(vc)
            issuer = try doc.seal(store, storePass)
            try store.storeDid(issuer)
            
            
            let id: DIDURL = issuer.getDefaultPublicKey()
            let sk: String = try store.loadPrivateKey(issuer.subject!, id: id)
            let data: Data = try store.decryptFromBase64(storePass, sk)
            let binSk: [UInt8] = [UInt8](data)
            writeTo("issuer." + id.fragment + ".sk", Base58.base58FromBytes(binSk))
            
            var json = try issuer.description(true)
            writeTo("issuer.normalized.json", json);
            
            json = try formatJson(json)
            writeTo("issuer.json", json);
            
            json = try issuer.description(false)
            writeTo("issuer.compact.json", json);
            print(try issuer.isValid())
    }
    
    
    func createTestDocument() throws {
            let doc = try store.newDid(storePass)
            
            // Test document with two embedded credentials
            print("Generate test DID: \(doc.subject!)...")
            
            var temp = try TestData.generateKeypair()
            _ = try doc.addAuthenticationKey("key2", temp.getPublicKeyBase58())
//            writeTo("document.key2.sk", Base58.encode(temp.serialize()))
            
            temp = try TestData.generateKeypair()
            _ = try doc.addAuthenticationKey("key3", temp.getPublicKeyBase58())
//            writeTo("document.key3.sk", Base58.encode(temp.serialize()))
            
            temp = try TestData.generateKeypair()
            let controller = DerivedKey.getIdString(try temp.getPublicKeyBytes())
            _ = try doc.addAuthorizationKey("recovery", controller, temp.getPublicKeyBase58())
            
            _ = try doc.addService("openid", "OpenIdConnectVersion1.0Service", "https://openid.example.com/");
            _ = try doc.addService("vcr", "CredentialRepositoryService", "https://did.example.com/credentials");
            _ = try doc.addService("carrier", "CarrierAddress", "carrier://X2tDd1ZTErwnHNot8pTdhp7C7Y9FxMPGD8ppiasUT4UsHH2BpF1d");
            
            let selfIssuer = try Issuer(doc)
            var props: OrderedDictionary<String, String> = OrderedDictionary()
            props["name"] = "John"
            props["gender"] = "Male"
            props["nation"] = "Singapore"
            props["language"] = "English"
            props["email"] = "john@example.com"
            props["twitter"] = "@john"
            let vcProfile: VerifiableCredential = try selfIssuer.seal(for: doc.subject!, "profile", ["BasicProfileCredential", "SelfProclaimedCredential"], props, storePass)
            
            _ = try Issuer(issuer)
            
            props = OrderedDictionary()
            props["email"] = "john@example.com"
            let vcEmail: VerifiableCredential = try selfIssuer.seal(for: doc.subject!, "email", ["BasicProfileCredential", "InternetAccountCredential", "EmailCredential"], props, storePass)
            _ = doc.addCredential(vcProfile)
            _ = doc.addCredential(vcEmail)
            test = try doc.seal(store, storePass)
            try store.storeDid(test)
            
            var id = test.getDefaultPublicKey()
            let sk = try store.loadPrivateKey(test.subject!, id: id)
            let data: Data = try store.decryptFromBase64(storePass, sk)
            let binSk = [UInt8](data)
            writeTo("document." + id.fragment + ".sk", Base58.base58FromBytes(binSk))
            
            var json = try test.description(true)
            writeTo("document.normalized.json", json)
            
            json = try formatJson(json)
            writeTo("document.json", json);
            
            json = try test.description(false)
            writeTo("document.compact.json", json);
            print(try test.isValid())
            
            // Profile credential
            print("Generate credential:  \(vcProfile.id!)...")
            json = vcProfile.description(true)
            writeTo("vc-profile.normalized.json", json);
            
            json = try formatJson(json)
            writeTo("vc-profile.json", json);
            
            json = vcProfile.description(false)
            writeTo("vc-profile.compact.json", json);
            print(try vcProfile.isValid())
            
            // email credential
            print("Generate credential:  \(vcEmail.id!)...")
            json = vcEmail.description(true)
            writeTo("vc-email.normalized.json", json);
            
            json = try formatJson(json)
            writeTo("vc-email.json", json);
            
            json = vcEmail.description(false)
            writeTo("vc-email.compact.json", json);
            print(try vcEmail.isValid())
            
            // Passport credential
            id = try DIDURL(test.subject!, "passport")
            print("Generate credential:  \(id)...")
            props = OrderedDictionary()
            props["nation"] = "Singapore"
            props["passport"] = "S653258Z07"
            
            let vcPassport = try selfIssuer.seal(for: doc.subject!, "passport", ["BasicProfileCredential", "SelfProclaimedCredential"], props, storePass)
            
            json = vcPassport.description(true)
            writeTo("vc-passport.normalized.json", json)
            
            json = try formatJson(json)
            writeTo("vc-passport.json", json)
            
            json = vcPassport.description(false)
            writeTo("vc-passport.compact.json", json)
            print(try vcPassport.isValid())
            
            // Twitter credential
            id = try DIDURL(test.subject!, "twitter")
            print("Generate credential:  \(id)...")
            
            props = OrderedDictionary()
            props["twitter"] = "@john"
            let vcTwitter =  try selfIssuer.seal(for: doc.subject!, "twitter", ["InternetAccountCredential", "TwitterCredential"], props, storePass)
            
            json = vcTwitter.description(true)
            writeTo("vc-twitter.normalized.json", json)
            
            json = try formatJson(json)
            writeTo("vc-twitter.json", json)
            
            json = vcTwitter.description(false)
            writeTo("vc-twitter.compact.json", json)
            
        print(try vcPassport.isValid())
        
        // Presentation with above credentials
        print("Generate presentation...")
        
        let vp: VerifiablePresentation = try VerifiablePresentation.seal(for: test.subject!, store, [vcProfile, vcEmail, vcPassport, vcTwitter], "https://example.com/", "873172f58701a9ee686f0630204fee59", storePass)
        json = vp.description
        writeTo("vp.normalized.json", json)
        
        json = try formatJson(json)
        writeTo("vp.json", json)
        
        print(try vcPassport.isValid())
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
                        print("...")
                    }
                    wait(interval: 30)
                }
                
                let doc = try store.newDid(storePass)
                let selfIssuer: Issuer = try Issuer(doc)
                
                let did: DID = doc.subject!
                let credential: VerifiableCredential = VerifiableCredential()
                credential.id = try DIDURL(did, "profile")
                credential.types = ["BasicProfileCredential", "SelfProclaimedCredential"]
                
                let cs: CredentialSubject = CredentialSubject(did)
                cs.addProperty("name", "John")
                cs.addProperty("nation", "Singapore")
                cs.addProperty("language", "English")
                cs.addProperty("email", "john@example.com")
                
                let vcProfile: VerifiableCredential = try selfIssuer.seal(for: did, "profile", credential.types, cs.properties, storePass)
                
                cs.properties.removeAll(keepCapacity: 0)
                cs.addProperty("email", "john@gmail.com")
                credential.types.removeAll()
                credential.types = ["BasicProfileCredential", "InternetAccountCredential", "EmailCredential"]
                
                let vcEmail: VerifiableCredential = try selfIssuer.seal(for: did, "email", credential.types, cs.properties, storePass)
                
                cs.properties.removeAll(keepCapacity: 0)
                cs.addProperty("nation", "Singapore")
                cs.addProperty("passport", "S653258Z07")
                
                credential.types.removeAll()
                credential.types = ["BasicProfileCredential", "SelfProclaimedCredential"]
                
                let vcPassport: VerifiableCredential = try selfIssuer.seal(for: did, "passport", credential.types, cs.properties, storePass)
                
                cs.properties.removeAll(keepCapacity: 0)
                cs.addProperty("twitter", "@john")
                
                credential.types.removeAll()
                credential.types = ["InternetAccountCredential", "TwitterCredential"]
                
                let vcTwitter: VerifiableCredential = try selfIssuer.seal(for: did, "twitter", credential.types, cs.properties, storePass)
                
                _ = doc.addCredential(vcProfile)
                _ = doc.addCredential(vcEmail)
                _ = doc.addCredential(vcPassport)
                _ = doc.addCredential(vcTwitter)
                _ = try doc.seal(store, storePass)
                try store.storeDid(doc)
                
                print("******** Publishing DID:\(doc.subject!)")
                _ = try store.publishDid(doc.subject!, storePass)
                
                while true {
                    wait(interval: 30)
                    let d = try doc.subject?.resolve(true)
                    if d != nil {
                        try store.storeDid(d!)
                        print(" OK")
                        break
                    }
                    else {
                        print("...")
                    }
                }
                dids = "\(dids)\(doc.subject!)\n"
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
