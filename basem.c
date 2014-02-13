/*
Copyright (C) 2014 Florian Mayer 

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if not, see <http://www.gnu.org/licenses>.

Additional permission under GNU GPL version 3 section 7

If you modify this Program, or any covered work, by linking or combining it with [name of library] (or a modified version of that library), containing parts covered by the terms of [name of library's license], the licensors of this Program grant you additional permission to convey the resulting work. 
----
summary: This tool, called basem, brings semaphor functionality to the
shell. It has a very simple CLI described in the function usage().
Semaphores can be created as well as destroyed and the common Operations (p and v)
are usable by specifying the -p or -v flag. The Semaphores are referenced
by the Sys-V IPC keys. 
*/

#include <stdio.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <errno.h>
#include "basem.h"

#define HANDLE_SEMGET_RC switch(errno){ \
	case EACCES: \
		printf("Insufficient permissions for existing semaphor\n");\
		break;\
	case EEXIST: \
		printf("Semaphor with key %d exists!\n", semkey); \
		break; \
	case EINVAL: \
		printf("[SEMGET] invalid parameter\n"); \
		break;\
	case ENOENT: \
		printf("No semaphor could be found\n"); \
		break;\
	case ENOMEM: \
		printf("System has not enough memory left!\n"); \
		break;\
	case ENOSPC: \
		printf("Systemlimit for semaphores has exceeded!\n"); \
		break;\
	default: perror("[SEMGET]"); break;\
}\

#define HANDLE_SEMOP_RC switch(errno){\
	case E2BIG: \
		printf("Too many operations!\n"); \
		break; \
	case EACCES: \
		printf("Insufficient permissions!\n"); \
		break;\
	case EAGAIN: \
		printf("IPC_NOWAIT\n"); \
		break; \
	case EINTR: \
		printf("Semop syscall was interrupted by a signal\n"); \
		break;\
	case EIDRM: \
		printf("Semaphor has been removed removed!\n"); \
		break;\
	case EINVAL: \
		printf("No corresponding semaphor for id: %d\n", semid); \
		break;\
}\

#define HANDLE_SEMCTL_RC switch(errno){\
	case EPERM: \
		printf("EUID does not match the owners/creators UID!\n"); \
		break;\
	case EACCES: \
		printf("Insufficient permissions!\n"); \
		break;\
	case EIDRM: \
		printf("Id has been removed removed!\n"); \
		break;\
	case ERANGE: \
		printf("[SEMCTL] Attempt to set value outside of range\n"); \
		break;\
	case EINVAL: \
		printf("False semaphor id: %d\n", semid); \
		break;\
	default: perror("[SEMCTL]"); break;\
}\

/* Prints the general usage informations for the user */
void usage(){
	printf("[usage] <progname>\
	[-c | --create ] //create semaphor \n \
	[ -d | --destroy ] //destroy semaphor \n \
	[ -p | --pop ] //Do the p-operation on the given semaphor \n \
	[ -v | --vop] // Do the v-operation on the given semaphor \n \
	[ -r | --rights ] // Define the rights to use for newly created semaphors \n \
	[ -k | --key ] // Define the Key to use for the newly created semaphor\n \
	[ -i | --init-value] //Set the default value while creation \n \
		\n\n \
	Examples \n\
	$ <prog> --create --rights 0600 --value 100 --key 12345 \n\
	# creates a semaphor. If it is created, this will be a nop. \n\
\n\
	$ <prog> --destroy --key 12345 \n\
	# destroys the previously created semaphor \n\
	# It exits with 0 if successful and 1 if otherwise \n\
\n\
	$ <prog> --key 12345 --pop \n\
	# Does the p-Op on the Semaphor (key: 12345) \n\
\n\
	$ <prog> --key 12345 --vop \n\
	# Does the v-Op on the Semaphor (key: 12345)\
\n\
	The standard values for permission and value are 0600 and 0\n");
}

/* Prints general version information */
void version(){
	printf("version.\n");	
}

/* Implements the p-Operation on a Semaphor.
	Param: semid = A valid Sys-V semaphor identifier 
	Return: 0 	*/
int pop(int semid){
	struct sembuf buf;
	buf.sem_num=0;
	buf.sem_op=-1;
	buf.sem_flg=0;
	if(semop(semid, &buf, 1)){
		HANDLE_SEMOP_RC
		return -1;
	}
	return 0;
}

/* Implements the v-Operation on a Semaphor.
	Param: semid = A valid Sys-V semaphor identifier
	Return: 0 */
int vop(int semid){
	struct sembuf buf;
	buf.sem_num=0;
	buf.sem_op=1;
	buf.sem_flg=0;
	if(semop(semid, &buf, 1)){
		HANDLE_SEMOP_RC
		return -1;
	}
	return 0;
}

/* Tries to get an existing semaphor id 
	Param: semkey = A valid Sys-V IPC-key
	Return: Returncode of the semget systemcall */
int takesem(key_t semkey){
	int semid;

	if((semid = semget(semkey, 0, 0)) < 0 ){
		HANDLE_SEMGET_RC
	}
	return semid;
}

/* Initializes a new semaphor
	Param: semkey = A new Sys-V IPC-key the System should use
		permissions = The permissions for the new semaphor
		value = The initial value of the semaphor.
	Return: A valid semaphor id. If creation fails, -1 is returned */
int seminit(key_t semkey, int permissions, int value){
	int semid;
	union semun {
		int val;
		struct semid_ds *buf;
		unsigned short int *array;
		struct seminfo *__buf;
	} arg;

	if ((semid = semget(semkey, 1, permissions | IPC_CREAT | IPC_EXCL)) < 0){
		HANDLE_SEMGET_RC
		return -1; /* -1 for readility. semid could be used too ... */
	}
	arg.val = value;
	if (semctl(semid, 0, SETVAL, arg) < 0) { 
		HANDLE_SEMCTL_RC
		return -1;
	}
	return semid;
}

/* Destroys a created semaphor
	Param: semid = A valid semid
	Return: 0. -1 if the systemcall semctl fails */
int semdest(int semid){
	if(semctl(semid, 0, IPC_RMID, 0) < 0) {
		HANDLE_SEMCTL_RC
		return -1;
	}
	return 0;
}

/* Just a debug function */
void args_to_string(struct settings *a){
	printf("c: %d, d: %d, perm: %d, key: %d, value: %d, pop: %d, vop: %d\n", a->create, a->destroy,
		 a->permissions, a->key, a->value, a->pop, a->vop);	
}

/* Parses the arguments of the program using the getopt-long function 
	Param: argc and argv. The unmodified values that the main-function gets as Parameter.
	Return: A pointer to a populated settings structure */
struct settings *parse_args(int argc, char **argv){
	int r=0, j=0, i; /* used for string -> octal conversion */
	int c, op_i; /* used for getopt_long */
	struct settings *s = (struct settings *) malloc(sizeof(struct settings));
	static struct option long_opts[] = {
		{"help", no_argument, 0, 'h'},
		{"version", no_argument, 0, 'l'},
		{"create", no_argument, 0, 'c'},
		{"destroy", no_argument, 0, 'd'},
		{"init-value", required_argument, 0, 'i'},
		{"key", required_argument, 0, 'k'},
		{"vop", no_argument, 0, 'v'},
		{"pop", no_argument, 0, 'p'},
		{"rights", required_argument, 0, 'r'}
	};
	if(s == NULL){
		return NULL;
	}
	s->help = 0; s->version = 0;
	s->create = 0; s->destroy = 0;
	s->pop = 0; s->vop = 0;
	s->permissions = STD_PERM; s->key = 0;
	s->value = 0;

	while ((c = getopt_long(argc, argv, OPTSTRING, long_opts, &op_i)) != -1){
		switch (c) {
			case 'h': 
				s->help = TRUE;
				break;
			case 'l':
				s->version = TRUE;
				break;
			case 'c': 
				s->create = TRUE; 
				break;
			case 'd': 
				s->destroy = TRUE; 
				break;
			case 'i': 
				s->value = atoi(optarg); 
				break;
			case 'k': 
				s->key = atoi(optarg); 
				break;
			case 'v': 
				s->vop = TRUE; 
				break;
			case 'p': 
				s->pop = TRUE; 
				break;
			case 'r': 
				/* Converts the string representation of the permissioncode into binary */
				s->permissions = 0;
				for (i=strlen(optarg)-1; i>=0; i--){
					if(optarg[i] < 48 || optarg[i] > 57){
						printf("Permissions have to be specified as octal number!\n");
						return NULL;
					}
					r += pow(8, j++) * (optarg[i]-48);
				}
				j=0;
				do {
					i = r%2;
					s->permissions += pow(2, j++) * i;
				} while(r/=2);

				break;
			default: 
				return NULL;
				break;
		}
	}
	return s;	
}

/* Binds it all together */
int main(int argc, char **argv){
	struct settings *args = parse_args(argc, argv);			
	if(args == NULL){
		usage();
		return EXIT_SUCCESS;
	}
	int semid;
	
	//args_to_string(args);
	if (args->version){
		version();
	} else if (args->help){
		usage();
	} else if (args->create && args->key){
		/* if permissions (-r | --rights) is omitted, the value 600(8) is used
			if value (-v | --value) is omitted, the value 0 is used */
		if(seminit(args->key, args->permissions, args->value) < 0)
			return EXIT_FAILURE;
	} else if (args->destroy && args->key){
		if((semid = takesem(args->key)) < 0)
			exit(1);
		if(semdest(semid))
			exit(1);
	} else if (args->pop && args->key) {
		if((semid = takesem(args->key)) < 0)
			exit(1);
		if(pop(semid) < 0)
			exit(1);
	} else if (args->vop && args->key){
		if((semid = takesem(args->key)) < 0)
			exit(1);
		if(vop(semid) < 0)
			exit(1);
	} else {
		usage();
	}
	return EXIT_SUCCESS;
}
