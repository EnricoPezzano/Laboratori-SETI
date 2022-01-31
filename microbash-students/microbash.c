//#error Please read the accompanying microbash.pdf before hacking this source code (and removing this line).

/*
 * Micro-bash v2.0
 *
 * Programma sviluppato a supporto del laboratorio di Sistemi di
 * Elaborazione e Trasmissione dell'Informazione del corso di laurea
 * in Informatica presso l'Università degli Studi di Genova, a.a. 2021/2022.
 *
 * Copyright (C) 2021 by Giovanni Lagorio <giovanni.lagorio@unige.it>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>

void fatal(const char * const msg)
{
	fprintf(stderr, "%s\n", msg);
	exit(EXIT_FAILURE);
}

void fatal_errno(const char * const msg)
{
	perror(msg);
	exit(EXIT_FAILURE);
}

void *my_malloc(size_t size)
{
	void *rv = malloc(size);																											//puntatore void -> punta a un tipo non definito
	if (!rv)
		fatal_errno("my_malloc");
	return rv;
}

void *my_realloc(void *ptr, size_t size)
{
	void *rv = realloc(ptr, size);
	if (!rv)
		fatal_errno("my_realloc");
	return rv;
}

char *my_strdup(char *ptr)
{
	char *rv = strdup(ptr);																												//duplica la stringa
	if (!rv)
		fatal_errno("my_strdup");
	return rv;
}

#define malloc I_really_should_not_be_using_a_bare_malloc												//non posso usare malloc, realloc o strdup della libc
#define realloc I_really_should_not_be_using_a_bare_realloc
#define strdup I_really_should_not_be_using_a_bare_strdup

static const int NO_REDIR = -1;

typedef enum { CHECK_OK = 0, CHECK_FAILED = -1 } check_t;												//enum assegna stringhe a valori interi

static const char *const CD = "cd";

typedef struct {																																//typedef rinomina le variabili
	int n_args;
	char **args; // in an execv*-compatible format; i.e., args[n_args]=0
	char *out_pathname; // 0 if no output-redirection is present
	char *in_pathname; // 0 if no input-redirection is present
} command_t;

typedef struct {
	int n_commands;
	command_t **commands;
} line_t;

void free_command(command_t * const c)
{
	assert(c==0 || c->n_args==0 || (c->n_args > 0 && c->args[c->n_args] == 0)); /* sanity-check: if c is not null, then it is either empty (in case of parsing error) or its args are properly NULL-terminated */
	/*** TO BE DONE START ***/
		free(c);																																		//basta così? la free è sufficiente?
	/*** TO BE DONE END ***/
}

void free_line(line_t * const l)
{
	assert(l==0 || l->n_commands>=0); /* sanity-check */
	/*** TO BE DONE START ***/
		free(l);																																		//basta così? la free è sufficiente?
	/*** TO BE DONE END ***/
}

#ifdef DEBUG																																		//"ifdef (qualcosa)" sta per "se (qualcosa) è definito, allora faccio fino a #endif"
	void print_command(const command_t * const c)																	//forse print_command e print_line sono cose per debug di Lagorio? DEBUG non viene definito da nessuna parte
	{
		if (!c) {
			printf("Command == NULL\n");
			return;
		}
		printf("[ ");
		for(int a=0; a<c->n_args; ++a)
			printf("%s ", c->args[a]);
		assert(c->args[c->n_args] == 0);																						//assert fa finire il programma se il suo contenuto è falso
		printf("] ");
		printf("in: %s out: %s\n", c->in_pathname, c->out_pathname);
	}

	void print_line(const line_t * const l)
	{
		if (!l) {
			printf("Line == NULL\n");
			return;
		}
		printf("Line has %d command(s):\n", l->n_commands);
		for(int a=0; a<l->n_commands; ++a)
			print_command(l->commands[a]);
	}
#endif

command_t *parse_cmd(char * const cmdstr)
{
	static const char *const BLANKS = " \t";																												//divido per caratteri che sono spazio o tab
	command_t * const result = my_malloc(sizeof(*result));																					//alloco una variabile di tipo command_t per infilarci dentro i pezzi che ottengo
	memset(result, 0, sizeof(*result));																															//copio 0 per sizeof(*result) volte in result   (pulisco result per il prossimo comando che arriva)
	char *saveptr, *tmp;
	tmp = strtok_r(cmdstr, BLANKS, &saveptr);																												//cmdstr viene divisa per ogni BLANKS che incontra e i pezzi vengono salvati in tmp
	while (tmp) {
		result->args = my_realloc(result->args, (result->n_args + 2)*sizeof(char *));
		if (*tmp=='<') {																																							//se trovo redirezione input
			if (result->in_pathname) {
				fprintf(stderr, "Parsing error: cannot have more than one input redirection\n");
				goto fail;
			}
			if (!tmp[1]) {
				fprintf(stderr, "Parsing error: no path specified for input redirection\n");
				goto fail;
			}
			result->in_pathname = my_strdup(tmp+1);
		} else if (*tmp == '>') {																																			//se trovo redirezione output
			if (result->out_pathname) {
				fprintf(stderr, "Parsing error: cannot have more than one output redirection\n");
				goto fail;
			}
			if (!tmp[1]) {
				fprintf(stderr, "Parsing error: no path specified for output redirection\n");
				goto fail;
			}
			result->out_pathname = my_strdup(tmp+1);
		} else {
			if (*tmp=='$') {
				/* Make tmp point to the value of the corresponding environment variable, if any, or the empty string otherwise */
				/*** TO BE DONE START ***/
				if (getenv(tmp+1)==NULL)																								//funziona?
					tmp="\0";																															//asterisco o meno?
				else
					tmp=getenv(tmp+1);																										//asterisco o meno?
				/*** TO BE DONE END ***/
			}
			result->args[result->n_args++] = my_strdup(tmp);
			result->args[result->n_args] = 0;
		}
		tmp = strtok_r(0, BLANKS, &saveptr);
	}
	if (result->n_args)
		return result;
	fprintf(stderr, "Parsing error: empty command\n");
fail:
	free_command(result);
	return 0;
}

line_t *parse_line(char * const line)
{
	static const char * const PIPE = "|";
	char *cmd, *saveptr;
	cmd = strtok_r(line, PIPE, &saveptr);
	if (!cmd)
		return 0;
	line_t *result = my_malloc(sizeof(*result));
	memset(result, 0, sizeof(*result));
	while (cmd) {
		command_t * const c = parse_cmd(cmd);
		if (!c) {
			free_line(result);
			return 0;
		}
		result->commands = my_realloc(result->commands, (result->n_commands + 1)*sizeof(command_t *));
		result->commands[result->n_commands++] = c;
		cmd = strtok_r(0, PIPE, &saveptr);
	}
	return result;
}

check_t check_redirections(const line_t * const l)
{
	assert(l);
	/* This function must check that:
	 * - Only the first command of a line can have input-redirection
	 * - Only the last command of a line can have output-redirection
	 * and return CHECK_OK if everything is ok, CHECK_FAILED otherwise
	 */
	/*** TO BE DONE START ***/
	for (int i=1; i<l->n_commands; i++)
		if (strcmp(l->commands[i]->in_pathname, "0"))
			return CHECK_FAILED;

	for (int i=l->n_commands-2; i>=0; i--)
		if (strcmp(l->commands[i]->out_pathname, "0"))
			return CHECK_FAILED;

	/*** TO BE DONE END ***/
	return CHECK_OK;
}

check_t check_cd(const line_t * const l)
{
	assert(l);
	/* This function must check that if command "cd" is present in l, then such a command
	 * 1) must be the only command of the line
	 * 2) cannot have I/O redirections
	 * 3) must have only one argument
	 * and return CHECK_OK if everything is ok, CHECK_FAILED otherwise
	 */
	/*** TO BE DONE START ***/
// se c'è cd deve essere l'unico comando
// e dopo che lo hai verificato devi vedere che non abbia redirezioni e che abbia un solo argomento
	int is_here = 0;
	for(int i=0; i < l->n_commands; i++)
		if(!strncmp(l->commands[i]->args[0], CD, 2)) //ci entra se trova cd
			is_here = 1;
	if (!is_here) //va bene che non ci sia, sarebbe un altro comando
		return CHECK_OK;
	else{
		if(!strncmp(l->commands[0]->args[0], CD, 2) && l->n_commands>1)
			return CHECK_FAILED;

		if(check_redirections(l) == CHECK_FAILED)
			return CHECK_FAILED;

		if(l->commands[0]->n_args != 2)
			return CHECK_FAILED;
	}

	/*** TO BE DONE END ***/
	return CHECK_OK;
}

void wait_for_children()
{
	/* This function must wait for the termination of all children processes.
	 * If a child exits with an exit-status!=0, then you should print a proper message containing its PID and exit-status.
	 * Similarly, if a child is killed by a signal, then you should print a message specifying its PID, signal number and name.
	 */
	/*** TO BE DONE START ***/
	int status;
	pid_t pid;																																	//qui ci salvo exit status
	while ((pid = wait (&status))!=-1) {																			//faccio wait finchè non ci sono più figli da aspettare. Quando non ci sono più figli wait() ritorna -1
		if (WIFEXITED(status) != 0) {
      int num = WEXITSTATUS(status);																						//qui ci entro se exit-status!=0
      printf("Process with ID %d terminated with status: %d\n", pid, num);
    }
    else if ( WIFSIGNALED(status)) {																						//qui ci entro se il filgio è stato interrotto da un segnale
      int num = WTERMSIG(status);
			char* processName = my_malloc(4096);
			sprintf (processName, "/proc/%d/cmdline", pid);														//DA CONTROLLARE QUANDO COMPILIAMO
      printf("Process %s with ID %d exited due to receiving signal %d\n", processName, pid,  num);
    }
	}																																							//bisogna stampare pid e nomi segnali e capire se c'è qualcosa di ridondante all'inizio
	/*** TO BE DONE END ***/
}

void redirect(int from_fd, int to_fd)
{
	/* If from_fd!=NO_REDIR, then the corresponding open file should be "moved" to to_fd.
	 * That is, use dup/dup2/close to make to_fd equivalent to the original from_fd, and then close from_fd
	 */
	/*** TO BE DONE START ***/
		if (from_fd!=NO_REDIR){
			if(dup2(from_fd, to_fd) == -1){
				close(to_fd);
				fatal_errno("dup2");
			}
			close (from_fd);
		}
	/*** TO BE DONE END ***/
}

void run_child(const command_t * const c, int c_stdin, int c_stdout)
{
	/* This function must:
	 * 1) create a child process, then, in the child
	 * 2) redirect c_stdin to STDIN_FILENO (=0)
	 * 3) redirect c_stdout to STDOUT_FILENO (=1)
	 * 4) execute the command specified in c->args[0] with the corresponding arguments c->args
	 * (printing error messages in case of failure, obviously)
	 */
	/*** TO BE DONE START ***/
		pid_t pid = fork();
		if(pid == -1)
			fatal_errno("fork");
		if (pid == 0) {																															//sono nel processo figlio
			c_stdin = STDIN_FILENO;																										//sistremo input
			c_stdout = STDOUT_FILENO;																									//sistemo output
			if (execvp (c->args[0], c->args) == -1)																		//posso fare una cosa del genere o devo dividere in due comandi separati?
				fatal_errno("execvp");
			//IMPORTANTE																															//DEVO FARE EXIT() O EXEC() FA TUTTO DA SOLA?
		}
	/*** TO BE DONE END ***/
}

void change_current_directory(char *newdir)
{
	/* Change the current working directory to newdir
	 * (printing an appropriate error message if the syscall fails)
	 */
	/*** TO BE DONE START ***/
		if(chdir(newdir) == -1)																											//funziona?
			fatal_errno("chdir");																											//basta veramente fare solamente chdir?
	/*** TO BE DONE END ***/
}

void close_if_needed(int fd)
{
	if (fd==NO_REDIR)
		return; // nothing to do
	if (close(fd))
		perror("close in close_if_needed");
}

void execute_line(const line_t * const l)
{
	if (strcmp(CD, l->commands[0]->args[0])==0) {
		assert(l->n_commands == 1 && l->commands[0]->n_args == 2);
		change_current_directory(l->commands[0]->args[1]);
		return;
	}
	int next_stdin = NO_REDIR;
	for(int a=0; a<l->n_commands; ++a) {
		int curr_stdin = next_stdin, curr_stdout = NO_REDIR;
		const command_t * const c = l->commands[a];
		if (c->in_pathname) {
			assert(a == 0);
			/* Open c->in_pathname and assign the file-descriptor to curr_stdin
			 * (handling error cases) */
			/*** TO BE DONE START ***/

			curr_stdin = open(c->in_pathname, O_RDWR);
			if(curr_stdin == -1)
				fatal_errno("opening error!");

			/*** TO BE DONE END ***/
		}
		if (c->out_pathname) {
			assert(a == (l->n_commands-1));
			/* Open c->out_pathname and assign the file-descriptor to curr_stdout
			 * (handling error cases) */
			/*** TO BE DONE START ***/

			curr_stdout = open(c->out_pathname, O_RDWR);
			if(curr_stdout == -1)
				fatal_errno("opening error!");

			/*** TO BE DONE END ***/
		} else if (a != (l->n_commands - 1)) { /* unless we're processing the last command, we need to connect the current command and the next one with a pipe */
			int fds[2];
			/* Create a pipe in fds, and set FD_CLOEXEC in both file-descriptor flags */
			/*** TO BE DONE START ***/

			if(pipe2(fds, O_CLOEXEC) == -1)
				fatal_errno("opening error!");

			/*** TO BE DONE END ***/
			curr_stdout = fds[1];
			next_stdin = fds[0];
		}
		run_child(c, curr_stdin, curr_stdout);
		close_if_needed(curr_stdin);
		close_if_needed(curr_stdout);
	}
	wait_for_children();
}

void execute(char * const line)
{
	line_t * const l = parse_line(line);
#ifdef DEBUG
	print_line(l);
#endif
	if (l) {
		if (check_redirections(l)==CHECK_OK && check_cd(l)==CHECK_OK)
			execute_line(l);
		free_line(l);
	}
}

int main()
{
	const char * const prompt_suffix = " $ ";
	const size_t prompt_suffix_len = strlen(prompt_suffix);
	for(;;) {
		char *pwd;
		/* Make pwd point to a string containing the current working directory.
		 * The memory area must be allocated (directly or indirectly) via malloc.
		 */
		/*** TO BE DONE START ***/
		// char* cwd = my_malloc(100); //da aggiustare address sanitizer...fa pasticci
		//getcwd(cwd, sizeof(cwd));
		// pwd = my_malloc(sizeof(cwd));
		// strcat(pwd, cwd);
		// if (getcwd(pwd, sizeof(pwd)) == NULL)																						//dimensione di PATH_MAX = 4096 ... getcwd indirettamente fa la malloc
		//	fatal_errno("getcwd");																										//errore in cwd
		/*** TO BE DONE END ***/
		pwd = my_realloc(pwd, strlen(pwd) + prompt_suffix_len + 1);
		strcat(pwd, prompt_suffix);
		char * const line = readline(pwd);
		free(pwd);
		if (!line) break;
		execute(line);
		free(line);
	}
}
