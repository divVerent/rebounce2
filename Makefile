# makefile for Rebounce 2
# supported targets: DOS, Linux

.PHONY: help
help:
	@echo Possible make targets:
#	@echo  "  game		(everything in the executable)"
#	@echo  "  dist          (create distribution)"
	@echo  "  datgame	(everything in the dat file)"
	@echo  "  datdist       (create distribution)"
	@echo  "  debuggame	(everything singular files)"
	@echo  "  clean		(remove all rebuildable files)"
	@echo  "  distclean	(remove the executable and configuration, too - BE WARNED)"
	@echo  "  zip		(create a zip archive containing the game using Info-ZIP)"
	@echo  "  pkzip		(create a zip archive containing the game using PKZIP)"
	@echo  "  z		(create a z archive containing the game)"
	@echo  "  gz		(create a gz file containing the game)"
	@echo  "  rarsfx	(create a RAR SFX containing the game)"
	@echo  "  editor	(the leveleditor)"

sources_wo_def = animate.cpp border.cpp coll.cpp dialogs.cpp \
                 enemies.cpp entity.cpp gates.cpp io.cpp keys.cpp main.cpp \
                 map.cpp milli.cpp objects.cpp player.cpp rebounce.cpp \
                 sensors.cpp sound.cpp sprite.cpp world.cpp

def = defs.cpp

objects = $(sources_wo_def:.cpp=.o)
makefiles = $(sources_wo_def:.cpp=.mak) $(def:.cpp=.mak)

sources := $(sources) $(def)

objects_manyfiles = $(objects) $(def:.cpp=.o)
objects_twofiles = $(objects) usedat.o
objects_allinone = $(objects) inexe.o

include config.out
include $(makefiles)

%.mak: %.cpp
	$(CXX) -MM $(CPPFLAGS) $< | perl sed.pl "s/$*\.o[ :]*/$*.o $*.inexe.o $*.usedat.o $@ : /g" > $*.temp
ifdef HOSTUNIX
	[ -s $*.temp ] && cp $*.temp $@ || $(RM) $@
else
	copy $*.temp $@
endif
	$(RM) $*.temp

usedat.o: $(def)
	$(CXX) -c $< -DALLINONE $(CPPFLAGS) $(CXXFLAGS) -o $@

inexe.o: $(def)
	$(CXX) -c $< -DINEXE $(CPPFLAGS) $(CXXFLAGS) -o $@

.PHONY: tags
tags:
	-ctags -I OBJECT=class,EXTEND=class *.cpp *.h || rm -f tags
	
.PHONY: debuggame
debuggame: tags $(objects_manyfiles)
	$(CXX) $(objects_manyfiles) -o rebounce2$(EXEC_SUFFIX) $(LDFLAGS) 

.PHONY: datgame
datgame: tags rebounce2.dat $(objects_twofiles)
	$(CXX) $(objects_twofiles) -o rebounce2$(EXEC_SUFFIX) $(LDFLAGS) 

# DOES NOT WORK (needs patches.dat anyway)
#.PHONY: game
#game: tags rebounce2.dat $(objects_allinone)
#	$(CXX) $(objects_allinone) -o rebounce2$(EXEC_SUFFIX) $(LDFLAGS) 
#	exedat -ac rebounce2$(EXEC_SUFFIX) rebounce2.dat

.PHONY: clean
clean:
	$(RM) *.o rebounce2.dat rebounce2 rebounce2.exe editor editor.exe

.PHONY: distclean
distclean: clean
	$(RM) rebounce2$(EXEC_SUFFIX) rebounce2.zip rebonce2.gz rebounce2.bz2 editor$(EXEC_SUFFIX) *.mak config.out
	$(RM) rebounce.sav scores.htm

rebounce2.dat: *.pcx *.map *.ent *.mid *.txt *.wav
	$(RM) rebounce2.dat
	dat rebounce2.dat -t DATA -a -c0 -k *.pcx *.map *.ent *.mid *.txt *.wav

# DOES NOT WORK (digmid does not look there)
#rebounce2-big.dat: *.pcx *.map *.ent *.mid *.txt *.wav patches.dat
#	cp patches.dat rebounce2-big.dat
#	dat rebounce2-big.dat -t DATA -a -c0 -k *.pcx *.map *.ent *.mid *.txt *.wav

patches.dat: default.cfg *.mid
	$(RM) patches.dat
	pat2dat patches.dat default.cfg *.mid

# DOES NOT WORK (needs patches.dat anyway)
#dist: game
#	rm -rf dist/rebounce2-$(OS)-standalone* || true
#	mkdir dist || true
#	mkdir dist/rebounce2-$(OS)-standalone || true
#	cp rebounce2$(EXEC_SUFFIX) dist/rebounce2-$(OS)-standalone
#	cp dll/$(OS)/* dist/rebounce2-$(OS)-standalone || true
#	cp patches.dat dist/rebounce2-$(OS)-standalone
#	cd dist/rebounce2-$(OS)-standalone; $(ZIPPER) ../rebounce2-$(OS)-standalone-withpatches.$(ZIPEXT) *

VERSION := $(shell grep VERSION main.cpp | head -n 1 | cut -d '"' -f 2)
datdist: datgame
	rm -rf dist/rebounce2-$(VERSION)-$(OS)* || true
	mkdir dist || true
	mkdir dist/rebounce2-$(OS) || true
	cp rebounce2$(EXEC_SUFFIX) dist/rebounce2-$(OS)
	cp rebounce2.dat dist/rebounce2-$(OS)
	cp dll/$(OS)/* dist/rebounce2-$(OS) || true
	cp patches.dat dist/rebounce2-$(OS)
	cd dist/rebounce2-$(OS); $(ZIPPER) ../rebounce2-$(VERSION)-$(OS).$(ZIPEXT) *

upx: upxgame
	@echo Nothing more to do

editor-def.o:
	$(CXX) -c defs.cpp -DEDITOR $(CPPFLAGS) $(CXXFLAGS) -o editor-def.o

objects_editor = editor-def.o editor.o dialogs.o io.o map.o milli.o sound.o sprite.o
editor: $(objects_editor)
	$(CXX) $(objects_editor) -o editor$(EXEC_SUFFIX) $(LDFLAGS)

trouble:
	@echo Never do this again!
	@echo -n 'bash$$ '
	@perl -pe '$$_ = "IGNORE" for values %SIG; $$_="" unless /^make trouble$$/; $$_ and exit; print "bash\$$ "'
