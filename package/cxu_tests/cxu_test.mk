################################################################################
#
# cxu_test
#
################################################################################

CXU_TEST_VERSION = 0.0.1
CXU_TEST_SITE = $(BR2_GNU_MIRROR)/bc
CXU_TEST_DEPENDENCIES = host-flex
CXU_TEST_LICENSE = GPL-2.0+, LGPL-2.1+
CXU_TEST_LICENSE_FILES = COPYING COPYING.LIB
CXU_TEST_CPE_ID_VENDOR = gnu
CXU_TEST_CONF_ENV = MAKEINFO=true

CXU_TEST_AUTORECONF = YES

$(eval $(autotools-package))
