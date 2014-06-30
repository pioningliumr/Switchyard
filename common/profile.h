// profile.h
//
// Class for reading INI configuration files.
//
// (C) Copyright 2013 Fred Gleason <fredg@paravelsystems.com>
//     All Rights Reserved.
//

#ifndef PROFILE_H
#define PROFILE_H

#include <vector>

#include <QtCore/QString>
#include <QtCore/QTime>
#include <QtNetwork/QHostAddress>

class ProfileLine
{
 public:
  ProfileLine();
  QString tag() const;
  void setTag(QString tag);
  QString value() const;
  void setValue(QString value);
  void clear();

 private:
  QString line_tag;
  QString line_value;
};


class ProfileSection
{
 public:
  ProfileSection();
  QString name() const;
  void setName(QString name);
  bool getValue(QString tag,QString *value) const;
  void addValue(QString tag,QString value);
  void clear();

 private:
  QString section_name;
  std::vector<ProfileLine> section_line;
};


class Profile
{
 public:
  Profile();
  QString source() const;
  bool setSource(const QString &filename);
  bool setSource(std::vector<QString> *values);
  QString stringValue(const QString &section,const QString &tag,
		      const QString &default_value="",bool *ok=0) const;
  int intValue(const QString &section,const QString &tag,
	       int default_value=0,bool *ok=0) const;
  int hexValue(const QString &section,const QString &tag,
	       int default_value=0,bool *ok=0) const;
  float floatValue(const QString &section,const QString &tag,
		   float default_value=0.0,bool *ok=0) const;
  double doubleValue(const QString &section,const QString &tag,
		    double default_value=0.0,bool *ok=0) const;
  bool boolValue(const QString &section,const QString &tag,
		 bool default_value=false,bool *ok=0) const;
  QTime timeValue(const QString &section,const QString &tag,
		  const QTime &default_value=QTime(),bool *ok=0);
  QHostAddress addressValue(const QString &section,const QString &tag,
			    const QHostAddress &default_value=QHostAddress(),
			    bool *ok=0);
  QHostAddress addressValue(const QString &section,const QString &tag,
			    const QString &default_value="",bool *ok=0);
  void clear();

 private:
  QString profile_source;
  std::vector<ProfileSection> profile_section;
};


#endif  // PROFILE_H