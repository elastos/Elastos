import Foundation

/**
    CarrierOptions defines several settings that control the way the carrier
    node connects to others.
 */
@objc(ELACarrierOptions)
public class CarrierOptions: NSObject {
    private var _persistentLocation: String?
    private var _udpEnabled: Bool = true

    private var _bootstrapSize: Int = 0
    //TODO: private var _bootstraps: BootstrapNode?

    /**
        The application defined persistent data location.
        The location must be set.
     */
    public var persistentLocation: String? {
        set {
            _persistentLocation = newValue
        }
        get {
            return _persistentLocation
        }
    }

    /**
        The option to decide to use udp transport or not. Setting this option
        to false will force Carrier node to use TCP only, which will
        potentially slow down the message to run through.
     */
    public var udpEnabled: Bool {
        set {
            _udpEnabled = newValue
        }
        get {
            return _udpEnabled
        }
    }

    //TODO
}

internal func convertCarrierOptionsToCOptions(_ options : CarrierOptions) -> COptions {
    var cOptions = COptions()

    cOptions.persistent_location = createCStringDuplicate(options.persistentLocation)
    //TODO:

    return cOptions
}

internal func cleanupCOptions(_ cOptions : COptions) {
    deallocCString(cOptions.persistent_location)
}
