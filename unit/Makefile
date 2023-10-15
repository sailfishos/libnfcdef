# -*- Mode: makefile-gmake -*-

all:
%:
	@$(MAKE) -C ndef_rec $*
	@$(MAKE) -C ndef_rec_sp $*
	@$(MAKE) -C ndef_rec_t $*
	@$(MAKE) -C ndef_rec_u $*
	@$(MAKE) -C ndef_tlv $*

clean: unitclean
	rm -f *~
	rm -f common/*~
	rm -f coverage/*~
	rm -f coverage/*.gcov
	rm -fr coverage/report
