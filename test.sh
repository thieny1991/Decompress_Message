g++ -std=c++11 -o main main.cpp -lpthread

for i in {1..5}
do
	./main < test$i.txt
done
