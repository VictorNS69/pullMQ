#include <stdio.h>
#include "comun.h"

Array array;

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

int fifo_push(FIFO *f, void *msg, size_t size)
{
	struct Node *node;
	node = (struct Node *)malloc(sizeof(struct Node));
	node->msg = malloc(size);
	memcpy(node->msg, msg, size);
	node->size = size;

	if (f->start == NULL)
	{
		int statusSD = 0;
		int client_fd;
		int awai_pop = 0;
		if (f->n_awaiting <= 0)
			awai_pop = -1;
		else
		{
			int client_fd = f->awaiting[0];
			f->n_awaiting--;
			int *temp = malloc(f->n_awaiting * sizeof(int));
			memcpy(
				temp,
				f->awaiting + 1,
				f->n_awaiting * sizeof(int));

			free(f->awaiting);
			f->awaiting = temp;
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
			fifo_push(f, msg, size);
			statusSD = -1;
		}
		if (statusSD >= 0)
			return 0;
		node->next = NULL;
		f->start = node;
	}
	else
		node->next = f->end;
	f->end = node;
	return 0;
}

int fifo_pop(FIFO *f, void **msg, size_t *size, bool blocking, int client_fd)
{
	if (f->start == NULL)
	{
		if (blocking)
		{
			f->n_awaiting++;
			f->awaiting = (int *)realloc(f->awaiting, f->n_awaiting * sizeof(int));
			f->awaiting[f->n_awaiting - 1] = client_fd;
			return 1;
		}
		else
		{
			*size = 0;
			return 2;
		}
	}
	struct Node *start = f->start;

	*msg = start->msg;
	*size = start->size;

	struct Node *second;
	int find = -1;
	struct Node *current = f->end;
	do
	{
		if (current->next == start)
		{
			second = current;
			find = 0;
			break;
		}
	} while ((current = current->next) != NULL);
	if (find < 0)
	{
		f->end = NULL;
		f->start = NULL;
		return 0;
	}

	second->next = NULL;
	f->start = second;

	return 0;
}

int get_index(const char *name)
{
	for (int i = 0; i < array.size; i++)
	{
		if (strcmp(array.array[i].name, name) == 0)
		{
			return i;
		}
	}
	return -1;
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
	FIFO f;
	int index;
	switch (request.operation)
	{
	case CREATE:
		if (get_index(request.queue_name) >= 0)
			status = -1;

		array.size++;
		array.array = (FIFO *)realloc(array.array, array.size * sizeof(*array.array));
		if (array.array == NULL)
		{
			status = -1;
		}
		FIFO fifo;
		FIFO *temp;
		temp = (FIFO *)malloc(sizeof(FIFO));
		temp->name = request.queue_name;
		temp->start = NULL;
		temp->end = NULL;
		temp->awaiting = malloc(0);
		temp->n_awaiting = 0;
		fifo = *temp;

		array.array[array.size - 1] = fifo;
		break;
	case DESTROY:
		if ((index = get_index(request.queue_name)) < 0)
		{
			status = -1;
		}

		f = array.array[index];
		free(array.array[index].name);
		struct Node *head = f.start;

		for (int i = 0; i < f.n_awaiting; i++)
		{
			int err = 100;
			send_response(f.awaiting[i], &err, sizeof(int));
		}
		free(f.awaiting);

		while (head != NULL)
		{
			free(head->msg);
			free(head);
			head = head->next;
		}
		array.size--;

		temp = malloc(array.size * sizeof(FIFO));
		memcpy(temp, array.array, index * sizeof(FIFO));

		if (index != array.size)
			memcpy(
				temp + index,
				array.array + index + 1,
				(array.size - index) * sizeof(FIFO));

		free(array.array);
		array.array = temp;
		break;
	case PUT:
		if ((index = get_index(request.queue_name)) < 0)
			status = -1;

		f = array.array[index];

		fifo_push(&f, request.msg, request.msg_len);

		array.array[index] = f;

		break;
	case GET:
		if ((index = get_index(request.queue_name)) < 0)
			status = -1;

		f = array.array[index];
		int status2;
		if ((status2 = fifo_pop(&f, &msg, &msg_len, request.blocking, client_fd)) < 0)
			status = -1;

		array.array[index] = f;
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

	array.array = (FIFO *)malloc(0);
	array.size = 0;

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