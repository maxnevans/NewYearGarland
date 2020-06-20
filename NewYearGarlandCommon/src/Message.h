#include "pch.h"

enum class ServerMessageType
{
    LIGHT,
    COLOR
};

struct ServerMessageColor
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
};

struct ServerMessageLight
{
    bool isPowered;
};

struct ServerMessage {
    ServerMessageType type;
    union {
        ServerMessageLight light;
        ServerMessageColor color;
    };
};