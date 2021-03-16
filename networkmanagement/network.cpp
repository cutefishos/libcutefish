#include "network.h"
#include "uiutils.h"
#include <QDebug>

#include <sys/types.h>
#include <pwd.h>

Network::Network(QObject *parent)
    : QObject(parent)
    , m_wirelessNetwork(nullptr)
{
    ::passwd *pw = ::getpwuid(::getuid());
    m_userName = QString::fromLocal8Bit(pw->pw_name);

    connect(NetworkManager::notifier(), &NetworkManager::Notifier::networkingEnabledChanged, this, &Network::enabledChanged);
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::wirelessEnabledChanged, this, &Network::onWirelessEnabledChanged);
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::wirelessHardwareEnabledChanged, this, &Network::wirelessHardwareEnabledChanged);
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::statusChanged, this, &Network::statusChanged);
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::activatingConnectionChanged, this, &Network::activatingConnectionChanged);

    for (const NetworkManager::Device::Ptr &device : NetworkManager::networkInterfaces()) {
        if (device->type() == NetworkManager::Device::Wifi) {
            NetworkManager::WirelessDevice::Ptr wifiDevice = device.staticCast<NetworkManager::WirelessDevice>();
            if (wifiDevice) {
                connect(wifiDevice.data(), &NetworkManager::WirelessDevice::availableConnectionAppeared, this, &Network::onWirelessNetworkAppeared);
                connect(wifiDevice.data(), &NetworkManager::WirelessDevice::networkAppeared, this, &Network::onWirelessNetworkAppeared);
            }
        }
    }

    statusChanged(NetworkManager::status());
    changeActiveConnections();
    updateIcons();
}

bool Network::isEnabled() const
{
    return NetworkManager::isNetworkingEnabled();
}

void Network::setEnabled(bool enabled)
{
    NetworkManager::setNetworkingEnabled(enabled);
}

bool Network::isWirelessEnabled() const
{
    return NetworkManager::isWirelessEnabled();
}

void Network::setWirelessEnabled(bool enabled)
{
    NetworkManager::setWirelessEnabled(enabled);
}

bool Network::isWirelessHardwareEnabled() const
{
    return NetworkManager::isWirelessHardwareEnabled();
}

QString Network::wirelessConnectionName() const
{
    return m_wirelessConnectionName;
}

QString Network::wirelessIconName() const
{
    return m_wirelessIconName;
}

Network::SortedConnectionType Network::connectionTypeToSortedType(NetworkManager::ConnectionSettings::ConnectionType type)
{
    switch (type) {
    case NetworkManager::ConnectionSettings::Adsl:
        return Network::Adsl;
        break;
    case NetworkManager::ConnectionSettings::Bluetooth:
        return Network::Bluetooth;
        break;
    case NetworkManager::ConnectionSettings::Cdma:
        return Network::Cdma;
        break;
    case NetworkManager::ConnectionSettings::Gsm:
        return Network::Gsm;
        break;
    case NetworkManager::ConnectionSettings::Infiniband:
        return Network::Infiniband;
        break;
    case NetworkManager::ConnectionSettings::OLPCMesh:
        return Network::OLPCMesh;
        break;
    case NetworkManager::ConnectionSettings::Pppoe:
        return Network::Pppoe;
        break;
    case NetworkManager::ConnectionSettings::Vpn:
        return Network::Vpn;
        break;
    case NetworkManager::ConnectionSettings::Wired:
        return Network::Wired;
        break;
    case NetworkManager::ConnectionSettings::Wireless:
        return Network::Wireless;
        break;
    default:
        return Network::Other;
        break;
    }
}

void Network::statusChanged(NetworkManager::Status status)
{
    if (status == NetworkManager::ConnectedLinkLocal ||
            status == NetworkManager::ConnectedSiteOnly ||
            status == NetworkManager::Connected) {
        changeActiveConnections();
    }

    if (status == NetworkManager::Disconnected) {
        resetWireless();
    }
}

void Network::changeActiveConnections()
{
    if (NetworkManager::status() != NetworkManager::Connected &&
        NetworkManager::status() != NetworkManager::ConnectedLinkLocal &&
        NetworkManager::status() != NetworkManager::ConnectedSiteOnly) {

        m_wirelessConnectionName = "";
        emit wirelessConnectionNameChanged();

        return;
    }

    QString activeConnections;
    QList<NetworkManager::ActiveConnection::Ptr> activeConnectionList = NetworkManager::activeConnections();
    std::sort(activeConnectionList.begin(), activeConnectionList.end(), [](const NetworkManager::ActiveConnection::Ptr &left, const NetworkManager::ActiveConnection::Ptr &right) {
        return Network::connectionTypeToSortedType(left->type()) < Network::connectionTypeToSortedType(right->type());
    });

    for (const NetworkManager::ActiveConnection::Ptr &active : activeConnectionList) {
        if (active->devices().isEmpty() && !UiUtils::isConnectionTypeSupported(active->type()))
            continue;

        NetworkManager::Device::Ptr device = NetworkManager::findNetworkInterface(active->devices().first());
        NetworkManager::Connection::Ptr connection = active->connection();
        bool connecting = false;
        bool connected = false;

        if (active->state() == NetworkManager::ActiveConnection::Activated)
            connected = true;
        else if (active->state() == NetworkManager::ActiveConnection::Activating)
            connecting = true;

        if (device->type() == NetworkManager::Device::Wifi) {
            if (m_wirelessConnectionName != connection->name()) {
                m_wirelessConnectionName = connection->name();
                emit wirelessConnectionNameChanged();

                updateIcons();
            }
        }

        connect(connection.data(), &NetworkManager::Connection::updated, this, &Network::changeActiveConnections, Qt::UniqueConnection);
    }
}

void Network::activatingConnectionChanged()
{
    updateIcons();
}

void Network::defaultChanged()
{
    statusChanged(NetworkManager::status());
}

void Network::onWirelessEnabledChanged()
{
    if (!isWirelessEnabled()) {
        resetWireless();
    } else {
        updateIcons();
    }

    emit wirelessEnabledChanged();
}

void Network::onWirelessNetworkAppeared(const QString& network)
{
    Q_UNUSED(network);

    updateIcons();
}

void Network::updateIcons()
{
    if (m_wirelessNetwork) {
        disconnect(m_wirelessNetwork.data(), nullptr, this, nullptr);
        m_wirelessNetwork.clear();
    }

    NetworkManager::ActiveConnection::Ptr connection = NetworkManager::activatingConnection();

    // Set icon based on the current primary connection if the activating connection is virtual
    // since we're not setting icons for virtual connections
    if (!connection
        || (connection && UiUtils::isConnectionTypeVirtual(connection->type()))
        || connection->type() == NetworkManager::ConnectionSettings::WireGuard) {
        connection = NetworkManager::primaryConnection();
    }

    /* Fallback: If we still don't have an active connection with default route or the default route goes through a connection
                 of generic type (some type of VPNs) we need to go through all other active connections and pick the one with
                 highest probability of being the main one (order is: vpn, wired, wireless, gsm, cdma, bluetooth) */
    if ((!connection && !NetworkManager::activeConnections().isEmpty()) || (connection && connection->type() == NetworkManager::ConnectionSettings::Generic)
                                                                        || (connection && connection->type() == NetworkManager::ConnectionSettings::Tun)) {
        for (const NetworkManager::ActiveConnection::Ptr &activeConnection : NetworkManager::activeConnections()) {
            const NetworkManager::ConnectionSettings::ConnectionType type = activeConnection->type();
            if (type == NetworkManager::ConnectionSettings::Bluetooth) {
                if (connection && connection->type() <= NetworkManager::ConnectionSettings::Bluetooth) {
                    connection = activeConnection;
                }
            } else if (type == NetworkManager::ConnectionSettings::Cdma) {
                if (connection && connection->type() <= NetworkManager::ConnectionSettings::Cdma) {
                    connection = activeConnection;
                }
            } else if (type == NetworkManager::ConnectionSettings::Gsm) {
                if (connection && connection->type() <= NetworkManager::ConnectionSettings::Gsm) {
                    connection = activeConnection;
                }
            } else if (type == NetworkManager::ConnectionSettings::Vpn) {
                connection = activeConnection;
            } else if (type == NetworkManager::ConnectionSettings::WireGuard) {
                connection = activeConnection;
            } else if (type == NetworkManager::ConnectionSettings::Wired) {
                if (connection && (connection->type() != NetworkManager::ConnectionSettings::Vpn
                                  || connection->type() != NetworkManager::ConnectionSettings::WireGuard)) {
                    connection = activeConnection;
                }
            } else if (type == NetworkManager::ConnectionSettings::Wireless) {
                if (connection && (connection->type() != NetworkManager::ConnectionSettings::Vpn &&
                                  (connection->type() != NetworkManager::ConnectionSettings::Wired))) {
                    connection = activeConnection;
                }
            }
        }
    }

    if (connection && !connection->devices().isEmpty()) {
        NetworkManager::Device::Ptr device = NetworkManager::findNetworkInterface(connection->devices().first());

        if (device) {
            NetworkManager::Device::Type type = device->type();

            if (type == NetworkManager::Device::Wifi) {
                NetworkManager::WirelessDevice::Ptr wifiDevice = device.objectCast<NetworkManager::WirelessDevice>();

                if (wifiDevice->mode() == NetworkManager::WirelessDevice::Adhoc) {
                    updateWirelessIconForSignalStrength(100);
                } else {
                    NetworkManager::AccessPoint::Ptr ap = wifiDevice->activeAccessPoint();
                    if (ap) {
                        updateWirelessIcon(device, ap->ssid());
                    }
                }
            } else if (type == NetworkManager::Device::Ethernet) {

            } else {
                resetWireless();
            }
        } else {
            resetWireless();
        }
    }
}

void Network::updateWirelessIcon(const NetworkManager::Device::Ptr &device, const QString& ssid)
{
    NetworkManager::WirelessDevice::Ptr wirelessDevice = device.objectCast<NetworkManager::WirelessDevice>();

    if (device) {
        m_wirelessNetwork = wirelessDevice->findNetwork(ssid);
    } else {
        m_wirelessNetwork.clear();
    }

    if (m_wirelessNetwork) {
        connect(m_wirelessNetwork.data(), &NetworkManager::WirelessNetwork::signalStrengthChanged, this, &Network::updateWirelessIconForSignalStrength, Qt::UniqueConnection);
        updateWirelessIconForSignalStrength(m_wirelessNetwork->signalStrength());
    } else {
        resetWireless();
    }
}

void Network::updateWirelessIconForSignalStrength(int strength)
{
    int iconStrength = 0;

    if (strength == 0)
        iconStrength = 0;
    else if (strength <= 25)
        iconStrength = 25;
    else if (strength <= 50)
        iconStrength = 50;
    else if (strength <= 75)
        iconStrength = 75;
    else if (strength <= 100)
        iconStrength = 100;

    m_wirelessIconName = QString("network-wireless-connected-%1").arg(iconStrength);
    emit wirelessIconNameChanged();
}

void Network::resetWireless()
{
    m_wirelessConnectionName = "";
    m_wirelessIconName = "";

    emit wirelessConnectionNameChanged();
    emit wirelessIconNameChanged();
}
