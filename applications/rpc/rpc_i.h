#pragma once
#include "rpc.h"
#include "pb.h"
#include "pb_decode.h"
#include "pb_encode.h"
#include "flipper.pb.h"

typedef void* (*RpcSystemAlloc)(Rpc*);
typedef void (*RpcSystemFree)(void*);
typedef void (*PBMessageHandler)(const PB_Main* msg_request, void* context);

typedef struct {
    bool (*decode_submessage)(pb_istream_t* stream, const pb_field_t* field, void** arg);
    PBMessageHandler message_handler;
    void* context;
} RpcHandler;

void rpc_encode_and_send(Rpc* rpc, PB_Main* main_message);
void rpc_encode_and_send_empty(Rpc* rpc, uint32_t command_id, PB_CommandStatus status);
void rpc_add_handler(Rpc* rpc, pb_size_t message_tag, RpcHandler* handler);

void* rpc_system_status_alloc(Rpc* rpc);
void* rpc_system_storage_alloc(Rpc* rpc);
void rpc_system_storage_free(void* ctx);
void rpc_print_message(const PB_Main* message);
