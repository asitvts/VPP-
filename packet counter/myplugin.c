#include <vlib/vlib.h>
#include <vlib/unix/plugin.h>
#include <vnet/vnet.h>
#include <vppinfra/types.h>
#include <vnet/feature/feature.h>


static u32 packet_count=0;


static uword packet_proc_func(vlib_main_t* vm, vlib_node_runtime_t* node, vlib_frame_t* frame){
    (void)node;
    (void)vm;

    u32 n_packets = frame->n_vectors;
    packet_count+= n_packets;
    //vlib_cli_output(vm, "inside the self created node currently");
    // cli_outputs can slow things down

    return n_packets;
}


VLIB_REGISTER_NODE(my_simple_node)={
    .name = "processing-node",
    .function = packet_proc_func,
    .type = VLIB_NODE_TYPE_INTERNAL,
    .n_next_nodes=1,
    .next_nodes={"error-drop"}
};




VNET_FEATURE_INIT(my_feature, static)={
    .arc_name="device-input",
    .node_name="processing-node",
    .runs_before= VNET_FEATURES("ethernet-input")
};

/*
device-input    ip4-input-no-checksum [0]
                ip4-input [1]      
                ip6-input [2]      
                mpls-input [3]      
                ethernet-input [4]    
                error-drop [5]      
                ip4-drop [6]       
                ip6-drop [7]       
                punt-dispatch [8]    
                esp4-decrypt-tun [9]   
                esp6-decrypt-tun [10]
*/




// registering the plugin
VLIB_PLUGIN_REGISTER()={
    .description="my own plugin with node registration",
    .version="1.0",
};







// cli func and it's command's registration
clib_error_t* packet_counter_func(vlib_main_t* vm, unformat_input_t* input, vlib_cli_command_t* cmd){

    (void)input;
    (void)cmd;
    vlib_cli_output(vm, "packet count : %d\n", packet_count);

    return 0;
}

VLIB_CLI_COMMAND(packet_counter)={
    .path = "show my_packet_counter",
    .short_help = "shows the number of packets going through this node",
    .function = packet_counter_func,
};