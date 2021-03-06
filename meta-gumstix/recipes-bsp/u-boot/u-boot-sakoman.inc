DESCRIPTION = "U-Boot - the Universal Boot Loader"
HOMEPAGE = "http://www.denx.de/wiki/U-Boot/WebHome"
SECTION = "bootloaders"
PRIORITY = "optional"
LICENSE = "GPLv2+"
LIC_FILES_CHKSUM = "file://COPYING;md5=1707d6db1d42237583f50183a5651ecb"

PROVIDES = "virtual/bootloader"

DEPENDS = "mtd-utils"

PACKAGE_ARCH = "${MACHINE_ARCH}"
PARALLEL_MAKE=""

EXTRA_OEMAKE = "CROSS_COMPILE=${TARGET_PREFIX}"

UBOOT_MACHINE ?= "${MACHINE}_config"
UBOOT_BINARY ?= "u-boot.img"
UBOOT_IMAGE ?= "u-boot-${MACHINE}-${PV}-${PR}.img"
UBOOT_SYMLINK ?= "u-boot-${MACHINE}.img"
UBOOT_MAKE_TARGET ?= "all"

do_configure () {
	oe_runmake ${UBOOT_MACHINE}
}

do_compile () {
	unset LDFLAGS
	unset CFLAGS
	unset CPPFLAGS
	oe_runmake ${UBOOT_MAKE_TARGET}
	oe_runmake tools env HOSTCC="${CC}" HOSTSTRIP="${STRIP}"
}

do_install () {
	install -d ${D}/boot
	install ${S}/${UBOOT_BINARY} ${D}/boot/${UBOOT_IMAGE}
	ln -sf ${UBOOT_IMAGE} ${D}/boot/${UBOOT_BINARY}

	if [ -e ${WORKDIR}/fw_env.config ] ; then
	    install -d ${D}${base_sbindir}
		install -d ${D}${sysconfdir}
		install -m 644 ${WORKDIR}/fw_env.config ${D}${sysconfdir}/fw_env.config
		install -m 755 ${S}/tools/env/fw_printenv ${D}${base_sbindir}/fw_printenv
		install -m 755 ${S}/tools/env/fw_printenv ${D}${base_sbindir}/fw_setenv
	fi

}

FILES_${PN} = "/boot"
# no gnu_hash in uboot.bin, by design, so skip QA
INSANE_SKIP_${PN} = "True"

PACKAGES += "${PN}-fw-utils"
FILES_${PN}-fw-utils = "${sysconfdir} ${base_sbindir}"
# u-boot doesn't use LDFLAGS for fw files, needs to get fixed, but until then:
INSANE_SKIP_${PN}-fw-utils = "True"

inherit deploy

addtask deploy before do_package after do_compile

do_deploy () {
	install -d ${DEPLOY_DIR_IMAGE}
	install ${S}/${UBOOT_BINARY} ${DEPLOY_DIR_IMAGE}/${UBOOT_IMAGE}

	cd ${DEPLOY_DIR_IMAGE}
	rm -f ${UBOOT_SYMLINK}
	ln -sf ${UBOOT_IMAGE} ${UBOOT_SYMLINK}
	ln -sf ${UBOOT_IMAGE} ${UBOOT_BINARY}

	if [ -e ${S}/MLO ] ; then
		install ${S}/MLO ${DEPLOY_DIR_IMAGE}/MLO-${MACHINE}-${PV}
		rm -f ${DEPLOY_DIR_IMAGE}/MLO-${MACHINE} ${DEPLOY_DIR_IMAGE}/MLO
		ln -sf MLO-${MACHINE}-${PV} ${DEPLOY_DIR_IMAGE}/MLO-${MACHINE}
		ln -sf MLO-${MACHINE}-${PV} ${DEPLOY_DIR_IMAGE}/MLO
	fi
}
