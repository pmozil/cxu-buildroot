################################################################################
#
# gzip_cxu (custom fork)
#
################################################################################

GZIP_CXU_VERSION = c8e167e3e8ae3e89a6d622462b5791c2d2b35266
GZIP_CXU_SITE = https://github.com/pmozil/gzip.git
GZIP_CXU_SITE_METHOD = git
GZIP_CXU_AUTORECONF = YES
GZIP_CXU_GIT_SUBMODULES = YES

define GZIP_CXU_PRE_CONFIGURE_HOOK
	cd $(@D) && ./bootstrap --gnulib-srcdir=./gnulib/
endef
GZIP_CXU_PRE_CONFIGURE_HOOKS += GZIP_CXU_PRE_CONFIGURE_HOOK

# Some other tools expect it to be in /bin
GZIP_CXU_CONF_OPTS = --exec-prefix=/

GZIP_CXU_LICENSE = GPL-3.0+
GZIP_CXU_LICENSE_FILES = COPYING
GZIP_CXU_CPE_ID_VENDOR = gnu

GZIP_CXU_CONF_ENV += gl_cv_func_fflush_stdin=yes
HOST_GZIP_CXU_CONF_ENV += gl_cv_func_fflush_stdin=yes
GZIP_CXU_CONF_ENV += ac_cv_path_shell=/bin/sh

$(eval $(autotools-package))
$(eval $(host-autotools-package))
