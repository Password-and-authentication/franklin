


int strlen(char *s) {
    int n = 0;
    while (++n && s[n]);
    return n;
}