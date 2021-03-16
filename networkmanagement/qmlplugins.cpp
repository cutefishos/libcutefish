#include "qmlplugins.h"
#include "networking.h"
#include "networkmodel.h"
#include "networkmodelitem.h"
#include "appletproxymodel.h"
#include "technologyproxymodel.h"
#include "wiressitemsettings.h"
#include "connectionicon.h"
#include "network.h"

#include <QQmlEngine>

void QmlPlugins::registerTypes(const char* uri)
{
    qmlRegisterUncreatableType<NetworkModelItem>(uri, 1, 0, "NetworkModelItem",
                                                QLatin1String("Cannot instantiate NetworkModelItem"));
    qmlRegisterType<AppletProxyModel>(uri, 1, 0, "AppletProxyModel");
    qmlRegisterType<Networking>(uri, 1, 0, "Networking");
    qmlRegisterType<NetworkModel>(uri, 1, 0, "NetworkModel");
    qmlRegisterType<TechnologyProxyModel>(uri, 1, 0, "TechnologyProxyModel");
    qmlRegisterType<WirelessItemSettings>(uri, 1, 0, "WirelessItemSettings");
    qmlRegisterType<ConnectionIcon>(uri, 1, 0, "ConnectionIcon");
    qmlRegisterType<Network>(uri, 1, 0, "Network");
}