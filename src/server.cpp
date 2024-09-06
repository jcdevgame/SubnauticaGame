/*
#include <server.hpp>

void server::startServer() {
    ENetAddress address;
    ENetHost* server;

    address.host = ENET_HOST_ANY;
    address.port = 1234;
    server = enet_host_create(&address, 32, 2, 0, 0);

    if (server == NULL) {
        fprintf(stderr, "An error occurred while trying to create an ENet server host.\n");
        exit(EXIT_FAILURE);
    }

    ENetEvent event;
    while (true) {
        while (enet_host_service(server, &event, 1000) > 0) {
            switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT:
                printf("A new client connected from %x:%u.\n", event.peer->address.host, event.peer->address.port);
                break;
            case ENET_EVENT_TYPE_RECEIVE:
                // Handle received data
                enet_packet_destroy(event.packet);
                break;
            case ENET_EVENT_TYPE_DISCONNECT:
                printf("Client disconnected.\n");
                break;
            }
        }
    }

    enet_host_destroy(server);
}
*/