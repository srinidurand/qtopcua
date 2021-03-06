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

#include "qfreeopcuanode.h"

#include "qfreeopcuaclient.h"
#include "qfreeopcuasubscription.h"
#include "qfreeopcuavalueconverter.h"

#include <QtOpcUa/qopcuamonitoredvalue.h>
#include <QtOpcUa/qopcuamonitoredevent.h>

#include <QtCore/qdatetime.h>
#include <QtCore/qdebug.h>
#include <QtCore/qloggingcategory.h>

#include <opc/ua/client/client.h>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_OPCUA_PLUGINS_FREEOPCUA)

QFreeOpcUaNode::QFreeOpcUaNode(OpcUa::Node node, OpcUa::UaClient *client)
    : m_node(node)
    , m_client(client)
{
}

QFreeOpcUaNode::~QFreeOpcUaNode()
{
}

QString QFreeOpcUaNode::displayName() const
{
    try {
        return QString::fromStdString(m_node.GetAttribute(
                                          OpcUa::AttributeId::DisplayName).Value.As<OpcUa::LocalizedText>().Text);
    } catch (const std::exception &ex) {
        qCWarning(QT_OPCUA_PLUGINS_FREEOPCUA) << "Failed to get BrowseName for node: " << ex.what();
    }
    return QString();
}

QOpcUa::Types QFreeOpcUaNode::type() const
{
    const OpcUa::Variant value = m_node.GetValue();
    switch (value.Type()) {
    case OpcUa::VariantType::BOOLEAN:
        return QOpcUa::Types::Boolean;
    case OpcUa::VariantType::SBYTE:
        return QOpcUa::Types::SByte;
    case OpcUa::VariantType::BYTE:
        return QOpcUa::Types::Byte;
    case OpcUa::VariantType::INT16:
        return QOpcUa::Types::Int16;
    case OpcUa::VariantType::UINT16:
        return QOpcUa::Types::UInt16;
    case OpcUa::VariantType::INT32:
        return QOpcUa::Types::Int32;
    case OpcUa::VariantType::UINT32:
        return QOpcUa::Types::UInt32;
    case OpcUa::VariantType::INT64:
        return QOpcUa::Types::Int64;
    case OpcUa::VariantType::UINT64:
        return QOpcUa::Types::UInt64;
    case OpcUa::VariantType::FLOAT:
        return QOpcUa::Types::Float;
    case OpcUa::VariantType::DOUBLE:
        return QOpcUa::Types::Double;
    case OpcUa::VariantType::STRING:
        return QOpcUa::Types::String;
    case OpcUa::VariantType::DATE_TIME:
        return QOpcUa::Types::DateTime;
    case OpcUa::VariantType::BYTE_STRING:
        return QOpcUa::Types::ByteString;
    case OpcUa::VariantType::NODE_Id:
        return QOpcUa::Types::NodeId;
    case OpcUa::VariantType::LOCALIZED_TEXT:
        return QOpcUa::Types::LocalizedText;
    case OpcUa::VariantType::GUId:
        return QOpcUa::Types::Guid;

    case OpcUa::VariantType::EXPANDED_NODE_Id:
    case OpcUa::VariantType::STATUS_CODE:
    case OpcUa::VariantType::QUALIFIED_NAME:
    case OpcUa::VariantType::XML_ELEMENT:
    case OpcUa::VariantType::NUL:
    case OpcUa::VariantType::EXTENSION_OBJECT:
    case OpcUa::VariantType::DATA_VALUE:
    case OpcUa::VariantType::VARIANT:
    case OpcUa::VariantType::DIAGNOSTIC_INFO:
        qCWarning(QT_OPCUA_PLUGINS_FREEOPCUA) << "Type resolution failed for " << (int)value.Type();
        break;
    }

    return QOpcUa::Types::Undefined;
}

QVariant QFreeOpcUaNode::value() const
{
    try {
        OpcUa::Variant val = m_node.GetValue();
        if (val.IsNul())
            return QVariant();
        else
            return QFreeOpcUaValueConverter::toQVariant(val);
    } catch (const std::exception &ex) {
        qCWarning(QT_OPCUA_PLUGINS_FREEOPCUA) << ex.what();
        return QVariant();
    }
}

QStringList QFreeOpcUaNode::childrenIds() const
{
    QStringList result;
    try {
        std::vector<OpcUa::Node> tmp = m_node.GetChildren();
        result.reserve(tmp.size());
        for (std::vector<OpcUa::Node>::const_iterator it = tmp.cbegin(); it != tmp.end(); ++it)
            result.append(QFreeOpcUaValueConverter::nodeIdToString(it->GetId()));
    } catch (const std::exception &ex) {
        qCWarning(QT_OPCUA_PLUGINS_FREEOPCUA) << "Failed to get child ids for node:" << ex.what();
    }

    return result;
}

QString QFreeOpcUaNode::nodeId() const
{
    try {
        return QFreeOpcUaValueConverter::nodeIdToString(m_node.GetId());
    } catch (const std::exception &ex) {
        qCWarning(QT_OPCUA_PLUGINS_FREEOPCUA) << "Failed to get id for node:" << ex.what();
        return QString();
    }
}

QOpcUaNode::NodeClass QFreeOpcUaNode::nodeClass() const
{
    try {
        int32_t temp = m_node.GetAttribute(OpcUa::AttributeId::NodeClass).Value.As<int32_t>();
        OpcUa::NodeClass nc = static_cast<OpcUa::NodeClass>(temp);

        switch (nc) {
        case OpcUa::NodeClass::Object:
            return QOpcUaNode::NodeClass::Object;
        case OpcUa::NodeClass::Variable:
            return QOpcUaNode::NodeClass::Variable;
        case OpcUa::NodeClass::Method:
            return QOpcUaNode::NodeClass::Method;
        case OpcUa::NodeClass::ObjectType:
            return QOpcUaNode::NodeClass::ObjectType;
        case OpcUa::NodeClass::VariableType:
            return QOpcUaNode::NodeClass::VariableType;
        case OpcUa::NodeClass::ReferenceType:
            return QOpcUaNode::NodeClass::ReferenceType;
        case OpcUa::NodeClass::DataType:
            return QOpcUaNode::NodeClass::DataType;
        case OpcUa::NodeClass::View:
            return QOpcUaNode::NodeClass::View;
        default:
            return QOpcUaNode::NodeClass::Undefined;
        }
    } catch (const std::exception &ex) {
        qCWarning(QT_OPCUA_PLUGINS_FREEOPCUA) << "Failed to get node class for node:" << ex.what();
        return QOpcUaNode::NodeClass::Undefined;
    }
}

bool QFreeOpcUaNode::setValue(const QVariant &value, QOpcUa::Types type)
{
    try {
        OpcUa::Variant toWrite = QFreeOpcUaValueConverter::toTypedVariant(value, type);
        if (toWrite.IsNul())
            return false;

        m_node.SetValue(toWrite);
        return true;
    } catch (const std::exception &ex) {
        qCWarning(QT_OPCUA_PLUGINS_FREEOPCUA) << "Could not write value to node " <<  nodeId();
        qCWarning(QT_OPCUA_PLUGINS_FREEOPCUA) << ex.what();
    }
    return false;
}

bool QFreeOpcUaNode::call(const QString &methodNodeId,
                            QVector<QOpcUa::TypedVariant> *args, QVector<QVariant> *ret)
{
    OpcUa::NodeId objectId;
    OpcUa::NodeId methodId;

    if (!m_client)
        return false;

    try {
        objectId = m_node.GetId();

        OpcUa::Node methodNode = m_client->GetNode(methodNodeId.toStdString());
        methodNode.GetBrowseName();
        methodId = methodNode.GetId();
    } catch (const std::exception &ex) {
        qCWarning(QT_OPCUA_PLUGINS_FREEOPCUA) << "Could not get node for method call";
        qCWarning(QT_OPCUA_PLUGINS_FREEOPCUA) << ex.what();
        return false;
    }

    try {
        std::vector<OpcUa::Variant> arguments;
        if (args) {
            arguments.reserve(args->size());
            for (const QOpcUa::TypedVariant &v: qAsConst(*args))
                arguments.push_back(QFreeOpcUaValueConverter::toTypedVariant(v.first, v.second));
        }
        OpcUa::CallMethodRequest myCallRequest;
        myCallRequest.ObjectId = objectId;
        myCallRequest.MethodId = methodId;
        myCallRequest.InputArguments = arguments;
        std::vector<OpcUa::CallMethodRequest> myCallVector;
        myCallVector.push_back(myCallRequest);


        std::vector<OpcUa::Variant> returnedValues = m_node.CallMethod(methodId, arguments);

        // status code of method call is checked via exception inside the node
        ret->reserve(returnedValues.size());
        for (auto it = returnedValues.cbegin(); it != returnedValues.cend(); ++it)
            ret->push_back(QFreeOpcUaValueConverter::toQVariant(*it));

        return true;

    } catch (const std::exception &ex) {
        qCWarning(QT_OPCUA_PLUGINS_FREEOPCUA) << "Method call failed: " << ex.what();
        return false;
    }
}

// Support for structures in freeopcua seems to be not implemented yet
QPair<QString, QString> QFreeOpcUaNode::readEui() const
{
    // Return empty QVariant
    return QPair<QString, QString>();
}

// Support for structures in freeopcua seems to be not implemented yet
QPair<double, double> QFreeOpcUaNode::readEuRange() const
{
    return QPair<double, double>(qQNaN(), qQNaN());
}

QVector<QPair<QVariant, QDateTime> > QFreeOpcUaNode::readHistorical(uint maxCount,
        const QDateTime &begin, const QDateTime &end) const
{
    // not supported with freeopcua
    Q_UNUSED(begin);
    Q_UNUSED(end);
    Q_UNUSED(maxCount);
    return QVector<QPair<QVariant, QDateTime>>();
}

bool QFreeOpcUaNode::writeHistorical(QOpcUa::Types type,
            const QVector<QPair<QVariant, QDateTime> > data)
{
    // not supported with freeopcua
    Q_UNUSED(type);
    Q_UNUSED(data);
    return false;
}

QT_END_NAMESPACE
