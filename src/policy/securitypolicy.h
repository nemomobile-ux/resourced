/*
 * Copyright (C) 2026 Chupligin Sergey <neochapay@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef SECURITYPOLICY_H
#define SECURITYPOLICY_H

#include <QObject>
#include <QString>

class SecurityPolicy : public QObject {
    Q_OBJECT

public:
    explicit SecurityPolicy(QObject* parent = nullptr);

    /**
     * Check if the sender (D-Bus service name) is allowed to
     * register a client or call resource methods.
     */
    bool isAllowedSender(const QString& sender) const;

private:
    QStringList m_whitelist;
};

#endif // SECURITYPOLICY_H
