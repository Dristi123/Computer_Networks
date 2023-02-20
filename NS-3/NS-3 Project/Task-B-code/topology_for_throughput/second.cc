#include <fstream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-dumbbell.h"
#include "ns3/flow-monitor-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("SixthScriptExample");


class MyApp : public Application
{
public:
  MyApp ();
  virtual ~MyApp ();

  /**
   * Register this type.
   * \return The TypeId.
   */
  static TypeId GetTypeId (void);
  void Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate);

private:
  virtual void StartApplication (void);
  virtual void StopApplication (void);

  void ScheduleTx (void);
  void SendPacket (void);

  Ptr<Socket>     m_socket;
  Address         m_peer;
  uint32_t        m_packetSize;
  uint32_t        m_nPackets;
  DataRate        m_dataRate;
  EventId         m_sendEvent;
  bool            m_running;
  uint32_t        m_packetsSent;
};

MyApp::MyApp ()
  : m_socket (0),
    m_peer (),
    m_packetSize (0),
    m_nPackets (0),
    m_dataRate (0),
    m_sendEvent (),
    m_running (false),
    m_packetsSent (0)
{
}

MyApp::~MyApp ()
{
  m_socket = 0;
}

/* static */
TypeId MyApp::GetTypeId (void)
{
  static TypeId tid = TypeId ("MyApp")
    .SetParent<Application> ()
    .SetGroupName ("Tutorial")
    .AddConstructor<MyApp> ()
    ;
  return tid;
}

void
MyApp::Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate)
{
  m_socket = socket;
  m_peer = address;
  m_packetSize = packetSize;
  m_nPackets = nPackets;
  m_dataRate = dataRate;
}

void
MyApp::StartApplication (void)
{
  m_running = true;
  m_packetsSent = 0;
  m_socket->Bind ();
  m_socket->Connect (m_peer);
  SendPacket ();
}

void
MyApp::StopApplication (void)
{
  m_running = false;

  if (m_sendEvent.IsRunning ())
    {
      Simulator::Cancel (m_sendEvent);
    }

  if (m_socket)
    {
      m_socket->Close ();
    }
}

void
MyApp::SendPacket (void)
{
  Ptr<Packet> packet = Create<Packet> (m_packetSize);
  m_socket->Send (packet);

  if (++m_packetsSent < m_nPackets)
    {
      ScheduleTx ();
    }
}

void
MyApp::ScheduleTx (void)
{
  if (m_running)
    {
      Time tNext (Seconds (m_packetSize * 8 / static_cast<double> (m_dataRate.GetBitRate ())));
      m_sendEvent = Simulator::Schedule (tNext, &MyApp::SendPacket, this);
    }
}


Ptr<PacketSink> sink;                         /* Pointer to the packet sink application */
uint64_t lastTotalRx = 0;    
AsciiTraceHelper asciiTraceHelper;
Ptr<OutputStreamWrapper> stream = asciiTraceHelper.CreateFileStream ("thr.txt");                 /* The value of the last total received bytes */

void
CalculateThroughput ()
{
  Time now = Simulator::Now ();                                         /* Return the simulator's virtual time. */
  double cur = (sink->GetTotalRx () - lastTotalRx) * (double) 8 / 1e5;  
  *stream->GetStream() << now.GetSeconds() <<" " <<cur<<std::endl;/* Convert Application RX Packets to MBits. */
  std::cout << now.GetSeconds () << "s: \t" << cur << " Mbit/s" << std::endl;
  lastTotalRx = sink->GetTotalRx ();
  Simulator::Schedule (MilliSeconds (100), &CalculateThroughput);
}

int
main (int argc, char *argv[])
{
  std::string tcpTypeId = "TcpINVS";
  CommandLine cmd (__FILE__);
  cmd.Parse (argc, argv);
  Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::" + tcpTypeId));
  //Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (200));
  Time::SetResolution (Time::NS);
  LogComponentEnable ("PacketSink", LOG_LEVEL_INFO);
  //LogComponentEnable ("BulkSendApplication", LOG_LEVEL_ALL);

  uint32_t nrightnodes = 4;
  uint32_t nleftnodes = 4;
  //uint32_t nodes=nrightnodes+nleftnodes+2;
  uint32_t nflows=1;
  //uint32_t npackets=500;
  
  NodeContainer leftnodes,bridge,rightnodes;
  NetDeviceContainer m_routerDevices,m_leftRouterDevices,m_leftLeafDevices,m_rightRouterDevices,m_rightLeafDevices;
  PointToPointHelper bottleneckHelper,leftHelper,rightHelper;
  bottleneckHelper.SetDeviceAttribute  ("DataRate", StringValue ("500Mbps"));
  bottleneckHelper.SetChannelAttribute ("Delay", StringValue ("0.25ms"));
  leftHelper.SetDeviceAttribute  ("DataRate", StringValue ("1000Mbps"));
  leftHelper.SetChannelAttribute ("Delay", StringValue ("0.25ms"));
  rightHelper.SetDeviceAttribute  ("DataRate", StringValue ("1000Mbps"));
  rightHelper.SetChannelAttribute ("Delay", StringValue ("0.25ms"));

  PointToPointDumbbellHelper p(nleftnodes,leftHelper,nrightnodes,rightHelper,bottleneckHelper);
  InternetStackHelper stack2;
  p.InstallStack(stack2);
  
  Ipv4AddressHelper address,address1,address2;
  
  address.SetBase ("10.1.1.0", "255.255.255.0");
  address1.SetBase ("10.2.1.0", "255.255.255.0");
  address2.SetBase ("10.3.1.0", "255.255.255.0");
  p.AssignIpv4Addresses(address1,address2,address);
  Ptr<Socket> firstsocket;
  for (uint32_t i = 0;i< nflows; i++) {
  uint16_t sinkPort = 8080;
  Address sinkAddress (InetSocketAddress (p.GetRightIpv4Address(i), sinkPort));
  PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
  ApplicationContainer sinkApps = packetSinkHelper.Install (p.GetRight(i));
  sink = StaticCast<PacketSink> (sinkApps.Get (0));
  sinkApps.Start (Seconds (0.));
  sinkApps.Stop (Seconds (2.));

  Ptr<Socket> ns3TcpSocket = Socket::CreateSocket (p.GetLeft(i), TcpSocketFactory::GetTypeId ());
  if(i==0) {
    firstsocket=ns3TcpSocket;
  }
  Ptr<MyApp> app = CreateObject<MyApp> ();
  app->Setup (ns3TcpSocket, sinkAddress, 536, 10000000, DataRate ("1000Mbps"));
  p.GetLeft(i)->AddApplication (app);
  app->SetStartTime (Seconds (1.));
  app->SetStopTime (Seconds (2.));
  }
  Simulator::Schedule (Seconds (1.1), &CalculateThroughput);
  

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  NS_LOG_INFO ("Run Simulation.");
  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll ();
  

  Simulator::Stop (Seconds (2));
  Simulator::Run ();
  int total_flow=0;
  float AvgThroughput = 0;
  uint32_t totalSentPackets = 0;
  uint32_t totalReceivedPackets = 0;
  uint32_t totalLostPackets = 0;
  Time delay;
  //float throughput1;
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());

  //std::ofstream w ("invsthr.txt", std::ios::out | std::ios::app);

  FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats ();
  for (auto i = stats.begin (); i != stats.end (); ++i) {
	  Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first); 
    //NS_LOG_UNCOND("----Flow ID:" <<iter->first);
    std::cout << "Flow " << i->first  << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
    
    std:: cout<<"Received Byte:"<<i->second.rxBytes<< std::endl;
    std:: cout<<"Sent Packets=" <<i->second.txPackets<< std::endl;
    uint32_t lost=i->second.txPackets-i->second.rxPackets;
    uint32_t loss=i->second.lostPackets;
    std:: cout<<"Lost Packets =" <<lost<< std::endl;
    std:: cout<<"Received Packets =" <<i->second.rxPackets<< std::endl;
    float packet_drop=i->second.rxPackets*100.0;
    packet_drop=packet_drop/i->second.txPackets;
    std:: cout<<"Packet delivery ratio =" <<packet_drop << "%"<< std::endl;
    std:: cout<<"Packet loss ratio =" << (lost*100.0)/i->second.txPackets << "%"<< std::endl;
    float fllowthroughput=i->second.rxBytes * 8.0/(i->second.timeLastRxPacket.GetSeconds()-i->second.timeFirstTxPacket.GetSeconds())/(1024*1024);
    std:: cout<<"Throughput =" <<fllowthroughput<<"Mbps"<< std::endl;
    delay=delay+i->second.delaySum;
    std:: cout<<"Delay =" <<delay<<"s"<< std::endl;
    *stream->GetStream () <<i->first<<" "<<fllowthroughput<<std::endl;
    totalSentPackets = totalSentPackets +(i->second.txPackets);
    totalReceivedPackets = totalReceivedPackets + (i->second.rxPackets);
    totalLostPackets = totalLostPackets + loss;
    AvgThroughput = AvgThroughput + fllowthroughput;
    total_flow = total_flow + 1;
    ///if(total_flow==1) throughput1=fllowthroughput;
}
  //float error=0.01;
  float avg = AvgThroughput/total_flow;
  float avg_delay=delay.GetSeconds()/total_flow;
  std:: cout<<"--------Average result of the whole simulation---------"<<std::endl;
  std:: cout<<"Total sent packets  =" << totalSentPackets<<std::endl;
  //std::cout <<"here"<<std::endl;
  std:: cout<<"Total Received Packets =" << totalReceivedPackets<<std::endl;
  std:: cout<<"Total Lost Packets =" << totalLostPackets<<std::endl;
  float drop_ratio=(totalLostPackets*100.0)/totalSentPackets;
  float delivery_ratio=(totalReceivedPackets*100.0)/totalSentPackets;
  std:: cout<<"Packet Loss Ratio =" << drop_ratio<<std::endl;
  std:: cout<<"Packet delivery Ratio =" << delivery_ratio<< "%"<<std::endl;
  //w<<error<<" "<<throughput1<<std::endl;
  std:: cout<<"Average Throughput =" << avg<< "Mbps"<<std::endl;
 
  std:: cout<<"Total End-to-end delay =" << avg_delay<<std::endl;
 
  std:: cout<<"Total Flows " << total_flow<<std::endl;
  Simulator::Destroy ();

  return 0;
}
