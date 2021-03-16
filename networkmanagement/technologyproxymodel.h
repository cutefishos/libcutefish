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

#ifndef TECHNOLOGYPROXYMODEL_H
#define TECHNOLOGYPROXYMODEL_H

#include <networkmanager_export.h>

#include <QtCore/QSortFilterProxyModel>

#include <NetworkManagerQt/ConnectionSettings>

class NETWORKMANAGER_EXPORT TechnologyProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(Type type READ type WRITE setType NOTIFY typeChanged)
    Q_PROPERTY(bool showInactiveConnections READ showInactiveConnections WRITE setShowInactiveConnections NOTIFY showInactiveConnectionsChanged)
    Q_PROPERTY(QAbstractItemModel *sourceModel READ sourceModel WRITE setSourceModel)

public:
    enum Type {
        UnknownType = 0,
        AdslType,
        BluetoothType,
        CdmaType,
        GsmType,
        OLPCMeshType,
        PppoeType,
        VpnType,
        WimaxType,
        WiredType,
        WirelessType
    };
    Q_ENUM(Type)

    TechnologyProxyModel(QObject *parent = nullptr);

    Type type() const;
    void setType(Type type);

    bool showInactiveConnections() const;
    void setShowInactiveConnections(bool value);

Q_SIGNALS:
    void typeChanged();
    void showInactiveConnectionsChanged();

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
    Type m_type = UnknownType;
    bool m_showInactiveConnections = false;
};

#endif //TECHNOLOGYPROXYMODEL_H
