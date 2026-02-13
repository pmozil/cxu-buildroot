CXU_TEST_SITE = $(TOPDIR)/package/foo/src

CXU_TEST_SITE_METHOD = local
CXU_TEST_LICENSE = MIT

CXU_TEST_BUILD_CMDS = $(MAKE) -C $(@D)
CXU_TEST_INSTALL_TARGET_CMDS = \
	$(INSTALL) -D -m 0755 $(@D)/foo $(TARGET_DIR)/usr/bin/foo

$(eval $(generic-package))

