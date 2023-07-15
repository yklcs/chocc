// #include <stdio.h>
// #include <stdlib.h>

int main(int argc, char *argv[]) {
  long n = strtol(argv[1], NULL, 0);
  for (int i = 1; i <= n; i++) {
    if (i % 15 == 0) {
      puts("fizzbuzz");
    } else if (i % 3 == 0) {
      puts("fizz");
    } else if (i % 5 == 0) {
      puts("buzz");
    } else {
      printf("%d\n", i);
    }
  }

  return 0;
}
