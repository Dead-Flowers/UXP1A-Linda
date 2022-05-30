#include "linda/linda.h"

int main() {
    key_t key = ftok("./key.k", 2137);
    TupleSpace client;
    client.open(key);
    client.output("(123, 456.789)");
    auto _ =client.read("(integer:*, float:*)", 10);

    puts("done\n");
    return 0;
}