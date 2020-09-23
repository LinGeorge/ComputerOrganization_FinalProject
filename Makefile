hds: Main.o
	g++ Main.o -o hds

Main.o: Main.cpp
	g++ -c Main.cpp

clean:
	rm *.o hds