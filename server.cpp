#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>

#define PORT 8080

int main()
{
	int server_fd, new_socket;
	struct sockaddr_in address;
	socklen_t addrlen = sizeof(address);

	// 1. Creazione del socket
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1)
	{
		std::cerr << "Errore nella creazione del socket\n";
		return 1;
	}

	// 2. Configurazione indirizzo e porta
	address.sin_family = AF_INET;          // IPv4
	address.sin_addr.s_addr = INADDR_ANY;  // Accetta connessioni da qualsiasi IP
	address.sin_port = htons(PORT);        // Converti il numero di porta

	// 3. Binding del socket
	if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0)
	{
		std::cerr << "Errore nel binding\n";
		return 1;
	}

	// 4. Mettere il server in ascolto
	if (listen(server_fd, 3) < 0)
	{
		std::cerr << "Errore nella listen()\n";
		return 1;
	}

	std::cout << "Ascoltando quella troia bastarda sulla porta " << PORT << "...\n";

	// 5. Accettare connessioni
	new_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen);
	if (new_socket < 0)
	{
		std::cerr << "Errore nell'accept()\n";
		return 1;
	}

	// 6. Rispondere al client
	const char* response = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, World!";
	send(new_socket, response, strlen(response), 0);

	// 7. Chiudere la connessione
	close(new_socket);
	close(server_fd);
	
	return 0;
}
