
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <unistd.h>
#include <signal.h>
#include <poll.h>
#include <pty.h>
#include <sys/signalfd.h>
#include <sys/timerfd.h>

#include <utempter.h>

#include "common.h"
#include "config.h"
#include "error.h"

struct PtyWatch
{
	SInt32		fdsig;
	sigset_t	default_signal_set;

	SInt32		fdtimer;

	SInt32		fdmaster;
	SInt32		fdslave;

	struct pollfd	pfds[3];
	SInt32		quit;

	char		*msg;
			/* Message length without trailing zero. (msglen + 1) <= msgcap.
			 */
	SInt64		msglen;
	SInt64		msgcap;
};

static SInt32 Init_Signal_Handling(struct PtyWatch *self)
{
	sigset_t all;
	SInt32 err;

	sigfillset(&all);
	err = sigprocmask(SIG_BLOCK, &all, &self->default_signal_set);
	if (UNLIKELY(err)) {
		PTYWATCH_ERROR("Failed to block signals: %d (%s)", errno, strerror(errno));
		return -errno;
	}

	self->fdsig = signalfd(-1, &all, 0);
	if (UNLIKELY(-1 == self->fdsig)) {
		PTYWATCH_ERROR("Failed to open signalfd: %d (%s)", errno, strerror(errno));
		return -errno;
	}

	return 0;
}

static SInt32 Fini_Signal_Handling(struct PtyWatch *self)
{
	SInt32 err;

	err = close(self->fdsig);
	if (UNLIKELY(err)) {
		PTYWATCH_ERROR("close() failed with errno %d (%s)", errno, strerror(errno));
	}

	err = sigprocmask(SIG_BLOCK, &self->default_signal_set, NULL);
	if (UNLIKELY(err)) {
		PTYWATCH_ERROR("Failed to reset default signal set");
		return -errno;
	}

	return 0;
}

static SInt32 Create_Timer(struct PtyWatch *self)
{
	self->fdtimer = timerfd_create(CLOCK_MONOTONIC, 0);
	if (UNLIKELY(-1 == self->fdtimer)) {
		PTYWATCH_ERROR("Failed to open timerfd: %d (%s)", errno, strerror(errno));
		return -errno;
	}

	return 0;
}

static SInt32 Destroy_Timer(struct PtyWatch *self)
{
	SInt32 err;

	err = close(self->fdtimer);
	if (UNLIKELY(err)) {
		PTYWATCH_ERROR("close() failed with errno %d (%s)", errno, strerror(errno));
	}

	return 0;
}

/* In order to receive wall messages we require an utmp file entry. We use libutempter to
 * do the job so that we do not have to worry about access permissions to /var/run/utmp. 
 * libutempter uses a seperate program (/usr/libexec/utempter/utempter on Fedora and RHEL)
 * to add and remove entries.
 */
static SInt32 Insert_Into_Utmp_Database(struct PtyWatch *self)
{
	SInt32 err;

	err = !utempter_add_record(self->fdmaster, NULL);
	if (UNLIKELY(err)) {
		PTYWATCH_FATAL("utempter_add_record() failed");	/* Fatal: No sense in continuing.
								 */
		return -1;
	}

	return 0;
}

static SInt32 Remove_From_Utmp_Database(struct PtyWatch *self)
{
	SInt32 err;

	err = !utempter_remove_record(self->fdmaster);
	if (UNLIKELY(err)) {
		PTYWATCH_ERROR("utempter_remove_record() failed");
		return -1;
	}

	return 0;
}

static SInt32 Empty_Msg(struct PtyWatch *self)
{
	if (UNLIKELY(0 == self->msgcap)) {
		self->msgcap = PTYWATCH_CONFIG_INITIAL_MSGCAP;

		self->msg = malloc(self->msgcap);
		if (UNLIKELY(!self->msg)) {
			PTYWATCH_FATAL("malloc() failed");
			return -ENOMEM;
		}
	}

	self->msglen = 0;
	self->msg[0] = 0;

	return 0;
}

static SInt32 Append_To_Msg(struct PtyWatch *self, const char *buf, SInt64 len)
{
	if ((self->msglen + len + 1) >= self->msgcap) {
		/* Not optimal but good enough for us. Messages will usually not be too
		 * large.
		 */
		while ((self->msglen + len + 1) >= self->msgcap) {
			self->msgcap *= 2;
		}

		self->msg = realloc(self->msg, self->msgcap);
		if (UNLIKELY(!self->msg)) {
			PTYWATCH_FATAL("realloc() failed");
			return -ENOMEM;
		}
	}

	*(char *)mempcpy(self->msg + self->msglen, buf, len) = 0;
	self->msglen += len;

	return 0;
}

static SInt32 Init(struct PtyWatch *self)
{
	SInt32 err;

	err = openpty(&self->fdmaster, &self->fdslave, NULL, NULL, NULL);
	if (UNLIKELY(err < 0)) {
		PTYWATCH_ERROR("openpty() failed with errno %d (%s)", errno, strerror(errno));
		return -errno;
	}

	err = Init_Signal_Handling(self);
	if (UNLIKELY(err))
		return err;

	err = Create_Timer(self);
	if (UNLIKELY(err))
		return err;

	err = Insert_Into_Utmp_Database(self);
	if (UNLIKELY(err))
		return err;

	err = Empty_Msg(self);
	if (UNLIKELY(err))
		return err;

	return 0;
}

static void Fini(struct PtyWatch *self)
{
	(void )Remove_From_Utmp_Database(self);
	(void )Destroy_Timer(self);
	(void )Fini_Signal_Handling(self);
}

static SInt32 Fill_Pollfds(struct PtyWatch *self)
{
	memset(self->pfds, 0, sizeof(self->pfds));

	self->pfds[0].fd     = self->fdsig;
	self->pfds[0].events = POLLIN;

	self->pfds[1].fd     = self->fdmaster;
	self->pfds[1].events = POLLIN;

	/* TODO Handle timer. */

	return 2;
}

static SInt32 Handle_Sigquit(struct PtyWatch *self, SInt32 signo)
{
	self->quit = 1;

	return 0;
}

static SInt32 Handle_Signal(struct PtyWatch *self)
{
	struct signalfd_siginfo info;
	SInt32 err;

	err = read(self->fdsig, &info, sizeof(info));
	if (UNLIKELY(err < 0)) {
		PTYWATCH_ERROR("read() failed with errno %d (%s)", errno, strerror(errno));
		return -errno;
	}
	if (err != sizeof(info)) {
		PTYWATCH_ERROR("read() returned %d (sizeof signalfd_siginfo is %d)\n", err, sizeof(info));
		return -1;
	}

	PTYWATCH_DEBUG("Received signal %d", info.ssi_signo);

	if ((SIGINT  == info.ssi_signo) ||
	    (SIGQUIT == info.ssi_signo) ||
	    (SIGTERM == info.ssi_signo)) {
		return Handle_Sigquit(self, info.ssi_signo);
	}

	PTYWATCH_WARN("Ignoring signal %d from pid %d (uid %d)", info.ssi_signo, info.ssi_pid, info.ssi_uid);

	return 0;
}

static SInt32 Read_From_Pty(struct PtyWatch *self)
{
	SInt32 err;
	char buf[128];

	err = read(self->fdmaster, buf, sizeof(buf));
	if (UNLIKELY(err < 0)) {
		PTYWATCH_ERROR("read() failed with errno %d (%s)", errno, strerror(errno));
		return -errno;
	}

	err = Append_To_Msg(self, buf, err);
	if (UNLIKELY(err)) {
		PTYWATCH_ERROR("Append_To_Msg() failed");
		return err;
	}

	return 0;
}

static SInt32 Arm_Timer(struct PtyWatch *self, SInt64 sec, SInt64 usec)
{
	SInt32 err;
	struct itimerspec ts;

	memset(&ts, 0, sizeof(ts));

	ts.it_value.tv_sec = sec;
	ts.it_value.tv_nsec = 1000*usec;

	err = timerfd_settime(self->fdtimer, 0, &ts, NULL);
	if (UNLIKELY(err < 0)) {
		PTYWATCH_ERROR("timerfd_settime() failed with errno %d (%s)", errno, strerror(errno));
		return -errno;
	}

	return err;
}

/* Reset the timer to the pre-defined relative time in the future.
 */
static SInt32 Reset_Timer(struct PtyWatch *self)
{
	return Arm_Timer(self, PTYWATCH_CONFIG_TIMEDIFF_SEC, PTYWATCH_CONFIG_TIMEDIFF_USEC);
}

static SInt32 Disarm_Timer(struct PtyWatch *self)
{
	return Arm_Timer(self, 0, 0);
}

static SInt32 Handle_Timer(struct PtyWatch *self, _Bool ignore)
{
	UInt64 dummy;
	SInt32 err;

	err = read(self->fdtimer, &dummy, sizeof(dummy));
	if (UNLIKELY(err < 0)) {
		PTYWATCH_ERROR("read() failed with errno %d (%s)", errno, strerror(errno));
		return -errno;
	}

	if (ignore)
		return 0;

	err = Disarm_Timer(self);
	if (UNLIKELY(err)) {
		PTYWATCH_ERROR("Disarm_Timer() failed");
	}

	/* TODO Notification.
	 */
	fprintf(stdout, self->msg);

	err = Empty_Msg(self);
	if (UNLIKELY(err)) {
		PTYWATCH_ERROR("Empty_Msg() failed");
		return err;
	}

	return 0;
}

static SInt32 Loop(struct PtyWatch *self)
{
	SInt32 err;
	SInt32 n;

	self->quit = 0;
	while (LIKELY(!self->quit)) {
		n = Fill_Pollfds(self);

		err = poll(self->pfds, n, -1);
		if (UNLIKELY(err < 0)) {
			PTYWATCH_ERROR("poll() failed with errno %d (%s)", errno, strerror(errno));
			return -1;
		}

		if (self->pfds[0].revents & POLLIN) {
			(void )Handle_Signal(self);
		}

		if (self->pfds[1].revents & POLLIN) {
			if (self->pfds[2].revents & POLLIN) {
				(void )Handle_Timer(self, 1);	/* Ignore */
			}

			(void )Read_From_Pty(self);
			(void )Reset_Timer(self);
		} else {
			if (self->pfds[2].revents & POLLIN) {
				(void )Handle_Timer(self, 0);
			}
		}
	}

	return 0;
}

int main(int argc, char **argv)
{
	struct PtyWatch ptywatch;
	SInt32 err;

	memset(&ptywatch, 0, sizeof(struct PtyWatch));

	err = Init(&ptywatch);
	if (UNLIKELY(err)) {
		PTYWATCH_FATAL("Setup() failed");
		return err;
	}

	err = Loop(&ptywatch);
	if (UNLIKELY(err)) {
		PTYWATCH_ERROR("Loop() failed");
	}

	Fini(&ptywatch);

	return 0;
}

