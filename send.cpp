#include "oscit/send.h"
#include "oscit/object.h"
#include "osc/OscOutboundPacketStream.h"
#include "ip/UdpSocket.h"

namespace oscit {

#define OSC_OUT_BUFFER_SIZE 2048

OscSend::OscSend(const IpEndpointName& pRemoteEndpoint)
{
  mSocket = new UdpTransmitSocket( pRemoteEndpoint );
}

OscSend::OscSend(const IpEndpointName& pRemoteEndpoint, int pPort)
{ 
	mSocket = new UdpTransmitSocket( IpEndpointName( pRemoteEndpoint.address, pPort ));
  
  // debugging
  char hostIpAddress[ IpEndpointName::ADDRESS_STRING_LENGTH ];
  // get host ip as string
  pRemoteEndpoint.AddressAsString(hostIpAddress);
  std::cout << "Reply to " << hostIpAddress << ":" << pPort << ".\n";
}

OscSend::~OscSend()
{
  delete mSocket;
}

void OscSend::send(const osc::OutboundPacketStream& pMsg)
{
  mSocket->Send(pMsg.Data(), pMsg.Size());
}

void OscSend::send(const std::string& pUrl, const Value val)
{
  // char buffer[OSC_OUT_BUFFER_SIZE]; // TODO: reuse buffer
  // osc::OutboundPacketStream oss( buffer, OSC_OUT_BUFFER_SIZE );
  // oss << osc::BeginBundleImmediate << osc::BeginMessage(pUrl.c_str()) << val << osc::EndMessage << osc::EndBundle;
  // send(oss);
}

void OscSend::send_all(std::list<OscSend*>& pRecipients, const std::string& pUrl, const char * pTypeTags, const Value pVal)
{ 
  std::list<OscSend*>::iterator it  = pRecipients.begin();
  std::list<OscSend*>::iterator end = pRecipients.end();
  
  //if (it == end) return;
  // TODO: optimize: keep this thing between queries (make this not a static and let the first recipient send to all others).
  char buffer[OSC_OUT_BUFFER_SIZE]; // TODO: reuse buffer
  osc::OutboundPacketStream oss( buffer, OSC_OUT_BUFFER_SIZE );
  oss << osc::BeginBundleImmediate << osc::BeginMessage(pUrl.c_str());
  values_to_stream(oss, pTypeTags, pVal);
  oss << osc::EndMessage << osc::EndBundle;
  
  while (it != end) (*it++)->send(oss);
}

void OscSend::values_to_stream (osc::OutboundPacketStream& pOut, const char * pTypeTags, const Value pVal)
{ 
  uint i = 0;
  char c;
  const Value * val = &pVal;
  if (strlen(pTypeTags) > 1) {
    val = val->values;
  }
  
  while ( (c = pTypeTags[i]) ) {
    switch (c)
    {
      case REAL_TYPE:
        pOut << val[i].r;
        break;
      case STRING_TYPE:
        pOut << val[i].s;
        break;
      default:  
        fprintf(stderr, "Unknown value type '%c'. Cannot send to osc.\n", c);
        break;
    }
    i++;
  }
}

} // namespace oscit