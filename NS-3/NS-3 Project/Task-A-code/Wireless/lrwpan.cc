#include <fstream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-dumbbell.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/sixlowpan-module.h"
#include "ns3/lr-wpan-module.h"
#include "ns3/mobility-module.h"
#include "ns3/ipv6-flow-classifier.h"
#include "ns3/flow-monitor-helper.h"

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

// static void
// CwndChange (Ptr<OutputStreamWrapper> stream, uint32_t oldCwnd, uint32_t newCwnd)
// {
//   NS_LOG_UNCOND (Simulator::Now ().GetSeconds () << "\t" << newCwnd);
//   *stream->GetStream () << Simulator::Now ().GetSeconds () <<"\t"<< newCwnd << std::endl;
// }

int
main (int argc, char *argv[])
{
    uint32_t nodes=8;
    uint32_t nflows=4;
    CommandLine cmd (__FILE__);
    uint32_t packets_sent=200;
    uint32_t size=200;
    uint32_t data_rate=(packets_sent*8*size)/(1024);
    
    
    cmd.Parse (argc, argv);
    Time::SetResolution (Time::NS);
    LogComponentEnable ("PacketSink", LOG_LEVEL_INFO);
    NodeContainer node_container;
    node_container.Create(nodes);
    MobilityHelper mobility;
    mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (5.0),
                                 "DeltaY", DoubleValue (10.0),
                                 "GridWidth", UintegerValue (3),
                                 "LayoutType", StringValue ("RowFirst"));
    
    
    mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                              "Speed", StringValue("ns3::ConstantRandomVariable[Constant=25.0]"),
                              "Bounds", RectangleValue(Rectangle(-300, 300, -300, 200)));
    mobility.Install(node_container);
    InternetStackHelper stack;
    LrWpanHelper lwpan;
    NetDeviceContainer devices=lwpan.Install(node_container);
    lwpan.AssociateToPan(devices,0);
    
    stack.Install(node_container);
    SixLowPanHelper sixlowpans;
    NetDeviceContainer sixlowpandevices=sixlowpans.Install(devices);
    Ipv6AddressHelper ipv6;
    ipv6.SetBase(Ipv6Address ("2001:f00d::"), Ipv6Prefix (64));
    Ipv6InterfaceContainer interfaces=ipv6.Assign(sixlowpandevices);

    interfaces.SetForwarding (0, true);
    interfaces.SetDefaultRouteInAllNodes (0);
        for (uint32_t i = 0; i < sixlowpandevices.GetN (); i++) {
         Ptr<NetDevice> dev = sixlowpandevices.Get (i);
        dev->SetAttribute ("UseMeshUnder", BooleanValue (true));
        dev->SetAttribute ("MeshUnderRadius", UintegerValue (10));
       }
    uint16_t sinkPort=9;
    data_rate=data_rate/100.0;
    std::string datarate=std::to_string(data_rate)+"kbps";
    for (uint32_t i = 0;i< nflows; i++) {
    
    Address sinkAddress = Inet6SocketAddress (Inet6SocketAddress (interfaces.GetAddress (nodes-i-1, 1), sinkPort) );
    Ptr<Socket> ns3TcpSocket = Socket::CreateSocket (node_container.Get (i), TcpSocketFactory::GetTypeId ());
    
    Ptr<MyApp> app = CreateObject<MyApp> ();
    app->Setup (ns3TcpSocket, sinkAddress, 200, 100000000,DataRate (datarate));
    node_container.Get (i)->AddApplication (app);
    app->SetStartTime (Seconds (1));
    app->SetStopTime (Seconds (5));

    PacketSinkHelper sinkApp ("ns3::TcpSocketFactory",
    Inet6SocketAddress (Ipv6Address::GetAny (), sinkPort));
    sinkApp.SetAttribute ("Protocol", TypeIdValue (TcpSocketFactory::GetTypeId ()));
    ApplicationContainer sinkApps = sinkApp.Install (node_container.Get(nodes-i-1));
    
    //sink = StaticCast<PacketSink> (sinkApps.Get (0));
    
    sinkApps.Start (Seconds (0.0));
    sinkApps.Stop (Seconds (5));

    }
    NS_LOG_INFO ("Run Simulation.");
  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll ();
  

  Simulator::Stop (Seconds (20));
  Simulator::Run ();
  int total_flow=0;
  float AvgThroughput = 0;
  uint32_t totalSentPackets = 0;
  uint32_t totalReceivedPackets = 0;
  uint32_t totalLostPackets = 0;
  Time delay;
  std::ofstream thr ("wdthroughput.txt", std::ios::out | std::ios::app);
  std::ofstream pdr ("wdpacket_delivery.txt", std::ios::out | std::ios::app);
  std::ofstream plr ("wdpacket_loss.txt", std::ios::out | std::ios::app);
  std::ofstream dl ("wddelay.txt", std::ios::out | std::ios::app);
  Ptr<Ipv6FlowClassifier> classifier = DynamicCast <Ipv6FlowClassifier> (flowmon.GetClassifier6 ());
  FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats ();
  for (auto i = stats.begin (); i != stats.end (); ++i) {
	  Ipv6FlowClassifier::FiveTuple t = classifier->FindFlow (i->first); 
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
    delay=delay+i->second.delaySum;
    std:: cout<<"Throughput =" <<fllowthroughput<<"Mbps"<< std::endl;
    totalSentPackets = totalSentPackets +(i->second.txPackets);
    totalReceivedPackets = totalReceivedPackets + (i->second.rxPackets);
    totalLostPackets = totalLostPackets + loss;
    AvgThroughput = AvgThroughput + fllowthroughput;
    total_flow = total_flow + 1;
}   
  //uint32_t speed=25;

  float avg = AvgThroughput/total_flow;
  float avg_delay=delay.GetSeconds()/total_flow;
  std:: cout<<"--------Average result of the whole simulation---------"<<std::endl;
  std:: cout<<"Total sent packets  =" << totalSentPackets<<std::endl;
  //std::cout <<"here"<<std::endl;
  std:: cout<<"Total Received Packets =" << totalReceivedPackets<<std::endl;
  std:: cout<<"Total Lost Packets =" << totalLostPackets<<std::endl;
  float drop_ratio=(totalLostPackets*100.0)/totalSentPackets;
  float delivery_ratio=(totalReceivedPackets*100.0)/totalSentPackets;
  std:: cout<<"Packet Loss Ratio =" << drop_ratio<< "%"<<std::endl;
  plr<<packets_sent<<" "<<drop_ratio<<std::endl;
  std:: cout<<"Packet delivery Ratio =" << delivery_ratio<< "%"<<std::endl;
  pdr<<packets_sent<<" "<<delivery_ratio<<std::endl;
  std:: cout<<"Average Throughput =" << avg<< "Mbps"<<std::endl;
  thr<<packets_sent<<" "<<avg<<std::endl;
  std:: cout<<"Total End-to-end delay =" << avg_delay<<std::endl;
  dl<<packets_sent<<" "<<avg_delay<<std::endl;
  std:: cout<<"Total Flows " << total_flow<<std::endl;
  Simulator::Destroy ();

  return 0;
}
