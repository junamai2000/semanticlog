mod_logtest.la: mod_logtest.slo
	$(SH_LINK) -rpath $(libexecdir) -module -avoid-version  mod_logtest.lo
DISTCLEAN_TARGETS = modules.mk
shared =  mod_logtest.la
