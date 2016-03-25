
#ifndef PTYWATCH_CONFIG_H_INCLUDED
#define PTYWATCH_CONFIG_H_INCLUDED 1

/* Initial capacity of the message buffer. Must be a positive number.
 */
#undef  PTYWATCH_CONFIG_INITIAL_MSGCAP
#define PTYWATCH_CONFIG_INITIAL_MSGCAP	1

/* In order to distinguish individual
 */
#undef	PTYWATCH_CONFIG_TIMEDIFF_SEC
#undef	PTYWATCH_CONFIG_TIMEDIFF_USEC
#define	PTYWATCH_CONFIG_TIMEDIFF_SEC	0
#define	PTYWATCH_CONFIG_TIMEDIFF_USEC	(500*1000)

#endif

