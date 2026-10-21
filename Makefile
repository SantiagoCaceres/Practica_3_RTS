# Makefile para el Sistema de Control de Automovil en Tiempo Real
# Compilacion modular utilizando GCC, POSIX Threads y POSIX Real-Time

CC = gcc
CFLAGS = -Wall -g
LIBS = -lpthread -lrt

# Nombre del ejecutable final
TARGET = sistema_control

# Archivos objeto
OBJS = rt_timing.o car_tasks.o main.o

# Regla principal
all: $(TARGET)

# Enlace de los objetos para generar el ejecutable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

# Compilacion de cada modulo individual
rt_timing.o: rt_timing.c rt_timing.h
	$(CC) $(CFLAGS) -c rt_timing.c $(LIBS)

car_tasks.o: car_tasks.c car_tasks.h rt_timing.h
	$(CC) $(CFLAGS) -c car_tasks.c $(LIBS)

main.o: main.c rt_timing.h car_tasks.h
	$(CC) $(CFLAGS) -c main.c $(LIBS)

# Regla de limpieza para eliminar ejecutables y archivos .o
clean:
	rm -f *.o $(TARGET)

.PHONY: all clean