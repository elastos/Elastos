/*
 * Copyright (c) 2020 Elastos Foundation
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

internal typealias CExtensionInviteCallback = @convention(c)
    (OpaquePointer?, UnsafePointer<Int8>?, UnsafePointer<Int8>, Int, UnsafeMutableRawPointer?) -> Swift.Void

@_silgen_name("extension_init")
internal func extension_init(_ carrier: OpaquePointer,
                             _ callback: CExtensionInviteCallback,
                             _ context: UnsafeMutableRawPointer) -> Swift.Int32


@_silgen_name("extension_cleanup")
internal func extension_cleanup(_ carrier: OpaquePointer) -> Swift.Void


internal typealias CExtensionInviteReplyCallback = @convention(c)
    (OpaquePointer?, UnsafePointer<Int8>, Int32, UnsafePointer<Int8>?, UnsafePointer<Int8>?, Int,
     UnsafeMutableRawPointer) -> Swift.Void

@_silgen_name("extension_invite_friend")
internal func extension_invite_friend(_ carrier: OpaquePointer,
                                      _ to: UnsafePointer<Int8>,
                                      _ data: UnsafeRawPointer,
                                      _ size: Int,
                                      _ callback: CExtensionInviteReplyCallback,
                                      _ context: UnsafeMutableRawPointer) -> Swift.Int32

@_silgen_name("extension_reply_friend_invite")
internal func extension_reply_friend_invite(_ carrier: OpaquePointer,
                                      _ to: UnsafePointer<Int8>,
                                      _ status: Int32,
                                      _ reason: UnsafePointer<Int8>?,
                                      _ data: UnsafeRawPointer?,
                                      _ len: Int) -> Swift.Int32
