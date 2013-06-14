.PHONY: all clean mapSort parQuick

all: mapSort parQuick

mapSort:
	mpic++ -O2 -o mapSort mapSort.cpp

parQuick:
	mpic++ -O2 -o parQuick parQuick.cpp

clean:
	rm parQuick mapSort
