 /*
 * tcp_ping.c: esempio di implementazione del processo "ping" con
 *             socket di tipo STREAM.
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
 * This function sends and wait for a reply on a socket.
 * int msg_size: message length
 * int msg_no: message sequence number (written into the message)
 * char message[msg_size]: buffer to send
 * int tcp_socket: socket file descriptor
 */
double do_ping(size_t msg_size, int msg_no, char message[msg_size], int tcp_socket)
{
	char rec_buffer[msg_size];
	ssize_t recv_bytes, sent_bytes;
	size_t offset;
	struct timespec send_time, recv_time;
        double RTT_ms;

    /*** write msg_no at the beginning of the message buffer ***/
/*** TO BE DONE START ***/

	//Sprintf se fallisce mi restituisce un numero negativo, 
	//il controllo tramite if mi permette di verificare se tutto è andato bene
	if(sprintf(message,"%d\n",msg_no) < 0) //Scrivo il contenuto di msg_no nel buffer message 
		printf("Error inserting msg_no at the beginning of the message buffer...\n"); //Se il numero restituito è < 0, allora c'è un errore

/*** TO BE DONE END ***/

    /*** Store the current time in send_time ***/
/*** TO BE DONE START ***/

	//Clock_gettime restituisce -1 in caso di errore, per cui salvo il valore ritornato in error per poter verificare dopo se c'è stato un errore.
	//clock_gettime salva il tempo rilevato nella struct di tipo timespec send_time, che deve essere passata per riferimento
	//CLOCK_TYPE è il clk_id, ovvero l'identificatore del particolare clock con cui agiamo (definito tramite DEFINE in pingpong.h)
	int error_send = clock_gettime(CLOCK_TYPE, &send_time);
   if (error_send < 0) //Se vero allora c'è un errore
      fail_errno("The storing of the current time in send_time hasn't been successful...");
      //Fail_errno è una funzione che stampa un messaggio di errore definita nel file ausiliario fail.c
      //Se ho un errore stampo il messaggio di "segnalazione"
																													    
/***TO BE DONE END ***/

    /*** Send the message through the socket (blocking)  ***/
/*** TO BE DONE START ***/
	
	//send(sockfd, buf, len, flags);
	//Sockfd indica il tipo di socket, buf e len indicano rispettivamente il buffer del messaggio e la sua lunghezza
	sent_bytes = send(tcp_socket, message, msg_size, 0); //Salvo il valore di ritorno di send per verificare eventuali errori
   if (sent_bytes < 0) //sent_bytes = -1 -> errore
   	fail_errno("The sending of the message through the socket hasn't been successful..."); //Stampo un messaggio di "segnalazione" dell'errore

/*** TO BE DONE END ***/

    /*** Receive answer through the socket (blocking) ***/
	for (offset = 0; (offset + (recv_bytes = recv(tcp_socket, rec_buffer + offset, sent_bytes - offset, MSG_WAITALL))) < msg_size; offset += recv_bytes) 
	{
		debug(" ... received %zd bytes back\n", recv_bytes);
		if (recv_bytes < 0)
			fail_errno("Error receiving data");
	}

    /*** Store the current time in recv_time ***/
/*** TO BE DONE START ***/

	//Stesso ragionamento fatto precedentemente per salvare l'ora in send_time
	int recvt = clock_gettime(CLOCK_TYPE, &recv_time);
   if (recvt < 0) //Se vero allora c'è un errore
      fail_errno("The storing of the current time in recv_time hasn't been successful..."); 
      //Messaggio di errore stampato tramite fail_errno

/*** TO BE DONE END ***/

	printf("tcp_ping received %zd bytes back\n", recv_bytes);

	RTT_ms = timespec_delta2milliseconds(&recv_time, &send_time);
        sscanf(rec_buffer,"%ld %ld, %ld %ld\n", &(recv_time.tv_sec), &(recv_time.tv_nsec),
                                                &(send_time.tv_sec), &(send_time.tv_nsec));
	RTT_ms -= timespec_delta2milliseconds(&send_time, &recv_time);
	return RTT_ms;
}




int main(int argc, char **argv)
{
	struct addrinfo gai_hints, *server_addrinfo;
	int msgsz, norep;
	int gai_rv;
	char ipstr[INET_ADDRSTRLEN];
	struct sockaddr_in *ipv4;
	int tcp_socket;
	char request[MAX_REQ], answer[MAX_ANSW];
	ssize_t nr;

	if (argc < 4)
		fail("Incorrect parameters provided. Use: tcp_ping PONG_ADDR PONG_PORT SIZE [NO_REP]\n");
	for (nr = 4, norep = REPEATS; nr < argc; nr++)
		if (*argv[nr] >= '1' && *argv[nr] <= '9')
			sscanf(argv[nr], "%d", &norep);
	if (norep < MINREPEATS)
		norep = MINREPEATS;
	else if (norep > MAXREPEATS)
		norep = MAXREPEATS;

    /*** Initialize hints in order to specify socket options ***/
	memset(&gai_hints, 0, sizeof gai_hints);

/*** TO BE DONE START ***/
	
	//gai_hints è una struct di tipo addrinfo
	gai_hints.ai_family = AF_INET; //Indico che il formato dell'indirizzo fa parte della famiglia AF_INET(IPV4)
	gai_hints.ai_socktype = SOCK_STREAM; //Il socket deve essere di tipo STREAM
	gai_hints.ai_protocol = 0;
	gai_hints.ai_flags = 0;

/*** TO BE DONE END ***/

    /*** call getaddrinfo() in order to get Pong Server address in binary form ***/
/*** TO BE DONE START ***/
	
	//argv[1]=node e argv[2]=service
	//getaddrinfo() prende queste variabili e restituisce un indirizzo internet che può essere utilizzato tramite bind() e connect()
	//gai_hints da informazioni sul tipo di servizio/socket richiesto, e che deve essere restituito tramite server_addrinfo
	gai_rv = getaddrinfo(argv[1],argv[2], &gai_hints, &server_addrinfo); 
   if(gai_rv != 0) //Se gai_rv != 0 c'è un errore
   	fail_errno("Error using getaddrinfo:");

/*** TO BE DONE END ***/

    /*** Print address of the Pong server before trying to connect ***/
	ipv4 = (struct sockaddr_in *)server_addrinfo->ai_addr;
	printf("TCP Ping trying to connect to server %s (%s) on port %s\n", argv[1], inet_ntop(AF_INET, &ipv4->sin_addr, ipstr, INET_ADDRSTRLEN), argv[2]);

    /*** create a new TCP socket and connect it with the server ***/
/*** TO BE DONE START ***/
	
	//Alla chiamata di socket() gli passo il dominio(protocol family->IPV4), il tipo di socket richiesto(SOCK_STREAM), e il protocollo utilizzato
	//dovrebbe ritornarmi un file descriptor riferito al socket richiesto
	tcp_socket = socket(server_addrinfo->ai_family, server_addrinfo->ai_socktype, server_addrinfo->ai_protocol);
   if(tcp_socket == -1) //Se tcp_socket è -1 allora la chiamata a socket ha dato qualche problema
   	strerror(tcp_socket); //Traduce tcp_socket in una stringa di descrizione dell'errore

	//connect() connette il socket specificato da tcp_socket all'indirizzo specificato da ai_addr
	//specificando inoltre anche la lunghezza dell'indirizzo tramite ai_addrlen
   nr = connect(tcp_socket,server_addrinfo->ai_addr, server_addrinfo->ai_addrlen);
   if (nr < 0) //Se nr = -1 allora c'è un errore
   	fail_errno("Connection error..."); //Messaggio di errore stampato tramite fail_errno

/*** TO BE DONE END ***/

	freeaddrinfo(server_addrinfo);
	if (sscanf(argv[3], "%d", &msgsz) != 1)
		fail("Incorrect format of size parameter");
	if (msgsz < MINSIZE)
		msgsz = MINSIZE;
	else if (msgsz > MAXTCPSIZE)
		msgsz = MAXTCPSIZE;
	printf(" ... connected to Pong server: asking for %d repetitions of %d bytes TCP messages\n", norep, msgsz);
	sprintf(request, "TCP %d %d\n", msgsz, norep);

    /*** Write the request on socket ***/
/*** TO BE DONE START ***/

	//Scrivo nel file descriptor tcp_socket il contenuto di lunghezza strlen(request) del buffer request
	nr = write(tcp_socket, request, strlen(request));
	if (nr < 0) //Se nr = -1 allora la chiamata a write ha generato un errore
   	fail_errno("TCP Ping could not send request to Pong server"); //Messaggio di errore stampato tramite fail_errno

/*** TO BE DONE END ***/

	nr = read(tcp_socket, answer, sizeof(answer));
	if (nr < 0)
		fail_errno("TCP Ping could not receive answer from Pong server");
		
    /*** Check if the answer is OK, and fail if it is not ***/
/*** TO BE DONE START ***/
	
	//Tramite strcmp() confronto la stringa di risposta answer e la stringa "OK" per verificare se sono uguali
	//se lo sono strcmp() restituisce 0
	if(strcmp(answer,"OK\n") != 0) //Non sono uguali
   	fail_errno("The answer isn't OK..."); //Messaggio di errore stampato tramite fail_errno

/*** TO BE DONE END ***/

    /*** else ***/
	printf(" ... Pong server agreed :-)\n");

	{
		double ping_times[norep];
		struct timespec zero, resolution;
		char message[msgsz];
		int rep;
		memset(message, 0, (size_t)msgsz);
		for(rep = 1; rep <= norep; ++rep) {
			double current_time = do_ping((size_t)msgsz, rep, message, tcp_socket);
			ping_times[rep - 1] = current_time;
			printf("Round trip time was %lg milliseconds in repetition %d\n", current_time, rep);
		}
		memset((void *)(&zero), 0, sizeof(struct timespec));
		if (clock_getres(CLOCK_TYPE, &resolution))
			fail_errno("TCP Ping could not get timer resolution");
		print_statistics(stdout, "TCP Ping: ", norep, ping_times, msgsz, timespec_delta2milliseconds(&resolution, &zero));
	}

	shutdown(tcp_socket, SHUT_RDWR);
	close(tcp_socket);
	exit(EXIT_SUCCESS);
}

