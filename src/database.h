/****************************************************************************
 *   Copyright (C) 2017 Savoir-faire Linux                                  *
 *   Author : Nicolas Jäger <nicolas.jager@savoirfairelinux.com>            *
 *   Author : Sébastien Blin <sebastien.blin@savoirfairelinux.com>          *
 *                                                                          *
 *   This library is free software; you can redistribute it and/or          *
 *   modify it under the terms of the GNU Lesser General Public             *
 *   License as published by the Free Software Foundation; either           *
 *   version 2.1 of the License, or (at your option) any later version.     *
 *                                                                          *
 *   This library is distributed in the hope that it will be useful,        *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *   Lesser General Public License for more details.                        *
 *                                                                          *
 *   You should have received a copy of the GNU General Public License      *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.  *
 ***************************************************************************/
#pragma once

// Std
#include <memory>
#include <string>

// Qt
#include <qobject.h>
#include <QtSql/QSqlQuery>

// Lrc
#include "namedirectory.h"

class Account; // TODO: move this class into lrc ns

namespace lrc
{

namespace api { namespace message { struct Info; }}

class Database : public QObject {
    Q_OBJECT

public:
    using MessagesMap = std::map<int, api::message::Info>;

    static constexpr auto ringDB = "ring.db"; // TODO: set path correctly for tests and release.

    Database();
    ~Database();

    // Messages related
    void addMessage(const std::string& accountId, const api::message::Info& message) const;
    void clearHistory(const std::string& accountId,
                      const std::string& uid,
                      bool removeContact = false) const;
    MessagesMap getHistory(const std::string& accountId, const std::string& uid) const;
    std::size_t numberOfUnreads(const std::string& accountId, const std::string& uid) const;
    void setMessageRead(int uid) const;

    // Contacts related
    void addContact(const std::string& contact, const QByteArray& payload) const;
    std::string getContactAttribute(const std::string& uid, const std::string& attribute) const;

Q_SIGNALS:
    void messageAdded(int uid, const std::string& accountId, const api::message::Info& msg) const;
    void contactAdded(const std::string& uid) const;

private Q_SLOTS:
    void slotRegisteredNameFound(const Account* account,
                                 NameDirectory::LookupStatus status,
                                 const QString& address,
                                 const QString& name) const;

private:
    std::unique_ptr<QSqlQuery> query_;
    QSqlDatabase db_;
};

} // namespace lrc