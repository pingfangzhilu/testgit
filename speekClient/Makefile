#PATH=/opt/buildroot-gcc463/usr/bin/
#CROSS_COMPILE	=$(PATH)mipsel-linux-

#NDK_PATH=/opt/android-ndk-r9d/toolchains/arm-linux-androideabi-4.8/prebuilt/linux-x86/bin
#CROSS_COMPILE=$(NDK_PATH)/arm-linux-androideabi-
#NDK_INC=/opt/android-ndk-r9d/platforms/android-9/arch-arm/usr/include/
#NDK_LIB=/opt/android-ndk-r9d/platforms/android-9/arch-arm/usr/lib/

CCC=$(CROSS_COMPILE)gcc


MY_LIB=-lbase 

CFLAGS = -Wall -I ./include  
LDFLAGS= $(MY_LIB) -L $(NDK_LIB) -lpthread     

TAR = client
all +=main.o

all +=network.o
all +=event.o

export CC
$(TAR): $(all)
	$(CCC) $(CFLAGS) -o $(TAR) $(all) $(LDFLAGS)
	$(RM) -f *.gch *.bak $(all) 
	
%.o:%.c
	$(CCC) $(CFLAGS) $(LDFLAGS) -c -o $@ $< 

.PHONY: clean
clean:
	rm -f $(TAR) $(all) 
