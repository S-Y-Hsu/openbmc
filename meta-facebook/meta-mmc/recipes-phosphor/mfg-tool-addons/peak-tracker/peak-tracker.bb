SUMMARY = "MFG Peak Voltage Tracker"
DESCRIPTION = "Exposes a fixed peak voltage value on D-Bus."
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10"

inherit meson systemd pkgconfig

DEPENDS += "sdbusplus boost"

SRC_URI = " \
    file://main.cpp \
    file://meson.build \
    file://mfg-peak-tracker.service \
"

S = "${UNPACKDIR}"

SYSTEMD_SERVICE:${PN} = "mfg-peak-tracker.service"

do_install:append() {
    install -d ${D}${systemd_system_unitdir}
    install -m 0644 ${UNPACKDIR}/mfg-peak-tracker.service ${D}${systemd_system_unitdir}
}