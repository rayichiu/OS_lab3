mmu: mmu.cpp
	g++ -std=c++11 mmu.cpp PageAlgo.cpp Process.cpp -o mmu

clean:
	rm -f mmu *~




