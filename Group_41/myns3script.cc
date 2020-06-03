#include "ns3/core-module.h"
#include "ns3/config.h"
#include "ns3/string.h"
#include "ns3/log.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"
#include "ns3/mobility-helper.h"
#include "ns3/mobility-module.h"
#include "ns3/on-off-helper.h"
#include "ns3/yans-wifi-channel.h"
#include "ns3/mobility-model.h"
#include "ns3/packet-sink.h"
#include "ns3/packet-sink-helper.h"
#include "ns3/tcp-westwood.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/wifi-module.h"
#include "ns3/flow-monitor.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/network-module.h"
#include <iostream>
#include <cstdlib>

NS_LOG_COMPONENT_DEFINE("ns3_assignment");
using namespace ns3;


int main() {

// Get model parameters

	uint32_t payload = 1000;				//TCP payload size
	std::string datarate;					//Appln layer datarate
	std::string tcpvariant;
	std::string phyRate = "HtMcs7";		//Physical Layer bitrate
	double simulation_time = 50;
	std::string RtsCtsThreshold;
	bool pcapTrace = true;					//Enable for PCAP tracing (default)
	bool asciiTrace = false;				//Enable for ASCII tracing
	
	std::cout<<"\nEnter application layer datarate:\t";
	std::cin>>datarate;
	std::cout<<"\nEnter TCP variant (TcpWestwood/TcpHybla):\t";
	std::cin>>tcpvariant;
	std::cout<<"\nEnter RTS/CTS threshold:\t";
	std::cin>>RtsCtsThreshold;
	
// Format model parameters to fit ns3

	datarate = datarate + std::string("Mbps");
	tcpvariant = std::string("ns3::") + tcpvariant;
	
// Set defualt L4 protocol to be specified tcpvariant

	if(tcpvariant.compare ("ns3::TcpWestwood") == 0){
		Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpWestwood::GetTypeId ()));
		Config::SetDefault ("ns3::TcpWestwood::ProtocolType", EnumValue (TcpWestwood::WESTWOOD));
	}
	
	else if(tcpvariant.compare ("ns3::TcpHybla") == 0){
		Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpHybla::GetTypeId()));
	}
	
	else{
		TypeId tcpTid;
      NS_ABORT_MSG_UNLESS (TypeId::LookupByNameFailSafe (tcpvariant, &tcpTid), "TypeId " << tcpvariant << " not found");		//if not westwood or hybla try lookup else abort
      Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TypeId::LookupByName (tcpvariant)));						//if lookup successful, set default L4 protocol
	}
	
// Set other default values

	std::string FragmentationThreshold = "1000";		//WLOG setting frag_threshold to payload size
	Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold" , StringValue(RtsCtsThreshold));
   Config::SetDefault("ns3::WifiRemoteStationManager::FragmentationThreshold" , StringValue(FragmentationThreshold));
   Config::SetDefault("ns3::TcpSocket::SegmentSize" , UintegerValue(payload));
	
	std::cout<<"\nDatarate is: "<<datarate<<" TCP variant is: "<<tcpvariant<<"\n";
	
	WifiHelper wifiHelper;
	wifiHelper.SetStandard(WIFI_PHY_STANDARD_80211n_5GHZ);
	
// Setup channel
	
	YansWifiChannelHelper wifichannel;
	wifichannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
	wifichannel.AddPropagationLoss("ns3::FriisPropagationLossModel");
	std::cout<<"\nChannel Setup Complete!\n";
	
// Setup Physical Layer
	
	YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
   wifiPhy.SetChannel (wifichannel.Create ());
   
   wifiPhy.Set ("TxPowerStart", DoubleValue (100.0));		//Min transmission power in dBm
   wifiPhy.Set ("TxPowerEnd", DoubleValue (100.0));		//Max transmission power in dBm
   wifiPhy.Set ("TxPowerLevels", UintegerValue (1));		//Number of transmission power levels
   wifiPhy.Set ("TxGain", DoubleValue (0));
   wifiPhy.Set ("RxGain", DoubleValue (0));
   wifiPhy.Set ("RxNoiseFigure", DoubleValue (10));		//Noise factor in dB
   wifiPhy.SetErrorRateModel ("ns3::YansErrorRateModel");
   wifiHelper.SetRemoteStationManager ("ns3::ConstantRateWifiManager","DataMode", StringValue(phyRate),"ControlMode", StringValue("HtMcs0"));
   
   WifiMacHelper wifimac;		//Helper for MAC level setup
   
   NodeContainer apnode , stanode1 , stanode2;
   NodeContainer stanodes;
   apnode.Create(1);		//Create the AP node, node 1
   stanode1.Create(1);
   stanode2.Create(1);	//The STA nodes, node 0 and node 2
   stanodes.Add(stanode1);
   stanodes.Add(stanode2);
   
   NodeContainer allnodes;		//Container to hold all nodes in order
   allnodes.Add(stanode1);
   allnodes.Add(apnode);
   allnodes.Add(stanode2);
   
   // Configure AP
   Ssid ssid = Ssid("ns3wifi");
   wifimac.SetType("ns3::ApWifiMac" , "Ssid" , SsidValue(ssid));
   NetDeviceContainer apDevice;		//Net device for AP
   apDevice = wifiHelper.Install(wifiPhy , wifimac , apnode);		//Install device with lower layer implementations as above
   
   // Configure STAs
   wifimac.SetType("ns3::StaWifiMac" , "Ssid" , SsidValue(ssid));
   NetDeviceContainer staDevices;
   staDevices = wifiHelper.Install(wifiPhy , wifimac , stanodes);
   std::cout<<"\nPhysical Layer setup complete!\n";
   
// Define node locations, ie , Mobility model

	MobilityHelper mobility;
	Ptr<ListPositionAllocator> position = CreateObject<ListPositionAllocator> ();
   position->Add (Vector (0.0, 0.0, 0.0));
   position->Add (Vector (0.0, 200.0, 0.0));
   position->Add (Vector (0.0, 400.0, 0.0));
   mobility.SetPositionAllocator(position);
   mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");  
   
   NodeContainer::Iterator i;		//container iterator
   for(i = allnodes.Begin() ; i != allnodes.End() ; ++i){
   	mobility.Install((*i));		//install mobility models into all nodes
   }
   std::cout<<"\nMobility Model setup complete!\n";
   
// Install Internet Stack

	InternetStackHelper stack;
	stack.Install(allnodes);		//install stack into all nodes
   
   Ipv4AddressHelper address;
   address.SetBase("10.0.0.0" , "255.255.255.0");
   Ipv4InterfaceContainer apinterface , stainterfaces;
   apinterface = address.Assign(apDevice);
   stainterfaces = address.Assign(staDevices);
   
   Ipv4GlobalRoutingHelper::PopulateRoutingTables();
   std::cout<<"\nInternet Stack setup complete!\n";
   
// Install TCP receiver on AP

	Ptr<PacketSink> sink;		//node 1's packet sink	
	uint32_t port_no = 10;
	PacketSinkHelper sinkhelper("ns3::TcpSocketFactory" , InetSocketAddress(Ipv4Address::GetAny() , port_no));
	ApplicationContainer serverapp = sinkhelper.Install(apnode);		//install the sink application into the AP and save it in a container
	sink = StaticCast<PacketSink> (serverapp.Get(0));
	std::cout<<"\nAP sink setup complete!\n";
	
// Install TCP transmitters on STAs

	Ipv4Address serverIp = apinterface.GetAddress(0);
	OnOffHelper clients ("ns3::TcpSocketFactory" , (InetSocketAddress (serverIp , port_no)));		//clients are given an OnOffApplication and the socket address of the server sink application
	clients.SetAttribute ("PacketSize", UintegerValue (payload));
   clients.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
   clients.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
   clients.SetAttribute ("DataRate", DataRateValue (DataRate (datarate)));
   //clients.SetAttribute("MaxPackets" , UintegerValue(10));
   ApplicationContainer clientapp = clients.Install (stanodes); 
   std::cout<<"\nSTA transmitter setup complete!\n";
	
// Setup flow monitor

	Ptr<FlowMonitor> flowmonitor;
	FlowMonitorHelper flowhelper;
	flowmonitor = flowhelper.InstallAll();
	std::cout<<"\nFlow Monitor setup complete!\n";	
  
// Start/Stop applications at nodes

	serverapp.Start(Seconds(0.0));		//start sink server application at t=0
	
	std::srand(time(NULL));				//set seed of random number gen to current time
	int startTime = (double)((std::rand()%5)+1);
	clientapp.Start(Seconds(startTime));
	
// Simulation start/stop

	Simulator::Stop(Seconds(simulation_time));
	std::cout<<"\nSTARTING SIMULATION!\n";
	
	if(pcapTrace){																					//Get .pcap trace file
		std::string filename;
		wifiPhy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11_RADIO);
		filename = std::string("AP") + RtsCtsThreshold;
		wifiPhy.EnablePcap(filename , apnode);
		filename = std::string("STA") + RtsCtsThreshold;
		wifiPhy.EnablePcap(filename , stanodes);
	}
	
	if(asciiTrace){																				//Get .tr trace file
		AsciiTraceHelper ascii;
		std::string filename;
		filename = std::string("ns3wifi") + RtsCtsThreshold + std::string(".tr");
		wifiPhy.EnableAsciiAll(ascii.CreateFileStream(filename));
	}
	
	
	Simulator::Run();
	std::string filename;
	filename = std::string("FlowMonitorOp") + RtsCtsThreshold + std::string(".xml");
	flowmonitor->SerializeToXmlFile(filename , true , true);
	Simulator::Destroy();
	
	double averageThroughput = ((sink->GetTotalRx()*8)/(1e6*simulation_time));
	std::cout << "\nAverage throughput: "<<averageThroughput<<" Mbit/s\n";

	return 0;
}
	
