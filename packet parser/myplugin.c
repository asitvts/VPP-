//#include <vlib/cli.h>
#include <vlib/vlib.h>
#include <vlib/unix/plugin.h>
#include <vnet/ethernet/packet.h>
#include <vnet/ip/ip_packet.h>
#include <vnet/vnet.h>
#include <vppinfra/types.h>
#include <vnet/feature/feature.h>
#include <vnet/ethernet/ethernet.h>
#include <vnet/ip/ip.h>

static u32 packet_count = 0;

// Packet processing function
static uword packet_proc_func(vlib_main_t* vm, vlib_node_runtime_t* node, vlib_frame_t* frame) {
    (void)node;
    u32 n_packets = frame->n_vectors;
    packet_count += n_packets;

    u32 *buffers = vlib_frame_vector_args(frame);

    for (u32 i = 0; i < n_packets; i++) {
        u32 bi = buffers[i];
        vlib_buffer_t *b = vlib_get_buffer(vm, bi);

        u8 *pkt_data = b->data;
        u32 pkt_len = vlib_buffer_length_in_chain(vm, b);

        if (pkt_len < sizeof(ethernet_header_t)) {
            continue; // Not enough data for Ethernet
        }

        ethernet_header_t *eth = (ethernet_header_t *)pkt_data;

        // Optional: print MAC addresses (debug)
        vlib_cli_output(vm, "Packet %u: src %02x:%02x:%02x:%02x:%02x:%02x dst %02x:%02x:%02x:%02x:%02x:%02x",
            i,
            eth->src_address[0], eth->src_address[1], eth->src_address[2],
            eth->src_address[3], eth->src_address[4], eth->src_address[5],
            eth->dst_address[0], eth->dst_address[1], eth->dst_address[2],
            eth->dst_address[3], eth->dst_address[4], eth->dst_address[5]);

        // Parse IPv4 if present

        if (ntohs(eth->type) == ETHERNET_TYPE_IP4 &&
        pkt_len >= sizeof(ethernet_header_t) + sizeof(ip4_header_t)) {

            ip4_header_t *ip4 =
            (ip4_header_t *)(pkt_data + sizeof(ethernet_header_t));

            vlib_cli_output(vm, "protocol is %d\n", ip4->protocol);
            // Parse TCP if present
            if (ip4->protocol == IP_PROTOCOL_TCP ) {
                vlib_cli_output(vm, "tcp packet\n");
            }

            // Parse ICMP (Ping)
            else if (ip4->protocol == IP_PROTOCOL_ICMP) {
                vlib_cli_output(vm, "icmp packet\n");
            }
            
            // ARP
            else if (ip4->protocol == IP_PROTOCOL_ICMP) {
                vlib_cli_output(vm, "ARP packet\n");
            }

            else if (ip4->protocol == IP_PROTOCOL_IGMP) {
                vlib_cli_output(vm, "IGMP packet\n");
            }

            else if (ip4->protocol == IP_PROTOCOL_UDP) {
                vlib_cli_output(vm, "UDP packet\n");
            }

            else{
                vlib_cli_output(vm, "some other form of packet\n");
            }
        
        }
        else if(ntohs(eth->type) == ETHERNET_TYPE_ARP){
            vlib_cli_output(vm, "ARP packet\n");
        }
        else{
             vlib_cli_output(vm, "some other form of packet\n");
        }

        vlib_cli_output(vm, "\n\n");
    }

    return n_packets;
}

// Register node
VLIB_REGISTER_NODE(my_simple_node) = {
    .name = "processing-node",
    .function = packet_proc_func,
    .type = VLIB_NODE_TYPE_INTERNAL,
    .n_next_nodes = 1,
    .next_nodes = {"error-drop"}
};

// Attach node to device-input feature arc
VNET_FEATURE_INIT(my_feature, static) = {
    .arc_name = "device-input",
    .node_name = "processing-node",
    .runs_before = VNET_FEATURES("ethernet-input")
};

// Plugin registration
VLIB_PLUGIN_REGISTER() = {
    .description = "Packet parsing plugin with node registration",
    .version = "1.0",
};

// CLI function to show packet count
clib_error_t* packet_counter_func(vlib_main_t* vm, unformat_input_t* input, vlib_cli_command_t* cmd) {
    (void)input;
    (void)cmd;
    vlib_cli_output(vm, "Total packets seen: %d\n", packet_count);
    return 0;
}

// CLI command registration
VLIB_CLI_COMMAND(packet_counter) = {
    .path = "show my_packet_counter",
    .short_help = "Shows the number of packets going through this node",
    .function = packet_counter_func,
};