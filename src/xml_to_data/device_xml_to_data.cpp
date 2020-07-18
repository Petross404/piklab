/***************************************************************************
 *   Copyright (C) 2005-2007 Nicolas Hadacek <hadacek@kde.org>             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "device_xml_to_data.h"

#include <qdir.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qregexp.h>

bool Device::XmlToDataBase::getFrequencyRange(OperatingCondition oc, Special special, QDomElement element)
{
  QDomElement range;
  for (QDomNode child=element.firstChild(); !child.isNull(); child=child.nextSibling()) {
    if ( child.nodeName()!="frequency_range" ) continue;
    if ( !child.isElement() ) qFatal("\"frequency_range\" should be an element");
    if ( child.toElement().attribute("name")!=oc.key() ) continue;
    Special s = Special::fromKey(child.toElement().attribute("special"));
    if ( s==Special::Nb_Types ) qFatal("Unrecognized special");
    if ( special!=s ) continue;
    if ( !range.isNull() ) qFatal("Duplicated \"frequency_range\"");
    range = child.toElement();
  }
  if ( range.isNull() ) return false;
  FrequencyRange frange;
  frange.operatingCondition = oc;
  frange.special = special;
  for (QDomNode child=range.firstChild(); !child.isNull(); child=child.nextSibling()) {
    if ( child.nodeName()=="frequency" ) {
      if ( !child.isElement() ) qFatal("Frequency is not an element");
      QDomElement frequency = child.toElement();
      bool ok1, ok2, ok3, ok4;
      RangeBox box;
      box.start.x = frequency.attribute("start").toDouble(&ok1);
      box.end.x = frequency.attribute("end").toDouble(&ok2);
      box.start.yMin = frequency.attribute("vdd_min").toDouble(&ok3);
      box.start.yMax = frequency.attribute("vdd_max").toDouble(&ok4);
      box.end.yMax = box.start.yMax;
      if ( !ok1 || !ok2 || !ok3 || !ok4
           || box.start.x<0.0 || box.start.x>box.end.x
           || box.start.yMin<0.0 || box.start.yMin>box.start.yMax )
        qFatal("Malformed frequency element");
      if ( frequency.attribute("vdd_min_end").isEmpty() ) box.end.yMin = box.start.yMin;
      else {
        box.end.yMin = frequency.attribute("vdd_min_end").toDouble(&ok1);
        if ( !ok1 || box.end.yMin>box.end.yMax ) qFatal("Malformed frequency element");
      }
      box.mode = frequency.attribute("mode");
      box.osc = frequency.attribute("osc");
      box.special = frequency.attribute("special");
      for (uint i=0; i<uint(frange.vdds.size()); i++)
        if ( box.start.x<frange.vdds[i].end.x && box.end.x>frange.vdds[i].start.x ) {
          if ( box.mode.isEmpty() && box.osc.isEmpty() && box.special.isEmpty() )
            qFatal("Overlapping frequency ranges");
          continue; // #### FIXME: ignore additionnal mode
      }
//      qDebug("add Freq Range: %s %s %f=[%f %f] %f=[%f %f]",
//             Device::FrequencyRange::TYPE_LABELS[type], Device::FrequencyRange::SPECIAL_LABELS[type],
//             box.start.x, box.start.yMin, box.start.yMax,
//             box.end.x, box.end.yMin, box.end.yMax);
      frange.vdds.push_back(box);
   }
  }
  if ( frange.vdds.size()==0 ) qFatal("Empty frequency range");
  _data->_frequencyRanges.push_back(frange);
  return true;
}

bool Device::XmlToDataBase::getMemoryTechnology(QDomElement element)
{
  QString s = element.attribute("memory_technology");
  _data->_memoryTechnology = MemoryTechnology::fromKey(s);
  if ( _data->_memoryTechnology!=MemoryTechnology::Nb_Types ) return true;
  if ( !s.isNull() ) qFatal("Unrecognized memory technology");
  return false;
}

void Device::XmlToDataBase::processDevice(QDomElement device)
{
  QString name = device.attribute("name").upper();
  if ( name.isEmpty() ) qFatal("Device has no name");
  if ( _map.contains(name) ) qFatal(QString("Device \"%1\" already defined").arg(name));
  _data = createData();
  _map[name] = _data;
  _data->_name = name;
  _data->_alternatives = QStringList::split(' ', device.attribute("alternative"));
  if ( _data->_alternatives.count() ) _alternatives[name] = _data->_alternatives;
  _data->_status = Status::fromKey(device.attribute("status"));
  switch (_data->_status.type()) {
    case Status::Nb_Types:
      qFatal("Unrecognized or absent device status");
      break;
    case Status::Future:
      if ( _data->_alternatives.count() ) qFatal("Future device has alternative");
      break;
    case Status::NotRecommended:
    case Status::Mature:
      if ( _data->_alternatives.count()==0 ) warning("Not-recommended/mature device has no alternative");
      break;
    case Status::InProduction:
    case Status::EOL:
    case Status::Unknown: break;
  }

  // document
  _data->_documents.webpage = device.attribute("document"); // ### REMOVE ME
  QDomElement documents = findUniqueElement(device, "documents", QString::null, QString::null);
  if ( documents.isNull() ) {
    if ( _data->_documents.webpage.isEmpty() ) qFatal("Missing \"documents\" element");
  } else {
    if ( !_data->_documents.webpage.isEmpty() ) qFatal("document should be removed from root element");
    _data->_documents.webpage = documents.attribute("webpage");
    if ( _data->_documents.webpage.isEmpty() ) qFatal("Missing webpage");
    _data->_documents.datasheet = documents.attribute("datasheet");
    QRegExp rexp("\\d{5}");
    if ( _data->_documents.datasheet=="?" ) warning("No datasheet specified");
    if ( !rexp.exactMatch(_data->_documents.datasheet) ) qFatal(QString("Malformed datasheet \"%1\" (5 digits)").arg(_data->_documents.datasheet));
    _data->_documents.progsheet = documents.attribute("progsheet");
    if ( _data->_documents.progsheet=="?" ) warning("No progsheet specified");
    if ( !rexp.exactMatch(_data->_documents.datasheet) ) qFatal(QString("Malformed progsheet \"%1\" (5 digits)").arg(_data->_documents.progsheet));
    _data->_documents.erratas = QStringList::split(" ", documents.attribute("erratas"));
    for (uint i=0; i<uint(_data->_documents.erratas.count()); i++) {
      QString errata = _data->_documents.erratas[i];
      if ( !rexp.exactMatch(errata) ) {
        QRegExp rexp2("\\d{5}e\\d");
        if ( !rexp2.exactMatch(errata) && !errata.startsWith("er") && errata.mid(2)!=_data->_name.lower() )
          qFatal(QString("Malformed erratas \"%1\" (5 digits or 5 digits + e + 1 digit or \"er\" + name)").arg(errata));
      }
    }
  }
  if ( _data->_documents.webpage=="?" ) warning("No webpage specified");
  else {
    QRegExp rexp("\\d{6}");
    if ( !rexp.exactMatch(_data->_documents.webpage) ) qFatal(QString("Malformed webpage \"%1\" (6 digits)").arg(_data->_documents.webpage));
    if ( _documents.contains(_data->_documents.webpage) )
      qFatal(QString("webpage duplicated (already used for %1)").arg(_documents[_data->_documents.webpage]));
    _documents[_data->_documents.webpage] = name;
  }

  // frequency ranges
  QStringList names;
  bool ok = false;
  FOR_EACH(OperatingCondition, oc) {
    names += oc.key();
    FOR_EACH(Special, special)
      if ( getFrequencyRange(oc, special, device) && special==Special::Normal ) ok = true;
  }
  if ( !ok ) qWarning("No normal frequency range defined");
  checkTagNames(device, "frequency_range", names);

  // memory technology
  if ( !getMemoryTechnology(device) ) qFatal("Memory technology not defined");

  // packages
  for (QDomNode child=device.firstChild(); !child.isNull(); child=child.nextSibling()) {
    if ( !child.isElement() || child.nodeName()!="package" ) continue;
    Package p = processPackage(child.toElement());
    QMap<QString, uint> pinLabels;
    for (uint i=0; i<p.nbPins(); i++) {
      if ( p.pinName(i).isEmpty() || p.pinName(i)=="N/C" ) continue;
      QStringList labels = QStringList::split("/", p.pinName(i));
      for(uint k=0; k<uint(labels.count()); k++) {
        if ( pinLabels.contains(labels[k]) ) pinLabels[labels[k]]++;
        else pinLabels[labels[k]] = 1;
      }
    }
    QStringList pnames = p.names();
    for (uint k=0; k<uint(_data->_packages.size()); k++) {
      QStringList names = _data->_packages[k].names();
      for (uint j=0; j<uint(names.count()); j++)
        if ( pnames.contains(names[j]) && _data->_packages[k].nbPins()==p.nbPins() ) qFatal("Duplicated package type");
    }
    if ( !pinLabels.isEmpty() ) checkPins(pinLabels);
    _data->_packages.push_back(p);
  }
}

Device::Package Device::XmlToDataBase::processPackage(QDomElement element)
{
  Package package;
  // nb pins
  bool ok;
  uint nb = element.attribute("nb_pins").toUInt(&ok);
  if ( !ok || nb==0 ) qFatal("Malformed \"nb_pins\"");
  package._pins.resize(nb);
  // types
  QStringList types = QStringList::split(" ", element.attribute("types"));
  if ( types.isEmpty() ) qFatal("No package types specified");
  for (uint k=0; k<uint(types.count()); k++) {
    uint i = 0;
    for (; Package::TYPE_DATA[i].name; i++) {
      if ( types[k]!=Package::TYPE_DATA[i].name ) continue;
      QStringList names = package.names();
      for (uint j=0; j<uint(names.count()); j++)
        if ( names[j]==Package::TYPE_DATA[i].name ) qFatal(QString("Duplicated package type %1").arg(types[k]));
      uint j = 0;
      for (; j<Package::MAX_NB; j++)
        if ( nb==Package::TYPE_DATA[i].nbPins[j] ) break;
      if ( j==Package::MAX_NB ) qFatal(QString("Package %1 does not have the correct number of pins %2 (%3)").arg(types[k]).arg(nb).arg(Package::TYPE_DATA[i].nbPins[0]));
      package._names += Package::TYPE_DATA[i].name;
      break;
    }
    if ( Package::TYPE_DATA[i].name==0 ) qFatal(QString("Unknown package type \"%1\"").arg(types[k]));
  }
  // pins
  QStringList names = package.names();
  if ( names.contains("sot23") ) {
    if ( names.count()!=1 ) qFatal("SOT23 should be a specific package");
  } else if ( (nb%2)!=0 ) qFatal(QString("\"nb_pins\" should be even for package \"%1\"").arg(names[0]));
  uint have_pins = false;
  QMemArray<bool> found(nb);
  found.fill(false);
  QDomNode child = element.firstChild();
  while ( !child.isNull() ) {
    if ( child.nodeName()=="pin" ) {
      if ( !child.isElement() ) qFatal("\"pin\" is not an element");
      QDomElement pin = child.toElement();
      bool ok;
      uint i = pin.attribute("index").toUInt(&ok);
      if ( !ok || i==0 || i>nb ) qFatal("Malformed pin index");
      if (found[i-1]) qFatal("Duplicated pin index");
      found[i-1] = true;
      QString name = pin.attribute("name");
      if ( !name.isEmpty() && name!="N/C" ) {
        QStringList labels = QStringList::split("/", name);
        if ( name.contains(" ") || labels.count()==0 ) qFatal("Malformed pin name");
        if ( name!=name.upper() ) qFatal("Pin name should be uppercase");
      }
      package._pins[i-1] = name;
      have_pins = true;
    }
   child = child.nextSibling();
  }
  if ( !have_pins ) ;//warning("Pins not specified"); // #### REMOVE ME !!
  else for (uint i=0; i<nb; i++) if ( !found[i] ) qFatal(QString("Pin #%1 not specified").arg(i+1));
  return package;
}

void Device::XmlToDataBase::parse()
{
  // process device files
  QStringList files = QDir::current().entryList("*.xml");
  for (uint i=0; i<uint(files.count()); i++) {
    _data = 0;
    QDomDocument doc = parseFile(files[i]);
    QDomElement root = doc.documentElement();
    if ( root.nodeName()!="device" ) qFatal("root node should be \"device\"");
    processDevice(root);
  }

  // check alternatives
  QMap<QString, QStringList>::const_iterator ait = _alternatives.begin();
  for (; ait!=_alternatives.end(); ++ait) {
    QStringList::const_iterator lit = ait.data().begin();
    for (; lit!=ait.data().end(); ++lit)
      if ( !_map.contains(*lit) ) qFatal(QString("Unknown alternative %1 for device %2").arg((*lit)).arg(ait.key()));
  }
}
