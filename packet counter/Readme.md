

````markdown
# VPP TAP Interface Packet Counter Plugin

This project includes a **VPP plugin** that counts packets passing through a custom node, along with the commands to run it and set up a TAP interface on Linux.

---

## Plugin Code

```c
#include <vlib/vlib.h>
#include <vlib/unix/plugin.h>
#include <vnet/vnet.h>
#include <vppinfra/types.h>
#include <vnet/feature/feature.h>

static u32 packet_count=0;

// Packet processing function
static uword packet_proc_func(vlib_main_t* vm, vlib_node_runtime_t* node, vlib_frame_t* frame){
    (void)node;
    (void)vm;

    u32 n_packets = frame->n_vectors;
    packet_count += n_packets;
    return n_packets;
}

// Registering the node
VLIB_REGISTER_NODE(my_simple_node) = {
    .name = "processing-node",
    .function = packet_proc_func,
    .type = VLIB_NODE_TYPE_INTERNAL,
    .n_next_nodes = 1,
    .next_nodes = {"error-drop"}
};

// Attach the node to the device-input feature arc
VNET_FEATURE_INIT(my_feature, static) = {
    .arc_name = "device-input",
    .node_name = "processing-node",
    .runs_before = VNET_FEATURES("ethernet-input")
};

// Registering the plugin
VLIB_PLUGIN_REGISTER() = {
    .description = "my own plugin with node registration",
    .version = "1.0",
};

// CLI function to show packet count
clib_error_t* packet_counter_func(vlib_main_t* vm, unformat_input_t* input, vlib_cli_command_t* cmd){
    (void)input;
    (void)cmd;
    vlib_cli_output(vm, "packet count : %d\n", packet_count);
    return 0;
}

// CLI command registration
VLIB_CLI_COMMAND(packet_counter) = {
    .path = "show my_packet_counter",
    .short_help = "shows the number of packets going through this node",
    .function = packet_counter_func,
};
````

---

## Running the Plugin in VPP

1. **Build and run VPP with the plugin:**

```bash
make build-release
make run-release
```

2. **VPP CLI commands to set up the TAP interface:**

```bash
create tap id 0 host-if-name vpp-tap0
set interface state tap0 up
set interface ip address tap0 10.10.1.1/24
set interface feature tap0 processing-node arc device-input
```

---

## Linux Host Setup

```bash
sudo ip addr add 10.10.1.2/24 dev vpp-tap0
sudo ip link set vpp-tap0 up
ping 10.10.1.1
```

* Assigns an IP to the host side of the TAP interface.
* Brings the interface up in Linux.
* Test connectivity with VPP via ping.

---

## Notes

* `processing-node` in the command specifies **the entry node in VPP’s packet processing graph**.
* `device-input` is **the node itself** that handles packets arriving from TAP.
* Without attaching the interface to the node using the `processing-node` command, packets from the TAP interface **will not be processed**.
* You can check the packet count in VPP using:

```bash
show my_packet_counter
```

```

---


