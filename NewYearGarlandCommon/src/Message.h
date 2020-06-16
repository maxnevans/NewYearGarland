#include "pch.h"

enum class ClientMessageType
{
    CONNECT,
    DISCONNECT,
};

struct ClientMessage
{
    ClientMessageType type;
    DWORD pid;
};

enum class ServerMessageType
{
    CONNECT,
    DISCONNECT,
    LIGHT
};

struct ServerMessageConnect
{
    struct {
        unsigned char r;
        unsigned char g;
        unsigned char b;
    } color;
};

struct ServerMessageLight
{
    bool isPowered;
};

struct ServerMessageDisconnect
{
    // TODO: fill message with server reason for disconnect
};

struct ServerMessage {
    ServerMessageType type;
    union {
        ServerMessageConnect connect;
        ServerMessageLight light;
        ServerMessageDisconnect disconnect;
    };
};