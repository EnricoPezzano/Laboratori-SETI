/*
 * udp_ping.c: esempio di implementazione del processo "ping" con
 *             socket di tipo DGRAM.
 *
 * versione 7.0
 *
 * Programma sviluppato a supporto del laboratorio di
 * Sistemi di Elaborazione e Trasmissione del corso di laurea
 * in Informatica classe L-31 presso l'Universita` degli Studi di
 * Genova, anno accademico 2020/2021.
 *
 * Copyright (C) 2013-2014 by Giovanni Chiola <chiolag@acm.org>
 * Copyright (C) 2015-2016 by Giovanni Lagorio <giovanni.lagorio@unige.it>
 * Copyright (C) 2017-2020 by Giovanni Chiola <chiolag@acm.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "pingpong.h"


/*
* This function sends and waits for a reply on a socket.
* char message[]: message to send
* size_t msg_size: message length
*/
double do_ping(size_t msg_size, int msg_no, char message[msg_size], int ping_socket, double timeout)
{
	int lost_count = 0, recv_errno;
	char answer_buffer[msg_size];
	ssize_t recv_bytes, sent_bytes;
	struct timespec send_time, recv_time;
	double roundtrip_time_ms;
	int re_try = 0;

    /*** write msg_no at the beginning of the message buffer ***/
/*** TO BE DONE START ***/

	//Sprintf se fallisce mi restituisce un numero negativo, 
	//il controllo tramite if mi permette di verificare se tutto è andato bene
	if(sprintf(message,"%d\n",msg_no) < 0) //Scrivo il contenuto di msg_no nel buffer message 
		printf("Error inserting msg_no at the beginning of the message buffer:\n"); //Se il numero restituito è < 0, allora c'è un errore

/*** TO BE DONE END ***/

	do {
		debug(" ... sending message %d\n", msg_no);
	/*** Store the current time in send_time ***/
/*** TO BE DONE START ***/

	//Clock_gettime restituisce -1 in caso di errore, per cui salvo il valore ritornato in error per poter verificare dopo se c'è stato un errore.
	//clock_gettime salva il tempo rilevato nella struct di tipo timespec send_time, che deve essere passata per riferimento
	//CLOCK_TYPE è il clk_id, ovvero l'identificatore del particolare clock con cui agiamo (definito tramite DEFINE in pingpong.h)
	int sent = clock_gettime(CLOCK_TYPE, &send_time);
   if (sent < 0) //Se vero allora c'è un errore
      fail_errno("The storing of the current time in send_time hasn't been successful:");
      //Fail_errno è una funzione che stampa un messaggio di errore definita nel file ausiliario fail.c
      //Se ho un errore stampo il messaggio di "segnalazione"

/*** TO BE DONE END ***/

	/*** Send the message through the socket (non blocking mode) ***/
/*** TO BE DONE START ***/

	//send(sockfd, buf, len, flags);
	//Sockfd indica il tipo di socket, buf e len indicano rispettivamente il buffer del messaggio e la sua lunghezza
	//Rispetto al tcp ho come Sockfd ping_socket
	sent_bytes = send(ping_socket, message, msg_size, MSG_DONTWAIT); //Salvo il valore di ritorno di send per verificare eventuali errori
   if (sent_bytes < 0) //sent_bytes = -1 -> errore
   	fail_errno("The sending of the message through the socket hasn't been successful:"); //Stampo un messaggio di "segnalazione" dell'errore

/*** TO BE DONE END ***/

	/*** Receive answer through the socket (non blocking mode, with timeout) ***/
/*** TO BE DONE START ***/

	recv_bytes = recv(ping_socket, answer_buffer, msg_size, MSG_DONTWAIT);
	//recv di tipo NONBLOCKING
	
	recv_errno = errno;
	//Salvo nella variabile recv_errno (usata dopo nel ciclo di controllo) il valore di errno

/*** TO BE DONE END ***/

	/*** Store the current time in recv_time ***/
/*** TO BE DONE START ***/

	//Stesso ragionamento fatto precedentemente per salvare l'ora in send_time
	int rect = clock_gettime(CLOCK_TYPE, &recv_time);
   if (rect < 0) //Se vero allora c'è un errore
      fail_errno("The storing of the current time in recv_time hasn't been successful:"); 
      //Messaggio di errore stampato tramite fail_errno

/*** TO BE DONE END ***/

		roundtrip_time_ms = timespec_delta2milliseconds(&recv_time, &send_time);

		while ( recv_bytes < 0 && (recv_errno == EAGAIN || recv_errno == EWOULDBLOCK)
		            && roundtrip_time_ms < timeout ) {
			recv_bytes = recv(ping_socket, answer_buffer, sizeof(answer_buffer), 0);
                        recv_errno = errno;
                        if ( recv_bytes < 0 && errno != EAGAIN && errno != EWOULDBLOCK )
			        fail_errno("UDP ping could not recv from UDP socket");
                        
		        if (clock_gettime(CLOCK_TYPE, &recv_time))
			    fail_errno("Cannot get receive-time");
			roundtrip_time_ms = timespec_delta2milliseconds(&recv_time, &send_time);
                        sscanf(answer_buffer,"%ld %ld, %ld %ld\n",
                                                &(recv_time.tv_sec), &(recv_time.tv_nsec),
                                                &(send_time.tv_sec), &(send_time.tv_nsec));
	                roundtrip_time_ms -= timespec_delta2milliseconds(&send_time, &recv_time);
		}
		if (recv_bytes < sent_bytes) {	/*time-out elapsed: packet was lost */
			lost_count++;
			if (recv_bytes < 0)
				recv_bytes = 0;
			printf("\n ... received %zd bytes instead of %zd (lost count = %d); re-trying ...\n", recv_bytes, sent_bytes, lost_count);
			if (++re_try > MAXUDPRESEND) {
				printf(" ... giving-up!\n");
				fail("too many lost datagrams");
			}
			printf(" ... re-trying ...\n");
		}
	} while (sent_bytes != recv_bytes);

	return roundtrip_time_ms;
}



int prepare_udp_socket(char *pong_addr, char *pong_port)
{
	struct addrinfo gai_hints, *pong_addrinfo = NULL;
	int ping_socket;
	int gai_rv;

    /*** Specify the UDP sockets' options ***/
	memset(&gai_hints, 0, sizeof gai_hints);
/*** TO BE DONE START ***/

	//gai_hints è una struct di tipo addrinfo
	gai_hints.ai_family = AF_INET; //Indico che il formato dell'indirizzo fa parte della famiglia AF_INET(IPV4)
	gai_hints.ai_socktype = SOCK_DGRAM; //Il socket deve essere di tipo DATAGRAM
	gai_hints.ai_protocol = 0;

/*** TO BE DONE END ***/

	if ((ping_socket = socket(gai_hints.ai_family, gai_hints.ai_socktype, gai_hints.ai_protocol)) == -1)
		fail_errno("UDP Ping could not get socket");

    /*** change socket behavior to NONBLOCKING ***/
/*** TO BE DONE START ***/

	//fcntl() fa l'operazione richiesta da cmd(F_SETFD) sul fd selezionato(PING_SOCKET)
	//Dico cosa fare tramite arg(O_RDWR e O_NONBLOCK)
	if(fcntl(ping_socket, F_SETFD, O_RDWR | O_NONBLOCK) != 0) //Se non restituisce 0 allora c'è un errore
		fail_errno("Error setting to NONBLOCKING(fcntl):");	

/*** TO BE DONE END ***/

    /*** call getaddrinfo() in order to get Pong Server address in binary form ***/
/*** TO BE DONE START ***/
	
	//getaddrinfo() restituisce un indirizzo internet che può essere utilizzato tramite bind() e connect()
	//gai_hints da informazioni sul tipo di servizio/socket richiesto, e che deve essere restituito tramite server_addrinfo
	gai_rv = getaddrinfo(pong_addr, pong_port, &gai_hints, &pong_addrinfo); 
   if(gai_rv != 0) //Se gai_rv != 0 c'è un errore
   	fail_errno("Error using getaddrinfo:"); //gai_strerror stampa la descrizione dell'errore avuto da getaddrinfo

/*** TO BE DONE END ***/

#ifdef DEBUG
	{
		char ipv4str[INET_ADDRSTRLEN];
		const char * const cp = inet_ntop(AF_INET, &(((struct sockaddr_in *)(pong_addrinfo-> ai_addr))->sin_addr), ipv4str, INET_ADDRSTRLEN);
		if (cp == NULL)
			printf(" ... inet_ntop() error!\n");
		else
			printf(" ... about to connect socket %d to IP address %s, port %hu\n",
			     ping_socket, cp, ntohs(((struct sockaddr_in *)(pong_addrinfo->ai_addr))->sin_port));
	}
#endif

    /*** connect the ping_socket UDP socket with the server ***/
/*** TO BE DONE START ***/

	//connect(socketfd, address, address_length);	
	int con = connect(ping_socket, pong_addrinfo->ai_addr, pong_addrinfo->ai_addrlen);
	if(con < 0) 
		fail_errno("Connection error:");

/*** TO BE DONE END ***/

	freeaddrinfo(pong_addrinfo);
	return ping_socket;
}



int main(int argc, char *argv[])
{
	struct addrinfo gai_hints, *server_addrinfo;
	int ping_socket, ask_socket;;
	int msg_size, norep;
	int gai_rv;
	char ipstr[INET_ADDRSTRLEN];
	struct sockaddr_in *ipv4;
	char request[40], answer[10];
	ssize_t nr;
	int pong_port;

	if (argc < 4)
		fail("Incorrect parameters provided. Use: udp_ping PONG_ADDR PONG_PORT MESSAGE_SIZE [NO_REPEAT]\n");
	for (nr = 4, norep = REPEATS; nr < argc; nr++)
		if (*argv[nr] >= '1' && *argv[nr] <= '9')
			sscanf(argv[nr], "%d", &norep);
	if (norep < MINREPEATS)
		norep = MINREPEATS;
	else if (norep > MAXREPEATS)
		norep = MAXREPEATS;
	if (sscanf(argv[3], "%d", &msg_size) != 1 || msg_size < MINSIZE || msg_size > MAXUDPSIZE)
		fail("Wrong message size");

    /*** Specify TCP socket options ***/
	memset(&gai_hints, 0, sizeof gai_hints);
/*** TO BE DONE START ***/
	
	gai_hints.ai_family= AF_INET; //IPV4
	gai_hints.ai_socktype = SOCK_STREAM; //Socket stream
	gai_hints.ai_protocol = 0;
	gai_hints.ai_flags = 0;

/*** TO BE DONE END ***/

    /*** call getaddrinfo() in order to get Pong Server address in binary form ***/
/*** TO BE DONE START ***/

	//argv[1]=node(addr) e argv[2]=service(port)
	//getaddrinfo() prende queste variabili e restituisce un indirizzo internet che può essere utilizzato tramite bind() e connect()
	//gai_hints da informazioni sul tipo di servizio/socket richiesto, e che deve essere restituito tramite server_addrinfo
	gai_rv = getaddrinfo(argv[1],argv[2], &gai_hints, &server_addrinfo); 
   if(gai_rv != 0) //Se gai_rv != 0 c'è un errore
   	gai_strerror(gai_rv);
	
/*** TO BE DONE END ***/

    /*** Print address of the Pong server before trying to connect ***/
	ipv4 = (struct sockaddr_in *)server_addrinfo->ai_addr;
	printf("UDP Ping trying to connect to server %s (%s) on TCP port %s\n", argv[1], inet_ntop(AF_INET, &ipv4->sin_addr, ipstr, INET_ADDRSTRLEN), argv[2]);

    /*** create a new TCP socket and connect it with the server ***/
/*** TO BE DONE START ***/

	//Alla chiamata di socket() gli passo il dominio(protocol family->IPV4 (AF_INET), il tipo di socket richiesto(SOCK_STREAM), e il protocollo utilizzato
	//dovrebbe ritornarmi un file descriptor riferito al socket richiesto
	ask_socket = socket(AF_INET, SOCK_STREAM, 0);
   if(ask_socket == -1) //Se tcp_socket è -1 allora la chiamata a socket ha dato qualche problema
   	fail_errno("Error opening socket:"); //C'è un errore

	//connect() connette il socket specificato da tcp_socket all'indirizzo specificato da ai_addr
	//specificando inoltre anche la lunghezza dell'indirizzo tramite ai_addrlen
   nr = connect(ask_socket,server_addrinfo->ai_addr, server_addrinfo->ai_addrlen);
   if (nr == -1) //Se nr = -1 allora c'è un errore
   	fail_errno("Connection error:"); //Messaggio di errore stampato tramite fail_errno
   
/*** TO BE DONE END ***/

	freeaddrinfo(server_addrinfo);
	printf(" ... connected to Pong server: asking for %d repetitions of %d bytes UDP messages\n", norep, msg_size);
	sprintf(request, "UDP %d %d\n", msg_size, norep);

    /*** Write the request on the TCP socket ***/
/** TO BE DONE START ***/

	//Scrivo nel file descriptor tcp_socket il contenuto di lunghezza strlen(request) del buffer request
	nr = write(ask_socket, request, strlen(request));
	if (nr < 0) //Se nr = -1 allora la chiamata a write ha generato un errore
   	fail_errno("TCP Ping could not send request to Pong server"); //Messaggio di errore stampato tramite fail_errno

/*** TO BE DONE END ***/

	nr = read(ask_socket, answer, sizeof(answer));
	if (nr < 0)
		fail_errno("UDP Ping could not receive answer from Pong server");
	if (nr==sizeof(answer))
		--nr;
	answer[nr] = 0;

    /*** Check if the answer is OK, and fail if it is not ***/
/*** TO BE DONE START ***/

	//Tramite strncmp() confronto la stringa di risposta answer e la stringa "OK" per verificare se sono uguali
	//aggiungendo un numero max di byte da leggere come limite
	//se lo sono strcmp() restituisce 0
	if(strncmp(answer,"OK",2) != 0) //Non sono uguali
   	fail_errno("The answer isn't OK:"); //Messaggio di errore stampato tramite fail_errno 

/*** TO BE DONE END ***/

    /*** else ***/
	sscanf(answer + 3, "%d\n", &pong_port);
	printf(" ... Pong server agreed to ping-pong using port %d :-)\n", pong_port);
	sprintf(answer, "%d", pong_port);
	shutdown(ask_socket, SHUT_RDWR);
	close(ask_socket);

	ping_socket = prepare_udp_socket(argv[1], answer);

	{
		char message[msg_size];
		memset(&message, 0, (size_t)msg_size);
		double ping_times[norep];
		struct timespec zero, resolution;
		int repeat;
		for (repeat = 0; repeat < norep; repeat++) {
			ping_times[repeat] = do_ping((size_t)msg_size, repeat + 1, message, ping_socket, UDP_TIMEOUT);
			printf("Round trip time was %6.3lf milliseconds in repetition %d\n", ping_times[repeat], repeat + 1);
		}
		memset((void *)(&zero), 0, sizeof(struct timespec));
		if (clock_getres(CLOCK_TYPE, &resolution) != 0)
			fail_errno("UDP Ping could not get timer resolution");
		print_statistics(stdout, "UDP Ping: ", norep, ping_times, msg_size, timespec_delta2milliseconds(&resolution, &zero));

	}

	close(ping_socket);
	exit(EXIT_SUCCESS);
}
