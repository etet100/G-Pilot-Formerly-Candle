#include "configuration.h"
#include <QMetaObject>
#include <QMetaProperty>
#include <QDebug>

Configuration::Configuration(QObject *parent)
    : QObject(parent),
    m_sender(parent),
    m_connection(parent),
    m_visualizer(parent),
    m_console(parent),
    m_parser(parent),
    m_ui(parent),
    m_machine(parent),
    m_persister(parent),
    m_provider(parent)
{
    m_modules << &m_sender
        << &m_connection
        << &m_visualizer
        << &m_console
        << &m_parser
        << &m_ui
        << &m_machine;
}

QString Configuration::language()
{
    return this->m_language;
}

void Configuration::setLanguage(QString language)
{
    this->m_language = language;
}

void Configuration::save()
{
    m_persister.open();
    for (ConfigurationModule* module : m_modules) {
        saveModule(module);
    }
    m_persister.close();
}

bool Configuration::persistByType(QString module, QString name, QVariant value, QString type)
{
    if (type == "QString") {
        m_persister.setString(module, name, value.toString());
    } else if (type == "int") {
        m_persister.setInt(module, name, value.toInt());
    } else if (type == "bool") {
        m_persister.setBool(module, name, value.toBool());
    } else if (type == "float") {
        m_persister.setFloat(module, name, value.toFloat());
    } else {
        return false;
    }

    return true;
}

void Configuration::saveModule(ConfigurationModule *module)
{
    const QMetaObject *metaObj = module->metaObject();
    // qDebug() << "MetaObject: " << metaObj->className();
    // qDebug() << "Property Count: " << metaObj->propertyCount();
    // qDebug() << "Property Offset: " << metaObj->classInfoCount();
    for (int i = 0; i < metaObj->propertyCount(); ++i) {
        QMetaProperty prop = metaObj->property(i);

        if (QString(prop.name()) == "objectName") continue;

        if (!persistByType(module->getSectionName(), prop.name(), prop.read(module), prop.typeName())) {
            QVariant value = module->customGet(prop.name());
            if (!persistByType(module->getSectionName(), prop.name(), value, value.typeName())) {
                // something went wrong
                // @TODO exception??
            }
        }
    }
}

void Configuration::setModuleDefaults(ConfigurationModule *module)
{
    QMap<QString, QVariant> defaults = module->getDefaults();

    const QMetaObject *metaObj = module->metaObject();
    for (int i = 0; i < metaObj->propertyCount(); ++i) {
        QMetaProperty prop = metaObj->property(i);

        if (QString(prop.name()) == "objectName" || QString(prop.name()) == "DEFAULTS") continue;

        QString type(prop.typeName());
        if (type == "QString") {
            prop.write(module, defaults[prop.name()].toString());
        } else if (type == "int") {
            prop.write(module, defaults[prop.name()].toInt());
        } else if (type == "bool") {
            prop.write(module, defaults[prop.name()].toBool());
        } else if (type == "float") {
            prop.write(module, defaults[prop.name()].toFloat());
        } else {
            module->customSet(prop.name(), defaults[prop.name()]);
        }
    }
}

void Configuration::load()
{
    m_provider.open();
    for (ConfigurationModule* module : m_modules) {
        loadModule(module);
    }
    m_provider.close();
}

void Configuration::setDefaults()
{
    for (ConfigurationModule* module : m_modules) {
        setModuleDefaults(module);
    }

    emit defaultConfigurationLoaded();
}

void Configuration::loadModule(ConfigurationModule *module)
{
    QMap<QString, QVariant> defaults = module->getDefaults();

    const QMetaObject *metaObj = module->metaObject();
    // qDebug() << "MetaObject: " << metaObj->className();
    // qDebug() << "Property Count: " << metaObj->propertyCount();
    // qDebug() << "Property Offset: " << metaObj->classInfoCount();
    for (int i = 0; i < metaObj->propertyCount(); ++i) {
        QMetaProperty prop = metaObj->property(i);

        if (QString(prop.name()) == "objectName" || QString(prop.name()) == "DEFAULTS") continue;

        QString type(prop.typeName());
        if (type == "QString") {
            prop.write(module, m_provider.getString(module->getSectionName(), QString(prop.name()), defaults[prop.name()].toString()));
        } else if (type == "int") {
            prop.write(module, m_provider.getInt(module->getSectionName(), QString(prop.name()), defaults[prop.name()].toInt()));
        } else if (type == "bool") {
            prop.write(module, m_provider.getBool(module->getSectionName(), QString(prop.name()), defaults[prop.name()].toBool()));
        } else if (type == "float") {
            prop.write(module, m_provider.getFloat(module->getSectionName(), QString(prop.name()), defaults[prop.name()].toFloat()));
        } else {
            module->customSet(prop.name(), m_provider.get(module->getSectionName(), QString(prop.name()), defaults[prop.name()]));
        }
    }
}

