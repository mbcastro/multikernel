#
# MIT License
#
# Copyright (c) 2011-2018 Pedro Henrique Penna <pedrohenriquepenna@gmail.com>
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.  THE SOFTWARE IS PROVIDED
# "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
# LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
# PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
# HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
# ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
# WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#

#=============================================================================
# Mailbox Unit Test
#=============================================================================

io-bin += mailbox-master
mailbox-master-srcs := $(SRCDIR)/kernel/arch/mppa/mailbox.c \
					   $(SRCDIR)/kernel/arch/mppa/core.c    \
					   $(SRCDIR)/kernel/arch/mppa/noc.c     \
					   $(SRCDIR)/test/mailbox/master.c

# Toolchain Configuration
mailbox-master-cflags += -D_KALRAY_MPPA_256_HIGH_LEVEL -DDEBUG_MAILBOX
mailbox-master-lflags := -lmppaipc

mailbox-objs := mailbox-master 
mailbox-name := mailbox.img

#=============================================================================
# MPPA Binary
#=============================================================================

mppa-bin := mailbox

include $(K1_TOOLCHAIN_DIR)/share/make/Makefile.kalray
