#include <string.h>
#include "server_command.h"
#include "command.h"
#include "print.h"
#include "server_common.h"

static void command_help(void *arg) {
    gl_printf("commands:\n");
    gl_printf("\tq, e, quit, exit   terminates the program.\n");
    gl_printf("\th, help            displays this help message.\n");
    gl_printf("\tv, version         display the program's version.\n");
}

static const gl_command_definition_t g_command_help = {
    .function = command_help,
    .aliases = {
        "help",
        "h",
        0
    }
};

static void command_version(void *arg) {
    gl_printf("version: " GHOSTLAB_VERSION "\n");
}

static const gl_command_definition_t g_command_version = {
    .function = command_version,
    .aliases = {
        "version",
        "v",
        0
    }
};

static void command_quit(void *arg) {
    g_quit = true;
}

static const gl_command_definition_t g_command_quit = {
    .function = command_quit,
    .aliases = {
        "quit",
        "exit",
        "q",
        "e",
        0
    }
};

static const gl_command_definition_t *gl_command_definitions_array[] = {
    [GL_COMMAND_TYPE_HELP] = &g_command_help,
    [GL_COMMAND_TYPE_VERSION] = &g_command_version,
    [GL_COMMAND_TYPE_QUIT] = &g_command_quit,
    
    [GL_COMMAND_TYPE_COUNT] = 0
};

const gl_command_definition_t **gl_command_definitions() {
    return gl_command_definitions_array;
}
