################################################################################
#
# cxu-tfmicro-demo
#
################################################################################

CXU_TFMICRO_DEMO_VERSION = 1.0
CXU_TFMICRO_DEMO_SITE = $(CXU_TFMICRO_DEMO_PKGDIR)/src
CXU_TFMICRO_DEMO_SITE_METHOD = local

CXU_TFMICRO_DEMO_DEPENDENCIES = tflite-micro

$(eval $(cmake-package))
