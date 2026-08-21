FROM ubuntu:18.04

# home directory
ENV HOME /home/me

# install prerequisite packages
RUN apt-get update && apt-get -y install build-essential automake git libncurses5-dev texinfo expat libexpat1-dev wget

WORKDIR ${HOME}
RUN git clone https://github.com/CS-F372-OS/pintos.git

# build toolchain
ENV SWD=/home/me/toolchain
ENV PINTOS_ROOT=/home/me/pintos
RUN mkdir -p $SWD 
RUN cd $PINTOS_ROOT && src/misc/toolchain-build.sh $SWD

# install qemu
RUN apt-get update && apt-get install -y qemu

# build Bochs from source
RUN apt-get install -y libx11-dev libxrandr-dev
RUN cd ${PINTOS_ROOT} && src/misc/bochs-2.6.2-build.sh $SWD/x86_64

# build pintos utility tools
ENV dest=$SWD/x86_64
RUN cd $PINTOS_ROOT/src/utils &&\
    make &&\
    cp backtrace pintos Pintos.pm pintos-gdb pintos-set-cmdline pintos-mkdisk setitimer-helper squish-pty squish-unix $dest/bin &&\
    mkdir $dest/misc &&\
    cp ../misc/gdb-macros $dest/misc

# install cgdb
RUN apt-get install -y cgdb

# add toolchain into path
RUN echo PATH=/home/me/toolchain/x86_64/bin:$PATH >> ~/.bashrc

# remove the pintos dir
RUN rm -rf /home/me/pintos

# docker will end executing immediatedly without this
CMD ["sleep", "infinity"]
