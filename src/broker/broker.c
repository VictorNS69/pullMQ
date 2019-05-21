#include <stdio.h>
#include "comun.h"

#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <stdbool.h>

Array array;
bool initialized = false;

int get_index(const char *name)
{
    for (int i = 0; i < array.size; i++)
    {
        if (strcmp(array.list[i].name, name) == 0)
        {
            return i;
        }
    }
    return -1;
}

int get(const char *queue_name, void **msg, size_t *size, bool blocking, int client_fd)
{
    // Obtenemos la cola, devuelve -1 en caso de que no exista
    FIFO q;
    int index;

    if ((index = get_index(queue_name)) < 0)
    {
        return -1;
    }
    q = array.list[index];
    int status;
    if (q.start == NULL)
    {
        if (blocking)
        {
            q.n_awaiting++;
            q.awaiting = (int *)realloc(q.awaiting, q.n_awaiting * sizeof(int));
            q.awaiting[q.n_awaiting - 1] = client_fd;
            status = 1;
        }
        else
        {
            status = -1;
        }
    }
    struct Node *first = q.start;

    *msg = first->msg;
    size = first->size;

    struct Node *second;
    int qs = -1;
    struct Node *current = q.end;
    do
    {
        if (current->next == first)
        {
            second = current;
            qs = 0;
        }
    } while ((current = current->next) != NULL);

    if (qs < 0)
    {
        q.end = NULL;
        q.start = NULL;
        status = 0;
    }

    second->next = NULL;
    q.start = second;

    //////////
    if (status != 0)
    {
        if (status == 1)
        {
            array.list[index] = q;
        }

        return status;
    }
    array.list[index] = q;
    return 0;
}

int destroyMQ(const char *name)
{
	// Obtenemos la cola, devuelve -1 en caso de que no exista
	FIFO q;
	int index = -1;
	if ((index = get_index(name)) < 0)
	{
		return -1;
	}
	q = array.list[index];
	free(array.list[index].name);
		struct Node *head = q.start;

	for(int i = 0; i < q.n_awaiting; i++)
	{
		send_error(q.awaiting[i]);
	}
	free(q.awaiting);

	while (head != NULL)
	{
		free(head->msg);
		free(head);
		head = head->next;
	}


	array.size--;

	// allocate an array with a size 1 less than the current one
	FIFO *temp = malloc(array.size * sizeof(FIFO));

	// copy everything BEFORE the index
	memcpy(temp, array.list, index * sizeof(FIFO));

	if (index != array.size)
		// copy everything AFTER the index
		memcpy(
			temp + index,
			array.list + index + 1,
			(array.size - index) * sizeof(FIFO));

	free(array.list);
	array.list = temp;
	return 0;
}

int put(const char *name, const void *msg, size_t size)
{
    // Obtenemos la cola, devueleve -1 en caso de que no exista
    FIFO q;
    int index;
    if ((index = get_index(name)) < 0)
    {
        return -1;
    }
    q = array.list[index];
    struct Node *node;
    node = (struct Node *)malloc(sizeof(struct Node));
    node->msg = malloc(size);
    memcpy(node->msg, msg, size);
    node->size = size;

    if (q.start == NULL)
    {
        if (q.n_awaiting > 0)
        {
            int client_fd = q.awaiting[0];
            q.n_awaiting--;
            int *temp = malloc(q.n_awaiting * sizeof(int));

            memcpy(
                temp,
                q.awaiting + 1,
                q.n_awaiting * sizeof(int));

            free(q.awaiting);
            q.awaiting = temp;

            size_t serialized_size = GET + size + sizeof(size);
            size_t offset = 0;
            char *response_serialized = 0;

            response_serialized = malloc(serialized_size);
            int operation = GET;

            memcpy(response_serialized + offset, &operation, sizeof(operation));
            offset += sizeof(int);

            memcpy(response_serialized + offset, &size, sizeof(size));
            offset += sizeof(size);

            memcpy(response_serialized + offset, msg, size);
            offset += size;

            uint32_t size_net = htonl(serialized_size);
            return 0;
        }
        node->next = NULL;
        q.start = node;
    }
    else
    {
        node->next = q.end;
    }
    q.end = node;
    array.list[index] = q;
    return 0;
}

int send_error(int clientfd)
{
    int i = 10;
    uint32_t size = htonl(sizeof(i));
    if (send(clientfd, &size, sizeof(size), 0) < 0)
    {
        return -1;
    }

    if (send(clientfd, &i, size, 0) < 0)
    {
        return -1;
    }
    return 0;
}

int process_request(const unsigned int clientfd)
{
    size_t request_len = 0;
    uint32_t request_len32 = 0;
    if (recv(clientfd, &request_len32, sizeof(size_t), 0) < 0)
    {
        send_error(clientfd);
        return 0;
    }
    if ((request_len = ntohl(request_len32)) < 0)
    {
        return 0;
    }

    char *request_serialized = malloc(request_len);
    if (request_serialized == NULL)
    {
        // TODO free serialized
        send_error(clientfd);
        return -1;
    }

    if (recv(clientfd, request_serialized, request_len, MSG_WAITALL) < 0)
    {
        // TODO free serialized
        send_error(clientfd);
        return -1;
    }

    ////////////////////
    //Request request = deserialize(request_serialized);
    Request request;

    request.operation = *((int *)request_serialized);
    request_serialized += sizeof(request.operation);

    request.queue_name_len = *((size_t *)request_serialized);
    request_serialized += sizeof(request.queue_name_len);

    request.queue_name = malloc(request.queue_name_len + 1);
    // TODO: maybe chatn to strcat??
    memcpy(request.queue_name, request_serialized, request.queue_name_len);
    request.queue_name[request.queue_name_len] = '\0';
    request_serialized += request.queue_name_len;
    if (request.operation == PUT)
    {
        request.msg_len = *((size_t *)request_serialized);
        request_serialized += sizeof(request.msg_len);
        request.msg = malloc(request.msg_len);
        memcpy(request.msg, request_serialized, request.msg_len);
    }
    else if (request.operation == GET)
    {
        request.blocking = *((char *)request_serialized) == '1';
    }
    //////////////////////////////
    void *msg;
    size_t msg_len = 0;
    int status;
    switch (request.operation)
    {
    case CREATE:
    	if (initialized == false)
	{
		array.list = (FIFO *)malloc(0);
		array.size = 0;
		initialized = true;
	}

	// Comprobar que no haya colas con ese nombre
	for (int i = 0; i < array.size; i++)
	{
		if (strcmp(array.list[i].name, request.queue_name) == 0)
		{
			status = -1;
		}
        break;
    }
    case DESTROY:
        status = destroyMQ(request.queue_name);
        break;
    case PUT:
        status = put(request.queue_name, request.msg, request.msg_len);
        break;
    case GET:
        status = get(request.queue_name, &msg, &msg_len, request.blocking, clientfd);
        break;
    }
    if (status == 1)
    {
        return 1;
    }

    free(request_serialized);
    size_t size = sizeof(status) + (request.operation == GET && status == 0 ? (msg_len + sizeof(msg_len)) : 0);
    size_t offset = 0;
    char *response_serialized = 0;
    response_serialized = malloc(size);
    int i = status < 0 ? status : request.operation;
    memcpy(response_serialized + offset, &i, sizeof(i));
    offset += sizeof(int);

    if (request.operation == GET && status == 0)
    {
        memcpy(response_serialized + offset, &msg_len, sizeof(msg_len));
        offset += sizeof(msg_len);

        memcpy(response_serialized + offset, msg, msg_len);
        offset += msg_len;
    }

    uint32_t size_net = htonl(size);
    if (send(clientfd, &size_net, sizeof(size_t), 0) < 0)
    {
        return -1;
    }

    if (send(clientfd, response_serialized, size, 0) < 0)
    {
        return -1;
    }
    free(response_serialized);
    msg = 0;
    if (request.operation == PUT)
    {
        free(request.msg);
    }

    return 0;
}

/* incluye toda la funcionalidad del sistema, implementando las operaciones
 * descritas en el apartado previo y gestionando las colas de mensajes.
 * Recibe como argumento el número del puerto por el que prestará servicio.
*/
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Uso: %s puerto\n", argv[0]);
        return 1;
    }
    // create the server
    char *host = getenv("BROKER_HOST");
    int sockfd;
    struct sockaddr_in self;
    struct hostent *he;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        exit(errno);
    }

    bzero(&self, sizeof(self));
    self.sin_family = AF_INET;
    int port = atoi(argv[1]);
    self.sin_port = htons(port);
    if ((he = gethostbyname(host)) == NULL)
    {
        exit(1);
    }
    memcpy(&self.sin_addr, he->h_addr_list[0], he->h_length);
    if (bind(sockfd, (struct sockaddr *)&self, sizeof(self)) != 0)
    {
        exit(errno);
    }

    if (listen(sockfd, 20) != 0)
    {
        exit(errno);
    }

    while (1)
    {
        int clientfd;
        struct sockaddr_in client_addr;
        socklen_t addrlen = sizeof(client_addr);

        clientfd = accept(sockfd, (struct sockaddr *)&client_addr, &addrlen);
        int status = process_request(clientfd);
        if (array.size != 1)
            if (status != 1)
            {
                close(clientfd);
            }
    }
    close(sockfd);

    return 0;
}
