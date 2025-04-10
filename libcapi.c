#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dlfcn.h>
#include <sys/wait.h>
#include "libcapi.h"

#define MAX_VARS 100
#define MAX_FUNCS 50
#define MAX_LINES 512

typedef struct {
    char name[64];
    char value[256];
} CapiVar;

typedef struct {
    char name[64];
    char lines[MAX_LINES][256];
    int line_count;
} CapiFunction;

static CapiVar vars[MAX_VARS];
static int var_count = 0;

static CapiFunction funcs[MAX_FUNCS];
static int func_count = 0;

static void set_variable(const char *name, const char *value) {
    for (int i = 0; i < var_count; i++) {
        if (strcmp(vars[i].name, name) == 0) {
            strcpy(vars[i].value, value);
            return;
        }
    }
    strcpy(vars[var_count].name, name);
    strcpy(vars[var_count].value, value);
    var_count++;
}

static const char *get_variable(const char *name) {
    for (int i = 0; i < var_count; i++) {
        if (strcmp(vars[i].name, name) == 0) {
            return vars[i].value;
        }
    }
    return "";
}

static void run_function(const char *name, int client_socket) {
    for (int i = 0; i < func_count; i++) {
        if (strcmp(funcs[i].name, name) == 0) {
            for (int j = 0; j < funcs[i].line_count; j++) {
                char *line = funcs[i].lines[j];
                if (strstr(line, "echo:")) {
                    char var[64];
                    sscanf(line, "echo: %[^;];", var);
                    const char *val = get_variable(var);
                    write(client_socket, val, strlen(val));
                }
            }
        }
    }
}

static void execute_shell(const char *cmd, int client_socket) {
    FILE *fp = popen(cmd, "r");
    if (!fp) return;
    char out[256];
    while (fgets(out, sizeof(out), fp)) {
        write(client_socket, out, strlen(out));
    }
    pclose(fp);
}

static void process_line(const char *line, int client_socket, CapiResponse *resp);

static void include_file(const char *path, int client_socket, CapiResponse *resp) {
    FILE *file = fopen(path, "r");
    if (!file) return;
    char line[512];
    while (fgets(line, sizeof(line), file)) {
        process_line(line, client_socket, resp);
    }
    fclose(file);
}

static void load_module(const char *mod_path) {
    void *handle = dlopen(mod_path, RTLD_LAZY);
    if (!handle) return;

    void (*mod_init)() = dlsym(handle, "capi_mod_init");
    if (mod_init) mod_init();
}

static void process_line(const char *line, int client_socket, CapiResponse *resp) {
    if (strncmp(line, "#", 1) == 0) return;

    if (strstr(line, "int ") == line) {
        char name[64], val[256];
        sscanf(line, "int %[^=]= \"%[^\"]", name, val);
        set_variable(name, val);
    } else if (strstr(line, "echo:") == line) {
        char var[64];
        sscanf(line, "echo: %[^;];", var);
        const char *val = get_variable(var);
        write(client_socket, val, strlen(val));
    } else if (strstr(line, "header:") == line) {
        char hdr[256];
        sscanf(line, "header: \"%[^\"]", hdr);
        if (resp->header_count < MAX_HEADERS) {
            strcpy(resp->headers[resp->header_count++], hdr);
        }
    } else if (strstr(line, "http.code:") == line) {
        sscanf(line, "http.code: %d;", &resp->status_code);
    } else if (strstr(line, "capi.forceclose()") != NULL) {
        resp->force_close = 1;
    } else if (strstr(line, "capi.exec:") == line) {
        char cmd[256];
        sscanf(line, "capi.exec: \"%[^\"]", cmd);
        execute_shell(cmd, client_socket);
    } else if (strstr(line, "capi.loadmod(") == line) {
        char mod[256];
        sscanf(line, "capi.loadmod(\"%[^\"]", mod);
        load_module(mod);
    } else if (strstr(line, "incl <") == line) {
        char incl_path[256];
        sscanf(line, "incl <%[^>]", incl_path);
        include_file(incl_path, client_socket, resp);
    } else if (strstr(line, "function ") == line) {
        CapiFunction *f = &funcs[func_count++];
        sscanf(line, "function %[^:]:", f->name);
        f->line_count = 0;
    } else if (strstr(line, "funclose;") == line) {
        // Nothing to do
    } else if (strchr(line, '(') && strchr(line, ')')) {
        char fname[64];
        sscanf(line, "%[^()]", fname);
        run_function(fname, client_socket);
    } else {
        // Inside function body
        if (func_count > 0) {
            CapiFunction *f = &funcs[func_count - 1];
            if (f->line_count < MAX_LINES)
                strcpy(f->lines[f->line_count++], line);
        }
    }
}

void process_capi_file(const char *filepath, int client_socket, CapiResponse *resp) {
    FILE *file = fopen(filepath, "r");
    if (!file) return;

    char line[512];
    while (fgets(line, sizeof(line), file)) {
        process_line(line, client_socket, resp);
        if (resp->force_close) break;
    }

    fclose(file);
}
