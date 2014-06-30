// syd.h
//
// Switchyard stream swiching daemon
//
// (C) 2014 Fred Gleason <fredg@paravelsystems.com>
//     All Rights Reserved.
//

#ifndef SYD_H
#define SYD_H

#include <stdint.h>

#include <QtCore/QObject>

#include "adv_server.h"
#include "config.h"
#include "lwrp_server.h"
#include "routing.h"
#include "rtp.h"

#define SYD_USAGE "-d\n"

class MainObject : public QObject
{
  Q_OBJECT;
 public:
  MainObject(QObject *parent=0);

 private slots:
  void exitData();

 private:
  Routing *syd_routing;
  LWRPServer *syd_lwrp;
  AdvServer *syd_adv;
  RTPServer *syd_rtp;
};


#endif  // SYD_H
