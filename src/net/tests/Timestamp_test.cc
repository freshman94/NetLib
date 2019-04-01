#include <net/Timestamp.h>
#include <vector>
#include <stdio.h>

void passByConstReference(const Timestamp& x) {
	printf("%s\n", x.toString().c_str());
}

void passByValue(Timestamp x) {
	printf("%s\n", x.toString().c_str());
}

void benchmark() {
	const int num = 1000 * 1000;

	std::vector<Timestamp> stamps;
	stamps.reserve(num);
	for (int i = 0; i < num; ++i)
		stamps.push_back(Timestamp::now());
	printf("%s\n", stamps.front().toString().c_str());
	printf("%s\n", stamps.back().toString().c_str());
	printf("%f\n", timeDifference(stamps.back(), stamps.front()));

}

int main(){
	Timestamp now(Timestamp::now());
	printf("%s\n", now.toString().c_str());
	passByValue(now);
	passByConstReference(now);
	benchmark();
}

