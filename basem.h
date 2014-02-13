int pop(int semid);
int vop(int semid);
int seminit(key_t semkey, int permissions, int value);
int semdest(int semid);
int takesem(key_t semkey);
struct settings *parse_arguments(int argc, char **argv);
void usage();

#define STD_PERM 0600
#define OPTSTRING "lhcdi:k:pvr:"
#define FALSE 0
#define TRUE 1

struct settings {
	char help;
	char create;
	char version;
	char destroy;
	char pop;
	char vop;
	int permissions;
	int key;
	int value;	
};
