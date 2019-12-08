import Foundation

public class DIDBackend: NSObject {
    
    private static var instance: DIDBackend!
    private var adaptor: DIDAdaptor!
    
    public init(_ adaptor: DIDAdaptor){
        self.adaptor = adaptor
        super.init()
    }
    
    public func create(_ doc: DIDDocument, _ signKey: DIDURL, _ passphrase: String) throws -> Bool {
        do {
            let request: IDChainRequest = try IDChainRequest(IDChainRequest.Operation.CREATE, doc)
            let jsonString: String = try request.sign(signKey, passphrase).toJson(true)
            
            return try adaptor.createIdTransaction(jsonString, nil)
        } catch  {
            throw DIDError.failue("Create ID transaction error: \(error.localizedDescription).")
        }
    }
    
    public func resolve(_ did: DID) throws -> DIDDocument? {
        do {
            let docJson = try adaptor.resolve(did.toExternalForm())
            guard docJson != nil else {
                return nil
            }
            var jsonString = docJson!.replacingOccurrences(of: " ", with: "")
            jsonString = jsonString.replacingOccurrences(of: "\n", with: "")
            let ordDic = JsonHelper.handleString(jsonString) as! OrderedDictionary<String, Any>
            let result = ordDic["result"] as! Array<Any>
            
            if (result.count == 0) {
                /*
                 JsonNode error = node.get("error");
                 throw new DIDResolveException("Resolve DID error("
                 + error.get("code").longValue() + ": "
                 + error.get("message").textValue());
                 */
                //            return nil
            }
            let re = result[0] as! OrderedDictionary<String, Any>
            let request: IDChainRequest = try IDChainRequest.fromJson(re)
            
            return request.doc
        } catch {
            throw DIDError.failue("Resolve DID error: \(error.localizedDescription)")
        }
    }
    
    public func update(_ doc: DIDDocument, _ signKey: DIDURL, _ passphrase: String) throws -> Bool {
        do {
            let request: IDChainRequest = try IDChainRequest(IDChainRequest.Operation.UPDATE, doc)
            let jsonStr: String = try request.sign(signKey, passphrase).toJson(true)
            return try adaptor.createIdTransaction(jsonStr, nil)
        } catch {
            throw  DIDError.failue("Update ID transaction error: \(error.localizedDescription).")
        }
    }
    
    public func deactivate(_ did: DID, _ signKey: DIDURL, _ passphrase: String) throws -> Bool {
        do {
            let request: IDChainRequest = try IDChainRequest(IDChainRequest.Operation.CREATE, did)
            let jsonStr: String = try request.sign(signKey, passphrase).toJson(true)
            return try adaptor.createIdTransaction(jsonStr, nil)
        } catch {
            throw  DIDError.failue("Deactivate ID transaction error: \(error.localizedDescription).")
        }
    }

}
