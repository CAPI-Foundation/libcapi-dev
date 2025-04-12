#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dlfcn.h>
#include <time.h>
#include <stdlib.h>
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

static void append_output(CapiResult *res, const char *data) {
    if (strlen(res->output) + strlen(data) < MAX_OUTPUT) {
        strcat(res->output, data);
    }
}

static void run_function(const char *name, CapiResult *res) {
    for (int i = 0; i < func_count; i++) {
        if (strcmp(funcs[i].name, name) == 0) {
            for (int j = 0; j < funcs[i].line_count; j++) {
                process_line(funcs[i].lines[j], res);
                if (res->force_close) return;
            }
        }
    }
}

static void execute_shell(const char *cmd, CapiResult *res) {
    FILE *fp = popen(cmd, "r");
    if (!fp) return;
    char out[256];
    while (fgets(out, sizeof(out), fp)) {
        append_output(res, out);
    }
    pclose(fp);
}

static void include_file(const char *path, CapiResult *res) {
    FILE *file = fopen(path, "r");
    if (!file) return;
    char line[512];
    while (fgets(line, sizeof(line), file)) {
        process_line(line, res);
        if (res->force_close) break;
    }
    fclose(file);
}

void report_error(const char *msg, int line, const char *file) {
    fprintf(stderr, "Error at line %d in %s: %s\n", line, file, msg);
}

void report_warning(const char *msg) {
    fprintf(stderr, "Warning: %s\n", msg);
}

void process_line(const char *line, CapiResult *res) {
    if (strncmp(line, "#", 1) == 0) return;

    if (strstr(line, "int ") == line) {
        char name[64], val[256];
        sscanf(line, "int %[^=]= \"%[^\"]", name, val);
        set_variable(name, val);
    } else if (strstr(line, "echo:") == line) {
        char var[64];
        sscanf(line, "echo: %[^;];", var);
        const char *val = get_variable(var);
        append_output(res, val);
        append_output(res, "\n");
    } else if (strstr(line, "capi.forceclose()") != NULL) {
        report_warning("capi.forceclose() Deprecated");
    } else if (strstr(line, "capi.exec:") == line) {
        char cmd[256];
        sscanf(line, "capi.exec: \"%[^\"]", cmd);
        execute_shell(cmd, res);
    } else if (strstr(line, "incl <") == line) {
        char incl_path[256];
        sscanf(line, "incl <%[^>]", incl_path);
        include_file(incl_path, res);
    } else if (strstr(line, "function ") == line) {
        CapiFunction *f = &funcs[func_count++];
        sscanf(line, "function %[^:]:", f->name);
        f->line_count = 0;
    } else if (strstr(line, "funclose;") == line) {
        // End of function definition
    } else if (strchr(line, '(') && strchr(line, ')')) {
        char fname[64];
        sscanf(line, "%[^()]", fname);
        run_function(fname, res);
    } else {
        // Inside a function
        if (func_count > 0) {
            CapiFunction *f = &funcs[func_count - 1];
            if (f->line_count < MAX_LINES)
                strcpy(f->lines[f->line_count++], line);
        }
    }
}

CapiResult process_capi_file(const char *filepath) {
    CapiResult res = { .output = "", .force_close = 0 };
    FILE *file = fopen(filepath, "r");
    if (!file) return res;

    char line[512];
    while (fgets(line, sizeof(line), file)) {
        process_line(line, &res);
        if (res.force_close) break;
    }

    fclose(file);
    return res;
}
