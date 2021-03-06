/* HETTAPE.C    (c) Copyright Roger Bowler, 1999-2009                */
/*              Hercules Tape Device Handler for HETTAPE             */

/* Original Author: Leland Lucius                                    */
/* Prime Maintainer: Ivan Warren                                     */
/* Secondary Maintainer: "Fish" (David B. Trout)                     */

// $Id: hettape.c 5587 2009-12-31 15:05:57Z rbowler $

/*-------------------------------------------------------------------*/
/* This module contains the HET emulated tape format support.        */
/*                                                                   */
/* The subroutines in this module are called by the general tape     */
/* device handler (tapedev.c) when the tape format is HETTAPE.       */
/*                                                                   */
/* Messages issued by this module are prefixed HHCTA4nn              */
/*-------------------------------------------------------------------*/

#include "hstdinc.h"
#include "hercules.h"  /* need Hercules control blocks               */
#include "tapedev.h"   /* Main tape handler header file              */

//#define  ENABLE_TRACING_STMTS     // (Fish: DEBUGGING)

#ifdef ENABLE_TRACING_STMTS
  #if !defined(DEBUG)
    #warning DEBUG required for ENABLE_TRACING_STMTS
  #endif
  // (TRACE, ASSERT, and VERIFY macros are #defined in hmacros.h)
#else
  #undef  TRACE
  #undef  ASSERT
  #undef  VERIFY
  #define TRACE       1 ? ((void)0) : logmsg
  #define ASSERT(a)
  #define VERIFY(a)   ((void)(a))
#endif

/*-------------------------------------------------------------------*/
/* Open an HET format file                                           */
/*                                                                   */
/* If successful, the het control blk is stored in the device block  */
/* and the return value is zero.  Otherwise the return value is -1.  */
/*-------------------------------------------------------------------*/
int open_het (DEVBLK *dev, BYTE *unitstat,BYTE code)
{
int             rc;                     /* Return code               */

    /* Check for no tape in drive */
    if (!strcmp (dev->filename, TAPE_UNLOADED))
    {
        build_senseX(TAPE_BSENSE_TAPEUNLOADED,dev,unitstat,code);
        return -1;
    }

    /* Open the HET file */
    rc = het_open (&dev->hetb, dev->filename, dev->tdparms.logical_readonly ? HETOPEN_READONLY : HETOPEN_CREATE );
    if (rc >= 0)
    {
        if(dev->hetb->writeprotect)
        {
            dev->readonly=1;
        }
        rc = het_cntl (dev->hetb,
                    HETCNTL_SET | HETCNTL_COMPRESS,
                    dev->tdparms.compress);
        if (rc >= 0)
        {
            rc = het_cntl (dev->hetb,
                        HETCNTL_SET | HETCNTL_METHOD,
                        dev->tdparms.method);
            if (rc >= 0)
            {
                rc = het_cntl (dev->hetb,
                            HETCNTL_SET | HETCNTL_LEVEL,
                            dev->tdparms.level);
                if (rc >= 0)
                {
                    rc = het_cntl (dev->hetb,
                                HETCNTL_SET | HETCNTL_CHUNKSIZE,
                                dev->tdparms.chksize);
                }
            }
        }
    }

    /* Check for successful open */
    if (rc < 0)
    {
        int save_errno = errno;
        het_close (&dev->hetb);
        errno = save_errno;

        logmsg (_("HHCTA401E %4.4X: Error opening %s: %s(%s)\n"),
                dev->devnum, dev->filename, het_error(rc), strerror(errno));

        strcpy(dev->filename, TAPE_UNLOADED);
        build_senseX(TAPE_BSENSE_TAPELOADFAIL,dev,unitstat,code);
        return -1;
    }

    /* Indicate file opened */
    dev->fd = 1;

    return 0;

} /* end function open_het */

/*-------------------------------------------------------------------*/
/* Close an HET format file                                          */
/*                                                                   */
/* The HET file is close and all device block fields reinitialized.  */
/*-------------------------------------------------------------------*/
void close_het (DEVBLK *dev)
{

    /* Close the HET file */
    het_close (&dev->hetb);

    /* Reinitialize the DEV fields */
    dev->fd = -1;
    strcpy (dev->filename, TAPE_UNLOADED);
    dev->blockid = 0;
    dev->fenced = 0;

    return;

} /* end function close_het */

/*-------------------------------------------------------------------*/
/* Rewind HET format file                                            */
/*                                                                   */
/* The HET file is close and all device block fields reinitialized.  */
/*-------------------------------------------------------------------*/
int rewind_het(DEVBLK *dev,BYTE *unitstat,BYTE code)
{
int rc;
    rc = het_rewind (dev->hetb);
    if (rc < 0)
    {
        /* Handle seek error condition */
        logmsg (_("HHCTA402E %4.4X: Error seeking to start of %s: %s(%s)\n"),
                dev->devnum, dev->filename, het_error(rc), strerror(errno));

        build_senseX(TAPE_BSENSE_REWINDFAILED,dev,unitstat,code);
        return -1;
    }
    dev->nxtblkpos=0;
    dev->prvblkpos=-1;
    dev->curfilen=1;
    dev->blockid=0;
    dev->fenced = 0;
    return 0;
}

/*-------------------------------------------------------------------*/
/* Read a block from an HET format file                              */
/*                                                                   */
/* If successful, return value is block length read.                 */
/* If a tapemark was read, the return value is zero, and the         */
/* current file number in the device block is incremented.           */
/* If error, return value is -1 and unitstat is set to CE+DE+UC      */
/*-------------------------------------------------------------------*/
int read_het (DEVBLK *dev, BYTE *buf, BYTE *unitstat,BYTE code)
{
int             rc;                     /* Return code               */

    rc = het_read (dev->hetb, buf);
    if (rc < 0)
    {
        /* Increment file number and return zero if tapemark was read */
        if (rc == HETE_TAPEMARK)
        {
            dev->curfilen++;
            dev->blockid++;
            return 0;
        }

        /* Handle end of file (uninitialized tape) condition */
        if (rc == HETE_EOT)
        {
            logmsg (_("HHCTA414E %4.4X: End of file (end of tape) "
                    "at block %8.8X in file %s\n"),
                    dev->devnum, dev->hetb->cblk, dev->filename);

            /* Set unit exception with tape indicate (end of tape) */
            build_senseX(TAPE_BSENSE_ENDOFTAPE,dev,unitstat,code);
            return -1;
        }

        logmsg (_("HHCTA415E %4.4X: Error reading data block "
                "at block %8.8X in file %s: %s(%s)\n"),
                dev->devnum, dev->hetb->cblk, dev->filename,
                het_error(rc), strerror(errno));

        /* Set unit check with equipment check */
        build_senseX(TAPE_BSENSE_READFAIL,dev,unitstat,code);
        return -1;
    }
    dev->blockid++;
    /* Return block length */
    return rc;

} /* end function read_het */

/*-------------------------------------------------------------------*/
/* Write a block to an HET format file                               */
/*                                                                   */
/* If successful, return value is zero.                              */
/* If error, return value is -1 and unitstat is set to CE+DE+UC      */
/*-------------------------------------------------------------------*/
int write_het (DEVBLK *dev, BYTE *buf, U16 blklen,
                      BYTE *unitstat,BYTE code)
{
int             rc;                     /* Return code               */
off_t           cursize;                /* Current size for size chk */

    /* Check if we have already violated the size limit */
    if(dev->tdparms.maxsize>0)
    {
        cursize=het_tell(dev->hetb);
        if(cursize>=dev->tdparms.maxsize)
        {
            build_senseX(TAPE_BSENSE_ENDOFTAPE,dev,unitstat,code);
            return -1;
        }
    }
    /* Write the data block */
    rc = het_write (dev->hetb, buf, blklen);
    if (rc < 0)
    {
        /* Handle write error condition */
        logmsg (_("HHCTA416E %4.4X: Error writing data block "
                "at block %8.8X in file %s: %s(%s)\n"),
                dev->devnum, dev->hetb->cblk, dev->filename,
                het_error(rc), strerror(errno));

        /* Set unit check with equipment check */
        build_senseX(TAPE_BSENSE_WRITEFAIL,dev,unitstat,code);
        return -1;
    }
    /* Check if we have violated the maxsize limit */
    /* Also check if we are passed EOT marker */
    if(dev->tdparms.maxsize>0)
    {
        cursize=het_tell(dev->hetb);
        if(cursize>dev->tdparms.maxsize)
        {
            logmsg (_("HHCTA430I %4.4X: max tape capacity exceeded\n"),
                    dev->devnum);
            if(dev->tdparms.strictsize)
            {
                logmsg (_("HHCTA431I %4.4X: max tape capacity enforced\n"),
                        dev->devnum);
                het_bsb(dev->hetb);
                cursize=het_tell(dev->hetb);
                ftruncate( fileno(dev->hetb->fd),cursize);
                dev->hetb->truncated=TRUE; /* SHOULD BE IN HETLIB */
            }
            build_senseX(TAPE_BSENSE_ENDOFTAPE,dev,unitstat,code);
            return -1;
        }
    }

    /* Return normal status */
    dev->blockid++;

    return 0;

} /* end function write_het */

/*-------------------------------------------------------------------*/
/* Write a tapemark to an HET format file                            */
/*                                                                   */
/* If successful, return value is zero.                              */
/* If error, return value is -1 and unitstat is set to CE+DE+UC      */
/*-------------------------------------------------------------------*/
int write_hetmark (DEVBLK *dev, BYTE *unitstat,BYTE code)
{
int             rc;                     /* Return code               */

    /* Write the tape mark */
    rc = het_tapemark (dev->hetb);
    if (rc < 0)
    {
        /* Handle error condition */
        logmsg (_("HHCTA417E %4.4X: Error writing tape mark "
                "at block %8.8X in file %s: %s(%s)\n"),
                dev->devnum, dev->hetb->cblk, dev->filename,
                het_error(rc), strerror(errno));

        /* Set unit check with equipment check */
        build_senseX(TAPE_BSENSE_WRITEFAIL,dev,unitstat,code);
        return -1;
    }

    /* Return normal status */
    dev->blockid++;

    return 0;

} /* end function write_hetmark */

/*-------------------------------------------------------------------*/
/* Synchronize a HET format file   (i.e. flush its buffers to disk)  */
/*                                                                   */
/* If successful, return value is zero.                              */
/* If error, return value is -1 and unitstat is set to CE+DE+UC      */
/*-------------------------------------------------------------------*/
int sync_het(DEVBLK *dev, BYTE *unitstat,BYTE code)
{
int             rc;                     /* Return code               */

    /* Perform the flush */
    rc = het_sync (dev->hetb);
    if (rc < 0)
    {
        /* Handle error condition */
        if (HETE_PROTECTED == rc)
            build_senseX(TAPE_BSENSE_WRITEPROTECT,dev,unitstat,code);
        else
        {
            logmsg (_("HHCTA488E %4.4X: Sync error on file %s: %s\n"),
                dev->devnum, dev->filename, strerror(errno));
            build_senseX(TAPE_BSENSE_WRITEFAIL,dev,unitstat,code);
        }
        return -1;
    }

    /* Return normal status */
    return 0;

} /* end function sync_het */

/*-------------------------------------------------------------------*/
/* Forward space over next block of an HET format file               */
/*                                                                   */
/* If successful, return value +1.                                   */
/* If the block skipped was a tapemark, the return value is zero,    */
/* and the current file number in the device block is incremented.   */
/* If error, return value is -1 and unitstat is set to CE+DE+UC      */
/*-------------------------------------------------------------------*/
int fsb_het (DEVBLK *dev, BYTE *unitstat,BYTE code)
{
int             rc;                     /* Return code               */

    /* Forward space one block */
    rc = het_fsb (dev->hetb);

    if (rc < 0)
    {
        /* Increment file number and return zero if tapemark was read */
        if (rc == HETE_TAPEMARK)
        {
            dev->blockid++;
            dev->curfilen++;
            return 0;
        }

        logmsg (_("HHCTA418E %4.4X: Error forward spacing "
                "at block %8.8X in file %s: %s(%s)\n"),
                dev->devnum, dev->hetb->cblk, dev->filename,
                het_error(rc), strerror(errno));

        /* Set unit check with equipment check */
        if(rc==HETE_EOT)
        {
            build_senseX(TAPE_BSENSE_ENDOFTAPE,dev,unitstat,code);
        }
        else
        {
            build_senseX(TAPE_BSENSE_READFAIL,dev,unitstat,code);
        }
        return -1;
    }

    dev->blockid++;

    /* Return +1 to indicate forward space successful */
    return +1;

} /* end function fsb_het */

/*-------------------------------------------------------------------*/
/* Backspace to previous block of an HET format file                 */
/*                                                                   */
/* If successful, return value will be +1.                           */
/* If the block is a tapemark, the return value is zero,             */
/* and the current file number in the device block is decremented.   */
/* If error, return value is -1 and unitstat is set to CE+DE+UC      */
/*-------------------------------------------------------------------*/
int bsb_het (DEVBLK *dev, BYTE *unitstat,BYTE code)
{
int             rc;                     /* Return code               */

    /* Back space one block */
    rc = het_bsb (dev->hetb);
    if (rc < 0)
    {
        /* Increment file number and return zero if tapemark was read */
        if (rc == HETE_TAPEMARK)
        {
            dev->blockid--;
            dev->curfilen--;
            return 0;
        }

        /* Unit check if already at start of tape */
        if (rc == HETE_BOT)
        {
            build_senseX(TAPE_BSENSE_LOADPTERR,dev,unitstat,code);
            return -1;
        }

        logmsg (_("HHCTA419E %4.4X: Error reading data block "
                "at block %8.8X in file %s: %s(%s)\n"),
                dev->devnum, dev->hetb->cblk, dev->filename,
                het_error(rc), strerror(errno));

        /* Set unit check with equipment check */
        build_senseX(TAPE_BSENSE_READFAIL,dev,unitstat,code);
        return -1;
    }

    dev->blockid--;

    /* Return +1 to indicate back space successful */
    return +1;

} /* end function bsb_het */

/*-------------------------------------------------------------------*/
/* Forward space to next logical file of HET format file             */
/*                                                                   */
/* If successful, return value is zero, and the current file number  */
/* in the device block is incremented.                               */
/* If error, return value is -1 and unitstat is set to CE+DE+UC      */
/*-------------------------------------------------------------------*/
int fsf_het (DEVBLK *dev, BYTE *unitstat,BYTE code)
{
int             rc;                     /* Return code               */

    /* Forward space to start of next file */
    rc = het_fsf (dev->hetb);
    if (rc < 0)
    {
        logmsg (_("HHCTA420E %4.4X: Error forward spacing to next file "
                "at block %8.8X in file %s: %s(%s)\n"),
                dev->devnum, dev->hetb->cblk, dev->filename,
                het_error(rc), strerror(errno));

        if(rc==HETE_EOT)
        {
            build_senseX(TAPE_BSENSE_ENDOFTAPE,dev,unitstat,code);
        }
        else
        {
            build_senseX(TAPE_BSENSE_READFAIL,dev,unitstat,code);
        }
        return -1;
    }

    /* Maintain position */
    dev->blockid = rc;
    dev->curfilen++;

    /* Return success */
    return 0;

} /* end function fsf_het */

/*-------------------------------------------------------------------*/
/* Check HET file is passed the allowed EOT margin                   */
/*-------------------------------------------------------------------*/
int passedeot_het (DEVBLK *dev)
{
off_t cursize;
    if(dev->fd>0)
    {
        if(dev->tdparms.maxsize>0)
        {
            cursize=het_tell(dev->hetb);
            if(cursize+dev->eotmargin>dev->tdparms.maxsize)
            {
                dev->eotwarning = 1;
                return 1;
            }
        }
    }
    dev->eotwarning = 0;
    return 0;
}

/*-------------------------------------------------------------------*/
/* Backspace to previous logical file of HET format file             */
/*                                                                   */
/* If successful, return value is zero, and the current file number  */
/* in the device block is decremented.                               */
/* If error, return value is -1 and unitstat is set to CE+DE+UC      */
/*-------------------------------------------------------------------*/
int bsf_het (DEVBLK *dev, BYTE *unitstat,BYTE code)
{
int             rc;                     /* Return code               */

    /* Exit if already at BOT */
    if (dev->curfilen==1 && dev->nxtblkpos == 0)
    {
        build_senseX(TAPE_BSENSE_LOADPTERR,dev,unitstat,code);
        return -1;
    }

    rc = het_bsf (dev->hetb);
    if (rc < 0)
    {
        logmsg (_("HHCTA421E %4.4X: Error back spacing to previous file "
                "at block %8.8X in file %s:\n %s(%s)\n"),
                dev->devnum, dev->hetb->cblk, dev->filename,
                het_error(rc), strerror(errno));

        build_senseX(TAPE_BSENSE_LOCATEERR,dev,unitstat,code);
        return -1;
    }

    /* Maintain position */
    dev->blockid = rc;
    dev->curfilen--;

    /* Return success */
    return 0;

} /* end function bsf_het */
