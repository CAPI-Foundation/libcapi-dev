#ifndef LIBCAPI_H
#define LIBCAPI_H

#define MAX_HEADERS 10

typedef struct {
    int status_code;         // Default: 200
    char headers[MAX_HEADERS][256];
    int header_count;
    int force_close;         // 1 = force 204 or disconnect
} CapiResponse;

void process_capi_file(const char *filepath, int client_socket, CapiResponse *response);

#endif
