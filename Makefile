ALLEGRO_VERSION=5.0.10
MINGW_VERSION=4.7.0
FOLDER=C:

FOLDER_NAME=\allegro-$(ALLEGRO_VERSION)-mingw-$(MINGW_VERSION)
PATH_ALLEGRO=$(FOLDER)$(FOLDER_NAME)
LIB_ALLEGRO=\lib\liballegro-$(ALLEGRO_VERSION)-monolith-mt.a
INCLUDE_ALLEGRO=\include

all: formasc2.exe formasc.exe

formasc.exe: formasc.o
	gcc -o formasc.exe formasc.o $(PATH_ALLEGRO)$(LIB_ALLEGRO)

formasc.o: formasc.c
	gcc -I $(PATH_ALLEGRO)$(INCLUDE_ALLEGRO) -c formasc.c	

formasc2.exe: formasc2.o
	gcc -o formasc2.exe formasc2.o $(PATH_ALLEGRO)$(LIB_ALLEGRO)

formasc2.o: formasc2.c
	gcc -I $(PATH_ALLEGRO)$(INCLUDE_ALLEGRO) -c formasc2.c	
	

clean:
	del formasc.o
	del formasc.exe
	del formasc2.o
	del formasc2.exe
