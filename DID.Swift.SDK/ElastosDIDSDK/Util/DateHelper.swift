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
