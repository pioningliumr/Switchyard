// sysyslog.cpp
//
// Cross-platform system logging routines for Switchyard
//
// (C) Copyright 2014 Fred Gleason <fredg@paravelsystems.com>
//     All Rights Reserved
//

#include "sysyslog.h"

void SyOpenLog(const QString &ident,int option,int facility)
{
#ifndef WIN32
  openlog(ident.toUtf8(),option,facility);
#endif  // WIN32
}


void SySyslog(int priority,const QString &msg)
{
#ifdef WIN32
  fprintf(stderr,"%s\n",(const char *)msg.toUtf8());
#else
  syslog(priority,"%s",(const char *)msg.toUtf8());
#endif  // WIN32
}
