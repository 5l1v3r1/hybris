/*
 * This file is part of the Hybris programming language interpreter.
 *
 * Copyleft of Simone Margaritelli aka evilsocket <evilsocket@gmail.com>
 *
 * Hybris is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Hybris is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Hybris.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <hybris.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>

HYBRIS_DEFINE_FUNCTION(hserial_open);
HYBRIS_DEFINE_FUNCTION(hserial_fcntl);
HYBRIS_DEFINE_FUNCTION(hserial_get_attr);
HYBRIS_DEFINE_FUNCTION(hserial_get_ispeed);
HYBRIS_DEFINE_FUNCTION(hserial_get_ospeed);
HYBRIS_DEFINE_FUNCTION(hserial_close);

HYBRIS_EXPORTED_FUNCTIONS() {
	{ "serial_open",       hserial_open },
	{ "serial_fcntl",      hserial_fcntl },
	{ "serial_get_attr",   hserial_get_attr },
	{ "serial_get_ispeed", hserial_get_ispeed },
	{ "serial_get_ospeed", hserial_get_ospeed },
	{ "serial_close",	   hserial_close },
	{ "", NULL }
};

static Object *__termios_type = H_UNDEFINED;

extern "C" void hybris_module_init( vm_t * vm ){
	/*
	 * Create and initialize termios structure.
	 */
    char *termios_attributes[] = { "c_iflag",  /* input mode flags */
								   "c_oflag",  /* output mode flags */
								   "c_cflag",  /* control mode flags */
								   "c_lflag",  /* local mode flags */
								   "c_line",   /* line discipline */
								   "c_cc",     /* [NCCS] control characters */
								   "c_ispeed", /* input speed */
								   "c_ospeed", /* output speed */
								 };

    __termios_type = HYBRIS_DEFINE_STRUCTURE( vm, "termios", 8, termios_attributes );

    Object *termios_c_cc   = (Object *)new BinaryObject(),
		   *termios_c_line = (Object *)new CharObject(0x00);

    for( int i = 0; i < NCCS; ++i ){
    	ob_cl_push_reference( termios_c_cc, (Object *)new CharObject(0x00) );
    }

    ob_set_attribute_reference( (Object *)__termios_type, "c_cc",   termios_c_cc );
    ob_set_attribute_reference( (Object *)__termios_type, "c_line", termios_c_line );

    /* open/fcntl - O_SYNC is only implemented on blocks devices and
     * on files located on an ext2 file system.
     */
    HYBRIS_DEFINE_CONSTANT( vm, "O_ACCMODE", gc_new_integer(O_ACCMODE) );
    HYBRIS_DEFINE_CONSTANT( vm, "O_RDONLY", gc_new_integer(O_RDONLY) );
    HYBRIS_DEFINE_CONSTANT( vm, "O_WRONLY", gc_new_integer(O_WRONLY) );
    HYBRIS_DEFINE_CONSTANT( vm, "O_RDWR", gc_new_integer(O_RDWR) );
    HYBRIS_DEFINE_CONSTANT( vm, "O_CREAT", gc_new_integer(O_CREAT) );
    HYBRIS_DEFINE_CONSTANT( vm, "O_EXCL", gc_new_integer(O_EXCL) );
    HYBRIS_DEFINE_CONSTANT( vm, "O_NOCTTY", gc_new_integer(O_NOCTTY) );
    HYBRIS_DEFINE_CONSTANT( vm, "O_TRUNC", gc_new_integer(O_TRUNC) );
    HYBRIS_DEFINE_CONSTANT( vm, "O_APPEND", gc_new_integer(O_APPEND) );
    HYBRIS_DEFINE_CONSTANT( vm, "O_NONBLOCK", gc_new_integer(O_NONBLOCK) );
    HYBRIS_DEFINE_CONSTANT( vm, "O_NDELAY", gc_new_integer(O_NDELAY) );
    HYBRIS_DEFINE_CONSTANT( vm, "O_SYNC", gc_new_integer(O_SYNC) );
    HYBRIS_DEFINE_CONSTANT( vm, "O_FSYNC", gc_new_integer(O_FSYNC) );
    HYBRIS_DEFINE_CONSTANT( vm, "O_ASYNC", gc_new_integer(O_ASYNC) );
    #ifdef __USE_GNU
    HYBRIS_DEFINE_CONSTANT( vm, "O_DIRECT", gc_new_integer(O_DIRECT) );
    HYBRIS_DEFINE_CONSTANT( vm, "O_DIRECTORY", gc_new_integer(O_DIRECTORY) );
    HYBRIS_DEFINE_CONSTANT( vm, "O_NOFOLLOW", gc_new_integer(O_NOFOLLOW) );
    HYBRIS_DEFINE_CONSTANT( vm, "O_NOATIME", gc_new_integer(O_NOATIME) );
    HYBRIS_DEFINE_CONSTANT( vm, "O_CLOEXEC", gc_new_integer(O_CLOEXEC) );
    #endif
    /* For now Linux has synchronisity options for data and read operations.
     * We define the symbols here but let them do the same as O_SYNC since
     * this is a superset.
     */
    #if defined __USE_POSIX199309 || defined __USE_UNIX98
    HYBRIS_DEFINE_CONSTANT( vm, "O_DSYNC", gc_new_integer(O_DSYNC) );
    HYBRIS_DEFINE_CONSTANT( vm, "O_RSYNC", gc_new_integer(O_RSYNC) );
    #endif
    #ifdef __USE_LARGEFILE64
    # if __WORDSIZE == 64
    HYBRIS_DEFINE_CONSTANT( vm, "O_LARGEFILE", gc_new_integer(O_LARGEFILE) );
    # else
    HYBRIS_DEFINE_CONSTANT( vm, "O_LARGEFILE", gc_new_integer(O_LARGEFILE) );
    # endif
    #endif
    /* Values for the second argument to `fcntl'.  */
    HYBRIS_DEFINE_CONSTANT( vm, "F_DUPFD", gc_new_integer(F_DUPFD) );
    HYBRIS_DEFINE_CONSTANT( vm, "F_GETFD", gc_new_integer(F_GETFD) );
    HYBRIS_DEFINE_CONSTANT( vm, "F_SETFD", gc_new_integer(F_SETFD) );
    HYBRIS_DEFINE_CONSTANT( vm, "F_GETFL", gc_new_integer(F_GETFL) );
    HYBRIS_DEFINE_CONSTANT( vm, "F_SETFL", gc_new_integer(F_SETFL) );
    #if __WORDSIZE == 64
    HYBRIS_DEFINE_CONSTANT( vm, "F_GETLK", gc_new_integer(F_GETLK) );
    HYBRIS_DEFINE_CONSTANT( vm, "F_SETLK", gc_new_integer(F_SETLK) );
    HYBRIS_DEFINE_CONSTANT( vm, "F_SETLKW", gc_new_integer(F_SETLKW) );
    /* Not necessary, we always have 64-bit offsets.  */
    HYBRIS_DEFINE_CONSTANT( vm, "F_GETLK64", gc_new_integer(F_GETLK64) );
    HYBRIS_DEFINE_CONSTANT( vm, "F_SETLK64", gc_new_integer(F_SETLK64) );
    HYBRIS_DEFINE_CONSTANT( vm, "F_SETLKW64", gc_new_integer(F_SETLKW64) );
    #else
    # ifndef __USE_FILE_OFFSET64
    HYBRIS_DEFINE_CONSTANT( vm, "F_GETLK", gc_new_integer(F_GETLK) );
    HYBRIS_DEFINE_CONSTANT( vm, "F_SETLK", gc_new_integer(F_SETLK) );
    HYBRIS_DEFINE_CONSTANT( vm, "F_SETLKW", gc_new_integer(F_SETLKW) );
    # else
    HYBRIS_DEFINE_CONSTANT( vm, "F_GETLK", gc_new_integer(F_GETLK) );
    HYBRIS_DEFINE_CONSTANT( vm, "F_SETLK", gc_new_integer(F_SETLK) );
    HYBRIS_DEFINE_CONSTANT( vm, "F_SETLKW", gc_new_integer(F_SETLKW) );
    # endif
    HYBRIS_DEFINE_CONSTANT( vm, "F_GETLK64", gc_new_integer(F_GETLK64) );
    HYBRIS_DEFINE_CONSTANT( vm, "F_SETLK64", gc_new_integer(F_SETLK64) );
    HYBRIS_DEFINE_CONSTANT( vm, "F_SETLKW64", gc_new_integer(F_SETLKW64) );
    #endif
    #if defined __USE_BSD || defined __USE_UNIX98
    HYBRIS_DEFINE_CONSTANT( vm, "F_SETOWN", gc_new_integer(F_SETOWN) );
    HYBRIS_DEFINE_CONSTANT( vm, "F_GETOWN", gc_new_integer(F_GETOWN) );
    #endif
    #ifdef __USE_GNU
    HYBRIS_DEFINE_CONSTANT( vm, "F_SETSIG", gc_new_integer(F_SETSIG) );
    HYBRIS_DEFINE_CONSTANT( vm, "F_GETSIG", gc_new_integer(F_GETSIG) );
    HYBRIS_DEFINE_CONSTANT( vm, "F_SETOWN_EX", gc_new_integer(F_SETOWN_EX) );
    HYBRIS_DEFINE_CONSTANT( vm, "F_GETOWN_EX", gc_new_integer(F_GETOWN_EX) );
    #endif
    #ifdef __USE_GNU
    HYBRIS_DEFINE_CONSTANT( vm, "F_SETLEASE", gc_new_integer(F_SETLEASE) );
    HYBRIS_DEFINE_CONSTANT( vm, "F_GETLEASE", gc_new_integer(F_GETLEASE) );
    HYBRIS_DEFINE_CONSTANT( vm, "F_NOTIFY", gc_new_integer(F_NOTIFY) );
    /* Duplicate file descriptor with close-on-exit set.  */
    HYBRIS_DEFINE_CONSTANT( vm, "F_DUPFD_CLOEXEC", gc_new_integer(F_DUPFD_CLOEXEC) );
    #endif
    /* sizeof(termios.c_cc)*/
    HYBRIS_DEFINE_CONSTANT( vm, "NCCS", gc_new_integer(NCCS) );
	/* c_cc characters */
	HYBRIS_DEFINE_CONSTANT( vm, "VINTR", gc_new_integer(VINTR) );
	HYBRIS_DEFINE_CONSTANT( vm, "VQUIT", gc_new_integer(VQUIT) );
	HYBRIS_DEFINE_CONSTANT( vm, "VERASE", gc_new_integer(VERASE) );
	HYBRIS_DEFINE_CONSTANT( vm, "VKILL", gc_new_integer(VKILL) );
	HYBRIS_DEFINE_CONSTANT( vm, "VEOF", gc_new_integer(VEOF) );
	HYBRIS_DEFINE_CONSTANT( vm, "VTIME", gc_new_integer(VTIME) );
	HYBRIS_DEFINE_CONSTANT( vm, "VMIN", gc_new_integer(VMIN) );
	HYBRIS_DEFINE_CONSTANT( vm, "VSWTC", gc_new_integer(VSWTC) );
	HYBRIS_DEFINE_CONSTANT( vm, "VSTART", gc_new_integer(VSTART) );
	HYBRIS_DEFINE_CONSTANT( vm, "VSTOP", gc_new_integer(VSTOP) );
	HYBRIS_DEFINE_CONSTANT( vm, "VSUSP", gc_new_integer(VSUSP) );
	HYBRIS_DEFINE_CONSTANT( vm, "VEOL", gc_new_integer(VEOL) );
	HYBRIS_DEFINE_CONSTANT( vm, "VREPRINT", gc_new_integer(VREPRINT) );
	HYBRIS_DEFINE_CONSTANT( vm, "VDISCARD", gc_new_integer(VDISCARD) );
	HYBRIS_DEFINE_CONSTANT( vm, "VWERASE", gc_new_integer(VWERASE) );
	HYBRIS_DEFINE_CONSTANT( vm, "VLNEXT", gc_new_integer(VLNEXT) );
	HYBRIS_DEFINE_CONSTANT( vm, "VEOL2", gc_new_integer(VEOL2) );
	/* c_iflag bits */
	HYBRIS_DEFINE_CONSTANT( vm, "IGNBRK", gc_new_integer(IGNBRK) );
	HYBRIS_DEFINE_CONSTANT( vm, "BRKINT", gc_new_integer(BRKINT) );
	HYBRIS_DEFINE_CONSTANT( vm, "IGNPAR", gc_new_integer(IGNPAR) );
	HYBRIS_DEFINE_CONSTANT( vm, "PARMRK", gc_new_integer(PARMRK) );
	HYBRIS_DEFINE_CONSTANT( vm, "INPCK", gc_new_integer(INPCK) );
	HYBRIS_DEFINE_CONSTANT( vm, "ISTRIP", gc_new_integer(ISTRIP) );
	HYBRIS_DEFINE_CONSTANT( vm, "INLCR", gc_new_integer(INLCR) );
	HYBRIS_DEFINE_CONSTANT( vm, "IGNCR", gc_new_integer(IGNCR) );
	HYBRIS_DEFINE_CONSTANT( vm, "ICRNL", gc_new_integer(ICRNL) );
	HYBRIS_DEFINE_CONSTANT( vm, "IUCLC", gc_new_integer(IUCLC) );
	HYBRIS_DEFINE_CONSTANT( vm, "IXON", gc_new_integer(IXON) );
	HYBRIS_DEFINE_CONSTANT( vm, "IXANY", gc_new_integer(IXANY) );
	HYBRIS_DEFINE_CONSTANT( vm, "IXOFF", gc_new_integer(IXOFF) );
	HYBRIS_DEFINE_CONSTANT( vm, "IMAXBEL", gc_new_integer(IMAXBEL) );
	HYBRIS_DEFINE_CONSTANT( vm, "IUTF8", gc_new_integer(IUTF8) );
	/* c_oflag bits */
	HYBRIS_DEFINE_CONSTANT( vm, "OPOST", gc_new_integer(OPOST) );
	HYBRIS_DEFINE_CONSTANT( vm, "OLCUC", gc_new_integer(OLCUC) );
	HYBRIS_DEFINE_CONSTANT( vm, "ONLCR", gc_new_integer(ONLCR) );
	HYBRIS_DEFINE_CONSTANT( vm, "OCRNL", gc_new_integer(OCRNL) );
	HYBRIS_DEFINE_CONSTANT( vm, "ONOCR", gc_new_integer(ONOCR) );
	HYBRIS_DEFINE_CONSTANT( vm, "ONLRET", gc_new_integer(ONLRET) );
	HYBRIS_DEFINE_CONSTANT( vm, "OFILL", gc_new_integer(OFILL) );
	HYBRIS_DEFINE_CONSTANT( vm, "OFDEL", gc_new_integer(OFDEL) );
	HYBRIS_DEFINE_CONSTANT( vm, "NLDLY", gc_new_integer(NLDLY) );
	HYBRIS_DEFINE_CONSTANT( vm, "NL0", gc_new_integer(NL0) );
	HYBRIS_DEFINE_CONSTANT( vm, "NL1", gc_new_integer(NL1) );
	HYBRIS_DEFINE_CONSTANT( vm, "CRDLY", gc_new_integer(CRDLY) );
	HYBRIS_DEFINE_CONSTANT( vm, "CR0", gc_new_integer(CR0) );
	HYBRIS_DEFINE_CONSTANT( vm, "CR1", gc_new_integer(CR1) );
	HYBRIS_DEFINE_CONSTANT( vm, "CR2", gc_new_integer(CR2) );
	HYBRIS_DEFINE_CONSTANT( vm, "CR3", gc_new_integer(CR3) );
	HYBRIS_DEFINE_CONSTANT( vm, "TABDLY", gc_new_integer(TABDLY) );
	HYBRIS_DEFINE_CONSTANT( vm, "TAB0", gc_new_integer(TAB0) );
	HYBRIS_DEFINE_CONSTANT( vm, "TAB1", gc_new_integer(TAB1) );
	HYBRIS_DEFINE_CONSTANT( vm, "TAB2", gc_new_integer(TAB2) );
	HYBRIS_DEFINE_CONSTANT( vm, "TAB3", gc_new_integer(TAB3) );
	HYBRIS_DEFINE_CONSTANT( vm, "BSDLY", gc_new_integer(BSDLY) );
	HYBRIS_DEFINE_CONSTANT( vm, "BS0", gc_new_integer(BS0) );
	HYBRIS_DEFINE_CONSTANT( vm, "BS1", gc_new_integer(BS1) );
	HYBRIS_DEFINE_CONSTANT( vm, "FFDLY", gc_new_integer(FFDLY) );
	HYBRIS_DEFINE_CONSTANT( vm, "FF0", gc_new_integer(FF0) );
	HYBRIS_DEFINE_CONSTANT( vm, "FF1", gc_new_integer(FF1) );
	HYBRIS_DEFINE_CONSTANT( vm, "VTDLY", gc_new_integer(VTDLY) );
	HYBRIS_DEFINE_CONSTANT( vm, "VT0", gc_new_integer(VT0) );
	HYBRIS_DEFINE_CONSTANT( vm, "VT1", gc_new_integer(VT1) );
	HYBRIS_DEFINE_CONSTANT( vm, "XTABS", gc_new_integer(XTABS) );
	/* c_cflag bit meaning */
	HYBRIS_DEFINE_CONSTANT( vm, "CBAUD", gc_new_integer(CBAUD) );
	HYBRIS_DEFINE_CONSTANT( vm, "B0", gc_new_integer(B0) );
	HYBRIS_DEFINE_CONSTANT( vm, "B50", gc_new_integer(B50) );
	HYBRIS_DEFINE_CONSTANT( vm, "B75", gc_new_integer(B75) );
	HYBRIS_DEFINE_CONSTANT( vm, "B110", gc_new_integer(B110) );
	HYBRIS_DEFINE_CONSTANT( vm, "B134", gc_new_integer(B134) );
	HYBRIS_DEFINE_CONSTANT( vm, "B150", gc_new_integer(B150) );
	HYBRIS_DEFINE_CONSTANT( vm, "B200", gc_new_integer(B200) );
	HYBRIS_DEFINE_CONSTANT( vm, "B300", gc_new_integer(B300) );
	HYBRIS_DEFINE_CONSTANT( vm, "B600", gc_new_integer(B600) );
	HYBRIS_DEFINE_CONSTANT( vm, "B1200", gc_new_integer(B1200) );
	HYBRIS_DEFINE_CONSTANT( vm, "B1800", gc_new_integer(B1800) );
	HYBRIS_DEFINE_CONSTANT( vm, "B2400", gc_new_integer(B2400) );
	HYBRIS_DEFINE_CONSTANT( vm, "B4800", gc_new_integer(B4800) );
	HYBRIS_DEFINE_CONSTANT( vm, "B9600", gc_new_integer(B9600) );
	HYBRIS_DEFINE_CONSTANT( vm, "B19200", gc_new_integer(B19200) );
	HYBRIS_DEFINE_CONSTANT( vm, "B38400", gc_new_integer(B38400) );
	HYBRIS_DEFINE_CONSTANT( vm, "EXTA", gc_new_integer(EXTA) );
	HYBRIS_DEFINE_CONSTANT( vm, "EXTB", gc_new_integer(EXTB) );
	HYBRIS_DEFINE_CONSTANT( vm, "CSIZE", gc_new_integer(CSIZE) );
	HYBRIS_DEFINE_CONSTANT( vm, "CS5", gc_new_integer(CS5) );
	HYBRIS_DEFINE_CONSTANT( vm, "CS6", gc_new_integer(CS6) );
	HYBRIS_DEFINE_CONSTANT( vm, "CS7", gc_new_integer(CS7) );
	HYBRIS_DEFINE_CONSTANT( vm, "CS8", gc_new_integer(CS8) );
	HYBRIS_DEFINE_CONSTANT( vm, "CSTOPB", gc_new_integer(CSTOPB) );
	HYBRIS_DEFINE_CONSTANT( vm, "CREAD", gc_new_integer(CREAD) );
	HYBRIS_DEFINE_CONSTANT( vm, "PARENB", gc_new_integer(PARENB) );
	HYBRIS_DEFINE_CONSTANT( vm, "PARODD", gc_new_integer(PARODD) );
	HYBRIS_DEFINE_CONSTANT( vm, "HUPCL", gc_new_integer(HUPCL) );
	HYBRIS_DEFINE_CONSTANT( vm, "CLOCAL", gc_new_integer(CLOCAL) );
	HYBRIS_DEFINE_CONSTANT( vm, "CBAUDEX", gc_new_integer(CBAUDEX) );
	HYBRIS_DEFINE_CONSTANT( vm, "B57600", gc_new_integer(B57600) );
	HYBRIS_DEFINE_CONSTANT( vm, "B115200", gc_new_integer(B115200) );
	HYBRIS_DEFINE_CONSTANT( vm, "B230400", gc_new_integer(B230400) );
	HYBRIS_DEFINE_CONSTANT( vm, "B460800", gc_new_integer(B460800) );
	HYBRIS_DEFINE_CONSTANT( vm, "B500000", gc_new_integer(B500000) );
	HYBRIS_DEFINE_CONSTANT( vm, "B576000", gc_new_integer(B576000) );
	HYBRIS_DEFINE_CONSTANT( vm, "B921600", gc_new_integer(B921600) );
	HYBRIS_DEFINE_CONSTANT( vm, "B1000000", gc_new_integer(B1000000) );
	HYBRIS_DEFINE_CONSTANT( vm, "B1152000", gc_new_integer(B1152000) );
	HYBRIS_DEFINE_CONSTANT( vm, "B1500000", gc_new_integer(B1500000) );
	HYBRIS_DEFINE_CONSTANT( vm, "B2000000", gc_new_integer(B2000000) );
	HYBRIS_DEFINE_CONSTANT( vm, "B2500000", gc_new_integer(B2500000) );
	HYBRIS_DEFINE_CONSTANT( vm, "B3000000", gc_new_integer(B3000000) );
	HYBRIS_DEFINE_CONSTANT( vm, "B3500000", gc_new_integer(B3500000) );
	HYBRIS_DEFINE_CONSTANT( vm, "B4000000", gc_new_integer(B4000000) );
	HYBRIS_DEFINE_CONSTANT( vm, "__MAX_BAUD", gc_new_integer(__MAX_BAUD) );
	HYBRIS_DEFINE_CONSTANT( vm, "CIBAUD", gc_new_integer(CIBAUD) );
	HYBRIS_DEFINE_CONSTANT( vm, "CMSPAR", gc_new_integer(CMSPAR) );
	HYBRIS_DEFINE_CONSTANT( vm, "CRTSCTS", gc_new_integer(CRTSCTS) );
	/* c_lflag bits */
	HYBRIS_DEFINE_CONSTANT( vm, "ISIG", gc_new_integer(ISIG) );
	HYBRIS_DEFINE_CONSTANT( vm, "ICANON", gc_new_integer(ICANON) );
	HYBRIS_DEFINE_CONSTANT( vm, "XCASE", gc_new_integer(XCASE) );
	HYBRIS_DEFINE_CONSTANT( vm, "ECHO", gc_new_integer(ECHO) );
	HYBRIS_DEFINE_CONSTANT( vm, "ECHOE", gc_new_integer(ECHOE) );
	HYBRIS_DEFINE_CONSTANT( vm, "ECHOK", gc_new_integer(ECHOK) );
	HYBRIS_DEFINE_CONSTANT( vm, "ECHONL", gc_new_integer(ECHONL) );
	HYBRIS_DEFINE_CONSTANT( vm, "NOFLSH", gc_new_integer(NOFLSH) );
	HYBRIS_DEFINE_CONSTANT( vm, "TOSTOP", gc_new_integer(TOSTOP) );
	HYBRIS_DEFINE_CONSTANT( vm, "ECHOCTL", gc_new_integer(ECHOCTL) );
	HYBRIS_DEFINE_CONSTANT( vm, "ECHOPRT", gc_new_integer(ECHOPRT) );
	HYBRIS_DEFINE_CONSTANT( vm, "ECHOKE", gc_new_integer(ECHOKE) );
	HYBRIS_DEFINE_CONSTANT( vm, "FLUSHO", gc_new_integer(FLUSHO) );
	HYBRIS_DEFINE_CONSTANT( vm, "PENDIN", gc_new_integer(PENDIN) );
	HYBRIS_DEFINE_CONSTANT( vm, "IEXTEN", gc_new_integer(IEXTEN) );
	/* tcflow() and TCXONC use these */
	HYBRIS_DEFINE_CONSTANT( vm, "TCOOFF", gc_new_integer(TCOOFF) );
	HYBRIS_DEFINE_CONSTANT( vm, "TCOON",  gc_new_integer(TCOON) );
	HYBRIS_DEFINE_CONSTANT( vm, "TCIOFF", gc_new_integer(TCIOFF) );
	HYBRIS_DEFINE_CONSTANT( vm, "TCION",  gc_new_integer(TCION) );
	/* tcflush() and TCFLSH use these */
	HYBRIS_DEFINE_CONSTANT( vm, "TCIFLUSH",  gc_new_integer(TCIFLUSH) );
	HYBRIS_DEFINE_CONSTANT( vm, "TCOFLUSH",  gc_new_integer(TCOFLUSH) );
	HYBRIS_DEFINE_CONSTANT( vm, "TCIOFLUSH", gc_new_integer(TCIOFLUSH) );
	/* tcsetattr uses these */
	HYBRIS_DEFINE_CONSTANT( vm, "TCSANOW",   gc_new_integer(TCSANOW) );
	HYBRIS_DEFINE_CONSTANT( vm, "TCSADRAIN", gc_new_integer(TCSADRAIN) );
	HYBRIS_DEFINE_CONSTANT( vm, "TCSAFLUSH", gc_new_integer(TCSAFLUSH) );
}

HYBRIS_DEFINE_FUNCTION(hserial_open){
	if( ob_argc() != 2 ){
		hyb_error( H_ET_SYNTAX, "function 'serial_open' requires 2 parameters (called with %d)", ob_argc() );
	}
	ob_argv_type_assert( 0, otString,  "serial_open" );
	ob_argv_type_assert( 1, otInteger, "serial_open" );

	const char *dev  = string_argv(0).c_str();
	size_t		mode = int_argv(1);

	return (Object *)gc_new_integer( open( dev, mode ) );
}

HYBRIS_DEFINE_FUNCTION(hserial_fcntl){
	if( ob_argc() < 2 ){
		hyb_error( H_ET_SYNTAX, "function 'serial_fcntl' requires at least 2 parameters (called with %d)", ob_argc() );
	}
	ob_argv_type_assert( 0, otInteger, "serial_fcntl" );
	ob_argv_type_assert( 1, otInteger, "serial_fcntl" );

	int fd   = int_argv(0),
		cmd  = int_argv(1),
		cmd2 = 0;

	if( ob_argc() >= 3 ){
		ob_argv_type_assert( 2, otInteger, "serial_fcntl" );
		cmd2 = int_argv(2);
	}

	return (Object *)gc_new_integer( fcntl( fd, cmd, cmd2 ) );
}

HYBRIS_DEFINE_FUNCTION(hserial_get_attr){
	if( ob_argc() != 1 ){
		hyb_error( H_ET_SYNTAX, "function 'serial_get_attr' requires 1 parameter (called with %d)", ob_argc() );
	}
	ob_argv_type_assert( 0, otInteger, "serial_get_attr" );

	int fd = int_argv(0);
	struct termios attributes;

	if( tcgetattr( fd, &attributes ) == 0 ){
		Object *h_termios = ob_clone(__termios_type);

		ob_set_attribute_reference( h_termios, "c_iflag",  (Object *)gc_new_integer(attributes.c_iflag) );
		ob_set_attribute_reference( h_termios, "c_oflag",  (Object *)gc_new_integer(attributes.c_oflag) );
		ob_set_attribute_reference( h_termios, "c_cflag",  (Object *)gc_new_integer(attributes.c_cflag) );
		ob_set_attribute_reference( h_termios, "c_lflag",  (Object *)gc_new_integer(attributes.c_lflag) );
		ob_set_attribute_reference( h_termios, "c_line",   (Object *)gc_new_char(attributes.c_line) );
		ob_set_attribute_reference( h_termios, "c_ispeed", (Object *)gc_new_integer(attributes.c_ispeed) );
		ob_set_attribute_reference( h_termios, "c_ospeed", (Object *)gc_new_integer(attributes.c_ospeed) );

		Object *termios_c_cc = ob_get_attribute( h_termios, "c_cc" );
		for( int i = 0; i < NCCS; ++i ){
			ob_cl_push_reference( termios_c_cc, (Object *)new CharObject( attributes.c_cc[i] ) );
		}

		return h_termios;
	}
	else{
		return (Object *)gc_new_boolean(false);
	}
}

HYBRIS_DEFINE_FUNCTION(hserial_get_ispeed){
	if( ob_argc() != 1 ){
		hyb_error( H_ET_SYNTAX, "function 'serial_get_ispeed' requires 1 parameter (called with %d)", ob_argc() );
	}
	ob_argv_type_assert( 0, otInteger, "serial_get_ispeed" );

	int fd = int_argv(0);
	struct termios attributes;

	if( tcgetattr( fd, &attributes ) == 0 ){
		return (Object *)gc_new_integer( cfgetispeed(&attributes) );
	}
	else{
		return (Object *)gc_new_boolean(false);
	}
}

HYBRIS_DEFINE_FUNCTION(hserial_get_ospeed){
	if( ob_argc() != 1 ){
		hyb_error( H_ET_SYNTAX, "function 'serial_get_ospeed' requires 1 parameter (called with %d)", ob_argc() );
	}
	ob_argv_type_assert( 0, otInteger, "serial_get_ospeed" );

	int fd = int_argv(0);
	struct termios attributes;

	if( tcgetattr( fd, &attributes ) == 0 ){
		return (Object *)gc_new_integer( cfgetospeed(&attributes) );
	}
	else{
		return (Object *)gc_new_boolean(false);
	}
}

HYBRIS_DEFINE_FUNCTION(hserial_close){
	if( ob_argc() != 1 ){
		hyb_error( H_ET_SYNTAX, "function 'serial_close' requires 1 parameter (called with %d)", ob_argc() );
	}
	ob_argv_type_assert( 0, otInteger, "serial_close" );

	int fd = int_argv(0);

	return (Object *)gc_new_integer( close(fd) );
}
