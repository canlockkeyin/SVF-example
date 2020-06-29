void foo(const char* bar) {
    return;
}

void other(const char* bar) {
    foo(bar);
}

int main(void) {
    foo("a");
    foo("b");
    other("c");
    return 0;
}
