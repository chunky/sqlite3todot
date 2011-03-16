OUT=verymeta
BIN=sqlite3todot

all: clean
	gcc -o ${BIN} -lsqlite3 sqlite3todot.c
	rm -f meta.db
	cat meta.sql | sqlite3 meta.db
	./sqlite3todot meta.db meta.db > ${OUT}.dot
	sh ./testlayouts.sh ${OUT}.dot

clean:
	rm -rf *.o meta.db ${BIN} ${OUT}.* core testlayouts

