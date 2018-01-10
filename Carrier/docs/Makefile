SPHINXOPTS    =
SPHINXBUILD   = sphinx-build
SPHINXPROJ    = ElastosCarrierAPIs
SOURCEDIR     = source
BUILDDIR      = build
DOXYGEN       = doxygen
DOXYGEN_CONF  = $(SOURCEDIR)/carrier.cfg

help:
	@$(SPHINXBUILD) -M help "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)

.PHONY: help Makefile

%: Makefile
	@if [ ! -d $(BUILDDIR) ] ; then  \
		mkdir $(BUILDDIR) ;  \
	fi	
	@if [ "$@" != "clean" ] ; then  \
		$(DOXYGEN) $(DOXYGEN_CONF) ;  \
	fi
	@$(SPHINXBUILD) -M $@ "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)
