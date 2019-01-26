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
    A class representing the Carrier group peer information.
 */
@objc(ELACarrierGroupPeer)
public class CarrierGroupPeer: NSObject {

    /// Elastos carrier peer ID max length.
    public static let MAX_ID_LEN: Int = 45

    /// Elastos carrier peer name max length.
    public static let MAX_NAME_LEN: Int = 63

    internal override init() {
        super.init()
    }

    /**
        User ID.
     */
    public internal(set) var userId: String?

    /**
        Nickname, also as display name.
     */
    public var name: String?

    internal static func format(_ info: CarrierGroupPeer) -> String {
        return String(format: "userId[%@], name[%@]",
                      String.toHardString(info.userId),
                      String.toHardString(info.name))
    }

    public override var description: String {
        return CarrierGroupPeer.format(self)
    }
}

internal func convertCarrierGroupPeerToCGroupPeer(_ peer: CarrierGroupPeer) -> CGroupPeer {
    var cPeer = CGroupPeer();

    peer.userId?.writeToCCharPointer(&cPeer.userid)
    peer.name?.writeToCCharPointer(&cPeer.name)

    return cPeer
}

internal func convertCGroupPeerToCarrierGroupPeer(_ cPeer: CGroupPeer) -> CarrierGroupPeer {
    let peer = CarrierGroupPeer();
    var temp = cPeer;

    peer.userId = String(cCharPointer: &temp.userid)
    peer.name = String(cCharPointer: &temp.name)

    return peer
}
