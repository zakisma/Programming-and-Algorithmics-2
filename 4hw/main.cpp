#ifndef __PROGTEST__

#include <cassert>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <list>
#include <algorithm>
#include <memory>
#include <compare>
#include <functional>
#include <optional>

class CTimeStamp
{
public:
    CTimeStamp    ( int               year,
                    int               month,
                    int               day,
                    int               hour,
                    int               minute,
                    double            sec );
    int                   compare       ( const CTimeStamp & x ) const;
    friend std::ostream & operator <<   ( std::ostream     & os,
                                          const CTimeStamp & x );
private:
};
class CMail
{
public:
    CMail         ( const CTimeStamp & timeStamp,
                    const std::string & from,
                    const std::string & to,
                    const std::optional<std::string> & subject );
    int                   compareByTime ( const CTimeStamp & x ) const;
    int                   compareByTime ( const CMail      & x ) const;
    const std::string   & from          () const;
    const std::string   & to            () const;
    const std::optional<std::string>  & subject       () const;
    const CTimeStamp    & timeStamp     () const;
    friend std::ostream & operator <<   ( std::ostream     & os,
                                          const CMail      & x );
private:
};

// your code will be compiled in a separate namespace
namespace MysteriousNamespace {
#endif /* __PROGTEST__ */

//----------------------------------------------------------------------------------------
    class CMailLog {
    public:
        CMailLog() = default;
        ~CMailLog() = default;
        int parseLog(std::istream &in);
        std::list<CMail> listMail(const CTimeStamp &from, const CTimeStamp &to) const;
        std::set<std::string> activeUsers(const CTimeStamp &from, const CTimeStamp &to) const;

    private:
        struct CEmail {
            std::string m_from;
            std::optional<std::string> m_subject;

            CEmail() = default;
            explicit CEmail(const std::string &sender) : m_from(sender) {}
        };

        struct CTimestampComp {
            bool operator()(const CTimeStamp &a, const CTimeStamp &b) const {
                return a.compare(b) < 0;
            }
        };

        std::map<std::string, CEmail> m_processingEmails;
        std::multimap<CTimeStamp, CMail, CTimestampComp> m_emails;

        bool parseLogLineFunc(const std::string &line, CTimeStamp &timestamp,
                          std::string &mailID, std::string &emailMsg);

        int parseMonthFunc(const std::string &month) const;
    };

   
    bool CMailLog::parseLogLineFunc(const std::string &line, CTimeStamp &timestamp,
                                std::string &mailID, std::string &emailMsg) {
        std::istringstream iss(line);
        std::string month, relay;
        double second;
        int day, year, hour, minute;
        char colon;

        if (!(iss >> month >> day >> year >> hour >> colon >> minute >> colon >> second)) {
            return false;
        }

        int monthNum = parseMonthFunc(month);
        if (monthNum == 0) {
            return false;
        }

        if (!(iss >> relay >> mailID)) {
            return false;
        }

        if (mailID.empty() || mailID.back() != ':') {
            return false;
        }
        mailID = mailID.substr(0, mailID.size() - 1);

        std::getline(iss, emailMsg);
        if (emailMsg.size() > 0 && emailMsg[0] == ' ') {
            emailMsg = emailMsg.substr(1);
        }

        timestamp = CTimeStamp(year, monthNum, day, hour, minute, second);

        return true;
    }
    int CMailLog::parseMonthFunc(const std::string &month) const {
        if (month == "Jan") return 1;
        if (month == "Feb") return 2;
        if (month == "Mar") return 3;
        if (month == "Apr") return 4;
        if (month == "May") return 5;
        if (month == "Jun") return 6;
        if (month == "Jul") return 7;
        if (month == "Aug") return 8;
        if (month == "Sep") return 9;
        if (month == "Oct") return 10;
        if (month == "Nov") return 11;
        if (month == "Dec") return 12;
        return 0;
    }

    int CMailLog::parseLog(std::istream &in) {
        std::string line;
        int emailCount = 0;

        while (std::getline(in, line)) {
            CTimeStamp timestamp(0, 0, 0, 0, 0, 0.0);
            std::string mailID, emailMsg;

            if (!parseLogLineFunc(line, timestamp, mailID, emailMsg)) {
                continue;
            }

            if (emailMsg.compare(0, 5, "from=") == 0) {
                std::string sender = emailMsg.substr(5);
                m_processingEmails[mailID] = CEmail(sender);
            } else if (emailMsg.compare(0, 8, "subject=") == 0) {
                auto it = m_processingEmails.find(mailID);
                if (it != m_processingEmails.end()) {
                    it->second.m_subject = emailMsg.substr(8);
                }
            } else if (emailMsg.compare(0, 3, "to=") == 0) {
                auto it = m_processingEmails.find(mailID);
                if (it != m_processingEmails.end()) {
                    std::string recipient = emailMsg.substr(3);

                    CMail mail(timestamp, it->second.m_from, recipient, it->second.m_subject);
                    m_emails.emplace(timestamp, mail);
                    emailCount++;
                }
            }
            // skip other types
        }

        return emailCount;
    }

    std::list<CMail> CMailLog::listMail(const CTimeStamp &from, const CTimeStamp &to) const {
        std::list<CMail> result;

        auto it = m_emails.lower_bound(from);

        while (it != m_emails.end() && it->first.compare(to) <= 0) {
            result.push_back(it->second);
            it++;
        }

        return result;
    }

    std::set<std::string> CMailLog::activeUsers(const CTimeStamp &from, const CTimeStamp &to) const {
        std::set<std::string> usersSet;

        auto it = m_emails.lower_bound(from);

        while (it != m_emails.end() && it->first.compare(to) <= 0) {
            usersSet.insert(it->second.from());
            usersSet.insert(it->second.to());
            it++;
        }

        return usersSet;
    }
//----------------------------------------------------------------------------------------
#ifndef __PROGTEST__
} // namespace
std::string printMail(const std::list<CMail> &all) {
    std::ostringstream oss;
    for (const auto &mail: all)
        oss << mail << "\n";
    return oss.str();
}

int main() {
    MysteriousNamespace::CMailLog m;
    std::list<CMail> mailList;
    std::set<std::string> users;
    std::istringstream iss;

    iss.clear();
    iss.str(
            "Mar 29 2025 12:35:32.233 relay.fit.cvut.cz ADFger72343D: from=user1@fit.cvut.cz\n"
            "Mar 29 2025 12:37:16.234 relay.fit.cvut.cz JlMSRW4232Df: from=person3@fit.cvut.cz\n"
            "Mar 29 2025 12:55:13.023 relay.fit.cvut.cz JlMSRW4232Df: subject=New progtest homework!\n"
            "Mar 29 2025 13:38:45.043 relay.fit.cvut.cz Kbced342sdgA: from=office13@fit.cvut.cz\n"
            "Mar 29 2025 13:36:13.023 relay.fit.cvut.cz JlMSRW4232Df: to=user76@fit.cvut.cz\n"
            "Mar 29 2025 13:55:31.456 relay.fit.cvut.cz KhdfEjkl247D: from=PR-department@fit.cvut.cz\n"
            "Mar 29 2025 14:18:12.654 relay.fit.cvut.cz Kbced342sdgA: to=boss13@fit.cvut.cz\n"
            "Mar 29 2025 14:48:32.563 relay.fit.cvut.cz KhdfEjkl247D: subject=Business partner\n"
            "Mar 29 2025 14:58:32.000 relay.fit.cvut.cz KhdfEjkl247D: to=HR-department@fit.cvut.cz\n"
            "Mar 29 2025 14:25:23.233 relay.fit.cvut.cz ADFger72343D: mail undeliverable\n"
            "Mar 29 2025 15:02:34.231 relay.fit.cvut.cz KhdfEjkl247D: to=CIO@fit.cvut.cz\n"
            "Mar 29 2025 15:02:34.230 relay.fit.cvut.cz KhdfEjkl247D: to=CEO@fit.cvut.cz\n"
            "Mar 29 2025 15:02:34.230 relay.fit.cvut.cz KhdfEjkl247D: to=dean@fit.cvut.cz\n"
            "Mar 29 2025 15:02:34.230 relay.fit.cvut.cz KhdfEjkl247D: to=vice-dean@fit.cvut.cz\n"
            "Mar 29 2025 15:02:34.230 relay.fit.cvut.cz KhdfEjkl247D: to=archive@fit.cvut.cz\n");
    assert (m.parseLog(iss) == 8);
    mailList = m.listMail(CTimeStamp(2025, 3, 28, 0, 0, 0),
                          CTimeStamp(2025, 3, 29, 23, 59, 59));
    assert (printMail(mailList) == R"###(2025-03-29 13:36:13.023 person3@fit.cvut.cz -> user76@fit.cvut.cz, subject: New progtest homework!
2025-03-29 14:18:12.654 office13@fit.cvut.cz -> boss13@fit.cvut.cz
2025-03-29 14:58:32.000 PR-department@fit.cvut.cz -> HR-department@fit.cvut.cz, subject: Business partner
2025-03-29 15:02:34.230 PR-department@fit.cvut.cz -> CEO@fit.cvut.cz, subject: Business partner
2025-03-29 15:02:34.230 PR-department@fit.cvut.cz -> dean@fit.cvut.cz, subject: Business partner
2025-03-29 15:02:34.230 PR-department@fit.cvut.cz -> vice-dean@fit.cvut.cz, subject: Business partner
2025-03-29 15:02:34.230 PR-department@fit.cvut.cz -> archive@fit.cvut.cz, subject: Business partner
2025-03-29 15:02:34.231 PR-department@fit.cvut.cz -> CIO@fit.cvut.cz, subject: Business partner
)###");
    mailList = m.listMail(CTimeStamp(2025, 3, 28, 0, 0, 0),
                          CTimeStamp(2025, 3, 29, 14, 58, 32));
    assert (printMail(mailList) == R"###(2025-03-29 13:36:13.023 person3@fit.cvut.cz -> user76@fit.cvut.cz, subject: New progtest homework!
2025-03-29 14:18:12.654 office13@fit.cvut.cz -> boss13@fit.cvut.cz
2025-03-29 14:58:32.000 PR-department@fit.cvut.cz -> HR-department@fit.cvut.cz, subject: Business partner
)###");
    mailList = m.listMail(CTimeStamp(2025, 3, 30, 0, 0, 0),
                          CTimeStamp(2025, 3, 30, 23, 59, 59));
    assert (printMail(mailList) == "");
    users = m.activeUsers(CTimeStamp(2025, 3, 28, 0, 0, 0),
                          CTimeStamp(2025, 3, 29, 23, 59, 59));
    assert (users == std::set<std::string>(
            {"CEO@fit.cvut.cz", "CIO@fit.cvut.cz", "HR-department@fit.cvut.cz", "PR-department@fit.cvut.cz",
             "archive@fit.cvut.cz", "boss13@fit.cvut.cz", "dean@fit.cvut.cz", "office13@fit.cvut.cz",
             "person3@fit.cvut.cz", "user76@fit.cvut.cz", "vice-dean@fit.cvut.cz"}));
    users = m.activeUsers(CTimeStamp(2025, 3, 28, 0, 0, 0),
                          CTimeStamp(2025, 3, 29, 13, 59, 59));
    assert (users == std::set<std::string>({"person3@fit.cvut.cz", "user76@fit.cvut.cz"}));
    return EXIT_SUCCESS;
}

#endif /* __PROGTEST__ */
