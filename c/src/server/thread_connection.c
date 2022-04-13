#include <server/thread_connection.h>
#include <common/log.h>
#include <common/message.h>
#include <common/network.h>
#include <server/shared.h>

void *gl_thread_connection_main(void *arg) {
    uint32_t id = *(uint32_t *)arg;
    
    gl_log_push("connection %d thread started.\n", id);
    
    while (1) {
        gl_message_t msg = {0};
        int32_t r = gl_message_read(g_client_sockets[id], &msg);
        
        if (r > 0) {
            gl_message_printf(&msg);
            gl_message_free(&msg);
        } else {
            break;
        }
    }
    
    gl_socket_close(g_client_sockets[id]);
    
    gl_log_push("connection %d thread stopped.\n", id);
    
    return 0;
}