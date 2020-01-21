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

    public var did: DID? {
        return request.did
    }

    public var operation: IDChainRequest.Operation {
        return request.operation
    }

    public var payload: String {
        return request.toJson(false)
    }
    
    public func toJson() -> OrderedDictionary<String, Any> {
        var dic: OrderedDictionary<String, Any> = OrderedDictionary()
        dic[IDTransactionInfo.TXID] = transactionId
        dic[IDTransactionInfo.TIMESTAMP] = DateFormater.format(timestamp)
        let jsonstr = request.toJson(false)
        let string = JsonHelper.preHandleString(jsonstr)
        let json = JsonHelper.handleString(string) as! OrderedDictionary<String, Any>
        dic[IDTransactionInfo.OPERATION] = json

        return dic
    }
    
    public class func fromJson(_ json: OrderedDictionary<String, Any>) throws -> IDTransactionInfo {
        let txid = try JsonHelper.getString(json, IDTransactionInfo.TXID, false, "transaction id")
        let timestamp = try JsonHelper.getDate(json, IDTransactionInfo.TIMESTAMP, false, "transaction timestamp")
        
        let requjson = json[IDTransactionInfo.OPERATION] as? OrderedDictionary<String, Any>
        if (requjson == nil) {
            throw DIDError.didResolveError(_desc: "Missing ID operation.") 
        }
        
        let request = try IDChainRequest.fromJson(requjson!)
        return IDTransactionInfo(txid, timestamp!, request)
    }
}
