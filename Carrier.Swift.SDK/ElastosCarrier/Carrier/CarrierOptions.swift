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
    
    /**
     The option to decide to use udp transport or not. Setting this option
     to false will force Carrier node to use TCP only, which will
     potentially slow down the message to run through.
     */
    @objc public var udpEnabled: Bool
    
    @objc public var bootstrapNodes: [BootstrapNode]?

    @objc public var hivebootstrapNodes: [HiveBootstrapNode]?

}

internal func convertCarrierOptionsToCOptions(_ options : CarrierOptions) -> COptions {
    var cOptions = COptions()
    var cNodes: UnsafeMutablePointer<CBootstrapNode>?
    
    var cHiveNodes: UnsafeMutablePointer<CHiveBootstrapNode>?

    cOptions.persistent_location = createCStringDuplicate(options.persistentLocation)
    cOptions.udp_enabled = options.udpEnabled
    (cNodes, cOptions.bootstraps_size) = convertBootstrapNodesToCBootstrapNodes(options.bootstrapNodes!)
    (cHiveNodes, cOptions.hive_bootstraps_size) = convertBootstrapNodesToCHiveBootstrapNode(options.hivebootstrapNodes!)

    cOptions.bootstraps = UnsafePointer<CBootstrapNode>(cNodes)
    cOptions.hive_bootstraps = UnsafePointer<CHiveBootstrapNode>(cHiveNodes)

    return cOptions
}

internal func cleanupCOptions(_ cOptions : COptions) {
    deallocCString(cOptions.persistent_location)
    cleanupCBootstrap(cOptions.bootstraps!, cOptions.bootstraps_size)
    UnsafeMutablePointer<CBootstrapNode>(mutating: cOptions.bootstraps)?.deallocate()

    cleanupCBootstraphive(cOptions.hive_bootstraps!, cOptions.hive_bootstraps_size)
    UnsafeMutablePointer<CHiveBootstrapNode>(mutating: cOptions.hive_bootstraps)?.deallocate()
}

