APP=Blacklight
CREATOR=BLKT

all: $(APP).prc

$(APP): $(APP).c
	m68k-palmos-gcc -O2 -Wall -palmos3.1 $(APP).c -o $(APP)

$(APP).prc: $(APP)
	m68k-palmos-obj-res $(APP)
	build-prc $(APP).prc "Blacklight" $(CREATOR) *.$(APP).grc

clean:
	rm -f $(APP) $(APP).prc *.$(APP).grc
