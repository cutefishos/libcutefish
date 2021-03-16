
/****************************************************************************
 *
 * Copyright (C) 2017 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * $BEGIN_LICENSE:LGPLv3+$
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * $END_LICENSE$
 ***************************************************************************/

#include "networkmodel.h"
#include "networkmodelitem.h"
#include "technologyproxymodel.h"

static NetworkManager::ConnectionSettings::ConnectionType convertType(TechnologyProxyModel::Type type)
{
    switch (type) {
    case TechnologyProxyModel::UnknownType:
        return NetworkManager::ConnectionSettings::ConnectionType::Unknown;
    case TechnologyProxyModel::AdslType:
        return NetworkManager::ConnectionSettings::ConnectionType::Adsl;
    case TechnologyProxyModel::BluetoothType:
        return NetworkManager::ConnectionSettings::ConnectionType::Bluetooth;
    case TechnologyProxyModel::CdmaType:
        return NetworkManager::ConnectionSettings::ConnectionType::Cdma;
    case TechnologyProxyModel::GsmType:
        return NetworkManager::ConnectionSettings::ConnectionType::Gsm;
    case TechnologyProxyModel::OLPCMeshType:
        return NetworkManager::ConnectionSettings::ConnectionType::OLPCMesh;
    case TechnologyProxyModel::PppoeType:
        return NetworkManager::ConnectionSettings::ConnectionType::Pppoe;
    case TechnologyProxyModel::VpnType:
        return NetworkManager::ConnectionSettings::ConnectionType::Vpn;
    case TechnologyProxyModel::WimaxType:
        return NetworkManager::ConnectionSettings::ConnectionType::Wimax;
    case TechnologyProxyModel::WiredType:
        return NetworkManager::ConnectionSettings::ConnectionType::Wired;
    case TechnologyProxyModel::WirelessType:
        return NetworkManager::ConnectionSettings::ConnectionType::Wireless;
    default:
        break;
    }

    return NetworkManager::ConnectionSettings::ConnectionType::Unknown;
}

TechnologyProxyModel::TechnologyProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
    setSortCaseSensitivity(Qt::CaseInsensitive);
    setSortLocaleAware(true);
    sort(0, Qt::DescendingOrder);
}

TechnologyProxyModel::Type TechnologyProxyModel::type() const
{
    return m_type;
}

void TechnologyProxyModel::setType(Type type)
{
    if (m_type == type)
        return;
    m_type = type;
    Q_EMIT typeChanged();

    if (type == UnknownType)
        setFilterRole(0);
    else
        setFilterRole(NetworkModel::TypeRole);
}

bool TechnologyProxyModel::showInactiveConnections() const
{
    return m_showInactiveConnections;
}

void TechnologyProxyModel::setShowInactiveConnections(bool value)
{
    if (m_showInactiveConnections == value)
        return;

    m_showInactiveConnections = value;
    Q_EMIT showInactiveConnectionsChanged();
}

bool TechnologyProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    const QModelIndex index = sourceModel()->index(source_row, 0, source_parent);

    // Filter out slaves and duplicates
    const bool isSlave = sourceModel()->data(index, NetworkModel::SlaveRole).toBool();
    const bool isDuplicate = sourceModel()->data(index, NetworkModel::DuplicateRole).toBool();
    if (isSlave || isDuplicate)
        return false;

    // Connection and item type
    const NetworkManager::ConnectionSettings::ConnectionType type =
            static_cast<NetworkManager::ConnectionSettings::ConnectionType>(sourceModel()->data(index, NetworkModel::TypeRole).toUInt());
    const NetworkModelItem::ItemType itemType =
            static_cast<NetworkModelItem::ItemType>(sourceModel()->data(index, NetworkModel::ItemTypeRole).toUInt());

    // Filter-out certain connection types we are not interested in
    if (m_type != UnknownType) {
        if (type != convertType(m_type))
            return false;
    }

    // Filter-out access points
    if (itemType != NetworkModelItem::AvailableConnection &&
            itemType != NetworkModelItem::AvailableAccessPoint)
        return false;

    // Filter by state
    const NetworkManager::ActiveConnection::State state =
            static_cast<NetworkManager::ActiveConnection::State>(sourceModel()->data(index, NetworkModel::ConnectionStateRole).toUInt());
    if (!m_showInactiveConnections && state == NetworkManager::ActiveConnection::Deactivated)
        return false;

    // Filter on connection name
    const QString pattern = filterRegExp().pattern();
    if (!pattern.isEmpty()) {
        QString data = sourceModel()->data(index, Qt::DisplayRole).toString();
        if (data.isEmpty())
            data = sourceModel()->data(index, NetworkModel::NameRole).toString();
        return data.contains(pattern, Qt::CaseInsensitive);
    }

    return true;
}
