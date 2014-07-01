// lwrp_server.cpp
//
// Livewire Control Protocol Server
//
//   (C) Copyright 2014 Fred Gleason <fredg@paravelsystems.com>
//
//   All Rights Reserved.
//

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <syslog.h>
#include <unistd.h>

#include "lwrp_server.h"

LWRPServer::LWRPServer(Routing *routing)
  : QObject()
{
  QHostAddress addr;

  ctrl_routing=routing;

  //
  // Initialize LWRP Server
  //
  ctrl_read_mapper=new QSignalMapper(this);
  connect(ctrl_read_mapper,SIGNAL(mapped(int)),this,SLOT(readData(int)));
  ctrl_closed_mapper=new QSignalMapper(this);
  connect(ctrl_closed_mapper,SIGNAL(mapped(int)),this,SLOT(closedData(int)));
  ctrl_server=new QTcpServer(this);
  connect(ctrl_server,SIGNAL(newConnection()),this,SLOT(newConnectionData()));
  if (!ctrl_server->listen(QHostAddress::Any,SWITCHYARD_LWRP_PORT)){
    syslog(LOG_ERR,"unable to bind port %d",SWITCHYARD_LWRP_PORT);
    exit(256);
  }

  //
  // Initialize slots
  //
  for(unsigned i=0;i<SWITCHYARD_SLOTS;i++) {
    ctrl_routing->subscribe(ctrl_routing->srcAddress(i));
  }
}


void LWRPServer::newConnectionData()
{
  unsigned id=0;

  //
  // Get a slot
  //
  while((id<ctrl_client_connections.size())&&
	(ctrl_client_connections[id]!=NULL)) {
    id++;
  }
  if(id==ctrl_client_connections.size()) {  // Slots full, allocate a new one
    ctrl_client_connections.push_back(NULL);
  }

  //
  // Process the connection
  //
  ctrl_client_connections[id]=new ClientConnection();
  ctrl_client_connections[id]->setSocket(ctrl_server->nextPendingConnection());
  ctrl_read_mapper->setMapping (ctrl_client_connections[id]->socket(),id);
  connect(ctrl_client_connections[id]->socket(),SIGNAL(readyRead()),
	  ctrl_read_mapper,SLOT(map()));
  ctrl_closed_mapper->setMapping (ctrl_client_connections[id]->socket(),id);
  connect(ctrl_client_connections[id]->socket(),SIGNAL(connectionClosed()),
	  ctrl_closed_mapper,SLOT(map()));
}


void LWRPServer::readData(int id)
{
  char data[1500];
  int n;

  while((n=ctrl_client_connections[id]->socket()->read(data,1500))>0) {
    data[n]=0;
    for(int i=0;i<n;i++) {
      if(data[i]==10) { // End of line
	ParseCommand(id);
	ctrl_client_connections[id]->clearBuffer();
      }
      else {
	if(isprint(data[i])!=0) {
	  ctrl_client_connections[id]->appendCommandBuffer(data[i]);
	}
      }
    }
  }
}


void LWRPServer::closedData(int id)
{
  delete ctrl_client_connections[id];
  ctrl_client_connections[id]=NULL;
}


bool LWRPServer::ExecuteLogin(int id,QStringList &args)
{
  return true;
}


bool LWRPServer::ExecuteVer(int id,QStringList &args)
{
  SendCommand(id,QString().
     sprintf("VER LWRP:%s DEVN:\"%s\" SYSV:%s NSRC:%u/2 NDST:%u NGPI:0 NGPO:0",
	     SWITCHYARD_PROTOCOL_VERSION,SWITCHYARD_DEVICE_NAME,
	     SWITCHYARD_VERSION,SWITCHYARD_SLOTS,SWITCHYARD_SLOTS));
  return true;
}


bool LWRPServer::ExecuteIp(int ch,QStringList &args)
{
  SendCommand(ch,QString("IP address ")+
	      ctrl_routing->nicAddress().toString()+" "+
	      "netmask "+ctrl_routing->nicNetmask().toString()+" "+
	      "gateway 0.0.0.0 "+
	      "hostname "+ctrl_routing->hostName());

  return true;
}


bool LWRPServer::ExecuteSrc(int id,QStringList &args)
{
  bool found=true;
  QStringList fields;
  QString rtpa;
  int rtpe=-1;
  QString psnm;
  int slot=0;
  bool ok;
  bool save=false;

  if(args.size()==1) {
    SendCommand(id,"BEGIN");
    for(unsigned i=0;i<SWITCHYARD_SLOTS;i++) {
      SendCommand(id,SrcLine(i));
    }
    SendCommand(id,"END");
  }
  else {
    slot=args[1].toInt(&ok)-1;
    if((!ok)|(slot<0)||(slot>=SWITCHYARD_SLOTS)) {
      return false;
    }
    for(int i=2;i<args.size();i++) {
      found=false;
      fields=ParseField(AString(args[i]));
      if(fields[0]=="RTPE") {
	rtpe=fields[1].toUInt();
	found=true;
      }
      if(fields[0]=="PSNM") {
	psnm=fields[1];
	found=true;
      }
      if(fields[0]=="RTPA") {
	rtpa=fields[1];
	found=true;
      }
      if(fields[0]=="INGN") {
	found=true;
      }
      if(fields[0]=="SHAB") {
	found=true;
      }
      if(fields[0]=="NCHN") {
	found=true;
      }
      if(fields[0]=="RTPP") {
	found=true;
      }
      if(!found) {
	return false;
      }
    }
    if(!rtpa.isNull()) {
      ctrl_routing->setSrcAddress(slot,rtpa);
      save=true;
    }
    if(!psnm.isNull()) {
      ctrl_routing->setSrcName(slot,psnm);
      save=true;
    }
    if(rtpe>=0) {
      ctrl_routing->setSrcEnabled(slot,rtpe);
      save=true;
    }
    BroadcastCommand(SrcLine(slot));
    if(save) {
      ctrl_routing->save();
    }
  }

  return true;
}


bool LWRPServer::ExecuteDst(int id,QStringList &args)
{
  bool found=true;
  QStringList fields;
  QStringList addrs;
  QString addr;
  QString name;
  int slot=0;
  bool ok;
  int streamno;
  bool save=false;

  if(args.size()==1) {
    SendCommand(id,"BEGIN");
    for(unsigned i=0;i<SWITCHYARD_SLOTS;i++) {
      SendCommand(id,DstLine(i));
    }
    SendCommand(id,"END");
  }
  else {
    slot=args[1].toInt(&ok)-1;
    if((!ok)|(slot<0)||(slot>=SWITCHYARD_SLOTS)) {
      return false;
    }
    for(int i=2;i<args.size();i++) {
      found=false;
      fields=ParseField(AString(args[i]));
      if(fields[0]=="ADDR") {
	addrs=fields[1].split("<");
	if(addrs[0].split(".").size()==4) {
	  addr=addrs[0].simplified();
	  found=true;
	}
	else {
	  streamno=addrs[0].toInt(&ok);
	  if(ok) {
	    if((streamno>0)&&(streamno<=0xFFFF)) {
	      addr=QString().sprintf("239.192.%d.%d",streamno/256,streamno%256);
	      found=true;
	    }
	    if(streamno==0) {
	      addr="0.0.0.0";
	      found=true;
	    }
	  }
	}
      }
      if(fields[0]=="NAME") {
	name=fields[1];
	found=true;
      }
      if(fields[0]=="NCHN") {
	found=true;
      }
      if(fields[0]=="LOAD") {
	found=true;
      }
      if(fields[0]=="OUGN") {
	found=true;
      }
      if(!found) {
	return false;
      }
    }
    if(!addr.isNull()) {
      if(addr!=ctrl_routing->dstAddress(slot).toString()) {
	UnsubscribeStream(slot);
	ctrl_routing->setDstAddress(slot,QHostAddress(addr));
	ctrl_routing->subscribe(ctrl_routing->dstAddress(slot));
	save=true;
      }
    }
    if(!name.isNull()) {
      ctrl_routing->setDstName(slot,name);
      save=true;
    }
    BroadcastCommand(DstLine(slot));
    if(save) {
      ctrl_routing->save();
    }
  }

  return true;
}


QString LWRPServer::SrcLine(int slot)
{
  return QString().
    sprintf("SRC %u PSNM:\"%s\" FASM:1 RTPE:%d RTPA:\"%s\" INGN:0 SHAB:0 NCHN:2 RTPP:240",
	    slot+1,
	    (const char *)ctrl_routing->srcName(slot).toAscii(),
	    ctrl_routing->srcEnabled(slot),
	    (const char *)ctrl_routing->srcAddress(slot).toString().toAscii());
}


QString LWRPServer::DstLine(int slot)
{
  return QString().
    sprintf("DST %u NAME:\"%s\" ADDR:\"%s\" NCHN:2 LOAD:0 OUGN:0",
	    slot+1,
	    (const char *)ctrl_routing->dstName(slot).toAscii(),
	    (const char *)ctrl_routing->dstAddress(slot).toString().toAscii());
}


QStringList LWRPServer::ParseField(const AString &field)
{
  QStringList ret=field.split(":");
  if(ret.size()>2) {
    for(int i=2;i<ret.size();i++) {
      ret[1]+=":"+ret[i];
    }
  }
  ret[0]=ret[0].toUpper();
  if(ret.size()<2) {
    ret.push_back("");
  }
  if(ret[1].left(1)=="\"") {
    ret[1]=ret[1].right(ret[1].length()-1);
  }
  if(ret[1].right(1)=="\"") {
    ret[1]=ret[1].left(ret[1].length()-1);
  }

  return ret;
}


void LWRPServer::ParseCommand(int id)
{
  bool ok=false;
  QStringList args=
    AString(ctrl_client_connections[id]->commandBuffer()).split(" ","\"");

  args[0]=args[0].toLower();
  if(args[0]=="login") {
    ok=ExecuteLogin(id,args);
  }
  if(args[0]=="ver") {
    ok=ExecuteVer(id,args);
  }
  if(args[0]=="ip") {
    ok=ExecuteIp(id,args);
  }
  if(args[0]=="src") {
    ok=ExecuteSrc(id,args);
  }
  if(args[0]=="dst") {
    ok=ExecuteDst(id,args);
  }
  if(!ok) {
    SendCommand(id,"ERROR 1000 bad command");
  }
}


void LWRPServer::BroadcastCommand(const QString &cmd)
{
  for(unsigned i=0;i<ctrl_client_connections.size();i++) {
    if(ctrl_client_connections[i]!=NULL) {
      SendCommand(i,cmd);
    }
  }
}


void LWRPServer::SendCommand(int ch,const QString &cmd)
{
  ctrl_client_connections[ch]->
    socket()->write((cmd+"\r\n").toAscii(),cmd.length()+2);
}


void LWRPServer::UnsubscribeStream(int slot)
{
  if(ctrl_routing->clkAddress()==ctrl_routing->dstAddress(slot)) {
    return;
  }
  for(int i=0;i<SWITCHYARD_SLOTS;i++) {
    if((ctrl_routing->dstAddress(i)==ctrl_routing->dstAddress(slot))&&
       (i!=slot)) {
      return;
    }
  }
  ctrl_routing->unsubscribe(ctrl_routing->dstAddress(slot));
}
