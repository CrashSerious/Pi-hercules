# ***************************************************************************
# Makefile for Hercules S/370, ESA/390 and z/Architecture emulator
# Process this file with 'automake' to produce Makefile.in
#
# $Id: Makefile.am 5617 2010-02-09 00:00:41Z fish $
#
# $Log$
# Revision 1.187  2009/01/18 00:52:19  jmaynard
# Fix Gentoo reported bug: unable to do multithreaded "make install" reliably
# if --enable-setuid-hercifc is specified.
#
# Revision 1.186  2008/12/29 11:03:10  jj
# Move HMC disk I/O functions to scedasd.c
#
# Revision 1.185  2008/12/04 17:39:02  rbowler
# Add ability to configure group name with --enable-setuid-hercifc=name
#
# Revision 1.184  2008/12/01 16:19:49  jj
# Check for licensed operating systems without impairing architectural
# compliance of IFL's
#
# Revision 1.183  2008/05/22 19:25:58  fish
# Flex FakeTape support
#
# Revision 1.182  2008/03/25 11:41:31  fish
# SCSI TAPE MODS part 1: groundwork: non-functional changes:
# rename some functions, comments, general restructuring, etc.
# New source modules awstape.c, omatape.c, hettape.c and
# tapeccws.c added, but not yet used (all will be used in a future
# commit though when tapedev.c code is eventually split)
#
# Revision 1.181  2008/03/02 22:50:35  rbowler
# Restore tabs corrupted by previous modification
#
# Revision 1.180  2008/03/01 14:19:29  rbowler
# Add new module general3.c for general-instructions-extension facility
#
# Revision 1.179  2008/02/19 11:49:18  ivan
# - Move setting of CPU priority after spwaning timer thread
# - Added support for Posix 1003.1e capabilities
#
# Revision 1.178  2007/06/02 13:46:41  rbowler
# PFPO framework
#
# Revision 1.177  2006/12/30 21:13:37  rbowler
# Backout rev 1.176
#
# Revision 1.175  2006/12/30 16:54:07  rbowler
# Reinstate missing changelog entries
#
# Revision 1.174  2006/12/30 15:37:01  jmaynard
# Removed a coupple of stray ^Ms.
#
# Revision 1.173  2006/12/30 01:20:09  fish
# Add new MSVC makefile includes to distribution
#
# Revision 1.172  2006/12/18 13:35:51  rbowler
# Add hstdint.h to make dist
#
# Revision 1.171  2006/12/13 17:16:30  rbowler
# Make libherc depend on libdecNumber
#
# Revision 1.170  2006/12/11 16:56:59  rbowler
# Update Makefile.am to search decNumber for include files
#
# Revision 1.169  2006/12/09 01:05:59  ivan
# Remove makefile-dll.msvc from Makefile.am dist list since it has been removed from the repository
#
# Revision 1.168  2006/12/08 09:43:16  jj
# Add CVS message log
#
# DD/MM/YY Description.......................................................
# 06/09/06 Change History added - Fish
# 06/09/06 HAO (Hercules Automatic Operator) support - Fish
# ***************************************************************************

AUTOMAKE_OPTIONS = foreign 1.5
ACLOCAL_AMFLAGS  = -I m4

lns=@LN_S@

SUBDIRS = decNumber m4 po util html man . crypto

LDADD = @LTLIBICONV@ @LTLIBINTL@ @LIBS@

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/decNumber

if BUILD_HERCIFC
  HERCIFC = hercifc
else
  HERCIFC=
endif

if BUILD_SHARED
  HERCLIN=herclin
else
  HERCLIN=
endif

fthreads_SRC  = fthreads.c
fishio_SRC    = w32chan.c
fishhang_SRC  = fishhang.c

if BUILD_FTHREADS
  FTHREADS    = $(fthreads_SRC)
  FISHIO      = $(fishio_SRC)
if BUILD_FISHHANG
    FISHHANG  = $(fishhang_SRC)
else
    FISHHANG  =
endif
else
  FTHREADS    =
  FISHHANG    =
  FISHIO      =
endif

# We need to still include the source for the dynamic modules
# in with the distribution regardless of whether or not the dyanmic
# modules themselves are to be built so if dynamic load not possible,
# then we need to include the source on the EXTRA_hercules_SOURCES
# statement so even though they won't/don't actually get built, they
# still nonetheless get included into distribution tarball. This is
# *similar* to (bit not identical to) the way the fthreads sources
# are handled. With the fthreads sources though, they can at least always
# be built (even though they may not always generate any actual code
# whenever they do) whereas the dynamic module source CANNOT (MUST not)
# even be *attempted* to be "built" if support for building dynamic modules
# doesn't even exist on the system doing the building.

dynamic_SRC        = dyngui.c   \
                     dyninst.c  \
                     hdteq.c

dyndev_SRC         = commadpt.c  \
                     comm3705.c  \
                     console.c   \
                     cardpch.c   \
                     cardrdr.c   \
                     sockdev.c   \
                     printer.c   \
                     tapedev.c   \
                     tapeccws.c  \
                     sllib.c     \
                     hetlib.c    \
                     awstape.c   \
                     faketape.c  \
                     hettape.c   \
                     omatape.c   \
                     scsitape.c  \
                     w32stape.c  \
                     ctc_lcs.c   \
                     ctc_ctci.c  \
                     ctcadpt.c   \
                     w32ctca.c   \
                     hchan.c     \
                     tuntap.c    \
                     qeth.c      \
                     con1052c.c


# -module         : create a dlopen'able module
# -no-undefined   : required on all platform that do not allow shared modules
#                   to have undefined symbols
# $(LDADD)        : Misc Linker flags set by autoconf
# -export-dynamic : so dlsym works (to be verified - not sure it is necessary)
# -avoid-version  : needed.. Otherwise libtool gives crazy names to Windows
#                   DLLs
#

if BUILD_SHARED
   XSTATIC =
else
   XSTATIC = -static
endif

if OPTION_DYNAMIC_LOAD
  DYNSRC         =
  LTDL           = ltdl.c

  DYNMOD_LD_FLAGS = -module         \
                   -no-undefined    \
                   $(XSTATIC)       \
                   -export-dynamic  \
                   -avoid-version

  DYNMOD_LD_ADD   = libherc.la    \
                   libhercs.la    \
                   libhercu.la    \
                   $(LDADD)

  LIB_LD_FLAGS    = -export-dynamic  \
                   $(XSTATIC)        \
                   -no-undefined     \
                   -avoid-version
else
  DYNSRC         = $(dyndev_SRC)
  LTDL           =
  DYNMOD_LD_FLAGS =
  DYNMOD_LD_ADD =
  LIB_LD_FLAGS    = $(XSTATIC)    \
                   -no-undefined  \
                   -avoid-version
endif

#
# List of Libtool shared libraries & loadable modules
#

  HERCLIBS = 

  HERCLIBS2 = libhercs.la  \
              libhercu.la \
              libherct.la \
              libhercd.la \
              libherc.la

  HERCMODS = dyngui.la   \
             dyninst.la  \
             hdteq.la    \
             hdt1403.la  \
             hdt2703.la  \
             hdt3705.la  \
             hdt3088.la  \
             hdt3270.la  \
             hdt3420.la  \
             hdt3505.la  \
             hdt2880.la  \
             hdt3525.la  \
             hdtqeth.la  \
             hdt1052c.la

if OPTION_DYNAMIC_LOAD
  modexec_LTLIBRARIES = $(HERCMODS)
endif

  noinst_LTLIBRARIES = $(HERCLIBS)
  lib_LTLIBRARIES = $(HERCLIBS2)

# The following causes the dyanmic modules to not even get built
# unless support for building dynamic modules exists on the build system

bin_PROGRAMS = hercules \
               dasdinit dasdisup dasdload dasdconv dasdls dasdcat dasdpdsu dasdseq \
               tapecopy tapemap tapesplt \
               cckdcdsk cckdcomp cckddiag cckdswap \
               dasdcopy \
               hetget hetinit hetmap hetupd \
               dmap2hrc \
               $(HERCIFC) \
               $(HERCLIN)

EXTRA_PROGRAMS = hercifc

if OPTION_DYNAMIC_LOAD

#
# For each module:
#
#   ModuleName_la_SOURCES      = <list of source files>
#   ModuleName_la_LDFLAGS      = $(DYNMOD_LD_FLAGS) (see above)
#   ModuleName_la_LIBADD       = libherc.la (the Core Hercules Shared Library)
#   ModuleName_la_DEPENDENCIES = libherc.la (may not be necessary)
#
  dyngui_la_SOURCES  = dyngui.c
  dyngui_la_LDFLAGS  = $(DYNMOD_LD_FLAGS)
  dyngui_la_LIBADD   = $(DYNMOD_LD_ADD)

  dyninst_la_SOURCES = dyninst.c
  dyninst_la_LDFLAGS = $(DYNMOD_LD_FLAGS)
  dyninst_la_LIBADD  = $(DYNMOD_LD_ADD)

  hdteq_la_SOURCES   = hdteq.c
  hdteq_la_LDFLAGS   = $(DYNMOD_LD_FLAGS)
  hdteq_la_LIBADD    = $(DYNMOD_LD_ADD)

  hdt1403_la_SOURCES = printer.c sockdev.c
  hdt1403_la_LDFLAGS = $(DYNMOD_LD_FLAGS)
  hdt1403_la_LIBADD  = $(DYNMOD_LD_ADD)

  hdt2880_la_SOURCES = hchan.c
  hdt2880_la_LDFLAGS = $(DYNMOD_LD_FLAGS)
  hdt2880_la_LIBADD  = $(DYNMOD_LD_ADD)

  hdt2703_la_SOURCES = commadpt.c
  hdt2703_la_LDFLAGS = $(DYNMOD_LD_FLAGS)
  hdt2703_la_LIBADD  = $(DYNMOD_LD_ADD)

  hdt3705_la_SOURCES = comm3705.c
  hdt3705_la_LDFLAGS = $(DYNMOD_LD_FLAGS)
  hdt3705_la_LIBADD  = $(DYNMOD_LD_ADD)

  hdt3088_la_SOURCES = ctc_lcs.c ctc_ctci.c ctcadpt.c w32ctca.c tuntap.c
  hdt3088_la_LDFLAGS = $(DYNMOD_LD_FLAGS)
  hdt3088_la_LIBADD  = $(DYNMOD_LD_ADD)

  hdt3270_la_SOURCES = console.c
  hdt3270_la_LDFLAGS = $(DYNMOD_LD_FLAGS)
  hdt3270_la_LIBADD  = $(DYNMOD_LD_ADD)

  hdt3420_la_SOURCES = tapedev.c tapeccws.c awstape.c faketape.c hettape.c omatape.c scsitape.c w32stape.c
  hdt3420_la_LDFLAGS = $(DYNMOD_LD_FLAGS)
  hdt3420_la_LIBADD  = $(DYNMOD_LD_ADD) libherct.la

  hdt3505_la_SOURCES = cardrdr.c sockdev.c
  hdt3505_la_LDFLAGS = $(DYNMOD_LD_FLAGS)
  hdt3505_la_LIBADD  = $(DYNMOD_LD_ADD)

  hdt3525_la_SOURCES = cardpch.c
  hdt3525_la_LDFLAGS = $(DYNMOD_LD_FLAGS)
  hdt3525_la_LIBADD  = $(DYNMOD_LD_ADD)

  hdtqeth_la_SOURCES = qeth.c
  hdtqeth_la_LDFLAGS = $(DYNMOD_LD_FLAGS)
  hdtqeth_la_LIBADD  = $(DYNMOD_LD_ADD)

  hdt1052c_la_SOURCES = con1052c.c
  hdt1052c_la_LDFLAGS = $(DYNMOD_LD_FLAGS)
  hdt1052c_la_LIBADD  = $(DYNMOD_LD_ADD)

endif

#
# Common data areas (shared) library
#

  libhercs_la_SOURCES = hsys.c

  libhercs_la_LDFLAGS = $(LIB_LD_FLAGS)

  libhercs_la_LIBADD  = $(LDADD)

#
# Tape utility subroutines (shared) library
#

  libherct_la_SOURCES = sllib.c  \
                        hetlib.c

  libherct_la_LDFLAGS = $(LIB_LD_FLAGS)

  libherct_la_LIBADD  = $(LDADD) libhercs.la libhercu.la

#
# DASD utility subroutines (shared) library
#

  libhercd_la_SOURCES = ckddasd.c   \
                        fbadasd.c   \
                        cckddasd.c  \
                        cckdutil.c  \
                        dasdtab.c   \
                        cache.c     \
                        dasdutil.c  \
                        shared.c

  libhercd_la_LDFLAGS = $(LIB_LD_FLAGS)

  libhercd_la_LIBADD  = $(LDADD) libhercs.la libhercu.la

#
## Pure Utility functions
#
  libhercu_la_SOURCES = version.c    \
                        hscutl.c     \
                        hscutl2.c    \
                        codepage.c   \
                        logger.c     \
                        logmsg.c     \
                        hdl.c        \
                        hostinfo.c   \
                        hsocket.c    \
                        memrchr.c    \
                        parser.c     \
                        pttrace.c    \
                        $(FTHREADS)  \
                        $(FISHHANG)  \
                        $(LTDL)

  libhercu_la_LDFLAGS = $(LIB_LD_FLAGS)

  libhercu_la_LIBADD  = $(LDADD) libhercs.la

#
# Core Hercules (shared) library
#

  libherc_la_SOURCES = hconsole.c   \
                       w32util.c    \
                       strsignal.c  \
                       impl.c       \
                       config.c     \
                       bldcfg.c     \
                       panel.c      \
                       history.c    \
                       fillfnam.c   \
                       ipl.c        \
                       assist.c     \
                       dat.c        \
                       stack.c      \
                       cpu.c        \
                       vstore.c     \
                       general1.c   \
                       general2.c   \
                       general3.c   \
                       pfpo.c       \
                       plo.c        \
                       control.c    \
                       crypto.c     \
                       io.c         \
                       decimal.c    \
                       service.c    \
                       scedasd.c    \
                       losc.c       \
                       chsc.c       \
                       opcode.c     \
                       diagnose.c   \
                       diagmssf.c   \
                       vm.c         \
                       vmd250.c     \
                       channel.c    \
                       external.c   \
                       float.c      \
                       trace.c      \
                       machchk.c    \
                       vector.c     \
                       xstore.c     \
                       cmpsc.c      \
                       sie.c        \
                       qdio.c       \
                       clock.c      \
                       timer.c      \
                       esame.c      \
                       ieee.c       \
                       dfp.c        \
                       machdep.h    \
                       httpserv.c   \
                       cgibin.c     \
                       loadparm.c   \
                       hsccmd.c     \
                       cmdtab.c     \
                       hao.c        \
                       hscmisc.c    \
                       sr.c         \
                       $(FISHIO)    \
                       $(DYNSRC)    \
                       ecpsvm.c

  EXTRA_libherc_la_SOURCES = $(fthreads_SRC)  \
                             $(fishhang_SRC)  \
                             $(fishio_SRC)    \
                             memrchr.c        \
                             $(dynamic_SRC)   \
                             $(extra_SRC)     \
                             $(dyndev_SRC)    \
                             ltdl.c

  libherc_la_LDFLAGS = $(LIB_LD_FLAGS)

  libherc_la_LIBADD = libhercs.la     \
                      libhercu.la     \
                      libherct.la     \
                      libhercd.la     \
                      decNumber/libdecNumber.la \
                      $(LDADD)

#
# THIS is the hercules executable.
#      hdlmain.c is requires so that dlopen(self) retrieves
#                the 'main' hdl symbols
#

if OPTION_DYNAMIC_LOAD

if BUILD_SHARED
  HLDFLAGS = -dlopen self
  HDEPS =

else
  HLDFLAGS = -dlopen self         \
             -dlopen dyngui.la    \
             -dlopen dyninst.la   \
             -dlopen hdteq.la     \
             -dlopen hdt1403.la   \
             -dlopen hdt3420.la   \
             -dlopen hdt2703.la   \
             -dlopen hdt3705.la   \
             -dlopen hdt3088.la   \
             -dlopen hdt3270.la   \
             -dlopen hdt3505.la   \
             -dlopen hdt3525.la   \
             -dlopen hdtqeth.la   \
             -dlopen hdt1052c.la

  HDEPS    = dyngui.la    \
             dyninst.la   \
             hdteq.la     \
             hdt1403.la   \
             hdt3420.la   \
             hdt2703.la   \
             hdt3705.la   \
             hdt3088.la   \
             hdt3270.la   \
             hdt3505.la   \
             hdt3525.la   \
             hdtqeth.la   \
             hdt1052c.la  \
             libherc.la   \
             libherct.la  \
             libhercd.la
endif

else
  HLDFLAGS =
  HDEPS    =

endif

  hercules_SOURCES      = bootstrap.c  \
                          hdlmain.c

  hercules_LDADD        = libherc.la libhercs.la $(LDADD)

  hercules_LDFLAGS      = $(HLDFLAGS)

  hercules_DEPENDENCIES = libherc.la libhercs.la $(HDEPS)

if BUILD_SHARED
herclin_SOURCES       = herclin.c hdlmain.c
herclin_LDADD        = libherc.la libhercs.la $(LDADD)
herclin_LDFLAGS      = $(HLDFLAGS)
herclin_DEPENDENCIES = libherc.la libhercs.la $(HDEPS)
endif

#
# side binaries
#

tools_LD_FLAGS =
tools_ADDLIBS = $(HERCLIBS2) $(LDADD)

if BUILD_HERCIFC
hercifc_SOURCES       = hercifc.c
hercifc_LDADD         = $(tools_ADDLIBS)
hercifc_LDFLAGS       = $(tools_LD_FLAGS)
endif


dasdinit_SOURCES      = dasdinit.c
dasdinit_LDADD        = $(tools_ADDLIBS)
dasdinit_LDFLAGS      = $(tools_LD_FLAGS)

dasdisup_SOURCES      = dasdisup.c
dasdisup_LDADD        = $(tools_ADDLIBS)
dasdisup_LDFLAGS      = $(tools_LD_FLAGS)

dasdload_SOURCES      = dasdload.c
dasdload_LDADD        = $(tools_ADDLIBS)
dasdload_LDFLAGS      = $(tools_LD_FLAGS)

dasdconv_SOURCES      = dasdconv.c
dasdconv_LDADD        = $(tools_ADDLIBS)
dasdconv_LDFLAGS      = $(tools_LD_FLAGS)

dasdls_SOURCES        = dasdls.c
dasdls_LDADD          = $(tools_ADDLIBS)
dasdls_LDFLAGS        = $(tools_LD_FLAGS)

dasdcat_SOURCES       = dasdcat.c
dasdcat_LDADD         = $(tools_ADDLIBS)
dasdcat_LDFLAGS       = $(tools_LD_FLAGS)

dasdpdsu_SOURCES      = dasdpdsu.c
dasdpdsu_LDADD        = $(tools_ADDLIBS)
dasdpdsu_LDFLAGS      = $(tools_LD_FLAGS)

dasdseq_SOURCES       = dasdseq.c
dasdseq_LDADD         = $(tools_ADDLIBS)
dasdseq_LDFLAGS       = $(tools_LD_FLAGS)

tapecopy_SOURCES      = tapecopy.c
tapecopy_LDADD        = $(tools_ADDLIBS)
tapecopy_LDFLAGS      = $(tools_LD_FLAGS)

tapemap_SOURCES       = tapemap.c
tapemap_LDADD         = $(tools_ADDLIBS)
tapemap_LDFLAGS       = $(tools_LD_FLAGS)

tapesplt_SOURCES      = tapesplt.c
tapesplt_LDADD        = $(tools_ADDLIBS)
tapesplt_LDFLAGS      = $(tools_LD_FLAGS)

cckdcdsk_SOURCES      = cckdcdsk.c
cckdcdsk_LDADD        = $(tools_ADDLIBS)
cckdcdsk_LDFLAGS      = $(tools_LD_FLAGS)

cckdcomp_SOURCES      = cckdcomp.c
cckdcomp_LDADD        = $(tools_ADDLIBS)
cckdcomp_LDFLAGS      = $(tools_LD_FLAGS)

cckddiag_SOURCES      = cckddiag.c
cckddiag_LDADD        = $(tools_ADDLIBS)
cckddiag_LDFLAGS      = $(tools_LD_FLAGS)

dasdcopy_SOURCES      = dasdcopy.c
dasdcopy_LDADD        = $(tools_ADDLIBS)
dasdcopy_LDFLAGS      = $(tools_LD_FLAGS)

cckdswap_SOURCES      = cckdswap.c
cckdswap_LDADD        = $(tools_ADDLIBS)
cckdswap_LDFLAGS      = $(tools_LD_FLAGS)

hetget_SOURCES        = hetget.c
hetget_LDADD          = $(tools_ADDLIBS)
hetget_LDFLAGS        = $(tools_LD_FLAGS)

hetinit_SOURCES       = hetinit.c
hetinit_LDADD         = $(tools_ADDLIBS)
hetinit_LDFLAGS       = $(tools_LD_FLAGS)

hetmap_SOURCES        = hetmap.c
hetmap_LDADD          = $(tools_ADDLIBS)
hetmap_LDFLAGS        = $(tools_LD_FLAGS)

hetupd_SOURCES        = hetupd.c
hetupd_LDADD          = $(tools_ADDLIBS)
hetupd_LDFLAGS        = $(tools_LD_FLAGS)

dmap2hrc_SOURCES      = dmap2hrc.c
dmap2hrc_LDADD        = $(tools_ADDLIBS)
dmap2hrc_LDFLAGS      = $(tools_LD_FLAGS)

#
# files that are not 'built' per-se
#
#
# Also contains some WIN32 only source files
#

EXTRA_DIST = autoconf/config.rpath \
             autoconf/mkinstalldirs \
             COPYRIGHT             \
             hercules.cnf          \
             cckdfix.c             \
             README.COMMADPT       \
             README.CVS            \
             README.DYNMOD         \
             README.ECPSVM         \
             README.HAO            \
             README.HDL            \
             README.ISSUES         \
             README.MINGW          \
             README.NETWORKING     \
             README.OSX            \
             README.TAPE           \
             README.HERCLOGO       \
             CHANGES               \
             RELEASE.NOTES         \
             README.MSVC           \
             README.SUN            \
             _TODO.txt             \
             makefile.msvc         \
             makefile-dllmod.msvc  \
             msvc.makefile.includes/BZIP2_DIR.msvc    \
             msvc.makefile.includes/BZIP2_FLAGS.msvc  \
             msvc.makefile.includes/BZIP2_RULES.msvc  \
             msvc.makefile.includes/CONFIG.msvc       \
             msvc.makefile.includes/DEBUG_RETAIL.msvc \
             msvc.makefile.includes/HERC_FLAGS.msvc   \
             msvc.makefile.includes/MOD_RULES1.msvc   \
             msvc.makefile.includes/MOD_RULES2.msvc   \
             msvc.makefile.includes/MODULES.msvc      \
             msvc.makefile.includes/OBJ_CODE.msvc     \
             msvc.makefile.includes/OUTDIR_RULES.msvc \
             msvc.makefile.includes/OUTPUT_DIRS.msvc  \
             msvc.makefile.includes/PCRE_DIR.msvc     \
             msvc.makefile.includes/PCRE_FLAGS.msvc   \
             msvc.makefile.includes/PCRE_RULES.msvc   \
             msvc.makefile.includes/PRIM_RULES.msvc   \
             msvc.makefile.includes/VERSION.msvc      \
             msvc.makefile.includes/ZLIB_DIR.msvc     \
             msvc.makefile.includes/ZLIB_FLAGS.msvc   \
             msvc.makefile.includes/ZLIB_RULES.msvc   \
             makefile.bat          \
             dynmake.bat           \
             Hercules.dsp          \
             Hercules.dsw          \
             Hercules.vcproj       \
             Hercules.sln          \
             hercver.rc            \
             build_pch.c           \
             conspawn.c            \
             getopt.c              \
             herclogo.txt          \
             hercules.ico


#
# Source Header files. No 'build' for those
#
noinst_HEADERS = fishhang.h     \
                 hostinfo.h     \
                 cpuint.h       \
                 feat370.h      \
                 feat390.h      \
                 feat900.h      \
                 featall.h      \
                 featchk.h      \
                 feature.h      \
                 esa390.h       \
                 opcode.h       \
                 clock.h        \
                 hercules.h     \
                 inline.h       \
                 dat.h          \
                 vstore.h       \
                 hbyteswp.h     \
                 dasdblks.h     \
                 hetlib.h       \
                 version.h      \
                 parser.h       \
                 dasdtab.h      \
                 sllib.h        \
                 htypes.h       \
                 fthreads.h     \
                 w32chan.h      \
                 w32ctca.h      \
                 tt32api.h      \
                 linklist.h     \
                 ieee-w32.h     \
                 httpmisc.h     \
                 devtype.h      \
                 codepage.h     \
                 ctcadpt.h      \
                 hercifc.h      \
                 tuntap.h       \
                 tapedev.h      \
                 scsitape.h     \
                 logger.h       \
                 commadpt.h     \
                 comm3705.h     \
                 cache.h        \
                 ecpsvm.h       \
                 memrchr.h      \
                 shared.h       \
                 hscutl.h       \
                 cmdtab.h       \
                 hdl.h          \
                 crypto.h       \
                 sockdev.h      \
                 ltdl.h         \
                 herc_getopt.h  \
                 service.h      \
                 chsc.h         \
                 pttrace.h      \
                 history.h      \
                 sr.h           \
                 hchan.h        \
                 fillfnam.h     \
                 hthreads.h     \
                 hostopts.h     \
                 w32util.h      \
                 hconsts.h      \
                 hmacros.h      \
                 hstructs.h     \
                 hexterns.h     \
                 hconsole.h     \
                 hextapi.h      \
                 hstdinc.h      \
                 hstdint.h      \
                 hsocket.h      \
                 w32stape.h     \
                 w32dl.h        \
                 hercwind.h     \
                 getopt.h       \
                 w32mtio.h      \
                 vmd250.h       \
                 $(extra_dynamic_SRC)

cckd: cckd2ckd$(EXEEXT)  \
      cckdcdsk$(EXEEXT)  \
      cckddiag$(EXEEXT)  \
      cckdcomp$(EXEEXT)  \
      cckdswap$(EXEEXT)  \
      dasdcopy$(EXEEXT)

#########################################################
#           B U I L D   R U L E S  . . .
#########################################################
#
#            ***  PROGRAMMING NOTE! ***
#
#  The tabs in the below statements ARE significant!
#  Not all make programs accept (handle correctly)
#  make files that use blanks instead of tabs in their
#  build rules. Thus in order to remain compatible
#  with older make programs the below build rules
#  MUST use *tabs* and NOT BLANKS!
#
#########################################################

tar: dist

install-exec-local:

install-exec-hook:
if BUILD_SHARED
	rm -f $(DESTDIR)$(libdir)/libherc*.a
	rm -f $(DESTDIR)$(modexecdir)/dyn*.a
	rm -f $(DESTDIR)$(modexecdir)/hdt*.a
else
	rm -f $(DESTDIR)$(libdir)/libherc*
endif
#
# NOTE : symbolic links point to FINAL destination (not to staged install)
#
	rm -f $(DESTDIR)$(bindir)/fba2cfba$(EXEEXT)
	(cd $(DESTDIR)$(bindir); @LN_S@ ./dasdcopy$(EXEEXT) fba2cfba$(EXEEXT))
	rm -f $(DESTDIR)$(bindir)/ckd2cckd$(EXEEXT)
	(cd $(DESTDIR)$(bindir); @LN_S@ ./dasdcopy$(EXEEXT) ckd2cckd$(EXEEXT))
	rm -f $(DESTDIR)$(bindir)/cfba2fba$(EXEEXT)
	(cd $(DESTDIR)$(bindir); @LN_S@ ./dasdcopy$(EXEEXT) cfba2fba$(EXEEXT))
	rm -f $(DESTDIR)$(bindir)/cckd2ckd$(EXEEXT)
	(cd $(DESTDIR)$(bindir); @LN_S@ ./dasdcopy$(EXEEXT) cckd2ckd$(EXEEXT))
if SETUID_HERCIFC
	chown root $(DESTDIR)$(bindir)/hercifc
if HERCIFC_GROUPSET
	chgrp $(HERCIFC_GROUPNAME) $(DESTDIR)$(bindir)/hercifc
endif
	chmod 0750 $(DESTDIR)$(bindir)/hercifc
	chmod +s   $(DESTDIR)$(bindir)/hercifc
	rm                             hercifc
endif

uninstall-local:

uninstall-hook:
	rm -f $(DESTDIR)$(bindir)/fba2cfba$(EXEEXT)
	rm -f $(DESTDIR)$(bindir)/ckd2cckd$(EXEEXT)
	rm -f $(DESTDIR)$(bindir)/cfba2fba$(EXEEXT)
	rm -f $(DESTDIR)$(bindir)/cckd2ckd$(EXEEXT)

%.s: %.c
	$(COMPILE) -S $<
