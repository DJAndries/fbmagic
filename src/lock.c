#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include "log.h"

#define LOCK_FILE "/var/lock/fbmagic.lock"

int fbmagic_lock_acquire(char should_block) {
	int fd = open(LOCK_FILE, O_RDWR | O_CREAT);
	struct flock lock;

	if (fd == 0) {
		mlog(LOG_ERROR, "Failed to open lock file");
		return 0;
	}

	chmod(LOCK_FILE, S_IRUSR | S_IWUSR | S_IWGRP | S_IRGRP | S_IWOTH | S_IROTH);

	lock.l_type = F_WRLCK;
	lock.l_start = 0;
	lock.l_whence = SEEK_SET;
	lock.l_len = 0;
	lock.l_pid = 0;

	if (fcntl(fd, should_block ? F_SETLKW : F_SETLK, &lock)) {
		mlog(LOG_DEBUG, "Failed to acquire lock");
		close(fd);
		return 0;
	}
	return fd;
}

void fbmagic_lock_release(int fd) {
	close(fd);
}
