PYTORCH_VERSION = v2.2.0
PYTORCH_SITE = https://github.com/pytorch/pytorch
PYTORCH_SITE_METHOD = git

PYTORCH_DEPENDENCIES = python3 python-numpy openblas protobuf

define PYTORCH_BUILD_CMDS
    cd $(@D) && python3 setup.py build
endef

define PYTORCH_INSTALL_TARGET_CMDS
    cd $(@D) && python3 setup.py install --root=$(TARGET_DIR)
endef

$(eval $(generic-package))
