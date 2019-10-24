import Foundation

class DIDBackend: NSObject {
    
    private static var instance: DIDBackend!
    private static var adaptor: DIDAdaptor!

    private init(_ adaptor: DIDAdaptor){
        DIDBackend.adaptor = adaptor
        super.init()
       }
    
    public static func createInstance(_ adaptor: DIDAdaptor) {
        if instance == nil {
            let didInstance = DIDBackend(adaptor)
            DIDBackend.adaptor = adaptor
            instance = didInstance
        }
    }
    
    public static func sharedInstance() throws -> DIDBackend {
        guard instance != nil else {
            throw DIDError.failue("Please call createInstance first.")
        }
        return instance
    }

    public class func create(_ doc: DIDDocument, _ signKey: DIDURL, _ passphrase: String) throws -> Bool {
        do {
            let request: IDChainRequest = try IDChainRequest(IDChainRequest.Operation.CREATE, doc)
            let jsonString: String = try request.sign(signKey, passphrase).toJson(true)
            
            return try adaptor.createIdTransaction(jsonString, nil)
        } catch  {
           throw DIDError.failue("Create ID transaction error: \(error.localizedDescription).")
        }
    }

    public class func update(_ doc: DIDDocument, _ signKey: DIDURL, _ passphrase: String) throws -> Bool {
        do {
            let request: IDChainRequest = try IDChainRequest(IDChainRequest.Operation.UPDATE, doc)
            let jsonStr: String = try request.sign(signKey, passphrase).toJson(true)
            return try adaptor.createIdTransaction(jsonStr, nil)
        } catch {
            throw  DIDError.failue("Update ID transaction error: \(error.localizedDescription).")
        }
    }

    public class func deactivate(_ did: DID, _ signKey: DIDURL, _ passphrase: String) throws -> Bool {
        do {
            let request: IDChainRequest = try IDChainRequest(IDChainRequest.Operation.CREATE, did)
            let jsonStr: String = try request.sign(signKey, passphrase).toJson(true)
            return try adaptor.createIdTransaction(jsonStr, nil)
        } catch {
            throw  DIDError.failue("Deactivate ID transaction error: \(error.localizedDescription).")
        }
    }

    public class func resolve(_ did: DID) throws -> DIDDocument? {
        do {
            let docJson = try adaptor.resolve(did.toExternalForm())
            guard docJson != nil else {
                return nil
            }
            let doc: DIDDocument = try DIDDocument.fromJson(docJson!)
            return doc
        } catch {
           throw DIDError.failue("Resolve DID error: \(error.localizedDescription)")
        }
    }
}
