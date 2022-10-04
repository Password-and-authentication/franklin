

#define asm __asm__

int
main(int argc, char** argv)
{
  asm("nop");
  asm("nop");
  asm("nop");
  int x = argc;
  int c = *argv[0];
  return 0;
}
