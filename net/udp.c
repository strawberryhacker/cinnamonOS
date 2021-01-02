
#include <net/udp.h>
#include <citrus/mem.h>
#include <citrus/kmalloc.h>
#include <stddef.h>
#include <net/ip.h>
#include <net/mac.h>
#include <citrus/error.h>
#include <citrus/panic.h>
#include <citrus/mem.h>

// Global UDP module
static struct udp udp;

void udp_init(void)
{
    list_init(&udp.ports);
}

void add_netbuf_to_port(struct netbuf* buf, u16 port)
{
    struct list_node* node;
    list_iterate(node, &udp.ports) {

        struct udp_port* p = list_get_entry(node, struct udp_port, node);

        if (p->port == port) {
            list_add_first(&buf->node, &p->packets);
        }
    }
}

struct netbuf* get_netbuf_from_port(u16 port)
{
    struct list_node* node;
    list_iterate(node, &udp.ports) {

        struct udp_port* p = list_get_entry(node, struct udp_port, node);

        if (p->port == port) {

            if (p->packets.next == &p->packets)
                return NULL;

            struct list_node* first_node = p->packets.next;
            list_delete_first(&p->packets);
            return list_get_entry(first_node, struct netbuf, node);
        }
    }
    return NULL;
}

struct netbuf* udp_rec(u16 port)
{
    struct netbuf* buf = get_netbuf_from_port(port);
    if (buf) {
        return buf;
    }
    
    mac_receive();

    buf = get_netbuf_from_port(port);
    if (buf) {
        return buf;
    }

    return NULL;
}

// Final step if this is a UDP packet
void udp_handle(struct netbuf* buf)
{
    u16 length = read_be16(buf->ptr + 4) - 8;

    u16 src_port = read_be16(buf->ptr + 0);
    u16 dest_port = read_be16(buf->ptr + 2);

    print("%d - %d \n", src_port, dest_port);

    buf->ptr += 8;
    buf->frame_len -= 8;

    add_netbuf_to_port(buf, dest_port);
}

void pp(void)
{
    struct list_node* port_node;
    struct list_node* packet_node;

    list_iterate(port_node, &udp.ports) {

        struct udp_port* port = list_get_entry(port_node, struct udp_port, node);
        print("Got a new port %d\n", port->port);
        list_iterate(packet_node, &port->packets) {

            struct netbuf* buf = list_get_entry(packet_node, struct netbuf, node);
            print("    New packet => %d\n", buf->number);
        }
    }
}

// Make a new port link
void udp_listen(u16 port)
{
    struct udp_port* udp_port = kmalloc(sizeof(struct udp_port));

    // Initialize the port packet list
    list_init(&udp_port->packets);

    // Add the port to the global UDP structure
    list_add_first(&udp_port->node, &udp.ports);

    udp_port->port = port;
}

// This takes in a packet buffer. The frame_length field in the netbuf should
// be set to the total packet size
void udp_send(struct netbuf* buf, u32 ip, u16 port)
{
    buf->ptr -= 4;

    // Lenght of the UDP packet including UDP header
    store_be16(buf->frame_len + 8, buf->ptr);
    buf->ptr -= 2;

    // Destination port
    store_be16(port, buf->ptr);
    buf->ptr -= 2;

    // Source port
    store_be16(port, buf->ptr);

    buf->frame_len += 8;

    ip_send(buf, ip);
}
