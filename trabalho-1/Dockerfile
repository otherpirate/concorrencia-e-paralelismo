FROM gcc:latest
COPY . /usr/src/myapp
WORKDIR /usr/src/myapp
RUN gcc -Wall -o trab -lpthread -lrt trab-1.c
CMD ["./trab"]
