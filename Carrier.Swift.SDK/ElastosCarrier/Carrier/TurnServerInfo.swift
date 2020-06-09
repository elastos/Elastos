
import Foundation

public class TurnServerInfo: NSObject {
    var server: String
    var port: Int
    var username: String
    var password: String
    var realm: String

    init(_ server: String, _ username: String, _ password: String, _ realm: String, _ port: Int) {
        self.server = server
        self.port = port
        self.username = username
        self.password = password
        self.realm = realm
    }

    internal class func convertTurnServerInfoToCTurnServer(_ info: TurnServerInfo) -> CTurnServer {
        var cServer = CTurnServer()

        cServer.port = Int16(info.port)
        info.server.writeToCCharPointer(&cServer.server)
        info.username.writeToCCharPointer(&cServer.username)
        info.password.writeToCCharPointer(&cServer.password)
        info.realm.writeToCCharPointer(&cServer.realm)

        return cServer
    }

    internal class func convertCTurnServerToTurnServerInfo(_ cInfo: CTurnServer) -> TurnServerInfo {

        var server = cInfo.server
        var username = cInfo.username
        var password = cInfo.password
        var realm = cInfo.realm

        let info = TurnServerInfo(String(cCharPointer: &server), String(cCharPointer: &username), String(cCharPointer: &password), String(cCharPointer: &realm), Int(cInfo.port))
        
        return info
    }
}
