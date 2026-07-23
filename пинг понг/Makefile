# ╔═══════════════════════════════════════════════════════════════╗
# ║  Makefile — кросс-платформенная сборка (Windows / Linux)    ║
# ║  "Собери меня, если сможешь" (c) 2024                       ║
# ╚═══════════════════════════════════════════════════════════════╝

# Определяем ОС
ifeq ($(OS),Windows_NT)
    # ─── Windows (MSYS2/MinGW) ───
    CC        = gcc
    CFLAGS    = -Wall -Wextra -O2 -std=c99
    INCLUDES  = -IC:/msys64/ucrt64/include/SDL2
    LIBS      = -LC:/msys64/ucrt64/lib -lmingw32 -lSDL2main -lSDL2 -mwindows
    TARGET    = pong.exe
    RM        = rm -f
else
    # ─── Linux ───
    CC        = gcc
    CFLAGS    = -Wall -Wextra -O2 -std=c99
    INCLUDES  = $(shell pkg-config --cflags sdl2)
    LIBS      = $(shell pkg-config --libs sdl2)
    TARGET    = pong
    RM        = rm -f
endif

SRC       = pong.c

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(INCLUDES) $(SRC) -o $(TARGET) $(LIBS)

clean:
	$(RM) $(TARGET)

run: $(TARGET)
	./$(TARGET)

# Эпилог:
# "Сборка прошла успешно. Теперь запусти 'make run' и наслаждайся."
# "Если не работает — проверь, установлена ли SDL2. Если и это не
#  помогло — иди гулять."

