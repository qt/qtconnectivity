/*
 * This file was generated by qdbusxml2cpp version 0.7
 * Command line was: qdbusxml2cpp -p manager_p.h:manager.cpp org.bluez.Manager.xml org.bluez.Manager
 *
 * qdbusxml2cpp is Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
 *
 * This is an auto-generated file.
 * Do not edit! All changes made to it will be lost.
 */

#ifndef MANAGER_P_H_1273205927
#define MANAGER_P_H_1273205927

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>

/*
 * Proxy class for interface org.bluez.Manager
 */
class OrgBluezManagerInterface: public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName()
    { return "org.bluez.Manager"; }

public:
    OrgBluezManagerInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = 0);

    ~OrgBluezManagerInterface();

public Q_SLOTS: // METHODS
    inline QDBusPendingReply<QDBusObjectPath> DefaultAdapter()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QLatin1String("DefaultAdapter"), argumentList);
    }

    inline QDBusPendingReply<QDBusObjectPath> FindAdapter(const QString &in0)
    {
        QList<QVariant> argumentList;
        argumentList << qVariantFromValue(in0);
        return asyncCallWithArgumentList(QLatin1String("FindAdapter"), argumentList);
    }

    inline QDBusPendingReply<QList<QDBusObjectPath> > ListAdapters()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QLatin1String("ListAdapters"), argumentList);
    }

Q_SIGNALS: // SIGNALS
    void AdapterAdded(const QDBusObjectPath &in0);
    void AdapterRemoved(const QDBusObjectPath &in0);
    void DefaultAdapterChanged(const QDBusObjectPath &in0);
};

namespace org {
  namespace bluez {
    typedef ::OrgBluezManagerInterface Manager;
  }
}
#endif
 
