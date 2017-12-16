#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#define TIMEOUT 10
#define SLEEP_TIME 1

int main(int argc, char *argv[])
{
	int sem_processor, sem_connector;
	
	sem_processor = notice_open(PROCESSOR);
	
	config_load_all();

	for ( ; ; ) {
		notice_wait(sem_processor);
		
		db_get_request();
		
		process();
		
		notice_fire(sem_connector);
	}
	
	return 0;
}