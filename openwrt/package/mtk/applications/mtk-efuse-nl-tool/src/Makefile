EXEC = mtk-efuse-tool
SRC = mtk_efuse.c

$(EXEC): $(SRC)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(SRC)

all: $(EXEC)

clean:
	rm -f $(EXEC) *.o
