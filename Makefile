# Makefile para el Sistema de Control de Automovil en Tiempo Real
# Soporte dinamico para PC (Nativo) y Raspberry Pi (Cross-Compilacion)

# Archivos objeto
OBJS = rt_timing.o car_tasks.o main.o
LIBS = -lpthread -lrt

# Regla por defecto compila para PC
all: pc

# ==========================================
# TARGET 1: PC NATIVO
# ==========================================
pc: CC = gcc
pc: CFLAGS = -Wall -g
pc: TARGET = sistema_control_pc
pc: $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LIBS)
	@echo "-> Compilacion nativa completada: ./$(TARGET)"

# ==========================================
# TARGET 2: RASPBERRY PI (CROSS-COMPILATION)
# ==========================================
rpi: CC = arm-linux-gnueabihf-gcc
rpi: CFLAGS = -Wall -g -static
rpi: TARGET = sistema_control_rpi
rpi: $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LIBS)
	@echo "-> Cross-compilacion completada: ./$(TARGET)"

# ==========================================
# REGLAS DE COMPILACION DE OBJETOS
# ==========================================
rt_timing.o: rt_timing.c rt_timing.h
	$(CC) $(CFLAGS) -c rt_timing.c

car_tasks.o: car_tasks.c car_tasks.h rt_timing.h
	$(CC) $(CFLAGS) -c car_tasks.c

main.o: main.c rt_timing.h car_tasks.h
	$(CC) $(CFLAGS) -c main.c

# ==========================================
# LIMPIEZA
# ==========================================
clean:
	rm -f *.o sistema_control_pc sistema_control_rpi
	@echo "-> Archivos binarios limpiados."

.PHONY: all pc rpi clean