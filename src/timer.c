#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#define TIMEOUT 10
#define SLEEP_TIME 1

int main(int argc, char *argv[])
{
	int sem_timer;
	int c;
	
	sem_timer = notice_open(TIMER);
	
	for ( ; ; ) {
		notice_wait(sem_timer);
		
		for ( ; ; ) {
			c = db_get_count();
			if (0 == c) {
				break;
			}
			
			process();
			
			sleep(SLEEP_TIME);
		}
	}
	return 0;
}

static int process()
{
	int sem_connector;
	sem_connector = db_handle_timeout(TIMEOUT);
	notice_fire(sem_connector);
	
	return 0;
}