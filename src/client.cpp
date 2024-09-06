/*
#include <client.hpp>

void client::startClient() {
    ENetAddress address;
    ENetHost* client;

    client = enet_host_create(NULL, 1, 2, 0, 0);

    if (client == NULL) {
        fprintf(stderr, "An error occurred while trying to create an ENet client host.\n");
        exit(EXIT_FAILURE);
    }

    enet_address_set_host(&address, "127.0.0.1");
    address.port = 1234;
    peer = enet_host_connect(client, &address, 2, 0);

    if (peer == NULL) {
        fprintf(stderr, "No available peers for initiating an ENet connection.\n");
        exit(EXIT_FAILURE);
    }

    ENetEvent event;
    while (enet_host_service(client, &event, 1000) > 0) {
        switch (event.type) {
        case ENET_EVENT_TYPE_CONNECT:
            printf("Connected to server.\n");
            break;
        case ENET_EVENT_TYPE_RECEIVE:
            // Handle received data
            enet_packet_destroy(event.packet);
            break;
        case ENET_EVENT_TYPE_DISCONNECT:
            printf("Disconnected from server.\n");
            break;
        }
    }

    enet_host_destroy(client);
}

void client::sendPlayerData(const player& player) {
    ENetPacket* packet = enet_packet_create(&player, sizeof(player), ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(peer, 0, packet);
}

*/