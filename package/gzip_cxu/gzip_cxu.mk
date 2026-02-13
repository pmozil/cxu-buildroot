################################################################################
#
# gzip_cxu (custom fork)
#
################################################################################

GZIP_VERSION = c8e167e3e8ae3e89a6d622462b5791c2d2b35266
GZIP_SITE = https://github.com/pmozil/gzip.git
GZIP_SITE_METHOD = git
GZIP_AUTORECONF = YES

# Some other tools expect it to be in /bin
GZIP_CONF_OPTS = --exec-prefix=/

GZIP_LICENSE = GPL-3.0+
GZIP_LICENSE_FILES = COPYING
GZIP_CPE_ID_VENDOR = gnu

GZIP_CONF_ENV += gl_cv_func_fflush_stdin=yes
HOST_GZIP_CONF_ENV += gl_cv_func_fflush_stdin=yes
GZIP_CONF_ENV += ac_cv_path_shell=/bin/sh

$(eval $(autotools-package))
$(eval $(host-autotools-package))
