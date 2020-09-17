all:
	g++ -o client client.cpp -std=c++11
	g++ -o serverA serverA.cpp -std=c++11
	g++ -o serverB serverB.cpp -std=c++11
	g++ -o aws aws.cpp -std=c++11

.PHONY: serverA
serverA:
	./serverA
.PHONY: serverB
serverB:
	./serverB
.PHONY: aws
aws:
	./aws
