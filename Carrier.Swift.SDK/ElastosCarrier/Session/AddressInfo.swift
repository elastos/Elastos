import Foundation

@objc(ELACarrierAddressInfo)
public class CarrierAddressInfo: NSObject {

    @objc(ELACarrierCandidateType)
    public enum CandidateType: Int, CustomStringConvertible {

        case Host

        case ServerReflexive

        case PeerReflexive

        case Relayed

        public var description: String {
            var value: String

            switch self {
            case .Host:
                value = "Host"
            case .ServerReflexive:
                value = "Server Reflexive"
            case .PeerReflexive:
                value = "Peer Reflexive"
            case .Relayed:
                value = "Relayed"
            }
            
            return value
        }
    }

    @objc(ELACarrierSocketAddress)
    public class SocketAddress: NSObject {
        public let hostname: String
        public let port: Int

        internal init(hostname: String,
                      port: Int) {
            self.hostname = hostname
            self.port = port
            super.init()
        }

        public override var description: String {
            return String(format: "hostname[%@], port[%d]",
                          String.toHardString(hostname),
                          port)
        }
    }

    public let candidateType: CandidateType
    public let address: SocketAddress
    public let relatedAddress: SocketAddress?

    internal init(candidateType: CandidateType,
                  address: String,
                  port: Int,
                  relatedAddress: String?,
                  relatedPort: Int?) {
        self.candidateType = candidateType
        self.address = SocketAddress(hostname: address, port: port)
        if relatedAddress != nil && relatedPort != nil {
            self.relatedAddress = SocketAddress(hostname: relatedAddress!, port: relatedPort!)
        }
        else {
            self.relatedAddress = nil
        }
        super.init()
    }

    internal static func format(_ info: CarrierAddressInfo) -> String {
        return String(format: "candidateType[%@], address[%@], relatedAddress[%@]",
                      info.candidateType.description,
                      String.toHardString(info.address.description),
                      String.toHardString(info.relatedAddress?.description))
    }

    public override var description: String {
        return CarrierAddressInfo.format(self)
    }
}

internal func convertCAddressInfoToCarrierAddressInfo(_ cInfo : CAddressInfo) -> CarrierAddressInfo {
    var temp = cInfo

    let candidateType = CarrierAddressInfo.CandidateType(rawValue: Int(temp.type.rawValue))!
    let address = String(cCharPointer: &temp.addr)
    let port = Int(temp.port)
    var relatedAddress : String? = nil
    var relatedPort : Int? = nil
    if temp.related_addr.0 != 0 {
        relatedAddress = String(cCharPointer: &temp.related_addr)
        relatedPort = Int(temp.related_port)
    }

    return CarrierAddressInfo(candidateType: candidateType,
                              address: address,
                              port: port,
                              relatedAddress: relatedAddress,
                              relatedPort: relatedPort)
}
