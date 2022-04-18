#include <common/types.h>
#include <errno.h>
#include <stdlib.h>
#include <getopt.h>
#include <pthread.h>
#include <cimgui/cimgui.h>
#include <common/log.h>
#include <common/gui.h>
#include <common/utils.h>
#include <common/memory.h>
#include <common/network.h>
#include <common/command.h>
#include <server/shared.h>
#include <server/command.h>
#include <server/thread_tcp.h>
#include <common/message.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include "message.h"
#include "common/array.h"
#include "common/game.h"
#include "common/maze.h"

static void draw_main_gui();

static int32_t g_exit_code = EXIT_SUCCESS;
static bool g_show_console = true;

int main(int argc, char **argv) {
    errno = 0;

    static struct option opts[] = {
        { "ip", required_argument, 0, 'i' },
        { "port", required_argument, 0, 'p' },
        { "multi-ip", required_argument, 0, 'I' },
        { "multi-port", required_argument, 0, 'P' },
        { "legacy-protocol", required_argument, 0, 'l' },
        { "help", no_argument, 0, 'h' },
        { "version", no_argument, 0, 'v' },
        {0, 0, 0, 0}
    };
    int32_t used_unknown_opt = 0;
    int32_t opt;
    while ((opt = getopt_long(argc, argv, "i:p:I:P:lhv", opts, 0)) != -1) {
        switch (opt) {
        case 'i':
            // TODO: Check if ip is valid, if invalid use default
            g_server_ip = gl_strdup(optarg);
            break;
        case 'p':
            // TODO: Check if port is valid, if invalid use default
            g_server_port = gl_strdup(optarg);
            break;
        case 'I':
            // TODO: Check if ip is valid, if invalid use default
            g_multicast_ip = gl_strdup(optarg);
            break;
        case 'P':
            // TODO: Check if port is valid, if invalid use default
            g_multicast_port = gl_strdup(optarg);
            break;
        case 'l':
            g_legacy_protocol = true;
            break;
        case 'h':
            gl_log_push("%s", g_help);
            goto cleanup;
        case 'v':
            gl_log_push("version: " GHOSTLAB_VERSION);
            goto cleanup;
        case '?':
            used_unknown_opt = 1;
            gl_log_push_error("use `-h` for more informations.\n");
            break;
        default:
            used_unknown_opt = 1;
            gl_log_push_error("option not yet implemented `%c`!\n", opt);
            gl_log_push_error("use `-h` for more informations.\n");
        }
    }

    if (used_unknown_opt) {
        goto error;
    }
    
    if (!g_server_ip) {
        g_server_ip = gl_strdup(GHOSTLAB_DEFAULT_SERVER_IP);
    }
    if (!g_server_port) {
        g_server_port = gl_strdup(GHOSTLAB_DEFAULT_SERVER_PORT);
    }
    if (!g_multicast_ip) {
        g_multicast_ip = gl_strdup(GHOSTLAB_DEFAULT_MULTICAST_IP);
    }
    if (!g_multicast_port) {
        g_multicast_port = gl_strdup(GHOSTLAB_DEFAULT_MULTICAST_PORT);
    }
    
    gl_message_set_mutex(g_gameplay_mutex);
    gl_message_add_functions();

    g_thread_tcp = gl_malloc(sizeof(pthread_t));
    pthread_create(g_thread_tcp, 0, gl_thread_tcp_main, 0);
    
    gl_gui_create("Ghostlab Server");

    while (!g_quit) {
        gl_gui_start_render(&g_quit);
        draw_main_gui();
        gl_gui_end_render();
    }

    goto cleanup;

    error:
    g_exit_code = -1;

    cleanup:
    {
        gl_message_t msg = {.type = GL_MESSAGE_TYPE_MULTI, 0};
        gl_message_execute(&msg, 0, 0);
    }

    gl_socket_close(&g_server_socket);

    if (g_thread_tcp) {
        pthread_join(*(pthread_t *)g_thread_tcp, 0);
        gl_free(g_thread_tcp);
    }
    
    for (uint32_t i = 0; i < gl_array_get_size(g_games); i++) {
        gl_maze_free(g_games[i].maze);
        gl_array_free(g_games[i].ghosts);
        gl_array_free(g_games[i].players);
    }
    gl_array_free(g_games);
    
    gl_free(g_server_ip);
    gl_free(g_server_port);
    gl_free(g_multicast_ip);
    gl_free(g_multicast_port);
    
    gl_log_free();

    gl_memory_check_for_leaks();

    gl_gui_free();

    return g_exit_code;
}

static void draw_main_gui() {
    {
        gl_igBegin("Ghostlab Server", g_show_console ? 0.6f : 1.0f);

        if (igBeginMenuBar()) {
            if (igBeginMenu("Options", true)) {
                igMenuItemBoolPtr("Show Logs ", 0, &g_show_console, true);
                igMenuItemBoolPtr("Quit", 0, &g_quit, true);
                igEndMenu();
            }
            igEndMenuBar();
        }
    
        if (gl_array_get_size(g_games) > 0) {
            if (igCollapsingHeaderTreeNodeFlags("Available games", 0)) {
                for (uint32_t i = 0; i < gl_array_get_size(g_games); i++) {
                    if (igCollapsingHeaderTreeNodeFlags(g_games[i].name, 0)) {
                        igText("There are %d player(s) connected%s", gl_array_get_size(g_games[i].players), gl_array_get_size(g_games[i].players) == 0 ? "." : ":");
                        for (uint32_t j = 0; j <  gl_array_get_size(g_games[i].players); j++) {
                            igText("- %s", g_games[i].players[j].id);
                        }
                    }
                }
            }
        }

        igEnd();
    }

    if (g_show_console) {
        gl_igConsole(gl_command_definitions(), GL_COMMAND_TYPE_COUNT);
    }
}
