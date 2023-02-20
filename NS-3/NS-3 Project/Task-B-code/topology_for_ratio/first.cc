/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include  "ns3/p2p.h"
#include "ns3/socket.h"
#include "ns3/packet-sink.h"
#include "ns3/flow-monitor-module.h"
// Default Network Topology
//
//       10.1.1.0
// n0 -------------- n1
//    point-to-point
//
 
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("FirstScriptExample");
uint32_t prev = 0;
Time prevTime = Seconds (0);

// Calculate throughput
AsciiTraceHelper ascii;
Ptr<OutputStreamWrapper> file=ascii.CreateFileStream("throughput.tr");
static void
TraceThroughput (Ptr<FlowMonitor> monitor)
{
  FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats ();
  auto itr = stats.begin ();
  Time curTime = Now ();
  //std::ofstream thr ("throughput.tr", std::ios::out | std::ios::app);
  std:: ostream *thr=file->GetStream();
  *thr <<  curTime << " " << 8 * (itr->second.txBytes - prev) / (1000 * 1000 * (curTime.GetSeconds () - prevTime.GetSeconds ())) << std::endl;
  prevTime = curTime;
  prev = itr->second.txBytes;
  Simulator::Schedule (Seconds (0.2), &TraceThroughput, monitor);
}

// static void CwndTracer (Ptr<OutputStreamWrapper> stream, uint32_t oldval, uint32_t newval)
// {
//   *stream->GetStream () << Simulator::Now ().GetSeconds () << " " << newval << std::endl;
// }

// void TraceCwnd (uint32_t nodeId, uint32_t socketId)
// {
//   AsciiTraceHelper ascii;
//   Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream ("cwnd2.dat");
//   Config::ConnectWithoutContext ("/NodeList/" + std::to_string (nodeId) + "/$ns3::TcpL4Protocol/SocketList/" + std::to_string (socketId) + "/CongestionWindow", MakeBoundCallback (&CwndTracer, stream));
// }
// static void
// CwndChange (Ptr<OutputStreamWrapper> stream, uint32_t oldCwnd, uint32_t newCwnd)
// {
//   NS_LOG_UNCOND (Simulator::Now ().GetSeconds () << "\t" << newCwnd);
//   *stream->GetStream () << Simulator::Now ().GetSeconds () << "\t" << oldCwnd << "\t" << newCwnd << std::endl;
// }

int
main (int argc, char *argv[])
{
  std::string tcpTypeId = "TcpINVS";
  CommandLine cmd (__FILE__);
  cmd.Parse (argc, argv);
  Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::" + tcpTypeId));
  //Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (200));
  
  Time::SetResolution (Time::NS);
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("PacketSink", LOG_LEVEL_INFO);
  //LogComponentEnable ("BulkSendApplication", LOG_LEVEL_ALL);

  uint32_t nrightnodes = 2;
  uint32_t nleftnodes = 2;
  //uint32_t nflows=2;
  
  NodeContainer leftnodes,bridge,rightnodes;
  NetDeviceContainer m_routerDevices,m_leftRouterDevices,m_leftLeafDevices,m_rightRouterDevices,m_rightLeafDevices;
  PointToPointHelper bottleneckHelper,leftHelper,rightHelper,leftHelper2,rightHelper2;
  bottleneckHelper.SetDeviceAttribute  ("DataRate", StringValue ("500Mbps"));
  bottleneckHelper.SetChannelAttribute ("Delay", StringValue ("13ms"));
  leftHelper.SetDeviceAttribute  ("DataRate", StringValue ("1000Mbps"));
  leftHelper.SetChannelAttribute ("Delay", StringValue ("13ms"));
  leftHelper2.SetDeviceAttribute  ("DataRate", StringValue ("1000Mbps"));

  leftHelper2.SetChannelAttribute ("Delay", StringValue ("13ms"));

  rightHelper.SetDeviceAttribute  ("DataRate", StringValue ("1000Mbps"));
  rightHelper.SetChannelAttribute ("Delay", StringValue ("13ms"));
  rightHelper2.SetDeviceAttribute  ("DataRate", StringValue ("1000Mbps"));

  rightHelper2.SetChannelAttribute ("Delay", StringValue ("13ms"));

  p2p p(nleftnodes,leftHelper,leftHelper2,nrightnodes,rightHelper2,rightHelper,bottleneckHelper);
  InternetStackHelper stack2;
  p.InstallStack(stack2);
  
  Ipv4AddressHelper address,address1,address2;
  
  address.SetBase ("10.1.1.0", "255.255.255.0");
  address1.SetBase ("10.2.1.0", "255.255.255.0");
  address2.SetBase ("10.3.1.0", "255.255.255.0");
  p.AssignIpv4Addresses(address1,address2,address);
  
  
  Ptr<Socket> ns3TcpSocket ;
  

  BulkSendHelper source("ns3::TcpSocketFactory",
                         InetSocketAddress (p.GetRightIpv4Address(0), 9));
            
  source.SetAttribute ("SendSize", UintegerValue (582));
  //SendSize
  source.SetAttribute ("MaxBytes", UintegerValue (0));
  ApplicationContainer sourceApps = source.Install (p.GetLeft(0));
  sourceApps.Start (Seconds (1.0));
  //Simulator::Schedule (Seconds (0.2), &TraceCwnd, 2, 0);
  sourceApps.Stop (Seconds (15.0));
  PacketSinkHelper sink ("ns3::TcpSocketFactory",
                         InetSocketAddress (Ipv4Address::GetAny (), 9));
  ApplicationContainer sinkApps = sink.Install (p.GetRight(0));
  sinkApps.Start (Seconds (1.0));
  sinkApps.Stop (Seconds (15.0));


  BulkSendHelper source2("ns3::TcpSocketFactory",
                         InetSocketAddress (p.GetRightIpv4Address(1), 9));
            
  source2.SetAttribute ("SendSize", UintegerValue (582));
  //SendSize
  source2.SetAttribute ("MaxBytes", UintegerValue (0));
  ApplicationContainer sourceApps2 = source2.Install (p.GetLeft(1));
  sourceApps2.Start (Seconds (1.5));
  //Simulator::Schedule (Seconds (0.2), &TraceCwnd, 2, 0);
  sourceApps2.Stop (Seconds (15.0));
  PacketSinkHelper sink2 ("ns3::TcpSocketFactory",
                         InetSocketAddress (Ipv4Address::GetAny (), 9));
  ApplicationContainer sinkApps2 = sink.Install (p.GetRight(1));
  sinkApps2.Start (Seconds (1.5));
  sinkApps2.Stop (Seconds (15.0));

  std::cout << "\nAverage throughput: "  << " Mbit/s"<<std::endl;
  //std::ofstream w ("renoratio.txt", std::ios::out | std::ios::app);
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  NS_LOG_INFO ("Run Simulation.");
  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll ();
  AsciiTraceHelper asciiTraceHelper;
  Ptr<OutputStreamWrapper> stream = asciiTraceHelper.CreateFileStream ("val.txt");
  //Config::ConnectWithoutContext ("/NodeList/" + std::to_string (0) + "/$ns3::TcpL4Protocol/SocketList/" + std::to_string (0) + "/CongestionWindow", MakeBoundCallback (&CwndChange, stream));
  //bottleneckHelper.EnableAsciiAll (ascii.CreateFileStream ("tcp.tr"));
  //dummy->TraceConnectWithoutContext ("CongestionWindow", MakeBoundCallback (&CwndChange, stream));
  Simulator::Stop (Seconds (15.0));
  Simulator::Run ();
  int total_flow=0;
  float AvgThroughput = 0;
  uint32_t totalSentPackets = 0;
  uint32_t totalReceivedPackets = 0;
  uint32_t totalLostPackets = 0;
  float throughput1;
  float throughput3;

  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats ();
  for (auto i = stats.begin (); i != stats.end (); ++i) {
	  Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first); 
    //NS_LOG_UNCOND("----Flow ID:" <<iter->first);
    std::cout << "Flow " << i->first  << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
    
    std:: cout<<"Received Byte:"<<i->second.rxBytes<< std::endl;
    std:: cout<<"Sent Packets=" <<i->second.txPackets<< std::endl;
    uint32_t lost=i->second.txPackets-i->second.rxPackets;
    std:: cout<<"Lost Packets =" <<lost<< std::endl;
    std:: cout<<"Received Packets =" <<i->second.rxPackets<< std::endl;
    float packet_drop=i->second.rxPackets*100.0;
    packet_drop=packet_drop/i->second.txPackets;
    std:: cout<<"Packet delivery ratio =" <<packet_drop << "%"<< std::endl;
    std:: cout<<"Packet loss ratio =" << (lost*100.0)/i->second.txPackets << "%"<< std::endl;
    std:: cout<<"DElay =" << i->second.delaySum.GetSeconds() << "%"<< std::endl;
    float fllowthroughput=i->second.rxBytes * 8.0/(i->second.timeLastRxPacket.GetSeconds()-i->second.timeFirstTxPacket.GetSeconds())/(1024*1024);
    
    std:: cout<<"Throughput =" <<fllowthroughput<<"Mbps"<< std::endl;
    *stream->GetStream () <<i->first<<" "<<fllowthroughput<<std::endl;
    totalSentPackets = totalSentPackets +(i->second.txPackets);
    totalReceivedPackets = totalReceivedPackets + (i->second.rxPackets);
    totalLostPackets = totalLostPackets + lost;
    AvgThroughput = AvgThroughput + fllowthroughput;
    total_flow = total_flow + 1;
    if(total_flow==1) throughput1=fllowthroughput;
    if(total_flow==3) throughput3=fllowthroughput;
}
  float ratio;
  
  //uint32_t time=320;
  if(throughput1>throughput3) ratio=throughput1/throughput3;
  else ratio= throughput3/throughput1;
  float avg = AvgThroughput/total_flow;
  std::cout<<"Ratiiooo"<<ratio<<std::endl;
  std:: cout<<"--------Average result of the whole simulation---------"<<std::endl;
  std:: cout<<"Total sent packets  =" << totalSentPackets<<std::endl;
  //std::cout <<"here"<<std::endl;
  std:: cout<<"Total Received Packets =" << totalReceivedPackets<<std::endl;
  std:: cout<<"Total Lost Packets =" << totalLostPackets<<std::endl;
  float drop_ratio=(totalLostPackets*100.0)/totalSentPackets;
  float delivery_ratio=(totalReceivedPackets*100.0)/totalSentPackets;
  std:: cout<<"Packet Loss Ratio =" << drop_ratio<< "%"<<std::endl;
  std:: cout<<"Packet delivery Ratio =" << delivery_ratio<< "%"<<std::endl;
  std:: cout<<"Average Throughput =" << avg<< "Mbps"<<std::endl;
  std:: cout<<"Total Flows " << total_flow<<std::endl;
  //w<<time<<" "<<ratio<<std::endl;
  // FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats ();
  
  // for(std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i) {
  //std::cout <<"here"<<std::endl;
  // }
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");

  // AsciiTraceHelper ascii;
  // bottleneckHelper.EnableAsciiAll (ascii.CreateFileStream ("tcp.tr"));
  
  // Simulator::Schedule (Seconds (0 + 0.000001), &TraceThroughput, monitor);
  // Simulator::Stop (Seconds(10) + TimeStep (1));
  //std::cout <<"here"<<std::endl;
  
  return 0;
}
