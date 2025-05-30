// MQTT CONFIG
#define DEVICE_TELEMETRY_TOPIC "v1/devices/me/telemetry"
#define DEVICE_ATTRIBUTES_TOPIC "v1/devices/me/attributes"
#define DEVICE_ATTRIBUTES_TOPIC_REQUEST "v1/devices/me/attributes/request/"
#define DEVICE_ATTRIBUTES_TOPIC_RESPONSE "v1/devices/me/attributes/response/"
#define DEVICE_RPC_TOPIC_REQUEST "v1/devices/me/rpc/request/"
#define DEVICE_RPC_TOPIC_RESPONSE "v1/devices/me/rpc/response/"

#define GATEWAY_TOPIC_CONNECT " v1/gateway/connect"
#define GATEWAY_TOPIC_DISCONNECT " v1/gateway/disconnect"
#define GATEWAY_TELEMETRY_TOPIC "v1/gateway/telemetry"
#define GATEWAY_RPC_TOPIC "v1/gateway/rpc"
#define GATEWAY_CLAIMING_TOPIC "v1/gateway/claiming"
#define GATEWAY_ATTRIBUTES_TOPIC "v1/gateway/attributes"
#define GATEWAY_ATTRIBUTES_TOPIC_REQUEST "v1/gateway/attributes/request"
#define GATEWAY_ATTRIBUTES_TOPIC_RESPONSE "v1/gateway/attributes/response"


// DEVICE JSON CONFIG 
#define WRITE_FILE "w"
#define READ_FILE "r"
#define FILE_CONFIG_NAME "mindgateway.json"
#define FILE_CONFIG_PATH "/mindgateway.json"

// HTTP CONFIG
const char HTTP_MESSAGE_500[] PROGMEM = {"{\"message\":\"Internal Server Error\"}"};
const char HTTP_MESSAGE_SAVE_FAILED[] PROGMEM = {"{\"message\":\"Save failed\"}"};
const char HTTP_MESSAGE_SAVED[] PROGMEM = {"{\"message\":\"Saved\"}"};
// #define HTTP_MESSAGE_500 "{\"message\":\"Internal Server Error\"}"
// #define HTTP_MESSAGE_SAVED "{\"message\":\"Saved\"}"

// Compress
// #define var32(val1, val2) ((uint32_t) ((uint32_t) val2 << 16 | (uint32_t) val1))
// #define mergevar(val1, val2, val3, val4) ((uint64_t) ((uint64_t) val4 << 48 | (uint64_t) val3 << 32 | (uint64_t) val2 << 16 | (uint64_t) val1))

// Struct

// struct Web {
//     char user[32];
//     char pass[32];
//     char host[32];
//     unsigned short port;
//     bool auth;
// };

// struct Network {
//     char ip[32];
//     char mask[32];
//     char gw[32];
//     char dns[32];
//     bool dhcp;
// };

// struct System
// {
//     bool dhcp;
//     char ip[16];
//     char mask[16];
//     char gw[16];
//     char dns[16];
//     bool ntp;
//     char ntph[32];
//     unsigned short ntpp;
//     char tz;
//     bool rtc;
// };
