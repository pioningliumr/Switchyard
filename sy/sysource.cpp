// sysource.cpp
//
// Container class for LiveWire source attributes
//
// (C) 2015 Fred Gleason <fredg@paravelsystems.com>
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of version 2.1 of the GNU Lesser General Public
//    License as published by the Free Software Foundation;
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU Lesser General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, 
//    Boston, MA  02111-1307  USA
//

#include "sysource.h"

SySource::SySource(const QHostAddress &s_addr,const QString &name,bool enabled)
  : SyEndPoint()
{
  src_stream_address=s_addr;
  src_name=name;
  src_label="";
  src_enabled=enabled;
  src_channels=0;
  src_packet_size=0;
  src_shareable=false;
}


SySource::SySource()
  : SyEndPoint()
{
  src_name="";
  src_label="";
  src_enabled=false;
  src_channels=0;
  src_packet_size=0;
  src_shareable=false;
}


QHostAddress SySource::streamAddress() const
{
  return src_stream_address;
}


void SySource::setStreamAddress(const QHostAddress &addr)
{
  src_stream_address=addr;
}


QString SySource::name() const
{
  return src_name;
}


void SySource::setName(const QString &name)
{
  src_name=name;
}


QString SySource::label() const
{
  return src_label;
}


void SySource::setLabel(const QString &str)
{
  src_label=str;
}


bool SySource::enabled() const
{
  return src_enabled;
}


void SySource::setEnabled(bool state)
{
  src_enabled=state;
}


unsigned SySource::channels() const
{
  return src_channels;
}


void SySource::setChannels(unsigned chans)
{
  src_channels=chans;
}


unsigned SySource::packetSize() const
{
  return src_packet_size;
}


void SySource::setPacketSize(unsigned size)
{
  src_packet_size=size;
}


bool SySource::shareable() const
{
  return src_shareable;
}


void SySource::setShareable(bool state)
{
  src_shareable=state;
}


QString SySource::dump() const
{
  QString ret="";

  ret+="streamAddress: "+streamAddress().toString()+"\n";
  ret+="name: "+name()+"\n";
  ret+="label: "+label()+"\n";
  ret+=QString().sprintf("enabled: %u\n",enabled());
  ret+=QString().sprintf("channels: %u\n",channels());
  ret+=QString().sprintf("packetSize: %u\n",packetSize());
  ret+=QString().sprintf("shareable: %u\n",shareable());

  return ret;
}
