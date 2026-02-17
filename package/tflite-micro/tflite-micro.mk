################################################################################
#
# tflite-micro
#
################################################################################

TFLITE_MICRO_VERSION = main
TFLITE_MICRO_SITE = $(call github,tensorflow,tflite-micro,$(TFLITE_MICRO_VERSION))
TFLITE_MICRO_LICENSE = Apache-2.0
TFLITE_MICRO_LICENSE_FILES = LICENSE

TFLITE_MICRO_INSTALL_STAGING = YES
TFLITE_MICRO_INSTALL_TARGET = NO

ifeq ($(BR2_RISCV_64),y)
TFLITE_MICRO_ARCH = riscv64
else
TFLITE_MICRO_ARCH = riscv32
endif

TFLITE_MICRO_MAKE_OPTS = \
	TARGET=linux \
	TARGET_ARCH=$(TFLITE_MICRO_ARCH) \
	CC="$(TARGET_CC)" \
	CXX="$(TARGET_CXX)" \
	AR="$(TARGET_AR)"

define TFLITE_MICRO_BUILD_CMDS
	# Remove auxiliary directories
	rm -rf $(@D)/tensorflow/lite/micro/examples
	rm -rf $(@D)/tensorflow/lite/micro/integration_tests
	rm -rf $(@D)/tensorflow/lite/micro/benchmarks
	rm -rf $(@D)/tensorflow/lite/micro/models

	# Neuter the explicitly included tests file so Make ignores it
	echo "" > $(@D)/tensorflow/lite/micro/tools/make/tests.inc

	# Build the core library
	$(TARGET_MAKE_ENV) env -u GIT_DIR $(MAKE) -C $(@D) \
		-f tensorflow/lite/micro/tools/make/Makefile \
		$(TFLITE_MICRO_MAKE_OPTS) \
		microlite
endef

define TFLITE_MICRO_INSTALL_STAGING_CMDS
	$(INSTALL) -D -m 0644 $(@D)/gen/*/lib/libtensorflow-microlite.a \
		$(STAGING_DIR)/usr/lib/libtensorflow-microlite.a

	cd $(@D) && find tensorflow signal -type f -name "*.h" -exec \
		$(INSTALL) -D -m 0644 {} $(STAGING_DIR)/usr/include/{} \;
endef
$(eval $(generic-package))
