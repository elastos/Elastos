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

import Foundation;

@objc(ELABootstrapNode)
public class BootstrapNode: NSObject {

    /**
        Ipv4 address.
     */
    public var ipv4: String?

    /**
        Ipv6 address.
     */
    public var ipv6: String?

    /**
        Port.
     */
    public var port: String?

    /**
        public address.
     */
    public var publicKey: String?

    internal static func format(_ node: BootstrapNode) -> String {
        return String(format: "ipv4[%@], ipv6[%@], port[%@], publicKey[%@]",
                      String.toHardString(node.ipv4),
                      String.toHardString(node.ipv6),
                      String.toHardString(node.port),
                      String.toHardString(node.publicKey))
    }

    public override var description: String {
        return BootstrapNode.format(self)
    }
}

internal func convertBootstrapNodesToCBootstrapNodes(_ nodes: [BootstrapNode]) -> (UnsafeMutablePointer<CBootstrapNode>?, Int) {
    var cNodes: UnsafeMutablePointer<CBootstrapNode>?

    cNodes = UnsafeMutablePointer<CBootstrapNode>.allocate(capacity: nodes.count)
    if cNodes == nil {
        return (nil, 0)
    }

    for (index, node) in nodes.enumerated() {

        var cNode = CBootstrapNode()
        node.ipv4?.withCString { (ipv4) in
            cNode.ipv4 = UnsafePointer<Int8>(strdup(ipv4))
        }
        node.ipv6?.withCString { (ipv6) in
            cNode.ipv6 = UnsafePointer<Int8>(strdup(ipv6));
        }
        node.port?.withCString { (port) in
            cNode.port = UnsafePointer<Int8>(strdup(port));
        }
        node.publicKey?.withCString { (publicKey) in
            cNode.public_key = UnsafePointer<Int8>(strdup(publicKey));
        }
        
        (cNodes! + index).initialize(to: cNode)
    }

    return (cNodes, nodes.count)
}

internal func cleanupCBootstrap(_ cNodes: UnsafePointer<CBootstrapNode>, _ count: Int) -> Void {

    for index in 0..<count {
        let cNode: CBootstrapNode = (cNodes + index).pointee
        if cNode.ipv4 != nil {
            free(UnsafeMutablePointer<Int8>(mutating: cNode.ipv4))
        }
        if cNode.ipv6 != nil {
            free(UnsafeMutablePointer<Int8>(mutating: cNode.ipv6))
        }
        if cNode.port != nil {
            free(UnsafeMutablePointer<Int8>(mutating: cNode.port))
        }
        if cNode.public_key != nil {
            free(UnsafeMutablePointer<Int8>(mutating: cNode.public_key))
        }
    }
}
