
import Foundation
@testable import ElastosDIDSDK
import SPVWrapper

public typealias PasswordCallback = (_ walletDir: String, _ walletId: String) -> String?
public class SPVAdaptor: DIDAdapter {
    public func createIdTransaction(_ payload: String, _ memo: String?, _ confirms: Int, _ callback: @escaping (String, Int, String?) -> Void) {
        
        var password = passwordCallback!(walletDir, walletId)
        if password == nil {
            password = ""
        }

        var confirm = confirms
        if confirm < 0 {
            confirm = 0
        } else if confirm > 1 {
            confirm = 1
        }
        SPV.createIdTransactionEx(handle, payload, memo, confirm, password!) { (txid, status, msg) in
            callback(txid, status, msg)
        }
    }

    var walletDir: String!
    var walletId: String!
    var network: String!
    var handle: OpaquePointer!
    public var passwordCallback: PasswordCallback?
    
    public init(_ walletDir: String, _ walletId: String, _ network: String, _ resolver: String, _ passwordCallback: @escaping PasswordCallback) {
        
       handle = SPV.create(walletDir, walletId, network, resolver)
        self.walletDir = walletDir
        self.walletId = walletId
        self.network = network
        self.passwordCallback = passwordCallback
    }
    
    public func destroy() {
        SPV.destroy(handle)
        handle = nil
    }
    
    public func isAvailable() throws -> Bool {
       return SPV.isAvailable(handle)
    }

    public func resolve(_ requestId: String, _ did: String, _ all: Bool) throws -> String {
        var resuleString: String?
        let url:URL! = URL(string: "http://api.elastos.io:21606")
        var request:URLRequest! = URLRequest.init(url: url, cachePolicy: .useProtocolCachePolicy, timeoutInterval: 60)
        request.httpMethod = "POST"
        request.setValue("application/json", forHTTPHeaderField: "Content-Type")
        request.setValue("application/json", forHTTPHeaderField: "Accept")
        let parameters: [String: Any] = [
            "jsonrpc": "2.0",
            "method": "resolvedid",
            "params": ["did":did, "all": all],
            "id": requestId
        ]
        request.httpBody = try! JSONSerialization.data(withJSONObject: parameters, options: .prettyPrinted)
        let semaphore = DispatchSemaphore(value: 0)
        let task = URLSession.shared.dataTask(with: request) { data, response, error in
            guard let data = data,
                let response = response as? HTTPURLResponse,
                error == nil else { // check for fundamental networking error
                    semaphore.signal()
                    return
            }
            guard (200 ... 299) ~= response.statusCode else { // check for http errors
                semaphore.signal()
                return
            }
            let responseString = String(data: data, encoding: .utf8)
            print("responseString = \(String(describing: responseString))")
            resuleString = responseString
            semaphore.signal()
        }
        task.resume()
        semaphore.wait()
        
        guard resuleString != nil else {
            throw DIDError.didResolveError("Unkonw error.")
        }
        return resuleString!
    }
}

