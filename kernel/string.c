


int strlen(char *s) {
    int n = 0;
    while (++n && s[n]);
    return n;
}


void print(char *);

void itoa(int x, char* s) {
    int r, i = 0, arr[10];

    do {
        r = x % 10;
        arr[i] = r;
    } while (++i && (x /= 10));
    r = 0;
    while (i--) {
        s[r++] = arr[i] + 48;
    }
}