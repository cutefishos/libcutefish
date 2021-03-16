#ifndef NETWORK_H
#define NETWORK_H

#include <QObject>
#include <NetworkManagerQt/Device>
#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/WirelessNetwork>

class Network : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(bool wirelessEnabled READ isWirelessEnabled WRITE setWirelessEnabled NOTIFY wirelessEnabledChanged)
    Q_PROPERTY(bool wirelessHardwareEnabled READ isWirelessHardwareEnabled NOTIFY wirelessHardwareEnabledChanged)
    Q_PROPERTY(QString wirelessConnectionName READ wirelessConnectionName NOTIFY wirelessConnectionNameChanged)
    Q_PROPERTY(QString wirelessIconName READ wirelessIconName NOTIFY wirelessIconNameChanged)

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

    explicit Network(QObject *parent = nullptr);

    bool isEnabled() const;
    void setEnabled(bool enabled);

    bool isWirelessEnabled() const;
    void setWirelessEnabled(bool enabled);

    bool isWirelessHardwareEnabled() const;

    QString wirelessConnectionName() const;
    QString wirelessIconName() const;

    static SortedConnectionType connectionTypeToSortedType(NetworkManager::ConnectionSettings::ConnectionType type);

signals:
    void enabledChanged();
    void wirelessEnabledChanged();
    void wirelessHardwareEnabledChanged();
    void wirelessConnectionNameChanged();
    void wirelessIconNameChanged();

private slots:
    void statusChanged(NetworkManager::Status status);
    void changeActiveConnections();
    void activatingConnectionChanged();
    void defaultChanged();

    void onWirelessEnabledChanged();
    void onWirelessNetworkAppeared(const QString& network);

    void updateIcons();
    void updateWirelessIcon(const NetworkManager::Device::Ptr &device, const QString& ssid);
    void updateWirelessIconForSignalStrength(int strength);

    void resetWireless();

private:
    QString m_userName;
    QString m_wirelessConnectionName;
    QString m_wirelessIconName;
    NetworkManager::WirelessNetwork::Ptr m_wirelessNetwork;
};

#endif
