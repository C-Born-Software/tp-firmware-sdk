# -*-makefile-*-
#
# Copyright (C) 2017 by Marius Hellmeier (marius.hellmeier@wago.com), WAGO GmbH & Co. KG
#
# See CREDITS for details about who has contributed to this project.
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_WBM_NG) += wbm-ng

#
# Paths and names
#
WBM_NG_VERSION        := 3.4.2
WBM_NG                := wbm-pfc-$(WBM_NG_VERSION)
WBM_NG_URL            := $(call jfrog_template_to_url, WBM_NG)
WBM_NG_SUFFIX         := $(suffix $(WBM_NG_URL))
WBM_NG_SOURCE         := $(SRCDIR)/$(WBM_NG)$(WBM_NG_SUFFIX)
WBM_NG_MD5             = $(shell [ -f $(WBM_NG_MD5_FILE) ] && cat $(WBM_NG_MD5_FILE))
WBM_NG_MD5_FILE       := $(WBM_NG_SOURCE).md5
WBM_NG_ARTIFACT        = $(call jfrog_get_filename,$(WBM_NG_URL))

WBM_NG_BUILDROOT_DIR  := $(BUILDDIR)/wbm-ng
WBM_NG_DIR            := $(WBM_NG_BUILDROOT_DIR)
ifeq ($(PTXCONF_WBM),y)
WBM_NG_TARGET_DIR     := /var/www/wbm-ng
else
WBM_NG_TARGET_DIR     := /var/www/wbm
endif
WBM_NG_LICENSE        := unknown
WBM_NG_MAKE_ENV       :=

# Generated blacklist: don't install plugins which are deactivated in ptxdist menuconfig
# The blacklist contains associations between defines and plugin/folder names included
# in the wbm-pfs default plugins inside the archive.
WBM_NG_PLUGINS_BLACKLIST :=
ifndef WBM_NG_FEATURE_EXAMPLE_A
	WBM_NG_PLUGINS_BLACKLIST += wbm-plugin-example-a
endif

# ----------------------------------------------------------------------------
# Get
# ----------------------------------------------------------------------------

$(WBM_NG_SOURCE):
	@$(call targetinfo)
	$(call ptx/in-path, PTXDIST_PATH, scripts/wago/artifactory.sh) fetch \
        '$(WBM_NG_URL)' '$@' '$(WBM_NG_MD5_FILE)'

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

# use ptxdist default

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

$(STATEDIR)/wbm-ng.prepare:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

$(STATEDIR)/wbm-ng.compile:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

$(STATEDIR)/wbm-ng.install:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/wbm-ng.targetinstall:
	@$(call targetinfo)

	@$(call install_init, wbm-ng)
	@$(call install_fixup, wbm-ng, PRIORITY, optional)
	@$(call install_fixup, wbm-ng, SECTION, base)
	@$(call install_fixup, wbm-ng, AUTHOR,"Marius Hellmeier, WAGO GmbH \& Co. KG")
	@$(call install_fixup, wbm-ng, DESCRIPTION, missing)

	# create target directory itself
	@$(call install_copy, wbm-ng, 0, 0, 0755, $(WBM_NG_TARGET_DIR))

	# loop over all files and subdirectories (deep)
	# additionally apply a filter to the ./plugins folder, to filter out
	# plugins named in the blacklist
	@cd $(WBM_NG_DIR) && \
	for object in $$( find $$( printf " -path ./plugins/%s -prune -o " $(WBM_NG_PLUGINS_BLACKLIST) ) \( -path './series' -o -path './.ptxdist*' -o -path './.pc*' \) -prune -o -print ); do \
		if test -f $$object -a ! -h $$object; then \
			$(call install_copy, wbm-ng, 0, 0, 0644, $(WBM_NG_DIR)/$$object, $(WBM_NG_TARGET_DIR)/$$object); \
		elif test -d $$object -a ! -h $$object; then \
			$(call install_copy, wbm-ng, 0, 0, 0755, $(WBM_NG_TARGET_DIR)/$$object); \
		fi; \
	done;

	# install config-tools and backup-restore script for WBM related settings
	@$(call install_copy, wbm-ng, 0, 0, 0750, $(PTXDIST_WORKSPACE)/projectroot/etc/config-tools/get_wbm, /etc/config-tools/get_wbm);
	@$(call install_copy, wbm-ng, 0, 0, 0750, $(PTXDIST_WORKSPACE)/projectroot/etc/config-tools/config_wbm, /etc/config-tools/config_wbm);
	@$(call install_copy, wbm-ng, 0, 0, 0750, $(PTXDIST_WORKSPACE)/projectroot/etc/config-tools/backup-restore/backup_wbm, /etc/config-tools/backup-restore/backup_wbm);

	# register oauth client and resource server
	@$(call install_copy,        wbm-ng, 0, 124, 0770, /etc/authd)
	@$(call install_copy,        wbm-ng, 0, 124, 0770, /etc/authd/clients)
	@$(call install_copy,        wbm-ng, 0, 124, 0770, /etc/authd/resource_servers)
	@$(call install_alternative, wbm-ng, 0, 124, 0660, /etc/authd/clients/wbm.conf)
	@$(call install_alternative, wbm-ng, 0, 124, 0660, /etc/authd/resource_servers/wbm.conf)

	@$(call install_finish, wbm-ng)
	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

$(STATEDIR)/wbm-ng.clean:
	@$(call targetinfo)
	@rm -rf $(PTXCONF_SYSROOT_TARGET)$(WBM_NG_TARGET_DIR)
	@$(call clean_pkg, WBM_NG)
	@rm -rf $(WBM_NG_BUILDROOT_DIR)

# vim: syntax=make
