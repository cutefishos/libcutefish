#include "qmlplugins.h"
#include "networkmodel.h"
#include "networkmodelitem.h"
#include "appletproxymodel.h"
#include "technologyproxymodel.h"
#include "wiressitemsettings.h"
#include "connectionicon.h"
#include "network.h"
#include "identitymodel.h"
#include "handler.h"
#include "enabledconnections.h"
#include "enums.h"
#include "wifisettings.h"

#include <QQmlEngine>

void QmlPlugins::registerTypes(const char* uri)
{
    qmlRegisterUncreatableType<NetworkModelItem>(uri, 1, 0, "NetworkModelItem",
                                                QLatin1String("Cannot instantiate NetworkModelItem"));
    qmlRegisterType<AppletProxyModel>(uri, 1, 0, "AppletProxyModel");
    qmlRegisterType<NetworkModel>(uri, 1, 0, "NetworkModel");
    qmlRegisterType<TechnologyProxyModel>(uri, 1, 0, "TechnologyProxyModel");
    qmlRegisterType<WirelessItemSettings>(uri, 1, 0, "WirelessItemSettings");
    qmlRegisterType<ConnectionIcon>(uri, 1, 0, "ConnectionIcon");
    qmlRegisterType<Network>(uri, 1, 0, "Network");
    qmlRegisterType<IdentityModel>(uri, 1, 0, "IdentityModel");
    qmlRegisterType<Handler>(uri, 1, 0, "Handler");
    qmlRegisterType<EnabledConnections>(uri, 1, 0, "EnabledConnections");
    qmlRegisterType<WifiSettings>(uri, 1, 0, "WifiSettings");
    qmlRegisterUncreatableType<Enums>(uri, 1, 0, "Enums", "You cannot create Enums on yourself");
}