/*
 * Copyright (c) 2018 Elastos Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

import Foundation

/**
 CarrierOptions defines several settings that control the way the carrier
 node connects to others.
 */
@objc(ELACarrierOptions)
public class CarrierOptions: NSObject {
    
    @objc public override init() {
        udpEnabled = true
        super.init()
    }
    
    /**
     The application defined persistent data location.
     The location must be set.
     */
    @objc public var persistentLocation: String?
    
    @objc public var secret_key: Data?

    /**
     The option to decide to use udp transport or not. Setting this option
     to false will force Carrier node to use TCP only, which will
     potentially slow down the message to run through.
     */
    @objc public var udpEnabled: Bool
    
    @objc public var bootstrapNodes: [BootstrapNode]?

    @objc public var expressNodes: [ExpressNode]?

    @objc public var loglevel: CarrierLogLevel = .Debug

}

internal func convertCarrierOptionsToCOptions(_ options : CarrierOptions) -> COptions {
    var cOptions = COptions()
    var cNodes: UnsafeMutablePointer<CBootstrapNode>?
    var cExpressNodes: UnsafeMutablePointer<CExpressNode>?

    cOptions.persistent_location = createCStringDuplicate(options.persistentLocation)
    cOptions.secret_key = createCDataDuplicate(options.secret_key)
    cOptions.udp_enabled = options.udpEnabled
    (cNodes, cOptions.bootstraps_size) = convertBootstrapNodesToCBootstrapNodes(options.bootstrapNodes!)
    cOptions.bootstraps = UnsafePointer<CBootstrapNode>(cNodes)
    cOptions.log_level = convertCarrierLogLevelToCLogLevel(options.loglevel)

    if options.expressNodes != nil {
        (cExpressNodes, cOptions.express_nodes_size) = convertBootstrapNodesToCExpressNode(options.expressNodes!)
        cOptions.express_nodes = UnsafePointer<CExpressNode>(cExpressNodes)
    }

    return cOptions
}

internal func cleanupCOptions(_ cOptions : COptions) {
    deallocCString(cOptions.persistent_location)
    cleanupCBootstrap(cOptions.bootstraps!, cOptions.bootstraps_size)
    UnsafeMutablePointer<CBootstrapNode>(mutating: cOptions.bootstraps)?.deallocate()
    deallocCData(cOptions.secret_key)
    guard cOptions.express_nodes != nil else {
        return
    }
    cleanupCBootstraphive(cOptions.express_nodes!, cOptions.express_nodes_size)
    UnsafeMutablePointer<CExpressNode>(mutating: cOptions.express_nodes)?.deallocate()
}

@inline(__always) internal func createCDataDuplicate(_ field: Data?) -> UnsafePointer<Int8>? {
    if field != nil {
        return UnsafePointer<Int8>(field!.withUnsafeBytes {
            return strdup($0)
        })
    } else {
        return nil
    }
}

@inline(__always) internal func deallocCData(_ field: UnsafePointer<Int8>?) {
    if (field != nil) {
        free(UnsafeMutablePointer<Int8>(mutating: field))
    }
}
