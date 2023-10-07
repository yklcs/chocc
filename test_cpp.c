#define COMPILE
#define ADD(X, Y) X + Y
#define NULL() 0
#define ONE 1
#define ONE_PLUS 1 +
#define ID(X) X
#define TEST() ID(0)
#define EMPTY
#define STR(X) #X
#define X "HELLO"
#define Z Y
#define Y X
#define HI "HI"
#undef HI
#ifdef HI
int hi = 1;
#endif
#define CAT(A, B) A##B
#define RECURSE(X) RECURSE(X)

#ifdef COMPILE

int a = ADD(1, ADD(2, 3));
int b = ONE_PLUS 2;
int *c = NULL() EMPTY;
int d = ID(TEST());
char *e = STR(a + b + c);
char *f = Z;
int g = CAT(1, 2);
int h = g CAT(<, <) 2;
int i = RECURSE(0);
char *j = HI;

#endif
