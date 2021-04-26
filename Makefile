TARGET = msp430fr-flasher

all: $(TARGET)

CFLAGS = -g -I. -Wall -Wpedantic

OBJS = bsl.o main.o uart_if.o utils.o

$(TARGET): $(OBJS)
	$(CC) -o $@ $^

clean:
	rm -f $(TARGET) $(OBJS)
