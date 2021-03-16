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

#include <NetworkManagerQt/ConnectionSettings>
#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/VpnSetting>
#include <NetworkManagerQt/Settings>

#include <QDBusConnectionInterface>

#if WITH_MODEMMANAGER_SUPPORT
#  include <NetworkManagerQt/GsmSetting>
#  include <NetworkManagerQt/ModemDevice>
#  include <ModemManagerQt/ModemDevice>
#endif

#include "uiutils.h"
#include "networking.h"
#include "networkmodel.h"

#include <sys/types.h>
#include <pwd.h>

#include <QDebug>

Networking::Networking(QObject *parent)
    : QObject(parent)
    , m_lastWirelessEnabled(isWirelessEnabled())
    , m_lastMobileEnabled(isMobileEnabled())
{
    ::passwd *pw = ::getpwuid(::getuid());
    m_userName = QString::fromLocal8Bit(pw->pw_name);

    connect(NetworkManager::notifier(), &NetworkManager::Notifier::networkingEnabledChanged,
            this, &Networking::enabledChanged);
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::wirelessEnabledChanged,
            this, &Networking::wirelessEnabledChanged);
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::wirelessHardwareEnabledChanged,
            this, &Networking::wirelessHardwareEnabledChanged);
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::wwanEnabledChanged,
            this, &Networking::mobileEnabledChanged);
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::wwanHardwareEnabledChanged,
            this, &Networking::mobileHardwareEnabledChanged);

    connect(NetworkManager::notifier(), &NetworkManager::Notifier::statusChanged,
            this, &Networking::statusChanged);
    doChangeActiveConnections();
    statusChanged(NetworkManager::status());
}

bool Networking::isEnabled() const
{
    return NetworkManager::isNetworkingEnabled();
}

void Networking::setEnabled(bool enabled)
{
    NetworkManager::setNetworkingEnabled(enabled);
}

bool Networking::isWirelessEnabled() const
{
    return NetworkManager::isWirelessEnabled();
}

void Networking::setWirelessEnabled(bool enabled)
{
    NetworkManager::setWirelessEnabled(enabled);
}

bool Networking::isWirelessHardwareEnabled() const
{
    return NetworkManager::isWirelessHardwareEnabled();
}

bool Networking::isMobileEnabled() const
{
    return NetworkManager::isWwanEnabled();
}

void Networking::setMobileEnabled(bool enabled)
{
    NetworkManager::setWwanEnabled(enabled);
}

bool Networking::isMobileHardwareEnabled() const
{
    return NetworkManager::isWwanHardwareEnabled();
}

bool Networking::isAirplaneModeEnabled() const
{
    return !isWirelessEnabled() && !isMobileEnabled();
}

void Networking::setAirplaneModeEnabled(bool enabled)
{
    if (isAirplaneModeEnabled() == enabled)
        return;

    m_lastWirelessEnabled = isWirelessEnabled();
    m_lastMobileEnabled = isMobileEnabled();

    if (enabled) {
        setWirelessEnabled(false);
        setMobileEnabled(false);
    } else {
        if (m_lastWirelessEnabled)
            setWirelessEnabled(true);
        if (m_lastMobileEnabled)
            setMobileEnabled(true);
    }

    Q_EMIT airplaneModeEnabledChanged();
}

QString Networking::activeConnections() const
{
    return m_activeConnections;
}

QString Networking::networkStatus() const
{
    return m_networkStatus;
}

void Networking::activateConnection(const QString &connectionPath, const QString &device, const QString &specificObject)
{
    NetworkManager::Connection::Ptr connection = NetworkManager::findConnection(connectionPath);

    if (!connection) {
        qCWarning(gLcNm, "Unable to activate connection \"%s\"", qPrintable(connectionPath));
        return;
    }

    if (connection->settings()->connectionType() == NetworkManager::ConnectionSettings::Vpn) {
        NetworkManager::VpnSetting::Ptr vpnSetting = connection->settings()->setting(NetworkManager::Setting::Vpn).staticCast<NetworkManager::VpnSetting>();
        if (vpnSetting) {
            qCDebug(gLcNm, "Checking VPN \"%s\" type \"%s\"", qPrintable(connection->name()), qPrintable(vpnSetting->serviceType()));
        }
    }

#if 0
#if WITH_MODEMMANAGER_SUPPORT
    if (connection->settings()->connectionType() == NetworkManager::ConnectionSettings::Gsm) {
        NetworkManager::ModemDevice::Ptr nmModemDevice = NetworkManager::findNetworkInterface(device).objectCast<NetworkManager::ModemDevice>();
        if (nmModemDevice) {
            ModemManager::ModemDevice::Ptr mmModemDevice = ModemManager::findModemDevice(nmModemDevice->udi());
            if (mmModemDevice) {
                ModemManager::Modem::Ptr modem = mmModemDevice->interface(ModemManager::ModemDevice::ModemInterface).objectCast<ModemManager::Modem>();
                NetworkManager::GsmSetting::Ptr gsmSetting = connection->settings()->setting(NetworkManager::Setting::Gsm).staticCast<NetworkManager::GsmSetting>();
                if (gsmSetting && gsmSetting->pinFlags() == NetworkManager::Setting::NotSaved &&
                        modem && modem->unlockRequired() > MM_MODEM_LOCK_NONE) {
                    QDBusInterface managerIface("org.kde.plasmanetworkmanagement", "/org/kde/plasmanetworkmanagement", "org.kde.plasmanetworkmanagement", QDBusConnection::sessionBus(), this);
                    managerIface.call("unlockModem", mmModemDevice->uni());
                    connect(modem.data(), &ModemManager::Modem::unlockRequiredChanged, this, &Handler::unlockRequiredChanged);
                    // m_tmpConnectionPath = connectionPath;
                    // m_tmpDevicePath = device;
                    // m_tmpSpecificPath = specificObject;
                    return;
                }
            }
        }
    }
#endif
#endif
    QDBusPendingReply<QDBusObjectPath> reply = NetworkManager::activateConnection(connectionPath, device, specificObject);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);
    watcher->setProperty("connectionName", connection->name());
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *self) {
        QDBusPendingReply<> reply = *self;

        if (reply.isError() || !reply.isValid()) {
            const QString error = reply.error().message();
            const QString connectionName = self->property("connectionName").toString();
        }

        self->deleteLater();
    });
}

void Networking::addAndActivateConnection(const QString &device, const QString &specificObject, const QString &password)
{
    NetworkManager::AccessPoint::Ptr ap;
    NetworkManager::WirelessDevice::Ptr wifiDev;
    for (const NetworkManager::Device::Ptr &dev : NetworkManager::networkInterfaces()) {
        if (dev->type() == NetworkManager::Device::Wifi) {
            wifiDev = dev.objectCast<NetworkManager::WirelessDevice>();
            ap = wifiDev->findAccessPoint(specificObject);
            if (ap)
                break;
        }
    }

    if (!ap)
        return;

    NetworkManager::ConnectionSettings::Ptr settings = NetworkManager::ConnectionSettings::Ptr(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    settings->setId(ap->ssid());
    settings->setUuid(NetworkManager::ConnectionSettings::createNewUuid());
    settings->setAutoconnect(true);
    settings->addToPermissions(m_userName, QString());

    NetworkManager::WirelessSetting::Ptr wifiSetting = settings->setting(NetworkManager::Setting::Wireless).dynamicCast<NetworkManager::WirelessSetting>();
    wifiSetting->setInitialized(true);
    wifiSetting = settings->setting(NetworkManager::Setting::Wireless).dynamicCast<NetworkManager::WirelessSetting>();
    wifiSetting->setSsid(ap->ssid().toUtf8());
    if (ap->mode() == NetworkManager::AccessPoint::Adhoc) {
        wifiSetting->setMode(NetworkManager::WirelessSetting::Adhoc);
    }

    NetworkManager::WirelessSecuritySetting::Ptr wifiSecurity = settings->setting(NetworkManager::Setting::WirelessSecurity).dynamicCast<NetworkManager::WirelessSecuritySetting>();
    NetworkManager::WirelessSecurityType securityType = NetworkManager::findBestWirelessSecurity(wifiDev->wirelessCapabilities(), true, (ap->mode() == NetworkManager::AccessPoint::Adhoc), ap->capabilities(), ap->wpaFlags(), ap->rsnFlags());

    if (securityType != NetworkManager::NoneSecurity) {
        wifiSecurity->setInitialized(true);
        wifiSetting->setSecurity("802-11-wireless-security");
    }

    if (securityType == NetworkManager::Leap ||
            securityType == NetworkManager::DynamicWep ||
            securityType == NetworkManager::Wpa2Eap ||
            securityType == NetworkManager::WpaEap) {
        if (securityType == NetworkManager::DynamicWep || securityType == NetworkManager::Leap) {
            wifiSecurity->setKeyMgmt(NetworkManager::WirelessSecuritySetting::Ieee8021x);
            if (securityType == NetworkManager::Leap)
                wifiSecurity->setAuthAlg(NetworkManager::WirelessSecuritySetting::Leap);
        } else {
            wifiSecurity->setKeyMgmt(NetworkManager::WirelessSecuritySetting::WpaEap);
        }
        // m_tmpConnectionUuid = settings->uuid();
        // m_tmpDevicePath = device;
        // m_tmpSpecificPath = specificObject;

        // TODO: Edit connection?
    } else {
        if (securityType == NetworkManager::StaticWep) {
            wifiSecurity->setKeyMgmt(NetworkManager::WirelessSecuritySetting::Wep);
            wifiSecurity->setWepKey0(password);
#if 0
            if (KWallet::Wallet::isEnabled())
                wifiSecurity->setWepKeyFlags(NetworkManager::Setting::AgentOwned);
#endif
        } else {
            if (ap->mode() == NetworkManager::AccessPoint::Adhoc)
                wifiSecurity->setKeyMgmt(NetworkManager::WirelessSecuritySetting::WpaNone);
            else
                wifiSecurity->setKeyMgmt(NetworkManager::WirelessSecuritySetting::WpaPsk);
            wifiSecurity->setPsk(password);
#if 0
            if (KWallet::Wallet::isEnabled())
                wifiSecurity->setPskFlags(NetworkManager::Setting::AgentOwned);
#endif
        }

        QDBusPendingReply<QDBusObjectPath> reply = NetworkManager::addAndActivateConnection(settings->toMap(), device, specificObject);
        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);
        watcher->setProperty("connectionName", settings->name());
        connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *self) {
            QDBusPendingReply<> reply = *self;

            if (reply.isError() || !reply.isValid()) {
                const QString error = reply.error().message();
                const QString connectionName = self->property("connectionName").toString();
            }

            self->deleteLater();
        });
    }

    settings.clear();
}

void Networking::deactivateConnection(const QString &connectionName, const QString &device)
{
    NetworkManager::Connection::Ptr connection = NetworkManager::findConnection(connectionName);

    if (!connection) {
        qCWarning(gLcNm, "Failed to deactivate connection \"%s\"", qPrintable(connectionName));
        return;
    }

    QDBusPendingReply<> reply;
    for (const NetworkManager::ActiveConnection::Ptr &active : NetworkManager::activeConnections()) {
        if (active->uuid() == connection->uuid() && ((!active->devices().isEmpty() && active->devices().first() == device) ||
                                                     active->vpn())) {
            if (active->vpn()) {
                reply = NetworkManager::deactivateConnection(active->path());
            } else {
                NetworkManager::Device::Ptr device = NetworkManager::findNetworkInterface(active->devices().first());
                if (device)
                    reply = device->disconnectInterface();
            }
        }
    }

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *self) {
        QDBusPendingReply<> reply = *self;

        if (reply.isError() || !reply.isValid()) {
            const QString error = reply.error().message();
            const QString connectionName = self->property("connectionName").toString();
        }

        self->deleteLater();
    });
}

void Networking::removeConnection(const QString &connectionPath)
{
    NetworkManager::Connection::Ptr con = NetworkManager::findConnection(connectionPath);

    if (!con || con->uuid().isEmpty()) {
        qCWarning(gLcNm) << "Not possible to remove connection " << connectionPath;
        return;
    }

    // Remove slave connections
    for (const NetworkManager::Connection::Ptr &connection : NetworkManager::listConnections()) {
        NetworkManager::ConnectionSettings::Ptr settings = connection->settings();
        if (settings->master() == con->uuid()) {
            connection->remove();
        }
    }

    QDBusPendingReply<> reply = con->remove();
    // QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);
    // watcher->setProperty("action", Networking::RemoveConnection);
    // watcher->setProperty("connection", con->name());
    // connect(watcher, &QDBusPendingCallWatcher::finished, this, [=] (QDBusPendingCallWatcher * watcher) {
    // });
}

Networking::SortedConnectionType Networking::connectionTypeToSortedType(NetworkManager::ConnectionSettings::ConnectionType type)
{
    switch (type) {
    case NetworkManager::ConnectionSettings::Adsl:
        return Networking::Adsl;
        break;
    case NetworkManager::ConnectionSettings::Bluetooth:
        return Networking::Bluetooth;
        break;
    case NetworkManager::ConnectionSettings::Cdma:
        return Networking::Cdma;
        break;
    case NetworkManager::ConnectionSettings::Gsm:
        return Networking::Gsm;
        break;
    case NetworkManager::ConnectionSettings::Infiniband:
        return Networking::Infiniband;
        break;
    case NetworkManager::ConnectionSettings::OLPCMesh:
        return Networking::OLPCMesh;
        break;
    case NetworkManager::ConnectionSettings::Pppoe:
        return Networking::Pppoe;
        break;
    case NetworkManager::ConnectionSettings::Vpn:
        return Networking::Vpn;
        break;
    case NetworkManager::ConnectionSettings::Wired:
        return Networking::Wired;
        break;
    case NetworkManager::ConnectionSettings::Wireless:
        return Networking::Wireless;
        break;
    default:
        return Networking::Other;
        break;
    }
}

void Networking::doChangeActiveConnections()
{
    for (const NetworkManager::ActiveConnection::Ptr &active : NetworkManager::activeConnections()) {
        connect(active.data(), &NetworkManager::ActiveConnection::default4Changed,
                this, &Networking::defaultChanged,
                Qt::UniqueConnection);
        connect(active.data(), &NetworkManager::ActiveConnection::default6Changed,
                this, &Networking::defaultChanged,
                Qt::UniqueConnection);
        connect(active.data(), &NetworkManager::ActiveConnection::stateChanged,
                this, &Networking::changeActiveConnections);
    }

    changeActiveConnections();
}

QString Networking::checkUnknownReason() const
{
    // Check if NetworkManager is running.
    if (!QDBusConnection::systemBus().interface()->isServiceRegistered(QLatin1String(NM_DBUS_INTERFACE)))
        return tr("NetworkManager not running");

    // Check for compatible NetworkManager version.
    if (NetworkManager::compareVersion(0, 9, 8) < 0)
        return tr("NetworkManager 0.9.8 required, found %1").arg(NetworkManager::version());

    return tr("Unknown");
}

void Networking::statusChanged(NetworkManager::Status status)
{
    switch (status) {
    case NetworkManager::ConnectedLinkLocal:
    case NetworkManager::ConnectedSiteOnly:
    case NetworkManager::Connected:
        m_networkStatus = tr("Connected");
        break;
    case NetworkManager::Asleep:
        m_networkStatus = tr("Inactive");
        break;
    case NetworkManager::Disconnected:
        m_networkStatus = tr("Disconnected");
        break;
    case NetworkManager::Disconnecting:
        m_networkStatus = tr("Disconnecting");
        break;
    case NetworkManager::Connecting:
        m_networkStatus = tr("Connecting");
        break;
    default:
        m_networkStatus = checkUnknownReason();
        break;
    }

    if (status == NetworkManager::ConnectedLinkLocal ||
            status == NetworkManager::ConnectedSiteOnly ||
            status == NetworkManager::Connected) {
        changeActiveConnections();
    } else {
        m_activeConnections = m_networkStatus;
        Q_EMIT activeConnectionsChanged();
    }

    Q_EMIT networkStatusChanged();
}

void Networking::changeActiveConnections()
{
    if (NetworkManager::status() != NetworkManager::Connected &&
            NetworkManager::status() != NetworkManager::ConnectedLinkLocal &&
            NetworkManager::status() != NetworkManager::ConnectedSiteOnly)
        return;

    QString activeConnections;
    const QString format = tr("%1: %2");

    QList<NetworkManager::ActiveConnection::Ptr> activeConnectionList = NetworkManager::activeConnections();
    std::sort(activeConnectionList.begin(), activeConnectionList.end(), [](const NetworkManager::ActiveConnection::Ptr &left, const NetworkManager::ActiveConnection::Ptr &right) {
        return Networking::connectionTypeToSortedType(left->type()) < Networking::connectionTypeToSortedType(right->type());
    });

    for (const NetworkManager::ActiveConnection::Ptr &active : activeConnectionList) {
        if (!active->devices().isEmpty() && UiUtils::isConnectionTypeSupported(active->type())) {
            NetworkManager::Device::Ptr device = NetworkManager::findNetworkInterface(active->devices().first());
            if (device && device->type() != NetworkManager::Device::Generic && device->type() <= NetworkManager::Device::Team) {
                bool connecting = false;
                bool connected = false;
                QString conType;
                QString status;
                NetworkManager::VpnConnection::Ptr vpnConnection;

                if (active->vpn()) {
                    conType = tr("VPN");
                    vpnConnection = active.objectCast<NetworkManager::VpnConnection>();
                } else {
                    conType = UiUtils::interfaceTypeLabel(device->type(), device);
                }

                if (vpnConnection && active->vpn()) {
                    if (vpnConnection->state() >= NetworkManager::VpnConnection::Prepare &&
                            vpnConnection->state() <= NetworkManager::VpnConnection::GettingIpConfig)
                        connecting = true;
                    else if (vpnConnection->state() == NetworkManager::VpnConnection::Activated)
                        connected = true;
                } else {
                    if (active->state() == NetworkManager::ActiveConnection::Activated)
                        connected = true;
                    else if (active->state() == NetworkManager::ActiveConnection::Activating)
                        connecting = true;
                }

                NetworkManager::Connection::Ptr connection = active->connection();
                if (connecting) {
                    status = tr("Connecting to %1").arg(connection->name());
                } else if (connected) {
                    status = tr("Connected to %1").arg(connection->name());
                }

                if (!activeConnections.isEmpty())
                    activeConnections += QLatin1Char('\n');
                activeConnections += format.arg(conType, status);

                connect(connection.data(), &NetworkManager::Connection::updated,
                        this, &Networking::changeActiveConnections,
                        Qt::UniqueConnection);
            }
        }
    }

    m_activeConnections = activeConnections;
    Q_EMIT activeConnectionsChanged();
}

void Networking::defaultChanged()
{
    statusChanged(NetworkManager::status());
}
