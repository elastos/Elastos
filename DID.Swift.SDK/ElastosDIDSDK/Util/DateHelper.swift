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

class DateHelper {
    class func currentDate()-> Date {
        let current  = Date()
        var calendar = Calendar(identifier: .gregorian)
        calendar.timeZone = TimeZone(abbreviation: "UTC")!

        var comps:DateComponents?
        comps = calendar.dateComponents([.year, .month, .day, .hour, .minute, .second], from: current)
        comps?.year = 0
        comps?.month = 0
        comps?.day = 0
        comps?.hour = 0
        comps?.minute = 0
        comps?.second = 0
        comps?.nanosecond = 0

        return calendar.date(byAdding: comps!, to: current) ?? Date()
    }

    class func formateDate(_ date: Date) -> String {
        let formatter = Foundation.DateFormatter()
        formatter.dateFormat = "yyyy-MM-dd'T'HH:mm:ss'Z'"
        formatter.timeZone = TimeZone.init(secondsFromGMT: 0)
        let localDate = formatter.string(from: date)
        return localDate
    }

    class func getTimeStamp(_ date: Date) -> Int {

        return Int(date.timeIntervalSince1970)
    }

    class func getTimeStampForString(_ date: Date) -> String {

        let time = Int(date.timeIntervalSince1970)
        return String(time)
    }

    class func getDateFromTimeStamp(_ timeStamp: Int?) -> Date? {
        guard timeStamp != nil else {
            return nil
        }
        let interval = TimeInterval.init(timeStamp!)

        return Date(timeIntervalSince1970: interval)
    }

    class func getDateFromTimeStampWithString(_ timeStamp: String?) -> Date? {
        guard timeStamp != nil else {
            return nil
        }
        let intTimeStamp = Int(timeStamp!)
        let interval = TimeInterval.init(intTimeStamp!)

        return Date(timeIntervalSince1970: interval)
    }

    class func isExipired(_ date: Date) -> Bool {
        return isExpired(DateHelper.currentDate(), date)
    }

    class func isExpired(_ date: Date, _ expirateDate: Date) -> Bool {

        return date > expirateDate
    }

    class func maxExpirationDate(_ date: Date) -> Date {
        var calendar = Calendar(identifier: .gregorian)
        calendar.timeZone = TimeZone.current
        var comps:DateComponents?

        comps = calendar.dateComponents([.year, .month, .day, .hour, .minute, .second], from: date)
        comps?.year = 5
        comps?.month = 0
        comps?.day = 0
        comps?.hour = 0
        comps?.minute = 0
        comps?.second = 0
        comps?.nanosecond = 0
        let realDate = calendar.date(byAdding: comps!, to: date) ?? Date()
        let hour = calendar.component(.hour, from: realDate)
        let minute = calendar.component(.minute, from: realDate)
        let second = calendar.component(.second, from: realDate)

        let useDate = calendar.date(bySettingHour: hour, minute: minute, second: second, of: realDate) ?? Date()

        return useDate
    }

    class func maxExpirationDate() -> Date {
        return maxExpirationDate(Date())
    }
}
