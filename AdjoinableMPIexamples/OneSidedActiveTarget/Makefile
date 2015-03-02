TDIRS=$(sort $(dir $(wildcard */)))

TDIRS_CHECK=$(addsuffix _all, $(TDIRS))
check : $(TDIRS_CHECK)

define generateAllDir
 $(1)_all: 
	+$(MAKE) -C $(1) 
endef   

$(foreach TDIR, $(TDIRS), $(eval $(call generateAllDir, $(TDIR))))

TDIRS_CLEAN=$(addsuffix _clean, $(TDIRS))
clean: $(TDIRS_CLEAN)

define generateCleanDir
 $(1)_clean: 
	+$(MAKE) -C $(1) clean
endef   

$(foreach TDIR, $(TDIRS), $(eval $(call generateCleanDir, $(TDIR))))

.PHONY: check $(TDIRS_CHECK) clean $(TDIRS_CLEAN)
