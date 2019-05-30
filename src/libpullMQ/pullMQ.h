/*
 *
 * NO MODIFICAR
 *
 */
#include <stdbool.h>
#include <stddef.h>

int createMQ(const char *cola);
int destroyMQ(const char *cola);

int put(const char *cola, const void *mensaje, size_t tam);
int get(const char *cola, void **mensaje, size_t *tam, bool blocking);

int get_socket();
int send_request(const unsigned int operation, const char *queue_name,
    const void *put_msg, size_t put_msg_len,
    void **get_msg, size_t *get_msg_len, bool blocking);