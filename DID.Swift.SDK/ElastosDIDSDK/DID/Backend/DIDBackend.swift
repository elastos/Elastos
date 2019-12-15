import Foundation

public class DIDBackend: NSObject {
    var adapter: DIDAdapter!
    
    public init(_ adapter: DIDAdapter){
        self.adapter = adapter
        super.init()
    }
    
    public func create(_ doc: DIDDocument, _ signKey: DIDURL, _ storepass: String) throws -> Bool {
        do {
            let request: IDChainRequest = try IDChainRequest.create(doc, signKey, storepass)
            let jsonString: String = request.toJson(true)
            
            return try adapter.createIdTransaction(jsonString, nil)
        } catch  {
            throw DIDError.failue("Create ID transaction error: \(error.localizedDescription).")
        }
    }
    
    public func resolve(_ did: DID) throws -> DIDDocument? {
        do {
            let res = try adapter.resolve(did.methodSpecificId)
            guard res != nil else {
                return nil
            }
            var jsonString = res!.replacingOccurrences(of: " ", with: "")
            jsonString = jsonString.replacingOccurrences(of: "\n", with: "")
            let ordDic = JsonHelper.handleString(jsonString) as! OrderedDictionary<String, Any>
            let result = ordDic["result"] as! Array<Any>
            
            if (result.count == 0) {
                return nil
            }
            let re = result[0] as! OrderedDictionary<String, Any>
            let request: IDChainRequest = try IDChainRequest.fromJson(re)
            if try !request.isValid() {
              throw  DIDError.failue("Signature verify failed.")
            }
            return request.doc
        } catch {
            throw DIDError.failue("Resolve DID error: \(error.localizedDescription)")
        }
    }
    
    public func update(_ doc: DIDDocument, _ signKey: DIDURL, _ storepass: String) throws -> Bool {
        do {
            let request: IDChainRequest = try IDChainRequest.update(doc, signKey, storepass)
            let jsonStr: String = request.toJson(true)
            return try adapter.createIdTransaction(jsonStr, nil)
        } catch {
            throw  DIDError.failue("Create ID transaction error. \(error.localizedDescription).")
        }
    }
    
    public func deactivate(_ did: DID, _ signKey: DIDURL, _ storepass: String) throws -> Bool {
        do {
            let request: IDChainRequest = try IDChainRequest.deactivate(did, signKey, storepass)
            let jsonStr: String = request.toJson(true)
            return try adapter.createIdTransaction(jsonStr, nil)
        } catch {
            throw  DIDError.failue("Deactivate ID transaction error: \(error.localizedDescription).")
        }
    }
}
