CC          = gcc
CFLAGS      = -c -pthread -std=c11
CFLAGS      += -Wall -Wno-unused-function
CFLAGS      += -g
INCLUDE     = `pkg-config --cflags glib-2.0`
LIBS        = `pkg-config --libs glib-2.0`

FONLIB      = fon/int-pair.o fon/fon.o fon/msg_if.o
OBJS        = ant-obj.o aco-table.o ant.o ant-model.o
SRCS        = $(OBJS:.o=.c)
TARGETS     = aco
UNIT_TESTS  = test-obj test-table

.PHONY: all clean

all: $(TARGETS) $(UNIT_TESTS)

aco: $(FONLIB) $(OBJS) main.o
	$(CC) $^ -o $@ $(LIBS)

# Unit test code
unit-test: $(UNIT_TESTS)

test-obj: ant-obj.o test-obj.o
	$(CC) $^ -o $@ $(LIBS)

test-table: aco-table.o test-table.o
	$(CC) $^ -o $@ $(LIBS)

%.o: %.c
	$(CC) $< $(INCLUDE) $(CFLAGS)

clean:
	$(RM) $(TARGETS) $(UNIT_TESTS) *.o

dep:
	gccmakedep $(SRCS) $(INCLUDE)


# THIS CODE WAS GERNERATED BY gccmakedep.
# DO NOT DELETE
ant-obj.o: ant-obj.c /usr/include/stdc-predef.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.8/include/stdint.h /usr/include/stdint.h \
 /usr/include/features.h /usr/include/x86_64-linux-gnu/sys/cdefs.h \
 /usr/include/x86_64-linux-gnu/bits/wordsize.h \
 /usr/include/x86_64-linux-gnu/gnu/stubs.h \
 /usr/include/x86_64-linux-gnu/gnu/stubs-64.h \
 /usr/include/x86_64-linux-gnu/bits/wchar.h /usr/include/arpa/inet.h \
 /usr/include/netinet/in.h /usr/include/x86_64-linux-gnu/sys/socket.h \
 /usr/include/x86_64-linux-gnu/sys/uio.h \
 /usr/include/x86_64-linux-gnu/sys/types.h \
 /usr/include/x86_64-linux-gnu/bits/types.h \
 /usr/include/x86_64-linux-gnu/bits/typesizes.h /usr/include/time.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.8/include/stddef.h /usr/include/endian.h \
 /usr/include/x86_64-linux-gnu/bits/endian.h \
 /usr/include/x86_64-linux-gnu/bits/byteswap.h \
 /usr/include/x86_64-linux-gnu/bits/byteswap-16.h \
 /usr/include/x86_64-linux-gnu/sys/select.h \
 /usr/include/x86_64-linux-gnu/bits/select.h \
 /usr/include/x86_64-linux-gnu/bits/sigset.h \
 /usr/include/x86_64-linux-gnu/bits/time.h \
 /usr/include/x86_64-linux-gnu/sys/sysmacros.h \
 /usr/include/x86_64-linux-gnu/bits/pthreadtypes.h \
 /usr/include/x86_64-linux-gnu/bits/uio.h \
 /usr/include/x86_64-linux-gnu/bits/socket.h \
 /usr/include/x86_64-linux-gnu/bits/socket_type.h \
 /usr/include/x86_64-linux-gnu/bits/sockaddr.h \
 /usr/include/x86_64-linux-gnu/asm/socket.h \
 /usr/include/asm-generic/socket.h \
 /usr/include/x86_64-linux-gnu/asm/sockios.h \
 /usr/include/asm-generic/sockios.h \
 /usr/include/x86_64-linux-gnu/bits/in.h /usr/include/stdlib.h \
 /usr/include/x86_64-linux-gnu/bits/waitflags.h \
 /usr/include/x86_64-linux-gnu/bits/waitstatus.h /usr/include/alloca.h \
 /usr/include/x86_64-linux-gnu/bits/stdlib-float.h /usr/include/stdio.h \
 /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.8/include/stdarg.h \
 /usr/include/x86_64-linux-gnu/bits/stdio_lim.h \
 /usr/include/x86_64-linux-gnu/bits/sys_errlist.h /usr/include/assert.h \
 /usr/include/string.h /usr/include/xlocale.h ant-obj.h ant-type.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.8/include/stdbool.h ant-def.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.8/include-fixed/limits.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.8/include-fixed/syslimits.h \
 /usr/include/limits.h /usr/include/x86_64-linux-gnu/bits/posix1_lim.h \
 /usr/include/x86_64-linux-gnu/bits/local_lim.h \
 /usr/include/linux/limits.h \
 /usr/include/x86_64-linux-gnu/bits/posix2_lim.h ant-parameter.h
aco-table.o: aco-table.c /usr/include/stdc-predef.h aco-table.h \
 ant-type.h /usr/lib/gcc/x86_64-linux-gnu/4.8/include/stdbool.h \
 /usr/include/stdlib.h /usr/include/features.h \
 /usr/include/x86_64-linux-gnu/sys/cdefs.h \
 /usr/include/x86_64-linux-gnu/bits/wordsize.h \
 /usr/include/x86_64-linux-gnu/gnu/stubs.h \
 /usr/include/x86_64-linux-gnu/gnu/stubs-64.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.8/include/stddef.h \
 /usr/include/x86_64-linux-gnu/bits/waitflags.h \
 /usr/include/x86_64-linux-gnu/bits/waitstatus.h /usr/include/endian.h \
 /usr/include/x86_64-linux-gnu/bits/endian.h \
 /usr/include/x86_64-linux-gnu/bits/byteswap.h \
 /usr/include/x86_64-linux-gnu/bits/types.h \
 /usr/include/x86_64-linux-gnu/bits/typesizes.h \
 /usr/include/x86_64-linux-gnu/bits/byteswap-16.h \
 /usr/include/x86_64-linux-gnu/sys/types.h /usr/include/time.h \
 /usr/include/x86_64-linux-gnu/sys/select.h \
 /usr/include/x86_64-linux-gnu/bits/select.h \
 /usr/include/x86_64-linux-gnu/bits/sigset.h \
 /usr/include/x86_64-linux-gnu/bits/time.h \
 /usr/include/x86_64-linux-gnu/sys/sysmacros.h \
 /usr/include/x86_64-linux-gnu/bits/pthreadtypes.h /usr/include/alloca.h \
 /usr/include/x86_64-linux-gnu/bits/stdlib-float.h /usr/include/stdio.h \
 /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.8/include/stdarg.h \
 /usr/include/x86_64-linux-gnu/bits/stdio_lim.h \
 /usr/include/x86_64-linux-gnu/bits/sys_errlist.h /usr/include/string.h \
 /usr/include/xlocale.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.8/include-fixed/limits.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.8/include-fixed/syslimits.h \
 /usr/include/limits.h /usr/include/x86_64-linux-gnu/bits/posix1_lim.h \
 /usr/include/x86_64-linux-gnu/bits/local_lim.h \
 /usr/include/linux/limits.h \
 /usr/include/x86_64-linux-gnu/bits/posix2_lim.h fon/algorithm.h
ant.o: ant.c /usr/include/stdc-predef.h fon/packet_if.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.8/include/stdint.h /usr/include/stdint.h \
 /usr/include/features.h /usr/include/x86_64-linux-gnu/sys/cdefs.h \
 /usr/include/x86_64-linux-gnu/bits/wordsize.h \
 /usr/include/x86_64-linux-gnu/gnu/stubs.h \
 /usr/include/x86_64-linux-gnu/gnu/stubs-64.h \
 /usr/include/x86_64-linux-gnu/bits/wchar.h fon/fon.h \
 /usr/include/glib-2.0/glib.h /usr/include/glib-2.0/glib/galloca.h \
 /usr/include/glib-2.0/glib/gtypes.h \
 /usr/lib/x86_64-linux-gnu/glib-2.0/include/glibconfig.h \
 /usr/include/glib-2.0/glib/gmacros.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.8/include/stddef.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.8/include-fixed/limits.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.8/include-fixed/syslimits.h \
 /usr/include/limits.h /usr/include/x86_64-linux-gnu/bits/posix1_lim.h \
 /usr/include/x86_64-linux-gnu/bits/local_lim.h \
 /usr/include/linux/limits.h \
 /usr/include/x86_64-linux-gnu/bits/posix2_lim.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.8/include/float.h \
 /usr/include/glib-2.0/glib/gversionmacros.h /usr/include/time.h \
 /usr/include/x86_64-linux-gnu/bits/time.h \
 /usr/include/x86_64-linux-gnu/bits/types.h \
 /usr/include/x86_64-linux-gnu/bits/typesizes.h /usr/include/xlocale.h \
 /usr/include/glib-2.0/glib/garray.h \
 /usr/include/glib-2.0/glib/gasyncqueue.h \
 /usr/include/glib-2.0/glib/gthread.h \
 /usr/include/glib-2.0/glib/gatomic.h /usr/include/glib-2.0/glib/gerror.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.8/include/stdarg.h \
 /usr/include/glib-2.0/glib/gquark.h \
 /usr/include/glib-2.0/glib/gbacktrace.h /usr/include/signal.h \
 /usr/include/x86_64-linux-gnu/bits/sigset.h \
 /usr/include/x86_64-linux-gnu/bits/signum.h \
 /usr/include/x86_64-linux-gnu/bits/siginfo.h \
 /usr/include/x86_64-linux-gnu/bits/sigaction.h \
 /usr/include/x86_64-linux-gnu/bits/sigcontext.h \
 /usr/include/x86_64-linux-gnu/bits/sigstack.h \
 /usr/include/x86_64-linux-gnu/sys/ucontext.h \
 /usr/include/x86_64-linux-gnu/bits/pthreadtypes.h \
 /usr/include/x86_64-linux-gnu/bits/sigthread.h \
 /usr/include/glib-2.0/glib/gbase64.h \
 /usr/include/glib-2.0/glib/gbitlock.h \
 /usr/include/glib-2.0/glib/gbookmarkfile.h \
 /usr/include/glib-2.0/glib/gbytes.h \
 /usr/include/glib-2.0/glib/gcharset.h \
 /usr/include/glib-2.0/glib/gchecksum.h \
 /usr/include/glib-2.0/glib/gconvert.h \
 /usr/include/glib-2.0/glib/gdataset.h /usr/include/glib-2.0/glib/gdate.h \
 /usr/include/glib-2.0/glib/gdatetime.h \
 /usr/include/glib-2.0/glib/gtimezone.h /usr/include/glib-2.0/glib/gdir.h \
 /usr/include/dirent.h /usr/include/x86_64-linux-gnu/bits/dirent.h \
 /usr/include/glib-2.0/glib/genviron.h \
 /usr/include/glib-2.0/glib/gfileutils.h \
 /usr/include/glib-2.0/glib/ggettext.h /usr/include/glib-2.0/glib/ghash.h \
 /usr/include/glib-2.0/glib/glist.h /usr/include/glib-2.0/glib/gmem.h \
 /usr/include/glib-2.0/glib/gnode.h /usr/include/glib-2.0/glib/ghmac.h \
 /usr/include/glib-2.0/glib/gchecksum.h \
 /usr/include/glib-2.0/glib/ghook.h \
 /usr/include/glib-2.0/glib/ghostutils.h \
 /usr/include/glib-2.0/glib/giochannel.h \
 /usr/include/glib-2.0/glib/gmain.h /usr/include/glib-2.0/glib/gpoll.h \
 /usr/include/glib-2.0/glib/gslist.h /usr/include/glib-2.0/glib/gstring.h \
 /usr/include/glib-2.0/glib/gunicode.h \
 /usr/include/glib-2.0/glib/gutils.h \
 /usr/include/glib-2.0/glib/gkeyfile.h \
 /usr/include/glib-2.0/glib/gmappedfile.h \
 /usr/include/glib-2.0/glib/gmarkup.h \
 /usr/include/glib-2.0/glib/gmessages.h \
 /usr/include/glib-2.0/glib/goption.h \
 /usr/include/glib-2.0/glib/gpattern.h \
 /usr/include/glib-2.0/glib/gprimes.h /usr/include/glib-2.0/glib/gqsort.h \
 /usr/include/glib-2.0/glib/gqueue.h /usr/include/glib-2.0/glib/grand.h \
 /usr/include/glib-2.0/glib/gregex.h \
 /usr/include/glib-2.0/glib/gscanner.h \
 /usr/include/glib-2.0/glib/gsequence.h \
 /usr/include/glib-2.0/glib/gshell.h /usr/include/glib-2.0/glib/gslice.h \
 /usr/include/glib-2.0/glib/gspawn.h \
 /usr/include/glib-2.0/glib/gstrfuncs.h \
 /usr/include/glib-2.0/glib/gstringchunk.h \
 /usr/include/glib-2.0/glib/gtestutils.h \
 /usr/include/glib-2.0/glib/gthreadpool.h \
 /usr/include/glib-2.0/glib/gtimer.h \
 /usr/include/glib-2.0/glib/gtrashstack.h \
 /usr/include/glib-2.0/glib/gtree.h \
 /usr/include/glib-2.0/glib/gurifuncs.h \
 /usr/include/glib-2.0/glib/gvarianttype.h \
 /usr/include/glib-2.0/glib/gvariant.h \
 /usr/include/glib-2.0/glib/gversion.h \
 /usr/include/glib-2.0/glib/deprecated/gallocator.h \
 /usr/include/glib-2.0/glib/deprecated/gcache.h \
 /usr/include/glib-2.0/glib/deprecated/gcompletion.h \
 /usr/include/glib-2.0/glib/deprecated/gmain.h \
 /usr/include/glib-2.0/glib/deprecated/grel.h \
 /usr/include/glib-2.0/glib/deprecated/gthread.h \
 /usr/include/x86_64-linux-gnu/sys/types.h /usr/include/endian.h \
 /usr/include/x86_64-linux-gnu/bits/endian.h \
 /usr/include/x86_64-linux-gnu/bits/byteswap.h \
 /usr/include/x86_64-linux-gnu/bits/byteswap-16.h \
 /usr/include/x86_64-linux-gnu/sys/select.h \
 /usr/include/x86_64-linux-gnu/bits/select.h \
 /usr/include/x86_64-linux-gnu/sys/sysmacros.h /usr/include/pthread.h \
 /usr/include/sched.h /usr/include/x86_64-linux-gnu/bits/sched.h \
 /usr/include/x86_64-linux-gnu/bits/setjmp.h fon/msg_if.h \
 /usr/include/string.h fon/msg_defs.h fon/packet_if.h fon/table_if.h \
 fon/fon-utils.h /usr/include/stdlib.h \
 /usr/include/x86_64-linux-gnu/bits/waitflags.h \
 /usr/include/x86_64-linux-gnu/bits/waitstatus.h /usr/include/alloca.h \
 /usr/include/x86_64-linux-gnu/bits/stdlib-float.h fon/common-defs.h \
 fon/algorithm.h aco-table.h ant-type.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.8/include/stdbool.h ant.h ant-def.h \
 ant-parameter.h ant-obj.h ant-model.h main.h /usr/include/stdio.h \
 /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
 /usr/include/x86_64-linux-gnu/bits/stdio_lim.h \
 /usr/include/x86_64-linux-gnu/bits/sys_errlist.h /usr/include/assert.h
ant-model.o: ant-model.c /usr/include/stdc-predef.h ant-model.h \
 ant-type.h /usr/lib/gcc/x86_64-linux-gnu/4.8/include/stdbool.h ant-def.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.8/include/stddef.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.8/include-fixed/limits.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.8/include-fixed/syslimits.h \
 /usr/include/limits.h /usr/include/features.h \
 /usr/include/x86_64-linux-gnu/sys/cdefs.h \
 /usr/include/x86_64-linux-gnu/bits/wordsize.h \
 /usr/include/x86_64-linux-gnu/gnu/stubs.h \
 /usr/include/x86_64-linux-gnu/gnu/stubs-64.h \
 /usr/include/x86_64-linux-gnu/bits/posix1_lim.h \
 /usr/include/x86_64-linux-gnu/bits/local_lim.h \
 /usr/include/linux/limits.h \
 /usr/include/x86_64-linux-gnu/bits/posix2_lim.h ant-parameter.h
