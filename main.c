

#define asm __asm__

extern void
exec(char* name);

int
main(int argc, char** argv)
{
  exec("/init");
  int xx = 10 / 0;
  asm("nop");
  asm("nop");
  asm("nop");
  int x = argc;
  int c = *argv[0];
  return 0;
}
