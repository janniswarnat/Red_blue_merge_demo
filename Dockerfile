FROM fehlfarbe/theli:latest

RUN sudo apt-get update && sudo apt-get install -y cmake libboost-thread-dev libboost-system-dev libmpfr-dev freeglut3-dev

RUN sudo mkdir /cgal
WORKDIR /CGAL
#RUN sudo wget https://github.com/CGAL/cgal/releases/download/releases%2FCGAL-4.7/CGAL-4.7.tar.gz
RUN sudo wget https://github.com/CGAL/cgal/releases/download/releases%2FCGAL-4.6.3/CGAL-4.6.3.tar.xz
#RUN sudo tar -xvf ./CGAL-4.7.tar.gz
RUN sudo tar -xvf ./CGAL-4.6.3.tar.xz
#WORKDIR /CGAL/CGAL-4.7
WORKDIR /CGAL/CGAL-4.6.3
RUN sudo cmake .
RUN sudo make

RUN pwd
RUN ls $CGAL_DIR
#COPY . /CGAL/CGAL-4.7/demo/Red_blue_merge_demo
COPY . /CGAL/CGAL-4.6.3/demo/Red_blue_merge_demo
#WORKDIR /CGAL/CGAL-4.7/demo/Red_blue_merge_demo
WORKDIR /CGAL/CGAL-4.6.3/demo/Red_blue_merge_demo
#RUN sudo cmake -DCGAL_DIR=/CGAL/CGAL-4.7 .
RUN sudo cmake -DCGAL_DIR=/CGAL/CGAL-4.6.3 .
RUN sudo make

CMD ./redblue