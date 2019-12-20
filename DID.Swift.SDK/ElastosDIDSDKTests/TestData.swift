
import XCTest
import ElastosDIDSDK

class TestData: XCTestCase {
    private static var dummyAdapter: DIDAdapter?
    private static var spvAdapter: DIDAdapter?
    private static var rootKey: HDKey?
    private static var index: Int?
    
    private var testIssuer: DIDDocument?
    private var issuerCompactJson: String?
    private var issuerNormalizedJson: String?
    private var testDocument: DIDDocument?
    private var testCompactJson: String?
    private var testNormalizedJson: String?
    private var profileVc: VerifiableCredential?
    private var profileVcCompactJson: String?
    private var profileVcNormalizedJson: String?
    private var emailVc: VerifiableCredential?
    private var emailVcCompactJson: String?
    private var emailVcNormalizedJson: String?
    private var passportVc: VerifiableCredential?
    private var passportVcCompactJson: String?
    private var passportVcNormalizedJson: String?
    private var twitterVc: VerifiableCredential?
    private var twitterVcCompactJson: String?
    private var twitterVcNormalizedJson: String?
    private var testVp: VerifiablePresentation?
    private var testVpNormalizedJson: String?
    private var restoreMnemonic: String?

    public func setupStore(_ dummyBackend: Bool) throws {
        var adapter: DIDAdapter = DummyAdapter()
        if dummyBackend {
            if TestData.dummyAdapter == nil {
                TestData.dummyAdapter = DummyAdapter() as? DIDAdapter
                adapter = TestData.dummyAdapter!
            }
        }
        else {
            if TestData.spvAdapter == nil {
                let cblock: PasswordCallback = ({(walletDir, walletId) -> String in return "test111111"})
                TestData.spvAdapter = (SPVAdaptor(walletDir, walletId, networkConfig, resolver, cblock) as? DIDAdapter)
            }
            deleteFile(storePath)
            adapter = TestData.spvAdapter!
        }
        try DIDStore.creatInstance("filesystem", storePath, adapter)
    }
    
    public func initIdentity() throws -> String {
        let mnemonic: String = HDKey.generateMnemonic(0)
        try DIDStore.shareInstance()?.initPrivateIdentity(0, mnemonic, passphrase, storePass, true)
        return mnemonic
    }
    
    func loadDIDDocument(_ fileName: String, _ type_: String) throws -> DIDDocument {
        let bundle = Bundle(for: type(of: self))
        let jsonPath = bundle.path(forResource: fileName, ofType: type_)
        let doc: DIDDocument = try DIDDocument.fromJson(path: jsonPath!)
        
        if DIDStore.isInitialized() {
            let store: DIDStore = try DIDStore.shareInstance()!
            try store.storeDid(doc)
        }
        return doc
    }
    
    func importPrivateKey(_ id: DIDURL, _ fileName: String, _ type: String) throws {
        let skBase58: String = try loadText(fileName, type)
        try DIDStore.shareInstance()?.storePrivateKey(id.did, id, skBase58, storePass)
    }
    
    func loadTestIssuer() throws -> DIDDocument {
        if testIssuer == nil {
            testIssuer = try loadDIDDocument("issuer", "json")
            try importPrivateKey((testIssuer?.getDefaultPublicKey())!, "issuer.primary", "sk")
        }
        return testIssuer!
    }
    
    func loadTestDocument() throws -> DIDDocument {
        _ = try loadTestIssuer()
        if testDocument == nil {
            testDocument = try loadDIDDocument("document", "json")
        }
        try importPrivateKey((testDocument?.getDefaultPublicKey())!, "document.primary", "sk")
        try importPrivateKey(testDocument!.getPublicKey("key2")!.id, "document.key2", "sk")
        try importPrivateKey(testDocument!.getPublicKey("key3")!.id, "document.key3", "sk")
        return testDocument!
    }
    
    func loadCredential(_ fileName: String, _ type_: String) throws -> VerifiableCredential {
        let buldle = Bundle(for: type(of: self))
        let filepath = buldle.path(forResource: fileName, ofType: type_)
        let json = try! String(contentsOf: URL(fileURLWithPath: filepath!), encoding: .utf8)
        let vc: VerifiableCredential = try VerifiableCredential.fromJson(json)
        if DIDStore.isInitialized() {
            try DIDStore.shareInstance()?.storeCredential(vc)
        }
        return vc
    }
    
    public func loadProfileCredential() throws -> VerifiableCredential? {
        if profileVc == nil {
            profileVc = try loadCredential("vc-profile", "json")
        }
        return profileVc
    }
    
    public func loadEmailCredential() throws -> VerifiableCredential {
        if emailVc == nil {
            emailVc = try loadCredential("vc-email", "json")
        }
        return emailVc!
    }
    
    public func loadPassportCredential() throws -> VerifiableCredential? {
        if passportVc == nil {
            passportVc = try loadCredential("vc-passport", "json")
        }
        return passportVc
    }
    
    public func loadTwitterCredential() throws -> VerifiableCredential {
        if twitterVc == nil {
            twitterVc = try loadCredential("vc-twitter", "json")
        }
        return twitterVc!
    }
    
    public func loadPresentation() throws -> VerifiablePresentation {
        if testVp == nil {
            let bl = Bundle(for: type(of: self))
            let jsonstr = bl.path(forResource: "vp", ofType: "json")
            testVp = try VerifiablePresentation.fromJson(jsonstr!)
        }
        return testVp!
    }
    
    func loadText(_ fileName: String, _ type_: String) throws -> String {
        let bl = Bundle(for: type(of: self))
        let filepath = bl.path(forResource: fileName, ofType: type_)
        let json = try! String(contentsOf: URL(fileURLWithPath: filepath!), encoding: .utf8)

        return json
    }
    
    public func loadIssuerCompactJson() throws -> String {
        if issuerCompactJson == nil {
            issuerCompactJson = try loadText("issuer.compact", "json")
        }
        return issuerCompactJson!
    }
    
    public func loadIssuerNormalizedJson() throws -> String {
        if issuerNormalizedJson == nil {
            issuerNormalizedJson = try loadText("issuer.normalized", "json")
        }
        return issuerNormalizedJson!
    }
    
    public func loadTestCompactJson() throws -> String {
        if testCompactJson == nil {
            testCompactJson = try loadText("document.compact", "json")
        }
        return testCompactJson!
    }
    
    public func loadTestNormalizedJson() throws -> String {
        if testNormalizedJson == nil {
            testNormalizedJson = try loadText("document.normalized", "json")
        }
        return testNormalizedJson!
    }
    
    public func loadProfileVcCompactJson() throws -> String {
        if profileVcCompactJson == nil {
            profileVcCompactJson = try loadText("vc-profile.compact", "json")
        }
        return profileVcCompactJson!
    }
    
    public func loadProfileVcNormalizedJson() throws -> String {
        if profileVcNormalizedJson == nil {
            profileVcNormalizedJson = try loadText("vc-profile.normalized", "json")
        }
        return profileVcNormalizedJson!
    }
    
    public func loadEmailVcCompactJson() throws -> String {
        if emailVcCompactJson == nil {
            emailVcCompactJson = try loadText("vc-email.compact", "json")
        }
        return emailVcCompactJson!
    }
    
    public func loadEmailVcNormalizedJson() throws -> String {
        if emailVcNormalizedJson == nil {
            emailVcNormalizedJson = try loadText("vc-email.normalized", "json")
        }
        return emailVcNormalizedJson!
    }
    
    public func loadPassportVcCompactJson() throws -> String {
        if passportVcCompactJson == nil {
            passportVcCompactJson = try loadText("vc-passport.compact", "json")
        }
        return passportVcCompactJson!
    }
    
    public func loadPassportVcNormalizedJson() throws -> String {
        if passportVcNormalizedJson == nil {
            passportVcNormalizedJson = try loadText("vc-passport.normalized", "json")
        }
        return passportVcNormalizedJson!
    }
    
    public func loadTwitterVcCompactJson() throws -> String {
        if twitterVcCompactJson == nil {
            twitterVcCompactJson = try loadText("vc-twitter.compact", "json")
        }
        return twitterVcCompactJson!
    }
    
    public func loadTwitterVcNormalizedJson() throws -> String {
        if twitterVcNormalizedJson == nil {
            twitterVcNormalizedJson = try loadText("vc-twitter.normalized", "json")
        }
        return twitterVcNormalizedJson!
    }
    
    public func loadPresentationNormalizedJson() throws -> String {
        if testVpNormalizedJson == nil {
            testVpNormalizedJson = try loadText("vp.normalized", "json")
        }
        return testVpNormalizedJson!
    }
    
    public func loadRestoreMnemonic() throws -> String {
        if restoreMnemonic == nil {
            // TODO: load test
            restoreMnemonic = try loadText("mnemonic", "restore")
        }
        return restoreMnemonic!
    }
    
    public func deleteFile() throws -> String {
        if restoreMnemonic == nil {
            // TODO: load test
            restoreMnemonic = try loadText("mnemonic", "restore")
        }
        return restoreMnemonic!
    }
    
    public class func generateKeypair() throws -> DerivedKey {
        if TestData.rootKey == nil {
            let mnemonic: String = HDKey.generateMnemonic(0)
            TestData.rootKey = try HDKey.fromMnemonic(mnemonic, "")
            TestData.index = 0
        }
        TestData.index = TestData.index! + 1
        return try TestData.rootKey!.derive(TestData.index!)
    }

    func deleteFile(_ path: String) {
        do {
            let filemanager: FileManager = FileManager.default
            var isdir = ObjCBool.init(false)
            let fileExists = filemanager.fileExists(atPath: path, isDirectory: &isdir)
            if fileExists && isdir.boolValue {
                if let dircontents = filemanager.enumerator(atPath: path) {
                    for case let url as URL in dircontents {
                        deleteFile(url.absoluteString)
                    }
                }
            }
            guard fileExists else {
                return
            }
            try filemanager.removeItem(atPath: path)
        } catch {
            print("deleteFile error: \(error)")
        }
    }
    
    func exists(_ dirPath: String) -> Bool {
        let fileManager = FileManager.default
        var isDir : ObjCBool = false
        if fileManager.fileExists(atPath: dirPath, isDirectory:&isDir) {
            if isDir.boolValue {
                return true
            }
        }
        return false
    }
    
    func existsFile(_ path: String) -> Bool {
        var isDirectory = ObjCBool.init(false)
        let fileExists = FileManager.default.fileExists(atPath: path, isDirectory: &isDirectory)
        return !isDirectory.boolValue && fileExists
    }
    
    func currentDateToWantDate(_ year: Int)-> Date {
        let current = Date()
        var calendar = Calendar(identifier: .gregorian)
        calendar.timeZone = TimeZone(abbreviation: "UTC")!
        var comps:DateComponents?
        
        comps = calendar.dateComponents([.year, .month, .day, .hour, .minute, .second], from: current)
        comps?.year = Constants.MAX_VALID_YEARS
        comps?.month = 0
        comps?.day = 0
        comps?.hour = 0
        comps?.minute = 0
        comps?.second = 0
        comps?.nanosecond = 0
        let realDate = calendar.date(byAdding: comps!, to: current) ?? Date()
        let hour = calendar.component(.hour, from: realDate)
        let useDate = calendar.date(bySettingHour: hour, minute: 00, second: 00, of: realDate) ?? Date()
        
        return useDate
    }
}
