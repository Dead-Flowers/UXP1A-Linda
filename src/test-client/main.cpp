#include "linda.h"

int main() {
    key_t key = ftok(KET_FILE_PATH, PROJECT_ID);
    TupleSpace client;
    client.open(key);
    client.output("(integer:123, float:456.789)");
    auto _ =client.read("(integer:*, float:*)", 10);

    puts("done\n");
    return 0;
}