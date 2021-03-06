/* PRINTER.C    (c) Copyright Roger Bowler, 1999-2009                */
/*              ESA/390 Line Printer Device Handler                  */

// $Id: printer.c 5487 2009-10-14 04:43:58Z fish $

/*-------------------------------------------------------------------*/
/* This module contains device handling functions for emulated       */
/* System/370 line printer devices.                                  */
/*-------------------------------------------------------------------*/

#include "hstdinc.h"

#include "hercules.h"

#include "devtype.h"

#include "opcode.h"

/*-------------------------------------------------------------------*/
/* Ivan Warren 20040227                                              */
/* This table is used by channel.c to determine if a CCW code is an  */
/* immediate command or not                                          */
/* The tape is addressed in the DEVHND structure as 'DEVIMM immed'   */
/* 0 : Command is NOT an immediate command                           */
/* 1 : Command is an immediate command                               */
/* Note : An immediate command is defined as a command which returns */
/* CE (channel end) during initialisation (that is, no data is       */
/* actually transfered. In this case, IL is not indicated for a CCW  */
/* Format 0 or for a CCW Format 1 when IL Suppression Mode is in     */
/* effect                                                            */
/*-------------------------------------------------------------------*/

/* Printer Specific : 1403 */
/* The following are considered IMMEDIATE commands : */
/* CTL-NOOP, Skip Channel 'n' Immediate, Block Data check , Allow Data Check
 * Space 1,2,3 Lines Immediate, UCS Gate Load, Load UCS Buffer & Fold,
 * Load UCS Buffer (No Fold)
 */

static BYTE printer_immed_commands[256]=
/*
 *0 1 2 3 4 5 6 7 8 9 A B C D E F
*/

{ 0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,
  0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,
  0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,
  0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,
  0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,
  0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,
  0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,
  0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,
  0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0};

/*-------------------------------------------------------------------*/
/* Internal macro definitions                                        */
/*-------------------------------------------------------------------*/
#define LINE_LENGTH     150

static void* spthread (DEVBLK* dev);        /*  (forward reference)  */

/*-------------------------------------------------------------------*/
/* Sockdev "OnConnection" callback function                          */
/*-------------------------------------------------------------------*/
static int onconnect_callback (DEVBLK* dev)
{
    TID tid;
    if (create_thread( &tid, DETACHED, spthread, dev, NULL ))
    {
        logmsg( _( "HHCPR015E Create spthread failed for %4.4X: errno=%d: %s\n" ),
            dev->devnum, errno, strerror( errno ) );
        return 0;
    }
    return 1;
}

/*-------------------------------------------------------------------*/
/* Thread to monitor the sockdev remote print spooler connection     */
/*-------------------------------------------------------------------*/
static void* spthread (DEVBLK* dev)
{
    BYTE byte;
    fd_set readset, errorset;
    struct timeval tv;
    int rc, fd = dev->fd;           // (save original fd)

    /* Fix thread name */
    {
        char thread_name[32];
        thread_name[sizeof(thread_name)-1] = 0;
        snprintf( thread_name, sizeof(thread_name)-1,
            "spthread %4.4X", dev->devnum );
        SET_THREAD_NAME( thread_name );
    }

    // Looooop...  until shutdown or disconnect...

    // PROGRAMMING NOTE: we do our select specifying an immediate
    // timeout to prevent our select from holding up (slowing down)
    // the device thread (which does the actual writing of data to
    // the client). The only purpose for our thread even existing
    // is to detect a severed connection (i.e. to detect when the
    // client disconnects)...

    while ( !sysblk.shutdown && dev->fd == fd )
    {
        if (dev->busy)
        {
            SLEEP(3);
            continue;
        }

        FD_ZERO( &readset );
        FD_ZERO( &errorset );

        FD_SET( fd, &readset );
        FD_SET( fd, &errorset );

        tv.tv_sec = 0;
        tv.tv_usec = 0;

        rc = select( fd+1, &readset, NULL, &errorset, &tv );

        if (rc < 0)
            break;

        if (rc == 0)
        {
            SLEEP(3);
            continue;
        }

        if (FD_ISSET( fd, &errorset ))
            break;

        // Read and ignore any data they send us...
        // Note: recv should complete immediately
        // as we know data is waiting to be read.

        ASSERT( FD_ISSET( fd, &readset ) );

        rc = recv( fd, &byte, sizeof(byte), 0 );

        if (rc <= 0)
            break;
    }

    obtain_lock( &dev->lock );

    // PROGRAMMING NOTE: the following tells us whether we detected
    // the error or if the device thread already did. If the device
    // thread detected it while we were sleeping (and subsequently
    // closed the connection) then we don't need to do anything at
    // all; just exit. If we were the ones that detected the error
    // however, then we need to close the connection so the device
    // thread can learn of it...

    if (dev->fd == fd)
    {
        dev->fd = -1;
        close_socket( fd );
        logmsg (_("HHCPR016I %s (%s) disconnected from device %4.4X (%s)\n"),
            dev->bs->clientname, dev->bs->clientip, dev->devnum, dev->bs->spec);
    }

    release_lock( &dev->lock );

    return NULL;

} /* end function spthread */

/*-------------------------------------------------------------------*/
/* Subroutine to open the printer file or pipe                       */
/*-------------------------------------------------------------------*/
static int
open_printer (DEVBLK *dev)
{
pid_t           pid;                    /* Child process identifier  */
char            pathname[MAX_PATH];     /* file path in host format  */
int             open_flags;             /* File open flags           */
#if !defined( _MSVC_ )
int             pipefd[2];              /* Pipe descriptors          */
int             rc;                     /* Return code               */
#endif

    /* Regular open if 1st char of filename is not vertical bar */
    if (!dev->ispiped)
    {
        int fd;

        /* Socket printer? */
        if (dev->bs)
            return (dev->fd < 0 ? -1 : 0);

        /* Normal printer */
        hostpath(pathname, dev->filename, sizeof(pathname));
        open_flags = O_BINARY | O_WRONLY | O_CREAT /* | O_SYNC */;
        if (dev->notrunc != 1)
        {
            open_flags |= O_TRUNC;
        }
        fd = open (pathname, open_flags,
                    S_IRUSR | S_IWUSR | S_IRGRP);
        if (fd < 0)
        {
            logmsg (_("HHCPR004E Error opening file %s: %s\n"),
                    dev->filename, strerror(errno));
            return -1;
        }

        /* Save file descriptor in device block and return */
        dev->fd = fd;
        return 0;
    }

    /* Filename is in format |xxx, set up pipe to program xxx */

#if defined( _MSVC_ )

    /* "Poor man's" fork... */
    pid = w32_poor_mans_fork ( dev->filename+1, &dev->fd );
    if (pid < 0)
    {
        logmsg (_("HHCPR006E %4.4X device initialization error: fork: %s\n"),
                dev->devnum, strerror(errno));
        return -1;
    }

    /* Log start of child process */
    logmsg (_("HHCPR007I pipe receiver (pid=%d) starting for %4.4X\n"),
            pid, dev->devnum);
    dev->ptpcpid = pid;

#else /* !defined( _MSVC_ ) */

    /* Create a pipe */
    rc = create_pipe (pipefd);
    if (rc < 0)
    {
        logmsg (_("HHCPR005E %4.4X device initialization error: pipe: %s\n"),
                dev->devnum, strerror(errno));
        return -1;
    }

    /* Fork a child process to receive the pipe data */
    pid = fork();
    if (pid < 0)
    {
        logmsg (_("HHCPR006E %4.4X device initialization error: fork: %s\n"),
                dev->devnum, strerror(errno));
        close_pipe ( pipefd[0] );
        close_pipe ( pipefd[1] );
        return -1;
    }

    /* The child process executes the pipe receiver program... */
    if (pid == 0)
    {
        /* Log start of child process */
        logmsg (_("HHCPR007I pipe receiver (pid=%d) starting for %4.4X\n"),
                getpid(), dev->devnum);

        /* Close the write end of the pipe */
        close_pipe ( pipefd[1] );

        /* Duplicate the read end of the pipe onto STDIN */
        if (pipefd[0] != STDIN_FILENO)
        {
            rc = dup2 (pipefd[0], STDIN_FILENO);
            if (rc != STDIN_FILENO)
            {
                logmsg (_("HHCPR008E %4.4X dup2 error: %s\n"),
                        dev->devnum, strerror(errno));
                close_pipe ( pipefd[0] );
                _exit(127);
            }
        } /* end if(pipefd[0] != STDIN_FILENO) */

        /* Close the original descriptor now duplicated to STDIN */
        close_pipe ( pipefd[0] );

        /* Redirect stderr (screen) to hercules log task */
        dup2(STDOUT_FILENO, STDERR_FILENO);

        /* Relinquish any ROOT authority before calling shell */
        SETMODE(TERM);

        /* Execute the specified pipe receiver program */
        rc = system (dev->filename+1);

        if (rc == 0)
        {
            /* Log end of child process */
            logmsg (_("HHCPR011I pipe receiver (pid=%d) terminating for %4.4X\n"),
                    getpid(), dev->devnum);
        }
        else
        {
            /* Log error */
            logmsg (_("HHCPR012E %4.4X Unable to execute %s: %s\n"),
                    dev->devnum, dev->filename+1, strerror(errno));
        }

        /* The child process terminates using _exit instead of exit
           to avoid invoking the panel atexit cleanup routine */
        _exit(rc);

    } /* end if(pid==0) */

    /* The parent process continues as the pipe sender */

    /* Close the read end of the pipe */
    close_pipe ( pipefd[0] );

    /* Save pipe write descriptor in the device block */
    dev->fd = pipefd[1];
    dev->ptpcpid = pid;

#endif /* defined( _MSVC_ ) */

    return 0;

} /* end function open_printer */

/*-------------------------------------------------------------------*/
/* Subroutine to write data to the printer                           */
/*-------------------------------------------------------------------*/
static void
write_buffer (DEVBLK *dev, char *buf, int len, BYTE *unitstat)
{
int             rc;                     /* Return code               */

    /* Write data to the printer file */
    if (dev->bs)
    {
        /* (socket printer) */
        rc = write_socket (dev->fd, buf, len);

        /* Check for socket error */
        if (rc < len)
        {
            /* Close the connection */
            if (dev->fd != -1)
            {
                int fd = dev->fd;
                dev->fd = -1;
                close_socket( fd );
                logmsg (_("HHCPR017I %s (%s) disconnected from device %4.4X (%s)\n"),
                    dev->bs->clientname, dev->bs->clientip, dev->devnum, dev->bs->spec);
            }

            /* Set unit check with intervention required */
            dev->sense[0] = SENSE_IR;
            *unitstat = CSW_CE | CSW_DE | CSW_UC;
        }
    }
    else
    {
        /* Write data to the printer file */
        rc = write (dev->fd, buf, len);

        /* Equipment check if error writing to printer file */
        if (rc < len)
        {
            logmsg (_("HHCPR003E %4.4X Error writing to %s: %s\n"),
                    dev->devnum, dev->filename,
                    (errno == 0 ? _("incomplete"): strerror(errno)));
            dev->sense[0] = SENSE_EC;
            *unitstat = CSW_CE | CSW_DE | CSW_UC;
        }
    }

} /* end function write_buffer */

/*-------------------------------------------------------------------*/
/* Initialize the device handler                                     */
/*-------------------------------------------------------------------*/
static int printer_init_handler (DEVBLK *dev, int argc, char *argv[])
{
int     i;                              /* Array subscript           */
int     sockdev  = 0;                   /* 1 == is socket device     */

    /* Forcibly disconnect anyone already currently connected */
    if (dev->bs && !unbind_device_ex(dev,1))
        return -1; // (error msg already issued)

    /* The first argument is the file name */
    if (argc == 0 || strlen(argv[0]) > sizeof(dev->filename)-1)
    {
        logmsg (_("HHCPR001E File name missing or invalid for printer %4.4X\n"),
                 dev->devnum);
        return -1;
    }

    /* Save the file name in the device block */
    strncpy (dev->filename, argv[0], sizeof(dev->filename));

    if(!sscanf(dev->typname,"%hx",&(dev->devtype)))
        dev->devtype = 0x1403;

    /* Initialize device dependent fields */
    dev->fd = -1;
    dev->printpos = 0;
    dev->printrem = LINE_LENGTH;
    dev->diaggate = 0;
    dev->fold = 0;
    dev->crlf = 0;
    dev->stopprt = 0;
    dev->notrunc = 0;
    dev->ispiped = (dev->filename[0] == '|');

    /* Process the driver arguments */
    for (i = 1; i < argc; i++)
    {
        if (strcasecmp(argv[i], "crlf") == 0)
        {
            dev->crlf = 1;
            continue;
        }

        /* sockdev means the device file is actually
           a connected socket instead of a disk file.
           The file name is the socket_spec (host:port)
           to listen for connections on.
        */
        if (!dev->ispiped && strcasecmp(argv[i], "sockdev") == 0)
        {
            sockdev = 1;
            continue;
        }

        if (strcasecmp(argv[i], "noclear") == 0)
        {
            dev->notrunc = 1;
            continue;
        }

        logmsg (_("HHCPR002E Invalid argument for printer %4.4X: %s\n"),
                dev->devnum, argv[i]);
        return -1;
    }

    /* Check for incompatible options */
    if (sockdev && dev->crlf)
    {
        logmsg (_("HHCPR019E Incompatible option specified for socket printer %4.4X: 'crlf'\n"),
                dev->devnum);
        return -1;
    }

    if (sockdev && dev->notrunc)
    {
        logmsg (_("HHCPR019E Incompatible option specified for socket printer %4.4X: 'noclear'\n"),
                dev->devnum);
        return -1;
    }

    /* If socket device, create a listening socket
       to accept connections on.
    */
    if (sockdev && !bind_device_ex( dev,
        dev->filename, onconnect_callback, dev ))
    {
        return -1;  // (error msg already issued)
    }

    /* Set length of print buffer */
    dev->bufsize = LINE_LENGTH + 8;

    /* Set number of sense bytes */
    dev->numsense = 1;

    /* Initialize the device identifier bytes */
    dev->devid[0] = 0xFF;
    dev->devid[1] = 0x28; /* Control unit type is 2821-1 */
    dev->devid[2] = 0x21;
    dev->devid[3] = 0x01;
    dev->devid[4] = dev->devtype >> 8;
    dev->devid[5] = dev->devtype & 0xFF;
    dev->devid[6] = 0x01;
    dev->numdevid = 7;

    /* Activate I/O tracing */
//  dev->ccwtrace = 1;

    return 0;
} /* end function printer_init_handler */

/*-------------------------------------------------------------------*/
/* Query the device definition                                       */
/*-------------------------------------------------------------------*/
static void printer_query_device (DEVBLK *dev, char **class,
                int buflen, char *buffer)
{
    BEGIN_DEVICE_CLASS_QUERY( "PRT", dev, class, buflen, buffer );

    snprintf (buffer, buflen, "%s%s%s%s%s",
                 dev->filename,
                (dev->bs      ? " sockdev"   : ""),
                (dev->crlf    ? " crlf"      : ""),
                (dev->notrunc ? " noclear"   : ""),
                (dev->stopprt ? " (stopped)" : ""));

} /* end function printer_query_device */

/*-------------------------------------------------------------------*/
/* Close the device                                                  */
/*-------------------------------------------------------------------*/
static int printer_close_device ( DEVBLK *dev )
{
int fd = dev->fd;

    if (fd == -1)
        return 0;

    dev->fd      = -1;
    dev->stopprt =  0;

    /* Close the device file */
    if ( dev->ispiped )
    {
#if !defined( _MSVC_ )
        close_pipe (fd);
#else /* defined( _MSVC_ ) */
        close (fd);
        /* Log end of child process */
        logmsg (_("HHCPR011I pipe receiver (pid=%d) terminating for %4.4X\n"),
                dev->ptpcpid, dev->devnum);
#endif /* defined( _MSVC_ ) */
        dev->ptpcpid = 0;
    }
    else
    {
        if (dev->bs)
        {
            /* Socket printer */
            close_socket (fd);
            logmsg (_("HHCPR018I %s (%s) disconnected from device %4.4X (%s)\n"),
                dev->bs->clientname, dev->bs->clientip, dev->devnum, dev->bs->spec);
        }
        else
        {
            /* Regular printer */
            close (fd);
        }
    }

    return 0;
} /* end function printer_close_device */

/*-------------------------------------------------------------------*/
/* Execute a Channel Command Word                                    */
/*-------------------------------------------------------------------*/
static void printer_execute_ccw (DEVBLK *dev, BYTE code, BYTE flags,
        BYTE chained, U16 count, BYTE prevcode, int ccwseq,
        BYTE *iobuf, BYTE *more, BYTE *unitstat, U16 *residual)
{
int             rc = 0;                 /* Return code               */
int             i;                      /* Loop counter              */
int             num;                    /* Number of bytes to move   */
char           *eor;                    /* -> end of record string   */
BYTE            c;                      /* Print character           */

    /* Reset flags at start of CCW chain */
    if (chained == 0)
    {
        dev->diaggate = 0;
    }

    /* Open the device file if necessary */
    if (dev->fd < 0 && !IS_CCW_SENSE(code))
        rc = open_printer (dev);
    else
    {
        /* If printer stopped, return intervention required */
        if (dev->stopprt && !IS_CCW_SENSE(code))
            rc = -1;
        else
            rc = 0;
    }

    if (rc < 0)
    {
        /* Set unit check with intervention required */
        dev->sense[0] = SENSE_IR;
        *unitstat = CSW_CE | CSW_DE | CSW_UC;
        return;
    }

    /* Process depending on CCW opcode */
    switch (code) {

    case 0x01:
    /*---------------------------------------------------------------*/
    /* WRITE WITHOUT SPACING                                         */
    /*---------------------------------------------------------------*/
        eor = "\r";
        goto write;

    case 0x09:
    /*---------------------------------------------------------------*/
    /* WRITE AND SPACE 1 LINE                                        */
    /*---------------------------------------------------------------*/
        eor = dev->crlf ? "\r\n" : "\n";
        goto write;

    case 0x11:
    /*---------------------------------------------------------------*/
    /* WRITE AND SPACE 2 LINES                                       */
    /*---------------------------------------------------------------*/
        eor = dev->crlf ? "\r\n\n" : "\n\n";
        goto write;

    case 0x19:
    /*---------------------------------------------------------------*/
    /* WRITE AND SPACE 3 LINES                                       */
    /*---------------------------------------------------------------*/
        eor = dev->crlf ? "\r\n\n\n" : "\n\n\n";
        goto write;

    case 0x89:
    /*---------------------------------------------------------------*/
    /* WRITE AND SKIP TO CHANNEL 1                                   */
    /*---------------------------------------------------------------*/
        eor = dev->crlf ? "\r\f" : "\f";
        goto write;

    case 0xC9:
    /*---------------------------------------------------------------*/
    /* WRITE AND SKIP TO CHANNEL 9                                   */
    /*---------------------------------------------------------------*/
        eor = dev->crlf ? "\r\n" : "\n";
        goto write;

    case 0xE1:
    /*---------------------------------------------------------------*/
    /* WRITE AND SKIP TO CHANNEL 12                                  */
    /*---------------------------------------------------------------*/
        eor = dev->crlf ? "\r\n" : "\n";
        goto write;

    write:
        /* Start a new record if not data-chained from previous CCW */
        if ((chained & CCW_FLAGS_CD) == 0)
        {
            dev->printpos = 0;
            dev->printrem = LINE_LENGTH;

        } /* end if(!data-chained) */

        /* Calculate number of bytes to write and set residual count */
        num = (count < dev->printrem) ? count : dev->printrem;
        *residual = count - num;

        /* Copy data from channel buffer to print buffer */
        for (i = 0; i < num; i++)
        {
            c = guest_to_host(iobuf[i]);

            if (dev->fold) c = toupper(c);
            if (c == 0) c = SPACE;

            dev->buf[dev->printpos] = c;
            dev->printpos++;
            dev->printrem--;
        } /* end for(i) */

        /* Perform end of record processing if not data-chaining */
        if ((flags & CCW_FLAGS_CD) == 0)
        {
            /* Truncate trailing blanks from print line */
            for (i = dev->printpos; i > 0; i--)
                if (dev->buf[i-1] != SPACE) break;

            /* Append carriage return and line feed(s) */
            strcpy ((char *)(dev->buf + i), eor);
            i += strlen(eor);

            /* Write print line */
            write_buffer (dev, (char *)dev->buf, i, unitstat);
            if (*unitstat != 0) break;

        } /* end if(!data-chaining) */

        /* Return normal status */
        *unitstat = CSW_CE | CSW_DE;
        break;

    case 0x03:
    /*---------------------------------------------------------------*/
    /* CONTROL NO-OPERATION                                          */
    /*---------------------------------------------------------------*/
        *unitstat = CSW_CE | CSW_DE;
        break;

    case 0x06:
    /*---------------------------------------------------------------*/
    /* DIAGNOSTIC CHECK READ                                         */
    /*---------------------------------------------------------------*/
        /* If not 1403, reject if not preceded by DIAGNOSTIC GATE */
        if (dev->devtype != 0x1403 && dev->diaggate == 0)
        {
            dev->sense[0] = SENSE_CR;
            *unitstat = CSW_CE | CSW_DE | CSW_UC;
            break;
        }

        /* Return normal status */
        *unitstat = CSW_CE | CSW_DE;
        break;

    case 0x07:
    /*---------------------------------------------------------------*/
    /* DIAGNOSTIC GATE                                               */
    /*---------------------------------------------------------------*/
        /* Command reject if 1403, or if chained to another CCW
           except a no-operation at the start of the CCW chain */
        if (dev->devtype == 1403 || ccwseq > 1
            || (chained && prevcode != 0x03))
        {
            dev->sense[0] = SENSE_CR;
            *unitstat = CSW_CE | CSW_DE | CSW_UC;
            break;
        }

        /* Set diagnostic gate flag */
        dev->diaggate = 1;

        /* Return normal status */
        *unitstat = CSW_CE | CSW_DE;
        break;

    case 0x0A:
    /*---------------------------------------------------------------*/
    /* DIAGNOSTIC READ UCS BUFFER                                    */
    /*---------------------------------------------------------------*/
        /* Reject if 1403 or not preceded by DIAGNOSTIC GATE */
        if (dev->devtype == 0x1403 || dev->diaggate == 0)
        {
            dev->sense[0] = SENSE_CR;
            *unitstat = CSW_CE | CSW_DE | CSW_UC;
            break;
        }

        /* Return normal status */
        *unitstat = CSW_CE | CSW_DE;
        break;

    case 0x12:
    /*---------------------------------------------------------------*/
    /* DIAGNOSTIC READ FCB                                           */
    /*---------------------------------------------------------------*/
        /* Reject if 1403 or not preceded by DIAGNOSTIC GATE */
        if (dev->devtype == 0x1403 || dev->diaggate == 0)
        {
            dev->sense[0] = SENSE_CR;
            *unitstat = CSW_CE | CSW_DE | CSW_UC;
            break;
        }

        /* Return normal status */
        *unitstat = CSW_CE | CSW_DE;
        break;

    case 0x0B:
    /*---------------------------------------------------------------*/
    /* SPACE 1 LINE IMMEDIATE                                        */
    /*---------------------------------------------------------------*/
        eor = dev->crlf ? "\r\n" : "\n";
        write_buffer (dev, eor, strlen(eor), unitstat);
        if (*unitstat != 0) break;

    /*
        *residual = 0;
    */
        *unitstat = CSW_CE | CSW_DE;
        break;

    case 0x13:
    /*---------------------------------------------------------------*/
    /* SPACE 2 LINES IMMEDIATE                                       */
    /*---------------------------------------------------------------*/
        eor = dev->crlf ? "\r\n\n" : "\n\n";
        write_buffer (dev, eor, strlen(eor), unitstat);
        if (*unitstat != 0) break;

    /*
        *residual = 0;
    */
        *unitstat = CSW_CE | CSW_DE;
        break;

    case 0x1B:
    /*---------------------------------------------------------------*/
    /* SPACE 3 LINES IMMEDIATE                                       */
    /*---------------------------------------------------------------*/
        eor = dev->crlf ? "\r\n\n\n" : "\n\n\n";
        write_buffer (dev, eor, strlen(eor), unitstat);
        if (*unitstat != 0) break;

    /*
        *residual = 0;
    */
        *unitstat = CSW_CE | CSW_DE;
        break;

    case 0x23:
    /*---------------------------------------------------------------*/
    /* UNFOLD                                                        */
    /*---------------------------------------------------------------*/
        dev->fold = 0;
        *unitstat = CSW_CE | CSW_DE;
        break;

    case 0x43:
    /*---------------------------------------------------------------*/
    /* FOLD                                                          */
    /*---------------------------------------------------------------*/
        dev->fold = 1;
        *unitstat = CSW_CE | CSW_DE;
        break;

    case 0x73:
    /*---------------------------------------------------------------*/
    /* BLOCK DATA CHECK                                              */
    /*---------------------------------------------------------------*/
    /*
        *residual = 0;
    */
        *unitstat = CSW_CE | CSW_DE;
        break;

    case 0x7B:
    /*---------------------------------------------------------------*/
    /* ALLOW DATA CHECK                                              */
    /*---------------------------------------------------------------*/
    /*
        *residual = 0;
    */
        *unitstat = CSW_CE | CSW_DE;
        break;

    case 0x8B:
    /*---------------------------------------------------------------*/
    /* SKIP TO CHANNEL 1 IMMEDIATE                                   */
    /*---------------------------------------------------------------*/
        eor = dev->crlf ? "\r\f" : "\f";
        write_buffer (dev, eor, strlen(eor), unitstat);
        if (*unitstat != 0) break;

    /*
        *residual = 0;
    */
        *unitstat = CSW_CE | CSW_DE;
        break;

    case 0xCB:
    /*---------------------------------------------------------------*/
    /* SKIP TO CHANNEL 9 IMMEDIATE                                   */
    /*---------------------------------------------------------------*/
        eor = dev->crlf ? "\r\n" : "\n";
        write_buffer (dev, eor, strlen(eor), unitstat);
        if (*unitstat != 0) break;

    /*
        *residual = 0;
    */
        *unitstat = CSW_CE | CSW_DE;
        break;

    case 0xE3: case 0xDB:
    /*---------------------------------------------------------------*/
    /* SKIP TO CHANNEL 12 IMMEDIATE (or 11)                          */
    /*---------------------------------------------------------------*/
        eor = dev->crlf ? "\r\n" : "\n";
        write_buffer (dev, eor, strlen(eor), unitstat);
        if (*unitstat != 0) break;

    /*
        *residual = 0;
    */
        *unitstat = CSW_CE | CSW_DE;
        break;

    case 0x63:
    /*---------------------------------------------------------------*/
    /* LOAD FORMS CONTROL BUFFER                                     */
    /*---------------------------------------------------------------*/
        /* Return normal status */
        *residual = 0;
        *unitstat = CSW_CE | CSW_DE;
        break;

    case 0xEB:
    /*---------------------------------------------------------------*/
    /* UCS GATE LOAD                                                 */
    /*---------------------------------------------------------------*/
        /* Command reject if not first command in chain */
        if (chained != 0)
        {
            dev->sense[0] = SENSE_CR;
            *unitstat = CSW_CE | CSW_DE | CSW_UC;
            break;
        }

        /* Return normal status */
        *unitstat = CSW_CE | CSW_DE;
        break;

    case 0xF3:
    /*---------------------------------------------------------------*/
    /* LOAD UCS BUFFER AND FOLD                                      */
    /*---------------------------------------------------------------*/
        /* For 1403, command reject if not chained to UCS GATE */
        /* Also allow ALLOW DATA CHECK to get TSS/370 working  */
        /* -- JRM 11/28/2007 */
        if (dev->devtype == 0x1403 &&
            ((prevcode != 0xEB) && (prevcode != 0x7B)))
        {
            dev->sense[0] = SENSE_CR;
            *unitstat = CSW_CE | CSW_DE | CSW_UC;
            break;
        }

        /* Set fold indicator and return normal status */
        dev->fold = 1;
    /*
        *residual = 0;
    */
        *unitstat = CSW_CE | CSW_DE;
        break;

    case 0xFB:
    /*---------------------------------------------------------------*/
    /* LOAD UCS BUFFER (NO FOLD)                                     */
    /*---------------------------------------------------------------*/
        /* For 1403, command reject if not chained to UCS GATE */
        if (dev->devtype == 0x1403 && prevcode != 0xEB)
        {
            dev->sense[0] = SENSE_CR;
            *unitstat = CSW_CE | CSW_DE | CSW_UC;
            break;
        }

        /* Reset fold indicator and return normal status */
        dev->fold = 0;
    /*
        *residual = 0;
    */
        *unitstat = CSW_CE | CSW_DE;
        break;

    case 0x04:
    /*---------------------------------------------------------------*/
    /* SENSE                                                         */
    /*---------------------------------------------------------------*/
        /* Calculate residual byte count */
        num = (count < dev->numsense) ? count : dev->numsense;
        *residual = count - num;
        if (count < dev->numsense) *more = 1;

        /* Copy device sense bytes to channel I/O buffer */
        memcpy (iobuf, dev->sense, num);

        /* Clear the device sense bytes */
        memset (dev->sense, 0, sizeof(dev->sense));

        /* Return unit status */
        *unitstat = CSW_CE | CSW_DE;
        break;

    case 0xE4:
    /*---------------------------------------------------------------*/
    /* SENSE ID                                                      */
    /*---------------------------------------------------------------*/
        /* Calculate residual byte count */
        num = (count < dev->numdevid) ? count : dev->numdevid;
        *residual = count - num;
        if (count < dev->numdevid) *more = 1;

        /* Copy device identifier bytes to channel I/O buffer */
        memcpy (iobuf, dev->devid, num);

        /* Return unit status */
        *unitstat = CSW_CE | CSW_DE;
        break;

    default:
    /*---------------------------------------------------------------*/
    /* INVALID OPERATION                                             */
    /*---------------------------------------------------------------*/
        /* Set command reject sense byte, and unit check status */
        dev->sense[0] = SENSE_CR;
        *unitstat = CSW_CE | CSW_DE | CSW_UC;

    } /* end switch(code) */

} /* end function printer_execute_ccw */


#if defined(OPTION_DYNAMIC_LOAD)
static
#endif
DEVHND printer_device_hndinfo = {
        &printer_init_handler,         /* Device Initialisation      */
        &printer_execute_ccw,          /* Device CCW execute         */
        &printer_close_device,         /* Device Close               */
        &printer_query_device,         /* Device Query               */
        NULL,                          /* Device Start channel pgm   */
        NULL,                          /* Device End channel pgm     */
        NULL,                          /* Device Resume channel pgm  */
        NULL,                          /* Device Suspend channel pgm */
        NULL,                          /* Device Read                */
        NULL,                          /* Device Write               */
        NULL,                          /* Device Query used          */
        NULL,                          /* Device Reserve             */
        NULL,                          /* Device Release             */
        NULL,                          /* Device Attention           */
        printer_immed_commands,        /* Immediate CCW Codes        */
        NULL,                          /* Signal Adapter Input       */
        NULL,                          /* Signal Adapter Output      */
        NULL,                          /* Hercules suspend           */
        NULL                           /* Hercules resume            */
};

/* Libtool static name colision resolution */
/* note : lt_dlopen will look for symbol & modulename_LTX_symbol */
#if !defined(HDL_BUILD_SHARED) && defined(HDL_USE_LIBTOOL)
#define hdl_ddev hdt1403_LTX_hdl_ddev
#define hdl_depc hdt1403_LTX_hdl_depc
#define hdl_reso hdt1403_LTX_hdl_reso
#define hdl_init hdt1403_LTX_hdl_init
#define hdl_fini hdt1403_LTX_hdl_fini
#endif

#if defined(OPTION_DYNAMIC_LOAD)
HDL_DEPENDENCY_SECTION;
{
     HDL_DEPENDENCY(HERCULES);
     HDL_DEPENDENCY(DEVBLK);
}
END_DEPENDENCY_SECTION


HDL_DEVICE_SECTION;
{
    HDL_DEVICE(1403, printer_device_hndinfo );
    HDL_DEVICE(3211, printer_device_hndinfo );
}
END_DEVICE_SECTION
#endif
