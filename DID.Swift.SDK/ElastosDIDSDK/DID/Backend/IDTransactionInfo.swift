
import Foundation

public class IDTransactionInfo {
    private static let TXID: String = "txid"
    private static let TIMESTAMP: String = "timestamp"
    private static let OPERATION: String = "operation"
    public var transactionId: String!
    public var timestamp: Date!
    public var request: IDChainRequest!
    
    public init(_ txid: String, _ timestamp: Date, _ request: IDChainRequest) {
        self.transactionId = txid;
        self.timestamp = timestamp;
        self.request = request;
    }
    
    public func getDid() -> DID? {
        return request.did
    }
    
    public func getOperation() -> IDChainRequest.Operation {
        return request.operation
    }
    
    public func getPayload() -> String {
        return request.toJson(false)
    }
    
    public func toJson() throws -> OrderedDictionary<String, Any> {
        var dic: OrderedDictionary<String, Any> = OrderedDictionary()
        dic[IDTransactionInfo.TXID] = transactionId
        dic[IDTransactionInfo.TIMESTAMP] = DateFormater.format(timestamp)
        let jsonstr = request.toJson(false)
        let json = JsonHelper.handleString(jsonstr) as! OrderedDictionary<String, Any>
        dic[IDTransactionInfo.OPERATION] = json

        return dic
    }
    
    public class func fromJson(_ json: OrderedDictionary<String, Any>) throws -> IDTransactionInfo {
        let txid = try JsonHelper.getString(json, IDTransactionInfo.TXID, false, nil, "transaction id")
        let timestamp = try DateFormater.getDate(json, IDTransactionInfo.TIMESTAMP, false, nil, "transaction timestamp")
        
        let requjson = json[IDTransactionInfo.OPERATION] as? OrderedDictionary<String, Any>
        if (requjson == nil) {
            throw DIDResolveError.failue("Missing ID operation.")
        }
        
        let request = try IDChainRequest.fromJson(requjson!)
        return IDTransactionInfo(txid, timestamp!, request)
    }
}
