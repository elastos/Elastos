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

public class ResolverCache {
    private static let TAG = "ResolverCache"
    private static let CACHE_INITIAL_CAPACITY = 16
    private static let CACHE_MAX_CAPACITY = 32
    private static var rootDir: String = ""
    private static var cache: LRUCache = LRUCache<AnyHashable, Any>(CACHE_MAX_CAPACITY)

    /// Set cache path.
    /// - Parameter rootPath: The path for cache.
    /// - Throws: if an error occurred, throw error.
    public class func setCacheDir(_ rootPath: String) throws {
        ResolverCache.rootDir = rootPath
        if try !exists_dir(rootPath) {
            let fileManager = FileManager.default
            try fileManager.createDirectory(atPath: rootDir, withIntermediateDirectories: true, attributes: nil)
        }
    }

    /// Set cache path.
    /// - Parameter url: The path for cache.
    /// - Throws: if an error occurred, throw error.
    public class func setCacheDir(_ url: URL) throws {
        try setCacheDir(url.absoluteString)
    }
    
    private class func getCacheDir() throws -> String {
        if rootDir == "" {
            throw DIDError.illegalArgument("No cache dir specified for ResolverCache")
        }
        return rootDir
    }

    private class func getFile(_ id: String) throws -> String {
        let root = try getCacheDir()
//        return root + "/" + ".did.elastos" + "/" + id
        return root + "/" + id
    }

    /// Clear cachei.
    /// - Throws: if an error occurred, throw error.
    public class func reset() throws {
        cache.clear()
        deleteFile(try getCacheDir())
    }

    /// Store resolve result in DID Store.
    /// - Parameter rr: The handle of ResolveResult.
    /// - Throws: if an error occurred, throw error.
    public class func store(_ rr: ResolveResult) throws {
        let id = rr.did.methodSpecificId
        let path = try getFile(id)
        let json: String = rr.toJson()
        let data: Data = json.data(using: .utf8)!
        let handle = FileHandle(forWritingAtPath:path)
        handle?.write(data)
        
        cache.setValue(rr, for: rr.did)
    }

    /// Load resolve result.
    /// - Parameters:
    ///   - did: The identify of resolve result.
    ///   - ttl: The timestamp of load resolve result.
    /// - Throws: if an error occurred, throw error.
    /// - Returns: If no error occurs, return the handle to ResolveResult. Otherwise, return nil.
    public class func load(_ did: DID, _ ttl: Int) throws -> ResolveResult? {
        let path = try getFile(did.methodSpecificId)
        
        if try exists(path) {
            return nil
        }
        
        do {
            let attr = try FileManager.default.attributesOfItem(atPath: path)
            let lastModifiedTime = attr[FileAttributeKey.modificationDate] as? Date
            let systemTime = Date()
            let interval = systemTime.timeIntervalSince(lastModifiedTime!)
            if Int(interval) > ttl {
                return nil
            }
        } catch {
            return nil
        }

        if cache.containsKey(for: did.methodSpecificId) {
            return cache.getValue(for: did.methodSpecificId) as? ResolveResult
        }
        
        let jsonString = try readTextFromPath(path)
        let result = try ResolveResult.fromJson(jsonString)
        cache.setValue(result, for: did.methodSpecificId)
        return result
    }
    
    // temp add
    private class func exists(_ dirPath: String) throws -> Bool {
        let fileManager = FileManager.default
        var isDir : ObjCBool = false
        fileManager.fileExists(atPath: dirPath, isDirectory:&isDir)
        let readhandle = FileHandle.init(forReadingAtPath: dirPath)
        let data: Data = (readhandle?.readDataToEndOfFile()) ?? Data()
        let str: String = String(data: data, encoding: .utf8) ?? ""
        return str.count > 0 ? true : false
    }
    
    private class func exists_dir(_ dirPath: String) throws -> Bool {
        let fileManager = FileManager.default
        var isDir : ObjCBool = false
        let re = fileManager.fileExists(atPath: dirPath, isDirectory:&isDir)
        return re && isDir.boolValue
    }
    
    private class func readTextFromPath(_ path: String) throws -> String {
        guard try exists(path) else {
            return ""
        }
        return try String(contentsOfFile:path, encoding: String.Encoding.utf8)
    }
    
    class func deleteFile(_ path: String) {
         do {
             let filemanager: FileManager = FileManager.default
             var isdir = ObjCBool.init(false)
             let fileExists = filemanager.fileExists(atPath: path, isDirectory: &isdir)
             if fileExists && isdir.boolValue {
                 if let dircontents = filemanager.enumerator(atPath: path) {
                     for case let url as URL in dircontents {
                         deleteFile(url.absoluteString)
                     }
                 }
             }
             guard fileExists else {
                 return
             }
             try filemanager.removeItem(atPath: path)
         } catch {
            Log.e(ResolverCache.TAG, "deleteFile error: \(error)")
         }
     }

    /// Invali date.
    /// - Parameter did: The identify of resolve result.
    /// - Throws: if an error occurred, throw error.
    public class func invalidate(_ did: DID) throws {
        let filePath = try getFile(did.methodSpecificId)
        deleteFile(filePath)
        cache.removeValue(for: did)
    }
}

