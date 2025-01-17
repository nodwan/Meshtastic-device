#include "RoutingPlugin.h"
#include "MeshService.h"
#include "NodeDB.h"
#include "Router.h"
#include "configuration.h"
#include "main.h"

RoutingPlugin *routingPlugin;

bool RoutingPlugin::handleReceivedProtobuf(const MeshPacket &mp, const Routing *r)
{
    printPacket("Routing sniffing", &mp);
    router->sniffReceived(&mp, r);

    // FIXME - move this to a non promsicious PhoneAPI plugin?
    // Note: we are careful not to send back packets that started with the phone back to the phone
    if ((mp.to == NODENUM_BROADCAST || mp.to == nodeDB.getNodeNum()) && (mp.from != 0)) {
        printPacket("Delivering rx packet", &mp);
        service.handleFromRadio(&mp);
    }
        
    return false; // Let others look at this message also if they want
}


MeshPacket *RoutingPlugin::allocReply()
{
    assert(currentRequest);

    // We only consider making replies if the request was a legit routing packet (not just something we were sniffing)
    if(currentRequest->decoded.portnum == PortNum_ROUTING_APP) {
        assert(0); // 1.2 refactoring fixme, Not sure if anything needs this yet?
        // return allocDataProtobuf(u);
    }
    return NULL;
}

void RoutingPlugin::sendAckNak(Routing_Error err, NodeNum to, PacketId idFrom)
{
    Routing c = Routing_init_default;
    
    c.error_reason = err;

    auto p = allocDataProtobuf(c);
    p->priority = MeshPacket_Priority_ACK;

    p->hop_limit = 0; // Assume just immediate neighbors for now
    p->to = to;
    p->decoded.request_id = idFrom;
    DEBUG_MSG("Sending an err=%d,to=0x%x,idFrom=0x%x,id=0x%x\n", err, to, idFrom, p->id);

    router->sendLocal(p); // we sometimes send directly to the local node
}

RoutingPlugin::RoutingPlugin()
    : ProtobufPlugin("routing", PortNum_ROUTING_APP, Routing_fields)
{
    isPromiscuous = true;
}


