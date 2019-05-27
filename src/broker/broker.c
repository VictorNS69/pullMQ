#include <stdio.h>
#include "comun.h"

Array queues;
int queue_create(FIFO *q, char *name);

int queue_push(FIFO *q, void *msg, size_t size);
int queue_pop(FIFO *q, void **msg, size_t *size, bool blocking, int client_fd);

int get_index(const char *name);

int send_response(int client_fd, void *data, size_t size);

Request deserialize(char *serialized);

int serialize(int operation, int status, void *msg, size_t msg_len, char **serialized, size_t *serialized_len);

int process_request(const unsigned int client_fd);

int create_server(int port);

int queue_push(FIFO *q, void *msg, size_t size)
{
	struct Node *node;
	node = (struct Node *)malloc(sizeof(struct Node));
	node->msg = malloc(size);
	memcpy(node->msg, msg, size);
	node->size = size;

	if (q->first == NULL)
	{
		int statusSD = 0;
		int client_fd;
		int awai_pop = 0;
		if (q->n_awaiting <= 0)
			awai_pop = -1;
		else
		{
			int client_fd = q->awaiting[0];
			q->n_awaiting--;
			int *temp = malloc(q->n_awaiting * sizeof(int));
			memcpy(
				temp,
				q->awaiting + 1,
				q->n_awaiting * sizeof(int));

			free(q->awaiting);
			q->awaiting = temp;
			awai_pop = client_fd;
		}
		if ((client_fd = awai_pop) < 0)
		{
			statusSD =  client_fd;
		}
		char *serialized;
		size_t serialized_len;
		if (serialize(GET, 0, msg, size, &serialized, &serialized_len) < 0 && statusSD == 0)
		{
			statusSD = -1;
		}
		if (send_response(client_fd, serialized, serialized_len) && statusSD == 0)
		{
			queue_push(q, msg, size);
			statusSD = -1;
		}
		if (statusSD >= 0)
			return 0;
		node->next = NULL;
		q->first = node;
	}
	else
		node->next = q->last;
	q->last = node;
	return 0;
}

int queue_pop(FIFO *q, void **msg, size_t *size, bool blocking, int client_fd)
{
	if (q->first == NULL)
	{
		if (blocking)
		{
			q->n_awaiting++;
			q->awaiting = (int *)realloc(q->awaiting, q->n_awaiting * sizeof(int));
			q->awaiting[q->n_awaiting - 1] = client_fd;
			return 1;
		}
		else
		{
			*size = 0;
			return 2;
		}
	}
	struct Node *first = q->first;

	*msg = first->msg;
	*size = first->size;

	struct Node *second;
	int find = -1;
	struct Node *current = q->last;
	do
	{
		if (current->next == first)
		{
			second = current;
			find = 0;
			break;
		}
	} while ((current = current->next) != NULL);
	if (find < 0)
	{
		q->last = NULL;
		q->first = NULL;
		return 0;
	}

	second->next = NULL;
	q->first = second;

	return 0;
}

int get_index(const char *name)
{
	for (int i = 0; i < queues.size; i++)
	{
		if (strcmp(queues.array[i].name, name) == 0)
		{
			return i;
		}
	}
	return -1;
}

int send_response(int client_fd, void *data, size_t size)
{
	uint32_t size_net = htonl(size);
	if (send(client_fd, &size_net, sizeof(size), MSG_NOSIGNAL) < 0)
	{
		return -1;
	}
	if (send(client_fd, data, size, MSG_NOSIGNAL) < 0)
	{
		return -1;
	}

	if (EPIPE == errno)
	{
		return -1;
	}

	return 0;
}

Request deserialize(char *serialized)
{

	Request request;

	request.operation = *((int *)serialized);
	serialized += sizeof(request.operation);

	request.queue_name_len = *((size_t *)serialized);
	serialized += sizeof(request.queue_name_len);

	request.queue_name = malloc(request.queue_name_len + 1);
	memcpy(request.queue_name, serialized, request.queue_name_len);
	request.queue_name[request.queue_name_len] = '\0';
	serialized += request.queue_name_len;

	if (request.operation == PUT)
	{
		request.msg_len = *((size_t *)serialized);
		serialized += sizeof(request.msg_len);

		request.msg = malloc(request.msg_len);
		memcpy(request.msg, serialized, request.msg_len);
	}
	else if (request.operation == GET)
		request.blocking = *((char *)serialized) == '1';
	return request;
}

int serialize(int operation, int status, void *msg, size_t msg_len, char **serialized, size_t *serialized_len)
{
	size_t size = sizeof(status) + (operation == GET && status == 0 ? (msg_len + sizeof(msg_len)) : 0);
	size_t offset = 0;
	char *response_serialized = 0;

	response_serialized = malloc(size);
	int i = status < 0 ? status : operation;
	memcpy(response_serialized + offset, &i, sizeof(i));
	offset += sizeof(int);

	if (operation == GET && (status == 0 || status == 2))
	{
		memcpy(response_serialized + offset, &msg_len, sizeof(msg_len));
		offset += sizeof(msg_len);

		memcpy(response_serialized + offset, msg, msg_len);
		offset += msg_len;
	}

	*serialized_len = size;
	*serialized = response_serialized;
	return 0;
}

int process_request(const unsigned int client_fd)
{
	size_t request_len = 0;
	uint32_t request_len32 = 0;

	if (recv(client_fd, &request_len32, sizeof(size_t), 0) < 0 ||
		(request_len = ntohl(request_len32)) < 0)
	{
		int err = 100;
		send_response(client_fd, &err, sizeof(int));
		return 0;
	}

	char *request_serialized = malloc(request_len);
	if (request_serialized == NULL ||
		recv(client_fd, request_serialized, request_len, MSG_WAITALL) < 0)
	{
		free(request_serialized);
		int err = 100;
		send_response(client_fd, &err, sizeof(int));
		return -1;
	}

	Request request = deserialize(request_serialized);

	void *msg;
	size_t msg_len = 0;
	int status = 0;
	FIFO q;
	int index;
	switch (request.operation)
	{
	case CREATE:
		if (get_index(request.queue_name) >= 0)
			status = -1;

		queues.size++;
		queues.array = (FIFO *)realloc(queues.array, queues.size * sizeof(*queues.array));
		if (queues.array == NULL)
		{
			status = -1;
		}
		FIFO queue;
		FIFO *temp;
		temp = (FIFO *)malloc(sizeof(FIFO));
		temp->name = request.queue_name;
		temp->first = NULL;
		temp->last = NULL;
		temp->awaiting = malloc(0);
		temp->n_awaiting = 0;
		queue = *temp;

		queues.array[queues.size - 1] = queue;
		break;
	case DESTROY:
		if ((index = get_index(request.queue_name)) < 0)
		{
			status = -1;
		}

		q = queues.array[index];
		free(queues.array[index].name);
		struct Node *head = q.first;

		for (int i = 0; i < q.n_awaiting; i++)
		{
			int err = 100;
			send_response(q.awaiting[i], &err, sizeof(int));
		}
		free(q.awaiting);

		while (head != NULL)
		{
			free(head->msg);
			free(head);
			head = head->next;
		}
		queues.size--;

		temp = malloc(queues.size * sizeof(FIFO));
		memcpy(temp, queues.array, index * sizeof(FIFO));

		if (index != queues.size)
			memcpy(
				temp + index,
				queues.array + index + 1,
				(queues.size - index) * sizeof(FIFO));

		free(queues.array);
		queues.array = temp;
		break;
	case PUT:
		if ((index = get_index(request.queue_name)) < 0)
			status = -1;

		q = queues.array[index];

		queue_push(&q, request.msg, request.msg_len);

		queues.array[index] = q;

		break;
	case GET:
		if ((index = get_index(request.queue_name)) < 0)
			status = -1;

		q = queues.array[index];
		int status2;
		if ((status2 = queue_pop(&q, &msg, &msg_len, request.blocking, client_fd)) < 0)
			status = -1;

		queues.array[index] = q;
		status = status2;

		break;
	}
	if (status == 1)
		return 1;

	free(request_serialized);

	char *serialized;
	size_t serialized_len;
	if (serialize(request.operation, status, msg, msg_len, &serialized, &serialized_len) < 0)
	{
		return -1;
	}
	return send_response(client_fd, serialized, serialized_len);
}

int main(int argc, char *argv[])
{

	if (argc != 2)
	{
		fprintf(stderr, "Uso: %s puerto\n", argv[0]);
		return 1;
	}

	queues.array = (FIFO *)malloc(0);
	queues.size = 0;

	int port = atoi(argv[1]);
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
	self.sin_port = htons(port);
	if ((he = gethostbyname(host)) == NULL)
		exit(1);

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
		int client_fd;
		struct sockaddr_in client_addr;
		socklen_t addrlen = sizeof(client_addr);

		client_fd = accept(sockfd, (struct sockaddr *)&client_addr, &addrlen);

		int status = process_request(client_fd);

		if (status != 1)
			close(client_fd);
	}
	close(sockfd);

	return 0;
}