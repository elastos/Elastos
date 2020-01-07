
import Foundation

public class SPV {
    
    public class func create(_ walletDir: String, _ walletId: String, _ network: String, _ resolver: String) -> OpaquePointer {
        return SpvDidAdapter_Create(walletDir, walletId, network, resolver)
    }
    
    public class func destroy(_ handle: OpaquePointer) {
        SpvDidAdapter_Destroy(handle)
    }
    
    public class func isAvailable(_ handle: OpaquePointer) -> Bool {
        let rc = SpvDidAdapter_IsAvailable(handle)
        return rc == 1
    }
    
    public class func createIdTransaction(_ handle: OpaquePointer, _ password: String, _ payload: String, _ memo: String?) throws -> String? {
        let re = SpvDidAdapter_CreateIdTransaction(handle, payload, memo, password)
        if re != nil {
           return String(cString: re!)
        }
        return nil
    }
    
    public class func resolve(_ requestId: String, _ did: String, _ all: Bool) throws -> String? {
//        let startIndex = did.index(did.startIndex, offsetBy: 12)
//        let id = String(did[startIndex..<did.endIndex])
        var resuleString = ""
        let url:URL! = URL(string: "http://api.elastos.io:21606") //  https://coreservices-didsidechain-privnet.elastos.org
        var request:URLRequest! = URLRequest.init(url: url, cachePolicy: .useProtocolCachePolicy, timeoutInterval: 60)
        request.httpMethod = "POST"
        request.setValue("application/json", forHTTPHeaderField: "Content-Type")
        request.setValue("application/json", forHTTPHeaderField: "Accept")
        let parameters: [String: Any] = [
            "jsonrpc": "2.0",
            "method": "resolvedid", //   getidxspayloads
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
            resuleString = responseString ?? ""
            semaphore.signal()
        }
        task.resume()
        semaphore.wait()
        guard resuleString != "" else {
            return nil
        }
        return resuleString
    }
}
