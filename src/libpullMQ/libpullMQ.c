#include "comun.h"
#include "pullMQ.h"

// BROKER_HOST: nombre de la máquina donde ejecuta el broker.
// BROKER_PORT: número de puerto TCP por el que está escuchando.

int get_socket(){
	int s_fd;
	struct sockaddr_in server;
	s_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (s_fd == -1)	{
		return -1;
	}
	struct hostent *hostent;
	char *host = getenv("BROKER_HOST");
	int port = atoi(getenv("BROKER_PORT"));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	if ((hostent = gethostbyname(host)) == NULL){
		return -1;
	}
	memcpy(&server.sin_addr, hostent->h_addr_list[0], hostent->h_length);
	if (connect(s_fd, (struct sockaddr *)&server, sizeof(server)) < 0){
		return -1;
	}
	return s_fd;
}

/** Envia una request
 *  Devuelve 0 si éxito o -1 si fallo
 */
int send_request(const unsigned int op, const char *name,
		const void *put_msg, size_t put_msg_len,
		void **get_msg, size_t *get_msg_len, bool blocking){
	int s_fd;
	char *serialized = 0;
	if ((s_fd = get_socket(&s_fd)) < 0){
		return -1;
	}
	size_t size = sizeof(op) +
		strlen(name) + sizeof(strlen(name)) + (op == PUT ? put_msg_len + sizeof(put_msg_len) : 0) +
		(op == GET ? sizeof(char) : 0);
	size_t offset = 0;
	serialized = calloc(1, size);
	memcpy(serialized + offset, &op, sizeof(op));
	offset += sizeof(op);
	size_t name_len = strlen(name);
	memcpy(serialized + offset, &name_len, sizeof(name_len));
	offset += sizeof(size_t);
	memcpy(serialized + offset, name, strlen(name));
	offset += strlen(name);
	if (op == PUT){
		memcpy(serialized + offset, &put_msg_len, sizeof(put_msg));
		offset += sizeof(put_msg_len);
		memcpy(serialized + offset, put_msg, put_msg_len);
		offset += put_msg_len;
	}
	if (op == GET){
		char blcoking;
		if (blocking == 1)
			blcoking = '1';
		else 
			blcoking = '0';

		memcpy(serialized + offset, &blcoking, sizeof(char));
		offset += sizeof(char);
	}
	uint32_t size_net = htonl(size);
	if (send(s_fd, &size_net, sizeof(size_t), 0) < 0 
			|| send(s_fd, serialized, size, 0) < 0){
		return -1;
	}
	free(serialized);
	size_t reply_len = 0;
	uint32_t reply_len32 = 0;
	if (recv(s_fd, &reply_len32, sizeof(size_t), MSG_WAITALL) < 0){
		return -1;
	}
	if ((reply_len = ntohl(reply_len32)) < 0){
		return 0;
	}
	char *reply = malloc(reply_len);
	if (recv(s_fd, reply, reply_len, MSG_WAITALL) < 0){
		return -1;
	}
	close(s_fd);
	int status = *((int *)reply);
	if (status != op){
		free(reply);
		return -1;
	}
	if (status == GET){
		*get_msg_len = 0;
		*get_msg = 0;
		char *msg_len = reply + sizeof(int);
		*get_msg_len = *((size_t *)msg_len);
		char *msg = msg_len + sizeof(size_t);
		*get_msg = malloc(*get_msg_len);
		memcpy(*get_msg, msg, *get_msg_len);
	}
	free(reply);
	return 0;
}

/* 0 si la operación se realizó satisfactoriamente y
 * un valor negativo en caso contrario
 * cola: nombre de la cola
 */
int createMQ(const char *cola){
	return send_request(CREATE, cola, NULL, 0, NULL, 0, false);
}

/* 0 si la operación se realizó satisfactoriamente y
* un valor negativo en caso contrario
* cola: nombre de la cola
*/
int destroyMQ(const char *cola){
	return send_request(DESTROY, cola, NULL, 0, NULL, 0, false);
}

/* 0 si la operación se realizó satisfactoriamente y
 * un valor negativo en caso contrario
 * cola: nombre de la cola
 * mensaje: mensaje
 * tam: tamaño del mensaje
 */
int put(const char *cola, const void *mensaje, size_t tam){
	return send_request(PUT, cola, mensaje, tam, NULL, 0, false);
}

/* 0 si la operación se realizó satisfactoriamente y
 * un valor negativo en caso contrario
 * cola: nombre de la cola
 * mensaje: mensaje
 * tam: tamaño del mensaje
 * blocking: indica si la cola es boqueante
 */
int get(const char *cola, void **mensaje, size_t *tam, bool blocking){
	return send_request(GET, cola, NULL, 0, mensaje, tam, blocking);
}