/*
    Copyright (C) 2019 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
    Copyright 2013-2014 Jan Grulich <jgrulich@redhat.com>
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.
    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef NETWORKING_H
#define NETWORKING_H

#include <networkmanager_export.h>
#include <QObject>

#include <NetworkManagerQt/Device>
#include <NetworkManagerQt/Manager>

class NETWORKMANAGER_EXPORT Networking : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(bool wirelessEnabled READ isWirelessEnabled WRITE setWirelessEnabled NOTIFY wirelessEnabledChanged)
    Q_PROPERTY(bool wirelessHardwareEnabled READ isWirelessHardwareEnabled NOTIFY wirelessHardwareEnabledChanged)
    Q_PROPERTY(bool mobileEnabled READ isMobileEnabled WRITE setMobileEnabled NOTIFY mobileEnabledChanged)
    Q_PROPERTY(bool mobileHardwareEnabled READ isMobileHardwareEnabled NOTIFY mobileHardwareEnabledChanged)
    Q_PROPERTY(bool airplaneModeEnabled READ isAirplaneModeEnabled WRITE setAirplaneModeEnabled NOTIFY airplaneModeEnabledChanged)
    Q_PROPERTY(QString activeConnections READ activeConnections NOTIFY activeConnectionsChanged)
    Q_PROPERTY(QString networkStatus READ networkStatus NOTIFY networkStatusChanged)

public:
    enum SortedConnectionType {
        Wired,
        Wireless,
        Gsm,
        Cdma,
        Pppoe,
        Adsl,
        Infiniband,
        OLPCMesh,
        Bluetooth,
        Vpn,
        Other
    };
    Q_ENUM(SortedConnectionType)

    enum HandlerAction {
        ActivateConnection,
        AddAndActivateConnection,
        AddConnection,
        DeactivateConnection,
        RemoveConnection,
        RequestScan,
        UpdateConnection,
        CreateHotspot,
    };
    Q_ENUM(HandlerAction)

    explicit Networking(QObject *parent = nullptr);

    bool isEnabled() const;
    void setEnabled(bool enabled);

    bool isWirelessEnabled() const;
    void setWirelessEnabled(bool enabled);

    bool isWirelessHardwareEnabled() const;

    bool isMobileEnabled() const;
    void setMobileEnabled(bool enabled);

    bool isMobileHardwareEnabled() const;

    bool isAirplaneModeEnabled() const;
    void setAirplaneModeEnabled(bool enabled);

    QString activeConnections() const;
    QString networkStatus() const;

    Q_INVOKABLE void activateConnection(const QString &connectionPath, const QString &device, const QString &specificObject);
    Q_INVOKABLE void addAndActivateConnection(const QString &device, const QString &specificObject, const QString &password = QString());
    Q_INVOKABLE void deactivateConnection(const QString &connectionName, const QString &device);
    Q_INVOKABLE void removeConnection(const QString &connectionPath);

    static SortedConnectionType connectionTypeToSortedType(NetworkManager::ConnectionSettings::ConnectionType type);

Q_SIGNALS:
    void enabledChanged();
    void wirelessEnabledChanged();
    void wirelessHardwareEnabledChanged();
    void mobileEnabledChanged();
    void mobileHardwareEnabledChanged();
    void airplaneModeEnabledChanged();
    void activeConnectionsChanged();
    void networkStatusChanged();

private:
    bool m_lastWirelessEnabled = false;
    bool m_lastMobileEnabled = false;
    QString m_userName;
    QString m_activeConnections;
    QString m_networkStatus;

    void doChangeActiveConnections();
    QString checkUnknownReason() const;

private Q_SLOTS:
    void statusChanged(NetworkManager::Status status);
    void changeActiveConnections();
    void defaultChanged();
};

#endif // NETWORKING_H
