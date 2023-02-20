/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 ResiliNets, ITTC, University of Kansas 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors: Siddharth Gangadhar <siddharth@ittc.ku.edu>,
 *          Truc Anh N. Nguyen <annguyen@ittc.ku.edu>,
 *          Greeshma Umapathi
 *
 * James P.G. Sterbenz <jpgs@ittc.ku.edu>, director
 * ResiliNets Research Group  http://wiki.ittc.ku.edu/resilinets
 * Information and Telecommunication Technology Center (ITTC)
 * and Department of Electrical Engineering and Computer Science
 * The University of Kansas Lawrence, KS USA.
 *
 * Work supported in part by NSF FIND (Future Internet Design) Program
 * under grant CNS-0626918 (Postmodern Internet Architecture),
 * NSF grant CNS-1050226 (Multilayer Network Resilience Analysis and Experimentation on GENI),
 * US Department of Defense (DoD), and ITTC at The University of Kansas.
 */

#include "tcp-invs.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "rtt-estimator.h"
#include "tcp-socket-base.h"
#include <cmath>
#include <bits/stdc++.h>

NS_LOG_COMPONENT_DEFINE ("TcpINVS");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (TcpINVS);

TypeId
TcpINVS::GetTypeId (void)
{
  static TypeId tid = TypeId("ns3::TcpINVS")
    .SetParent<TcpNewReno>()
    .SetGroupName ("Internet")
    .AddConstructor<TcpINVS>()
    
    .AddTraceSource("EstimatedBW", "The estimated bandwidth",
                    MakeTraceSourceAccessor(&TcpINVS::m_currentBW),
                    "ns3::TracedValueCallback::Double")
  ;
  return tid;
}

TcpINVS::TcpINVS (void) :
  TcpNewReno (),
  m_currentBW (0), 
  m_lastSampleBW (0),
  m_lastBW (0),        
  m_c(2),
  m_beta(0.5),
  m_gamma(0.75),
  m_last_rtt(0),
  m_rtt_min(0),
  m_Minrtt(Time::Max ()),
  m_Maxrtt(0),
  m_bwest(0),
  m_buffest(0),
  m_maxbuff(0),
  m_ackedSegments (0),
  m_IsCount (false),
  m_lastAck (0),
  m_measure(0)
{
  NS_LOG_FUNCTION (this);
}

TcpINVS::TcpINVS (const TcpINVS& sock) :
  TcpNewReno (sock),
  m_currentBW (sock.m_currentBW),
  m_lastSampleBW (sock.m_lastSampleBW),
  m_lastBW (sock.m_lastBW),
  m_c(sock.m_c),
  m_beta(sock.m_beta),
  m_gamma(sock.m_gamma),
  m_last_rtt(sock.m_last_rtt),
  m_rtt_min(sock.m_rtt_min),
  m_Minrtt(sock.m_Minrtt),
  m_Maxrtt(sock.m_Maxrtt),
  m_bwest(sock.m_bwest),
  m_buffest(sock.m_buffest),
  m_maxbuff(sock.m_maxbuff),
  m_ackedSegments (sock.m_ackedSegments),
  m_IsCount (sock.m_IsCount),
  m_lastAck (sock.m_lastAck),
  m_measure(sock.m_measure)

{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC ("Invoked the copy constructor");
}

TcpINVS::~TcpINVS (void)
{
}

void
TcpINVS::PktsAcked (Ptr<TcpSocketState> tcb, uint32_t packetsAcked,
                        const Time& rtt)
{
  NS_LOG_FUNCTION (this << tcb << packetsAcked << rtt);

  if (rtt.IsZero ())
    {
      NS_LOG_WARN ("RTT measured is zero!");
      return;
    }
  m_last_rtt=rtt;
  m_ackedSegments += packetsAcked;
  EstimateBW (rtt, tcb);
  EstimateRTT(rtt);
  double val=500.0/(m_currentBW+1);
  //std::cout<<val<<std::endl;
  if(val>1) m_bwest=val;
  //m_bwest=std::max((500.0/m_currentBW),1.0);
  else m_bwest=1;
  m_rtt_min=std::max((0.5/m_Minrtt.GetSeconds()),1.0);

}

void
TcpINVS::EstimateRTT(const Time &rtt)
{
   m_Minrtt = std::min (m_Minrtt, rtt);
   m_Maxrtt = std:: max(m_Maxrtt,rtt);
}

uint32_t TcpINVS::Update() {
      //std::cout<<"cccc"<<m_c<<std::endl;
      //std::cout<<"bww"<<m_bwest<<std::endl;
      //std::cout<<"rtttt"<<m_rtt_min<<std::endl;
      //std::cout<<"gammm"<<m_gamma<<std::endl;
      return m_c*(log2(m_bwest*pow(m_rtt_min,m_gamma))+1);
       //return 1;
}
void
TcpINVS::IncreaseWindow (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked) 
{
  //std::cout<<"inval"<<tcb->m_cWndsp<<std::endl;
  //std::cout<<"cwnvallll"<<tcb->m_cWnd<<std::endl;
  uint32_t k=Update();
  if(tcb->m_cWnd<tcb->m_ssThresh) {
    tcb->m_cWnd+=tcb->m_segmentSize;
    //tcb->m_cWnd+=1;
  }
  else if(tcb->m_cWnd>tcb->m_ssThresh && tcb->m_cWnd<tcb->m_cWndsp) {
    //std::cout<<"firstif"<<std::endl;
    uint32_t val = ((tcb->m_cWndsp-tcb->m_cWnd)/(k*tcb->m_cWndsp))*tcb->m_segmentSize;
    //uint32_t val = ((tcb->m_cWndsp-tcb->m_cWnd)/(k*tcb->m_cWndsp));
    tcb->m_cWnd+=val;
  }
  else {
    //std::cout<<"secondif"<<std::endl;
    //std::cout<<k<<std::endl;
    //std::cout<<tcb->m_cWndsp<<std::endl;
    uint32_t val = ((tcb->m_cWnd-tcb->m_cWndsp)/(k*tcb->m_cWnd))*tcb->m_segmentSize;
    //uint32_t val = ((tcb->m_cWnd-tcb->m_cWndsp)/(k*tcb->m_cWnd));
    tcb->m_cWnd+=val;
  }
}


void
TcpINVS::EstimateBW (const Time &rtt, Ptr<TcpSocketState> tcb)
{
  uint32_t delta=Simulator::Now().GetSeconds()-m_measure;
  NS_LOG_FUNCTION (this);

  NS_ASSERT (!rtt.IsZero ());
  if(delta>std::max(rtt.GetSeconds(),20000.0)) {
  //m_currentBW = m_ackedSegments * tcb->m_segmentSize / rtt.GetSeconds ();
  m_currentBW=1*m_currentBW+1*(m_ackedSegments * tcb->m_segmentSize)/delta;
  m_ackedSegments = 0;
  }
}

uint32_t
TcpINVS::GetSsThresh (Ptr<const TcpSocketState> tcb,
                          uint32_t bytesInFlight)
{

  m_maxbuff=m_Maxrtt.GetSeconds()*m_currentBW;
  double val=std::min(5.0,m_maxbuff);
  if(val<((m_last_rtt.GetSeconds()-m_Minrtt.GetSeconds())*m_currentBW)) return tcb->m_cWnd;
  
  return (0.5 * tcb->m_cWnd);
}

Ptr<TcpCongestionOps>
TcpINVS::Fork ()
{
  return CreateObject<TcpINVS> (*this);
}

} // namespace ns3
