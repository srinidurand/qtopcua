/****************************************************************************
**
** Copyright (C) 2015 basysKom GmbH, opensource@basyskom.com
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtOpcUa module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qopcuanode.h"
#include "qopcuanode_p.h"
#include "qopcuanodeimpl_p.h"
#include "qopcuaclient.h"
#include "qopcuamonitoredvalue.h"

#include <private/qopcuaclientimpl_p.h>
#include <private/qopcuaclient_p.h>
#include <private/qopcuamonitoredvalue_p.h>
#include <private/qopcuamonitoredevent_p.h>

QT_BEGIN_NAMESPACE

/*!
    \class QOpcUaNode
    \inmodule QtOpcUa

    \brief QOpcUaNode is a base class for plugin specific node objects which\
    allows access to attributes of an OPC UA node.

    The node is the basic building block of the OPC UA address space.
    It has attributes like browse name, value, associated properties and can have
    references to other nodes in the address space.
    Nodes are organized in namespaces and have IDs which can e.g. be numeric,
    a string, a namespace specific format (/opaque) or a globally unique identifier.
    A node is identified by the namespace id and the node ID.
    This identifier is usually given as a string:
    The identifier of a node residing in namespace 0 and having the numeric
    identifier 42, the string is \c ns=0;i=42, a node with a string
    identifier can be addressed via \c ns=0;s=myStringIdentifier.

    The node attributes are read from the server when the getter methods are
    called.

    Objects of this type are owned by the user and must be deleted when they are
    no longer needed.

    \section1 Working with Subscriptions
    Subscriptions are a concept in OPC UA to enable updating values by data
    changes or events instead of polling.
    The subscription has an interval in which it checks for value changes and
    sends updates to the client. For an event monitored item, there is no interval.

    Monitored Items are used to assign the node and the node's attribute or the
    event we want to keep up with to the subscription.

    \image subscriptions.png

    After calling valueMonitor() or eventMonitor(), a QOpcUaMonitoredValue or
    QOpcUaMonitoredEvent is returned.
    These objects emit a signal on a data change or event,
    depending on which type of subscription has been requested from the server.
*/

/*!
    \enum QOpcUaNode::NodeClass

    This enum specifies the class a node belongs to. OPC-UA specifies a fixed
    set of eight different classes.

    \value Undefined     The node class is not known. This is the case before
                         the NodeClass attribute has been read on the server.

    \value Object        An Object node.
    \value Variable      A Variable node.
    \value Method        A Method node.
    \value ObjectType    An ObjectType node.
    \value VariableType  A VariableType node.
    \value ReferenceType A ReferenceType node.
    \value DataType      A DataType node.
    \value View          A View node.
*/

/*!
    \internal QOpcUaNodeImpl is an opaque type (as seen from the public API).
    This prevents users of the public API to use this constructor (eventhough
    it is public).
*/
QOpcUaNode::QOpcUaNode(QOpcUaNodeImpl *impl, QOpcUaClient *client, QObject *parent)
    : QObject(*new QOpcUaNodePrivate(impl, client), parent)
{
}

QOpcUaNode::~QOpcUaNode()
{
}

/*!
    The DisplayName attribute of the OPC UA node.
*/
QString QOpcUaNode::displayName() const
{
    // m_client.isNull() is checked bc. someone might have deleted the client
    // leaving a dangling reference here
    if (d_func()->m_client.isNull() || d_func()->m_client->state() != QOpcUaClient::Connected)
        return QString();

    return d_func()->m_impl->displayName();
}

/*!
    The type attribute of the OPC UA node.
 */
QOpcUa::Types QOpcUaNode::type() const
{
    if (d_func()->m_client.isNull() || d_func()->m_client->state() != QOpcUaClient::Connected)
        return QOpcUa::Types::Undefined;

    return d_func()->m_impl->type();
}

/*!
    The value attribute of the OPC UA node.

    \sa setValue()
*/
QVariant QOpcUaNode::value() const
{
    if (d_func()->m_client.isNull() || d_func()->m_client->state() != QOpcUaClient::Connected)
        return QVariant();

    return d_func()->m_impl->value();
}

/*!
   QStringList filled with the node IDs of all child nodes of the OPC UA node.
*/
QStringList QOpcUaNode::childrenIds() const
{
    if (d_func()->m_client.isNull() || d_func()->m_client->state() != QOpcUaClient::Connected)
        return QStringList();

    return d_func()->m_impl->childrenIds();
}

/*!
    The ID of the OPC UA node.
*/
QString QOpcUaNode::nodeId() const
{
    if (d_func()->m_client.isNull() || d_func()->m_client->state() != QOpcUaClient::Connected)
        return QString();

    return d_func()->m_impl->nodeId();
}

/*!
   The node class of the OPC UA node.
*/
QOpcUaNode::NodeClass QOpcUaNode::nodeClass() const
{
    if (d_func()->m_client.isNull() || d_func()->m_client->state() != QOpcUaClient::Connected)
        return NodeClass::Undefined;

    return d_func()->m_impl->nodeClass();
}

/*!
    Writes the value given in \a value to the OPC UA node.
    The data type must be supplied in \a type.
    true is returned in case of success, false is returned when the
    write call fails.

    \sa value()
  */
bool QOpcUaNode::setValue(const QVariant &value, QOpcUa::Types type)
{
    if (d_func()->m_client.isNull() || d_func()->m_client->state() != QOpcUaClient::Connected)
        return false;

    return d_func()->m_impl->setValue(value, type);
}

/*!
    Reads value range from the OPC UA node and
    returns it as a pair of doubles containing the lower and upper limit.

    NaN values are returned if the read has failed or if the plugin does not
    support reading EURanges from the server.

    \warning Currently not supported by the FreeOPCUA backend
*/
QPair<double, double> QOpcUaNode::readEuRange() const
{
    if (d_func()->m_client.isNull() || d_func()->m_client->state() != QOpcUaClient::Connected)
        return QPair<double, double>(qQNaN(), qQNaN());

    return d_func()->m_impl->readEuRange();
}

/*!
    Reads Engineering unit information from the OPC UA node and returns it
    as a pair of strings containing the unit and its description.

    \warning Currently not supported by the FreeOPCUA backend
*/
QPair<QString, QString> QOpcUaNode::readEui() const
{
    if (d_func()->m_client.isNull() || d_func()->m_client->state() != QOpcUaClient::Connected)
        return QPair<QString, QString>();

    return d_func()->m_impl->readEui();
}

/*!
    Reads up to \a maxCount points of historical data from the OPC UA server
    for the node between the two timestamps \a begin and \a end.

    \sa writeHistorical()
*/
QVector<QPair<QVariant, QDateTime> > QOpcUaNode::readHistorical(
        uint maxCount, const QDateTime &begin, const QDateTime &end) const
{
    if (d_func()->m_client.isNull() || d_func()->m_client->state() != QOpcUaClient::Connected)
        return QVector<QPair<QVariant, QDateTime> >();

    return d_func()->m_impl->readHistorical(maxCount, begin, end);
}

/*!
    Write historical \a data from the OPC UA server of \a type QOpcUa::Types.

    Returns whether operation was successful.

    \sa readHistorical()
*/
bool QOpcUaNode::writeHistorical(QOpcUa::Types type,
        const QVector<QPair<QVariant, QDateTime> > data)
{
    Q_D(QOpcUaNode);
    if (d->m_client.isNull() || d_func()->m_client->state() != QOpcUaClient::Connected)
        return false;

    return d->m_impl->writeHistorical(type, data);
}

/*!
    Calls the OPC UA method \a methodNodeId with the parameters given via \a args. The result is
    returned in \a ret. The success of the service call is returned by the method.
*/
bool QOpcUaNode::call(const QString &methodNodeId,
        QVector<QOpcUa::TypedVariant> *args, QVector<QVariant> *ret)
{
    Q_D(QOpcUaNode);
    if (d->m_client.isNull() || d_func()->m_client->state() != QOpcUaClient::Connected)
        return false;

    return d->m_impl->call(methodNodeId, args, ret);
}

QDebug operator<<(QDebug dbg, const QOpcUaNode &node)
{
    dbg << "QOpcUaNode {"
        << "DisplayName:" << node.displayName()
        << "Id:" << node.nodeId()
        << "Class:" << node.nodeClass()
        << "}";
    return dbg;
}

QT_END_NAMESPACE
