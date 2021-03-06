/**
 * FFI - Foreign Function Interface
 *
 * A ForeignFunctionInterface (FFI) is an interface that allows calling code written
 * in one programming language, from another that is neither a superset nor a subset.
 *
 * тут у нас реализация ffi механизма. примеры в lib/opengl.scm, lib/sqlite.scm, etc
 *
 * FFI is Fatal Familial Insomnia too. Please, don't read this code at night...
 */

// Libc and Unicode: http://www.tldp.org/HOWTO/Unicode-HOWTO-6.html
//
// The Plan9 operating system, a variant of Unix, uses UTF-8 as character encoding
//   in all applications. Its wide character type is called `Rune', not `wchar_t'.
//
// Design Issues for Foreign Function Interfaces
// http://autocad.xarch.at/lisp/ffis.html

#if OLVM_FFI

#ifndef _WIN32
#include <alloca.h>
#endif

#define TVOID         (48)
//efine TSTRING       (3)
//efine TSTRINGWIDE   (22)
//efine TBVEC         (19) // todo: remove this (use fft-vptr or fft-void* is same)

#define TUNKNOWN      (62) // only for ffi, direct sending argument without processing
#define TANY          (63) //

// ffi type system
#define TFLOAT        (46) // '.' symbol
#define TDOUBLE       (47) // '/' symbol

// 50 is free
#define TINT8         (50)
#define TINT16        (51)
#define TINT32        (52)
#define TINT64        (53)
// 54 for 128 ?
// 55 for 256 ?

#define TUINT8        (55)
#define TUINT16       (56)
#define TUINT32       (57)
#define TUINT64       (58)
// 59 for 128 ?
// 60 for 256 ?


#define TMASK      0x0FFF

#define TCDECL     0x1000
#define TSTDCALL   0x2000
#define TFASTCALL  0x3000

#define FFT_PTR   0x10000
#define FFT_REF   0x20000

word d2ol(struct ol_t* ol, double v); // declared in olvm.c

// C preprocessor trick, some kind of "map"
// http://jhnet.co.uk/articles/cpp_magic !!
// http://stackoverflow.com/questions/319328/writing-a-while-loop-in-the-c-preprocessor
#define FIRST(a, ...) a
#define SECOND(a, b, ...) b
#define EMPTY()

#define EVAL(...)     EVAL1024(__VA_ARGS__)
#define EVAL1024(...) EVAL512(EVAL512(__VA_ARGS__))
#define EVAL512(...)  EVAL256(EVAL256(__VA_ARGS__))
#define EVAL256(...)  EVAL128(EVAL128(__VA_ARGS__))
#define EVAL128(...)  EVAL64(EVAL64(__VA_ARGS__))
#define EVAL64(...)   EVAL32(EVAL32(__VA_ARGS__))
#define EVAL32(...)   EVAL16(EVAL16(__VA_ARGS__))
#define EVAL16(...)   EVAL8(EVAL8(__VA_ARGS__))
#define EVAL8(...)    EVAL4(EVAL4(__VA_ARGS__))
#define EVAL4(...)    EVAL2(EVAL2(__VA_ARGS__))
#define EVAL2(...)    EVAL1(EVAL1(__VA_ARGS__))
#define EVAL1(...)    __VA_ARGS__

#define DEFER1(m) m EMPTY()
#define DEFER2(m) m EMPTY EMPTY()()
#define DEFER3(m) m EMPTY EMPTY EMPTY()()()
#define DEFER4(m) m EMPTY EMPTY EMPTY EMPTY()()()()

#define IS_PROBE(...) SECOND(__VA_ARGS__, 0)
#define PROBE() ~, 1

#define CAT(a,b) a ## b

#define NOT(x) IS_PROBE(CAT(_NOT_, x))
#define _NOT_0 PROBE()

#define BOOL(x) NOT(NOT(x))

#define IF_ELSE(condition) _IF_ELSE(BOOL(condition))
#define _IF_ELSE(condition) CAT(_IF_, condition)

#define _IF_1(...) __VA_ARGS__ _IF_1_ELSE
#define _IF_0(...)             _IF_0_ELSE

#define _IF_1_ELSE(...)
#define _IF_0_ELSE(...) __VA_ARGS__

#define HAS_ARGS(...) BOOL(FIRST(_END_OF_ARGUMENTS_ __VA_ARGS__)())
#define _END_OF_ARGUMENTS_() 0

#define MAP(m, first, ...)           \
  m(first)                           \
  IF_ELSE(HAS_ARGS(__VA_ARGS__))(    \
    DEFER2(_MAP)()(m, __VA_ARGS__)   \
  )()
#define _MAP() MAP

#define NEWLINE(x) x "\n\t"
#define __ASM__(...) __asm__(EVAL(MAP(NEWLINE, __VA_ARGS__)))

// platform defines:
// https://sourceforge.net/p/predef/wiki/Architectures/
//
// buildin assembly:
// http://locklessinc.com/articles/gcc_asm/
// http://www.agner.org/optimize/calling_conventions.pdf
// https://en.wikibooks.org/wiki/Embedded_Systems/Mixed_C_and_Assembly_Programming

// http://www.angelcode.com/dev/callconv/callconv.html
#if __amd64__ // x86-64 (LP64/LLP64)

// value returned in the rax
# if _WIN64 // Windows
// The x64 Application Binary Interface (ABI) uses a four register fast-call
// calling convention by default. Space is allocated on the call stack as a
// shadow store for callees to save those registers. There is a strict one-to-one
// correspondence between the arguments to a function call and the registers
// used for those arguments. Any argument that doesn’t fit in 8 bytes, or is
// not 1, 2, 4, or 8 bytes, must be passed by reference. There is no attempt
// to spread a single argument across multiple registers. The x87 register
// stack is unused. It may be used by the callee, but must be considered volatile
// across function calls. All floating point operations are done using the 16
// XMM registers. Integer arguments are passed in registers RCX, RDX, R8, and
// R9. Floating point arguments are passed in XMM0L, XMM1L, XMM2L, and XMM3L.
// 16-byte arguments are passed by reference. Parameter passing is described
// in detail in Parameter Passing. In addition to these registers, RAX, R10,
// R11, XMM4, and XMM5 are considered volatile. All other registers are non-volatile.
//
// The caller is responsible for allocating space for parameters to the callee,
// and must always allocate sufficient space to store four register parameters,
// even if the callee doesn’t take that many parameters.

// https://msdn.microsoft.com/en-us/library/9z1stfyw.aspx
// rcx, rdx, r8, r9 (xmm0, xmm1, ..., xmm3) are arguments,
// RAX, R10, R11, XMM4, and XMM5 are considered volatile

// http://www.gamasutra.com/view/news/171088/x64_ABI_Intro_to_the_Windows_x64_calling_convention.php

// rcx - argv
// edx - argc
// r8  - function
// r9d - type
long long x64_call(word argv[], long argc, void* function, long type);

__ASM__("x64_call:_x64_call:",  // "int $3",
	"pushq %rbp",
	"movq  %rsp, %rbp",

	"pushq %r9",
	"andl  $-16, %esp", // выровняем стек по 16-байтовой границе

	// get count of arguments
	"xor   %rax, %rax",
	"movl  %edx, %eax",

	// get last parameter
	"leaq  -8(%rcx, %rax,8), %r10",
	"subq  $4, %rax",
	"jbe   4f",

	// довыровняем стек, так как:
	//  The stack pointer must be aligned to 16 bytes in any region of code
	//  that isn’t part of an epilog or prolog, except within leaf functions.
	"movq  %rax, %rdx",
	"andq  $1, %rdx",
	"leaq  -16(%rsp,%rdx,8), %rsp",

"1:",
	"pushq (%r10)",
	"subq  $8, %r10",
	"decq  %rax",
	"jnz   1b",

	// 4. заполним обычные rcx, rdx, ... не проверяя количество аргументов и их
	// тип, так будет быстрее
"4:",
	"movq  %r8, %rax",
	"movsd 24(%rcx), %xmm3", "mov  24(%rcx), %r9",
	"movsd 16(%rcx), %xmm2", "mov  16(%rcx), %r8",
	"movsd  8(%rcx), %xmm1", "mov  8(%rcx), %rdx",
	"movsd  0(%rcx), %xmm0", "mov  0(%rcx), %rcx",
	"subq $32, %rsp", // extra free space for call
	"call *%rax",
	// вернем результат
	"cmpl $46, -8(%rbp)", // TFLOAT
	"je   51f",
	"cmpl $47, -8(%rbp)", // TDOUBLE
	"je   52f",
"9:",
	"leave",
	"ret",

"51:",
	"cvtss2sd %xmm0, %xmm0", // float->double
"52:",
	"movsd %xmm0, (%rsp)",
	"pop   %rax", // можно попасть, так как уже пушнули один r9 вверху (оптимизация)
	"jmp   9b");

# else      // System V (unix, linux, osx)
// rdi: argv[]
// rsi: ad[]
// edx: i
// ecx: d
// r8:  mask
// r9: function
// 16(rbp): type
long long x64_call(word argv[], double ad[], long i, long d, long mask, void* function, long type);

__ASM__("x64_call:_x64_call:", //"int $3",
	"pushq %rbp",
	"movq  %rsp, %rbp",

	"pushq %r9",
	"andq  $-16, %rsp",
	// 1. если есть флоаты, то заполним их
"1:",
//	"testq %rcx, %rcx",
//	"jz    2f",
	"movsd  0(%rsi), %xmm0",
	"movsd  8(%rsi), %xmm1",
	"movsd 16(%rsi), %xmm2",
	"movsd 24(%rsi), %xmm3",
	"movsd 32(%rsi), %xmm4",
	"movsd 40(%rsi), %xmm5",
	"movsd 48(%rsi), %xmm6",
	"movsd 56(%rsi), %xmm7",
//	"cmpq  $9,%rcx", // temp
//	"jne   2f",      // temp
//	"int   $3",      // temp
	// 2. проверим на "стековые" аргументы
"2:",
	"xorq  %rax, %rax",
	"subq  $8, %rcx",
	"cmovs %rax, %rcx", // = max(rcx-8, 0)
	"cmpl  $6, %edx",
	"cmova %rdx, %rax", //
	"addq  %rcx, %rax", // = max(rdx-6, 0) + max(rcx-8, 0)
	"testq %rax, %rax",
	"jz    4f", // no agruments for push to stack
	// забросим весь оверхед в стек с учетом очередности и маски
"3:",
	"andq  $1, %rax",
	"leaq  -16(%rsp,%rax,8),%rsp", // выравнивание стека по 16-байтной границе

	"leaq  -8(%rdi,%rdx,8), %rax",
	"leaq  56(%rsi,%rcx,8), %rsi", // last float argument  (if any) 56=8*8-8

	"subq  $6, %rdx",
"31:",
	"shrq  %r8",
	"jc    33f", // bit 0 was set, so push float value
"32:", // push fixed
	"cmpq  $0, %rdx",
	"jle   31b", // нету больше аргументов для fixed пуша
	"pushq (%rax)",
	"subq  $8, %rax",
	"decq  %rdx",
	"jnz   31b",
	"cmpq  $0, %rcx",
	"jle   4f",
	"jmp   31b",
"33:", // push float
	"cmpq  $0, %rcx",
	"jle   31b", // нету больше аргументов для float пуша
	"pushq (%rsi)",
	"subq  $8, %rsi",
	"decq  %rcx",
	"jnz   31b",
	"cmpq  $0, %rdx",
	"jle   4f",
	"jmp   31b",

	// 4. не проверяем количество аргументов, так будет быстрее
"4:",
	"movq  40(%rdi), %r9",
	"movq  32(%rdi), %r8",
	"movq  24(%rdi), %rcx",
	"movq  16(%rdi), %rdx",
	"movq   8(%rdi), %rsi",
	"movq   0(%rdi), %rdi",

	"callq *-8(%rbp)",

	// вернем результат
	"cmpl $46, 16(%rbp)", // TFLOAT
	"je   5f",
	"cmpl $47, 16(%rbp)", // TDOUBLE
	"je   6f",
"9:",
	"leave",
	"ret",

"5:",
	"cvtss2sd %xmm0, %xmm0", // float->double
"6:",
	"movsd %xmm0, (%rsp)",
	"popq  %rax", // corresponded push not required (we already pushed %r9)
	"jmp   9b");

// RDI, RSI, RDX, RCX (R10 in the Linux kernel interface[17]:124), R8, and R9
// while XMM0, XMM1, XMM2, XMM3, XMM4, XMM5, XMM6 and XMM7 are used for floats
# endif

#elif __i386__ // ILP32
// value returned in the edx:eax

// CDECL / STDCALL
// The cdecl (which stands for C declaration) is a calling convention
// that originates from the C programming language and is used by many
// C compilers for the x86 architecture.[1] In cdecl, subroutine
// arguments are passed on the stack. Integer values and memory
// addresses are returned in the EAX register, floating point values in
// the ST0 x87 register. Registers EAX, ECX, and EDX are caller-saved,
// and the rest are callee-saved. The x87 floating point registers ST0
// to ST7 must be empty (popped or freed) when calling a new function,
// and ST1 to ST7 must be empty on exiting a function. ST0 must also be
// empty when not used for returning a value.
//
// В нашем случае мы так или иначе восстанавливаем указатель стека, так что
// функция x86_call у нас будет универсальная cdecl/stdcall
long long x86_call(word argv[], long i, void* function, long type);

__ASM__("x86_call:_x86_call:", //"int $3",
	"pushl %ebp",
	"movl  %esp, %ebp",

	"movl  12(%ebp), %ecx",
	"test  %ecx, %ecx",
	"jz    1f",
	"movl  8(%ebp), %eax",
	"leal  -4(%eax,%ecx,4),%eax",
"0:",
	"pushl (%eax)",
	"subl  $4, %eax",
	"decl  %ecx",
	"jnz   0b",
"1:",
	"call  *16(%ebp)",

	"movl  20(%ebp), %ecx", // проверка возвращаемого типа
	"cmpl  $46, %ecx",      // TFLOAT
	"je    3f",
	"cmpl  $47, %ecx",      // TDOUBLE
	"je    3f",
"9:",
	"leave",
	"ret",

// с плавающей точкой мы всегда возвращаем double
"3:", // double
	"pushl %edx",
	"pushl %eax",
	"fstpl (%esp)",
	"popl  %eax",
	"popl  %edx",
	"jmp   9b");

#elif __EMSCRIPTEN__

typedef long long ret_t;
static
ret_t asmjs_call(word args[], int fmask, void* function, int type) {
//	printf("asmjs_call(%p, %d, %p, %d)\n", args, fmask, function, type);

	switch (fmask) {
	case 0b1:
		return (ret_t)(word)((word (*)  ())
					 function) ();

	case 0b10:
		return (ret_t)(word)((word (*)  (word))
					 function) (args[ 0]);

	case 0b100:
		return (ret_t)(word)((word (*)  (word, word))
		             function) (args[ 0], args[ 1]);
	case 0b1000:
		return (ret_t)(word)((word (*)  (word, word, word))
		             function) (args[ 0], args[ 1], args[ 2]);
	case 0b10000:
		return (ret_t)(word)((word (*)  (word, word, word, word))
		             function) (args[ 0], args[ 1], args[ 2], args[ 3]);
	case 0b11111:
//		printf("%f/%f/%f/%f\n", *(float*)&args[ 0], *(float*)&args[ 1], *(float*)&args[ 2], *(float*)&args[ 3]);
		return (ret_t)(word)((word (*)  (float, float, float, float))
		             function) (*(float*)&args[ 0], *(float*)&args[ 1], *(float*)&args[ 2], *(float*)&args[ 3]);
	case 0b100000:
		return (ret_t)(word)((word (*)  (word, word, word, word, word))
		             function) (args[ 0], args[ 1], args[ 2], args[ 3],
		                        args[ 4]);
	case 0b1000000:
		return (ret_t)(word)((word (*)  (word, word, word, word, word, word))
		             function) (args[ 0], args[ 1], args[ 2], args[ 3],
		                        args[ 4], args[ 5]);
	case 0b10000000:
		return (ret_t)(word)((word (*)  (word, word, word, word, word, word,
	                                  word))
	                 function) (args[ 0], args[ 1], args[ 2], args[ 3],
	                            args[ 4], args[ 5], args[ 6]);
	case 0b100000000:
		return (ret_t)(word)((word (*)  (word, word, word, word, word, word,
	                                  word, word))
	                 function) (args[ 0], args[ 1], args[ 2], args[ 3],
	                            args[ 4], args[ 5], args[ 6], args[ 7]);
	case 0b1000000000:
		return (ret_t)(word)((word (*)  (word, word, word, word, word, word,
	                                  word, word, word))
	                 function) (args[ 0], args[ 1], args[ 2], args[ 3],
	                            args[ 4], args[ 5], args[ 6], args[ 7],
	                            args[ 8]);
	// 10:
	case 0b10000000000:
		return (ret_t)(word)((word (*)  (word, word, word, word, word, word,
	                                  word, word, word, word))
	                 function) (args[ 0], args[ 1], args[ 2], args[ 3],
	                            args[ 4], args[ 5], args[ 6], args[ 7],
	                            args[ 8], args[ 9]);
	case 0b100000000000:
		return (ret_t)(word)((word (*)  (word, word, word, word, word, word,
	                                  word, word, word, word, word))
	                 function) (args[ 0], args[ 1], args[ 2], args[ 3],
	                            args[ 4], args[ 5], args[ 6], args[ 7],
	                            args[ 8], args[ 9], args[10]);
	case 0b1000000000000:
		return (ret_t)(word)((word (*)  (word, word, word, word, word, word,
	                                  word, word, word, word, word, word))
	                 function) (args[ 0], args[ 1], args[ 2], args[ 3],
	                            args[ 4], args[ 5], args[ 6], args[ 7],
	                            args[ 8], args[ 9], args[10], args[11]);
	default: fprintf(stderr, "Unsupported parameters count for ffi function: %ud", fmask);
		return 0;
	};
}

#else // other platforms

// http://byteworm.com/2010/10/12/container/ (lambdas in c)

// https://en.wikipedia.org/wiki/X86_calling_conventions
// x86 conventions: cdecl, syscall(OS/2), optlink(IBM)
// pascal(OS/2, MsWin 3.x, Delphi), stdcall(Win32),
// fastcall(ms), vectorcall(ms), safecall(delphi),
// thiscall(ms)

	// todo: ограничиться количеством функций поменьше
	//	а можно сделать все в одной switch:
	// todo: а можно лямбдой оформить и засунуть эту лябмду в функцию еще в get-proc-address
	// todo: проанализировать частоту количества аргументов и переделать все в
	//   бинарный if

	// __attribute__((stdcall))
/*						__stdcall // gcc style for lambdas in pure C
	int (*stdcall[])(char*) = {
			({ int $(char *str){ printf("Test: %s\n", str); } $; })
	};*/
	// http://www.agner.org/optimize/calling_conventions.pdf
	#define CALL(conv) \
		switch (i) {\
		case  0: return (ret_t)(word)((conv word (*)  ())\
						 function) ();\
		case  1: return (ret_t)(word)((conv word (*)  (word))\
						 function) (args[ 0]);\
		case  2: return (ret_t)(word)((conv word (*)  (word, word))\
		                 function) (args[ 0], args[ 1]);\
		case  3: return (ret_t)(word)((conv word (*)  (word, word, word))\
		                 function) (args[ 0], args[ 1], args[ 2]);\
		case  4: return (ret_t)(word)((conv word (*)  (word, word, word, word))\
		                 function) (args[ 0], args[ 1], args[ 2], args[ 3]);\
		case  5: return (ret_t)(word)((conv word (*)  (word, word, word, word, word))\
		                 function) (args[ 0], args[ 1], args[ 2], args[ 3],\
		                            args[ 4]);\
		case  6: return (ret_t)(word)((conv word (*)  (word, word, word, word, word, word))\
		                 function) (args[ 0], args[ 1], args[ 2], args[ 3],\
		                            args[ 4], args[ 5]);\
		case  7: return (ret_t)(word)((conv word (*)  (word, word, word, word, word, word, \
		                                  word))\
		                 function) (args[ 0], args[ 1], args[ 2], args[ 3],\
		                            args[ 4], args[ 5], args[ 6]);\
		case  8: return (ret_t)(word)((conv word (*)  (word, word, word, word, word, word, \
		                                  word, word))\
		                 function) (args[ 0], args[ 1], args[ 2], args[ 3],\
		                            args[ 4], args[ 5], args[ 6], args[ 7]);\
		case  9: return (ret_t)(word)((conv word (*)  (word, word, word, word, word, word, \
		                                  word, word, word))\
		                 function) (args[ 0], args[ 1], args[ 2], args[ 3],\
		                            args[ 4], args[ 5], args[ 6], args[ 7],\
		                            args[ 8]);\
		case 10: return (ret_t)(word)((conv word (*)  (word, word, word, word, word, word, \
		                                  word, word, word, word))\
		                 function) (args[ 0], args[ 1], args[ 2], args[ 3],\
		                            args[ 4], args[ 5], args[ 6], args[ 7],\
		                            args[ 8], args[ 9]);\
		case 11: return (ret_t)(word)((conv word (*)  (word, word, word, word, word, word, \
		                                  word, word, word, word, word))\
		                 function) (args[ 0], args[ 1], args[ 2], args[ 3],\
		                            args[ 4], args[ 5], args[ 6], args[ 7],\
		                            args[ 8], args[ 9], args[10]);\
		case 12: return (ret_t)(word)((conv word (*)  (word, word, word, word, word, word, \
		                                  word, word, word, word, word, word))\
		                 function) (args[ 0], args[ 1], args[ 2], args[ 3], \
		                            args[ 4], args[ 5], args[ 6], args[ 7], \
		                            args[ 8], args[ 9], args[10], args[11]);\
		default: fprintf(stderr, "Unsupported parameters count for ffi function: %d", i);\
			return 0;\
		};

#endif


// lisp->c convertors
static
word from_uint(word arg) {
	assert (is_reference(arg));
	// так как в стек мы все равно большое число сложить не сможем,
	// то возьмем только то, что влазит (первые два члена)
	return (car(arg) >> 8) | ((car(cdr(arg)) >> 8) << VBITS);
}

#if UINTPTR_MAX != 0xffffffffffffffff
static
long long from_ulong(word arg) {
	assert (is_reference(arg));
	long long v = car(arg) >> 8;
	int shift = VBITS;
	while (cdr(arg) != INULL) {
		v |= ((long long)(car(cdr(arg)) >> 8) << shift);
		shift += VBITS;
		arg = cdr(arg);
	}
	return v;
};
#endif

static
float from_int_to_float(word* arg) {
	// читаем длинное число в float формат
	assert (is_value(car(arg)));
	float f = (unsigned long)uvtoi(car(arg));
	float mul = HIGHBIT;
	while (is_reference(cdr(arg))) {
		arg = (word*)cdr(arg);
		f += (unsigned long)uvtoi(cdr(arg)) * mul;
		mul *= HIGHBIT;
	}
	assert (cdr(arg) == INULL);

	return f;
}

static
float from_rational(word arg) {
	word* pa = (word*)car(arg);
	word* pb = (word*)cdr(arg);

	float a = 0;
	if (is_value(pa))
		a = svtoi(pa);
	else {
		switch (reftype(pa)) {
		case TINTP:
			a = +from_int_to_float(pa);
			break;
		case TINTN:
			a = -from_int_to_float(pa);
			break;
		}
	}

	float b = 1;
	if (is_value(pb))
		b = svtoi(pb);
	else {
		switch (reftype(pb)) {
		case TINTP:
			b = +from_int_to_float(pb);
			break;
		case TINTN:
			b = -from_int_to_float(pb);
			break;
		}
	}

	return (a / b);
}

static
int to_int(word arg) {
	if (is_value(arg))
		return svtoi(arg);

	switch (reftype(arg)) {
	case TINTP:
		return (int)+from_uint(arg);
	case TINTN:
		return (int)-from_uint(arg);
	case TRATIONAL:
		return (int) from_rational(arg);
	case TCOMPLEX:
		return to_int(car(arg)); // return real part of value
	default:
		E("can't get int from %d", reftype(arg));
	}

	return 0;
}

/*
static
long to_long(word arg) {
	if (is_value(arg))
		return svtoi(arg);

	switch (reftype(arg)) {
	case TINTP:
		return (long)+from_int(arg);
	case TINTN:
		return (long)-from_int(arg);
	case TRATIONAL:
		return (long) from_rational(arg);
	case TCOMPLEX:
		return to_long(car(arg)); // return real part of value
	default:
		E("can't get int from %d", reftype(arg));
	}

	return 0;
}*/

// todo: заменить на вызов (float)to_double(arg)
static
float to_float(word arg) {
	if (is_value(arg))
		return svtoi(arg);

	switch (reftype(arg)) {
	case TINTP:
	case TINTN:
	case TRATIONAL:
		return (float) ol2f(arg);
	case TCOMPLEX:
		return to_float(car(arg)); // return real part of value
	case TINEXACT:
		return *(float*)&car(arg);
	}
	return 0;
}

static
double to_double(word arg) {
	if (is_value(arg))
		return svtoi (arg);

	switch (reftype(arg)) {
	case TINTP:
		return (double)+from_uint(arg);
		break;
	case TINTN:
		return (double)-from_uint(arg);
		break;
	case TRATIONAL:
		return (double) from_rational(arg);
		break;
	case TCOMPLEX:
		return to_double(car(arg)); // return real part of value
	case TINEXACT:
		return *(double*)&car(arg);
	}
	return 0;
}


// Главная функция механизма ffi:
PUBLIC
word* OL_ffi(OL* self, word* arguments)
{
	// get memory pointer
	heap_t* heap = &self->heap;
	word*
	fp = heap->fp;

	// a - function address
	// b - arguments (may be pair with req type in car and arg in cdr - not yet done)
	// c - '(return-type . argument-types-list)
	word* A = (word*)car(arguments); arguments = (word*)cdr(arguments); // function
	word* B = (word*)car(arguments); arguments = (word*)cdr(arguments); // rtty
	word* C = (word*)car(arguments); arguments = (word*)cdr(arguments); // args

	assert (is_vptr(A));
	assert ((word)B != INULL && (is_reference(B) && reftype(B) == TPAIR));
	assert ((word)C == INULL || (is_reference(B) && reftype(C) == TPAIR));
	// C[1] = return-type
	// C[2] = argument-types

	// todo: может выделять в общей куче,а не стеке? (кстати, да!)
	void *function = (void*)car(A);  assert (function);
	int returntype = value(car(B));

	word args[32]; // 16 double аргументов максимум
	int i = 0;     // актуальное количество аргументов

	static_assert(sizeof(float) <= sizeof(word), "float size should not exceed the word size");


#ifdef __EMSCRIPTEN__
	int fmask = 0; // маска для типа аргументов, (0-int, 1-float) + старший бит-маркер (установим в конце)
#endif

#if __amd64__ && __linux__ // LP64
	double ad[18]; // и для флоатов отдельный массив (amd64 specific)
	int d = 0;     // количество аргументов для float (amd64)
	long floatsmask = 0; // маска для флоатов // deprecated:, старший единичный бит - признак конца
#endif

	word* p = (word*)C;   // сами аргументы
	word* t = (word*)cdr (B); // rtty
	int has_wb = 0; // has write-back in arguments (speedup)

	while ((word)p != INULL) { // пока есть аргументы
		assert (reftype(p) == TPAIR); // assert(list)
		assert (reftype(t) == TPAIR); // assert(list)

		int type = value(car(t)); // destination type
		word arg = (word) car(p);

/*		// todo: add argument overriding as PAIR as argument value
		if (thetype (p[1]) == TPAIR) {
			type = value (((word*)p[1])[1]);
			arg = ((word*)p[1])[2];
		}*/

#if __amd64__ && __linux__ // LP64
		floatsmask <<= 1; // подготовим маску к следующему аргументу
#endif

		if (type == TANY) {
			if (is_value(arg))
#if UINTPTR_MAX == 0xffffffffffffffff // 64-bit machines
				type = TINT64;
#else
				type = TINT32;
#endif
			else {
				type = reftype (arg);
			}
		}

		args[i] = 0; // обнулим (теперь дальше сможем симулировать обнуление через break)

		if (arg == IFALSE) { // #false is universal "0" value
/*			switch (type) {
#if UINT64_MAX > SIZE_MAX
			case TINT64: case TUINT64:
				args[++i] = 0; 	// for 32-bits: double fills two words
#endif
#if __amd64__ && __linux__
			case TFLOAT:
				*(float*)&ad[d++] = 0;
				floatsmask|=1; --i;
				break;
#endif
			case TDOUBLE:
				#if __amd64__ && __linux__
					ad[d++] = 0;
					floatsmask++; --i;
				#endif
				#if UINT64_MAX > SIZE_MAX
					++i; 	// for 32-bits: double fills two words
				#endif
				break;

			}*/
		}
		else
		switch (type) {

		// целочисленные типы:
		case TINT8:  case TUINT8:
		case TINT16: case TUINT16:
		case TINT32: case TUINT32:
#if UINTPTR_MAX == 0xffffffffffffffff // 64-bit machines
		case TINT64: case TUINT64:
#endif
			if (is_value(arg))
				args[i] = svtoi(arg);
			else
			switch (reftype(arg)) {
			case TINTP:
				args[i] = +from_uint(arg);
				break;
			case TINTN:
				args[i] = -from_uint(arg);
				break;
			case TRATIONAL:
				*(long long*)&args[i] = from_rational(arg);
#if UINT64_MAX > UINTPTR_MAX // sizeof(long long) > sizeof(word) //__LP64__
				i++;
#endif
				break;
			default:
				E("can't cast %d to int", type);
				args[i] = 0; // todo: error
			}
			break;

#if UINTPTR_MAX != 0xffffffffffffffff // 32-bit machines
			case TINT64: case TUINT64: // long long
				if (is_value(arg))
					*(long long*)&args[i] = svtoi(arg);
				else
				switch (reftype(arg)) {
				case TINTP: // source type
					*(long long*)&args[i] = +from_ulong(arg);
					break;
				case TINTN:
					*(long long*)&args[i] = -from_ulong(arg);
					break;
				case TRATIONAL:
					*(long long*)&args[i] = from_rational(arg);
					break;
				default:
					E("can't cast %d to int64", type);
				}
				#if UINT64_MAX > UINTPTR_MAX // sizeof(long long) > sizeof(word) //__LP64__
					i++; // for 32-bits: long long values fills two words
				#endif
			break;
#endif

		//
		case TINT16 + FFT_REF:
		case TUINT16 + FFT_REF:
			has_wb = 1;
			//no break
		case TINT16 + FFT_PTR:
		case TUINT16 + FFT_PTR: {
			// todo: add tuples pushing
			if (arg == INULL) // empty array will be sent as nullptr
				break;

			int c = llen(arg);
			short* p = (short*) __builtin_alloca(c * sizeof(short)); // todo: new_raw_vector() ?
			args[i] = (word)p;

			word l = arg;
			while (c--)
				*p++ = (short)to_int(car(l)), l = cdr(l);
			break;
		}

		case TINT32 + FFT_REF:
		case TUINT32 + FFT_REF:
			has_wb = 1;
			//no break
		case TINT32 + FFT_PTR:
		case TUINT32 + FFT_PTR: {
			// todo: add tuples pushing
			if (arg == INULL) // empty array will be sent as nullptr
				break;

			int c = llen(arg);
			int* p = (int*) __builtin_alloca(c * sizeof(int)); // todo: new_raw_vector() ?
			args[i] = (word)p;

			word l = arg;
			while (c--)
				*p++ = to_int(car(l)), l = cdr(l);
			break;
		}

		// todo: case TINT64 + FFT_PTR:

		// с плавающей запятой:
		case TFLOAT:
			#if __amd64__ && __linux__
				*(float*)&ad[d++] = (float)to_double(arg);
				floatsmask|=1; --i;
			#else
				*(float*)&args[i] = to_float(arg);
			# ifdef __EMSCRIPTEN__
				fmask |= 1 << i;
			# endif
			#endif
			break;
		case TFLOAT + FFT_REF:
			has_wb = 1;
			//no break
		case TFLOAT + FFT_PTR: {
			if (arg == INULL) // empty array will be sent as nullptr
				break;

			int c = llen(arg);
			float* f = (float*) __builtin_alloca(c * sizeof(float));
			args[i] = (word)f;

			word l = arg;
			while (c--)
				*f++ = to_float(car(l)), l = cdr(l);
			break;
		}

		case TDOUBLE:
			#if __amd64__ && __linux__
				ad[d++] = to_double(arg);
				floatsmask++; --i;
			#else
				*(double*)&args[i] = to_double(arg);
				// no double for any call yet supported
			#endif
			#if UINT64_MAX > SIZE_MAX // sizeof(double) > sizeof(float) //__LP64__
				++i; 	// for 32-bits: double fills two words
			#endif
			break;

		case TDOUBLE + FFT_REF:
			has_wb = 1;
			// no break
		case TDOUBLE + FFT_PTR: {
			if (arg == INULL) // empty array will be sent as nullptr
				break;

			int c = llen(arg);
			double* p = (double*) __builtin_alloca(c * sizeof(double)); // todo: use new()
			args[i] = (word)p;

			word l = arg;
			while (c--)
				*p++ = to_double(car(l)), l = cdr(l);
			break;
		}

		// поинтер на данные
		case TUNKNOWN:
			args[i] = arg;
			break;

		// vptr should accept only vptr!
		case TVPTR:
			if (is_reference(arg))
				switch (reftype(arg)) {
				case TVPTR:
					args[i] = car(arg);
					break;
				case TBVEC: // can be used instead of vptr
					args[i] = (word) &car(arg);
					break;
				default:
					E("invalid parameter value (requested vptr)");
				}
			else
				E("invalid parameter value (requested vptr)");
			break;
		case TVPTR + FFT_REF:
			has_wb = 1;
			// no break
		case TVPTR + FFT_PTR: {
			if (arg == INULL) // empty array will be sent as nullptr
				break;
			if (reftype(arg) == TVPTR) // deprecated: single vptr value
				args[i] = (word) &car(arg);
			else {
				int c = llen(arg);
				void** p = (void**) __builtin_alloca(c * sizeof(void*)); // todo: use new()
				args[i] = (word)p;

				word l = arg;
				while (c--) // todo: add type check
					*p++ = (void*)car(l), l = cdr(l);
			}
			break;
		}

		// todo: change to is_rawdata
		case TSTRING:
			switch (reftype(arg)) {
			case TBVEC:
			case TSTRING:
				args[i] = (word) &car(arg);
				break;
			// todo: TSTRINGWIDE
			default:
				E("invalid parameter values (requested string)");
			}
			break;
		case TSTRING + FFT_PTR: {
			int size = llen(arg) + 1;

			// TODO: check the available memory and gun GC if necessary
			word* p = new (TBVEC, size, 0);
			args[i] = (word)++p;

			word src = arg;
			while (--size)
				*p++ = (word) &caar(src), src = cdr(src);
			break;
		}

		// todo: do fix for windows!
		//    Windows uses UTF-16LE encoding internally as the memory storage
		//    format for Unicode strings, it considers this to be the natural
		//    encoding of Unicode text. In the Windows world, there are ANSI
		//    strings (the system codepage on the current machine, subject to
		//    total unportability) and there are Unicode strings (stored
		//    internally as UTF-16LE).
//		#ifdef _WIN32
		case TSTRINGWIDE:
			switch (reftype(arg)) {
			case TBVEC:
			case TSTRING: {
				word hdr = reference(arg);
				int len = (hdrsize(hdr)-1)*sizeof(word) - padsize(hdr);
				short* unicode = (short*) __builtin_alloca(len * sizeof(short));

				short* p = unicode;
				char* s = (char*)&car(arg);
				for (int i = 1; i < len; i++)
					*p++ = (short)(*s++);
				*p = 0;

				args[i] = (word) unicode;
				break;
			}
			case TSTRINGWIDE: {
				int len = hdrsize(reference(arg));
				short* unicode = (short*) __builtin_alloca(len * sizeof(short));
				// todo: use new()
				// check the available memory in the heap
				// use builtin unlucky for calling the gc if no more memory

				short* p = unicode;
				word* s = &car(arg);
				for (int i = 1; i < len; i++)
					*p++ = (short)value(*s++);
				*p = 0;

				args[i] = (word) unicode;
			}
				break;
			default:
				E("invalid parameter values (requested string)");
			}
			break;
//		#endif

		case TCALLABLE: {
			if (is_callable(arg))
				args[i] = (word)car(arg);
			else
				E("invalid parameter values (requested callable)");
			break;
		}
/*
		case TTUPLE:
			switch (reftype(arg)) {
			case TTUPLE: { // ?
				// аллоцировать массив и сложить в него указатели на элементы кортежа
				int size = hdrsize(*(word*)arg);
				*fp++ = make_raw_header(TBVEC, size, 0);
				args[i] = (word)fp; // ссылка на массив указателей на элементы

				word* src = &car(arg);
				while (--size)
					*fp++ = (word)((word*)*src++ + 1);
				}
				break;
			default:
				args[i] = INULL; // todo: error
			}
			break;*/
//		case TRAWVALUE:
//			args[i] = (word)arg;
//			break;
		case TPORT: {
			if (arg == itosv(-1)) {
				args[i] = -1;
				break;
			}
			int portfd = port(arg);
			switch (portfd) {
			case 0: // stdin
				args[i] = (word) stdin;
				break;
			case 1: // stdout
				args[i] = (word) stdout;
				break;
			case 2: // stderr
				args[i] = (word) stderr;
				break;
			default:
				args[i] = portfd;
				break;
			}
			break;
		}
		case TVOID:
			args[i] = 0; // do nothing, just for better readability
			break;
		default:
			E("can't recognize %d type", type);
		}

		p = (word*)cdr(p); // (cdr p)
		t = (word*)cdr(t); // (cdr t)
		i++;
	}
#ifdef __EMSCRIPTEN__
	fmask |= 1 << i;
#endif
	assert ((word)t == INULL); // количество аргументов совпало!

	long long got = 0; // результат вызова функции (64 бита для возможного double)

	self->R[128 + 1] = (word)B;
	self->R[128 + 2] = (word)C;
	heap->fp = fp; // сохраним, так как в call могут быть вызваны коллейблы, и они попортят fp

//	if (floatsmask == 15)
//		__asm__("int $3");

#if __amd64__
	got =
	#	if __linux__
			x64_call(args, ad, i, d, floatsmask, function, returntype & 0x3F);
	#	else
			x64_call(args, i, function, returntype & 0x3F);
	#	endif

#elif __i386__
/*	// cdecl and stdcall in our case are same, so...
	switch (returntype >> 6) {
	case 0:
	case 1:
	case 2:
		//was: got = call(returntype, function, args, i);
		got = x86_call(args, i, function, returntype & 0x3F);
		break;
	// FASTCALL: (3)
	case 3:
	// THISCALL: (4)
	// ...
	default:
		E("Unsupported calling convention %d", returntype >> 6);
		break;
	}*/
	got =
			x86_call(args, i, function, returntype & 0x3F);
// arm calling http://infocenter.arm.com/help/topic/com.arm.doc.ihi0042f/IHI0042F_aapcs.pdf
#elif __arm__
	typedef long long ret_t;
	inline ret_t call(word args[], int i, void* function, int type) {
		CALL();
	}
	got = call(args, i, function, returntype & 0x3F);
#elif __aarch64__
	typedef long long ret_t;
	inline ret_t call(word args[], int i, void* function, int type) {
		CALL();
	}
	got = call(args, i, function, returntype & 0x3F);
#elif __mips__
	// https://acm.sjtu.edu.cn/w/images/d/db/MIPSCallingConventionsSummary.pdf
	typedef long long ret_t;
	inline ret_t call(word args[], int i, void* function, int type) {
		CALL();
	}
	got = call(args, i, function, returntype & 0x3F);
#elif __EMSCRIPTEN__
	got = asmjs_call(args, fmask, function, returntype & 0x3F);
#else // ALL other
/*	inline ret_t call_cdecl(word args[], int i, void* function, int type) {
		CALL(__cdecl);
	}
	inline ret_t call_stdcall(word args[], int i, void* function, int type) {
		CALL(__stdcall);
	}

	switch (returntype >> 6) {
	case 0:
	case 1:
		got = call_cdecl(args, i, function, returntype & 0x3F);
		break;
	case 2:
		got = call_stdcall(args, i, function, returntype & 0x3F);
		break;
	// FASTCALL: (3)
	case 3:
	// THISCALL: (4)
	// ...
	default:
		E("Unsupported calling convention %d", returntype >> 6);
		break;
	}*/
#endif

	// где гарантия, что C и B не поменялись?
	fp = heap->fp;
	B = (word*)self->R[128 + 1];
	C = (word*)self->R[128 + 2];

	if (has_wb) {
		// еще раз пробежимся по аргументам, может какие надо будет вернуть взад
		p = (word*)C;   // сами аргументы
		t = (word*)cdr(B); // rtti

		i = 0;
		while ((word)p != INULL) { // пока есть аргументы
			assert (reftype(p) == TPAIR); // assert(list)
			assert (reftype(t) == TPAIR); // assert(list)

			int type = value(car(t));
			word arg = (word) car(p);

			// destination type
			if (arg != IFALSE)
			switch (type) {

			// simplest case - all shorts are fits as value
			case TINT16 + FFT_REF: {
				// вот тут попробуем заполнить переменные назад
				int c = llen(arg);
				short* f = (short*)args[i];

				word l = arg;
				while (c--) {
					short value = *f++;
					word* ptr = &car(l);

					*ptr = itosv(value);
					l = cdr(l);
				}
				break;
			}
			case TUINT16 + FFT_REF: {
				// вот тут попробуем заполнить переменные назад
				int c = llen(arg);
				unsigned short* f = (unsigned short*)args[i];

				word l = arg;
				while (c--) {
					unsigned short value = *f++;
					word* ptr = &car(l);

					*ptr = itouv(value);
					l = cdr(l);
				}
				break;
			}

			case TINT32 + FFT_REF: {
				// вот тут попробуем заполнить переменные назад
				int c = llen(arg);
				int* f = (int*)args[i];

				word l = arg;
				while (c--) {
					int value = *f++;
					word* ptr = &car(l);

				#if UINTPTR_MAX != 0xffffffffffffffff  // 32-bit machines
					if (value > VMAX) {
						if (is_value(*ptr))
							E("got too large number to store");
						else {
							assert (is_npair(ptr) || is_npairn(ptr));
							if (value < 0) {
								*ptr = make_header(value < 0 ? TINTN : TINTP, 3);
								value = -value;
							}
							else
								*ptr = make_header(TINTP, 3);
							*(word*)&car(ptr) = itouv(value & VMAX);
							*(word*)&cadr(ptr) = itouv(value >> VBITS);
						}
					}
					else
				#endif
						*ptr = itosv(value);

					l = cdr(l);
				}
				break;
			}

			case TUINT32 + FFT_REF: {
				// вот тут попробуем заполнить переменные назад
				int c = llen(arg);
				unsigned int* f = (unsigned int*)args[i];

				word l = arg;
				while (c--) {
					int value = *f++;
					word* ptr = &car(l);

				#if UINTPTR_MAX != 0xffffffffffffffff  // 32-bit machines
					if (value > VMAX) {
						if (is_value(*ptr))
							E("got too large number to store");
						else {
							assert (is_npair(ptr) || is_npairn(ptr));
							*ptr = make_header(TINTP, 3);
							*(word*)&car(ptr) = itouv(value & VMAX);
							*(word*)&cadr(ptr) = itouv(value >> VBITS);
						}
					}
					else
				#endif
						*ptr = itouv(value);

					l = cdr(l);
				}
				break;
			}

			case TFLOAT + FFT_REF: {
				// вот тут попробуем заполнить переменные назад
				int c = llen(arg);
				float* f = (float*)args[i];

				word l = arg;
				while (c--) {
					float value = *f++;
					word num = car(l);
					assert (reftype(num) == TRATIONAL);
					// максимальная читабельность (todo: change like fto..)
					long n = value * 10000;
					long d = 10000;
					car(num) = itosv(n);
					cdr(num) = itosv(d);
					// максимальная точность (fixme: пока не работает как надо)
					//car(num) = itosv(value * VMAX);
					//cdr(num) = I(VMAX);

					l = cdr(l);
				}
				break;
			}

			case TVPTR + FFT_REF: {
				int c = llen(arg);
				void** f = (void**)args[i];

				word l = arg;
				while (c--) {
					void* value = *f++;
					word num = car(l);
					assert (reftype(num) == TVPTR);
					*(void**)&car(num) = value;

					l = cdr(l);
				}
				break;
			} }

			p = (word*) cdr(p);
			t = (word*) cdr(t);
			i++;
		}
	}

	word* result = (word*)IFALSE;
	switch (returntype & 0x3F) {
		// TFIXP - deprecated
		case TFIXP: // type-fix+ - если я уверен, что число заведомо меньше 0x00FFFFFF! (или сколько там в x64)
			result = (word*) itosv (got);
			break;

		case TINT8:
			result = (word*) itosn (*(char*)&got);  // TODO: change to __INT8_TYPE__
			break;
		case TINT16:
			result = (word*) itosn (*(short*)&got); // TODO: change to __INT16_TYPE__
			break;
		case TINT32:
			result = (word*) itosn (*(int*)&got);   // TODO: change to __INT32_TYPE__
			break;
		case TINT64: {
#if UINTPTR_MAX == 0xffffffffffffffff
			result = (word*) itosn (*(long long*)&got); // TODO: change to __INT64_TYPE__
#else
#	ifndef __EMSCRIPTEN__
			word* new_npair(int type, long long value) {
				long long a = value >> VBITS;
				word* b = (a > VMAX) ? new_npair(TPAIR, a) : new_pair(TPAIR, I(a & VMAX), INULL);
				word* p = new_pair(type, I(value & VMAX), b);
				return p;
			}
			word* ll2ol(long long val) {
				long long x5 = val;
				long long x6 = x5 < 0 ? -x5 : x5;
				int type = x5 < 0 ? TINTN : TINTP;
				return (x6 > VMAX) ? new_npair(type, x6) : (word*)make_value(x5 < 0 ? TFIXN : TFIXP, x6);
			};

			result = (word*) ll2ol (*(long long*)&got);
#	endif
#endif
			break;
		}

		case TUINT8:
			result = (word*) itoun (*(unsigned char*)&got); // TODO: change to __UINT8_TYPE__
			break;
		case TUINT16:
			result = (word*) itoun (*(unsigned short*)&got);// TODO: change to __UINT16_TYPE__
			break;
		case TUINT32:
			result = (word*) itoun (*(unsigned int*)&got);  // TODO: change to __UINT32_TYPE__
			break;
		case TUINT64: {
#if UINTPTR_MAX == 0xffffffffffffffff
			result = (word*) itoun (*(unsigned long long*)&got); // TODO: change to __UINT32_TYPE__
#else
#	ifndef __EMSCRIPTEN__
			word* new_unpair(int type, unsigned long long value) {
				unsigned long long a = value >> VBITS;
				word* b = (a > VMAX) ? new_unpair(TPAIR, a) : new_pair(TPAIR, I(a & VMAX), INULL);
				word* p = new_pair(type, I(value & VMAX), b);
				return p;
			}
			word* ul2ol(long long val) {
				unsigned long long x5 = val;
				return (x5 > VMAX) ? new_unpair(TINTP, x5) : (word*)make_value(x5 < 0 ? TFIXN : TFIXP, x5);
			};

			result = (word*) ul2ol (*(unsigned long long*)&got);
#	endif
#endif
			break;
		}


		case TPORT:
			result = (word*) make_port ((long)got);
			break;
		case TVOID:
			result = (word*) ITRUE;
			break;

		case TVPTR:
			if (got)
				result = new_vptr (got);
			break;

		case TSTRING:
			if (got) {
				int l = lenn((char*)(word)got, VMAX+1);
				/* TODO: enable again!
				if (fp + (l/sizeof(word)) > heap->end) {
					self->gc(self, l/sizeof(word));
					heap = &self->heap;
					fp = heap->fp;
				}*/
				result = new_string ((char*)(word)got, l);
			}
			break;
		case TSTRINGWIDE:
			if (got) {
				int l = lenn16((short*)(word)got, VMAX+1);
				/* TODO: enable again!
				if (fp + (l/sizeof(word)) > heap->end) {
					self->gc(self, l/sizeof(word));
					heap = &self->heap;
					fp = heap->fp;
				}*/
				word* p = result = new (TSTRINGWIDE, l+1);
				unsigned short* s = (unsigned short*)(word)got;
				while (l--) {
					*++p = I(*s++);
				}
			}
			break;

		// возвращаемый тип не может быть TRATIONAL, так как непонятна будет точность
		case TFLOAT:
		case TDOUBLE: {
			double value = *(double*)&got;

			heap->fp = fp;
			result = (word*) d2ol(self, value);
			fp = heap->fp;
			break;
		}
	}

	heap->fp = fp;
	return result;
}
#endif//OLVM_FFI


// --=( CALLABLES support )=-----------------------------
// --
#if OLVM_CALLABLES
//long long callback(OL* ol, int id, word* args) // win32
//long long callback(OL* ol, int id, long long* argi, double* argf, long long* others) // linux
// notes:
//	http://stackoverflow.com/questions/11270588/variadic-function-va-arg-doesnt-work-with-float

static
__attribute__((used))
long long callback(OL* ol, int id, int_t* argi
#if __amd64__
		, double* argf, int_t* rest //win64
#endif
		)
{
//	__asm("int $3");
	word* R = ol->R;

	ol->this = (word*)cdr (ol->R[NR + id]); // lambda для обратного вызова
//	ol->ticker = ol->bank ? ol->bank : 999; // зачем это? а не надо, так как без потоков работаем
//	ol->bank = 0;
	assert (is_reference(ol->this));
	assert (reftype (ol->this) != TTHREAD);

	// надо сохранить значения, иначе их уничтожит GC
	// todo: складывать их в память! и восстанавливать оттуда же
	R[NR + 0] = R[0]; // mcp?
//	R[NR + 1] = R[1]; // не надо
//	R[NR + 2] = R[2]; // не надо
	R[NR + 3] = R[3]; // continuation

	// вызовем колбек:
	R[0] = IFALSE;  // отключим mcp, мы пока не работаем с потоками из callable функций
	R[3] = IRETURN; // команда выхода из колбека
	ol->arity = 1;

	word types = car(ol->R[NR + id]);

	int a = 4;
//	R[a] = IFALSE;

	word* fp;
	fp = ol->heap.fp;

	int i = 0;
#if __amd64__ && __linux__
	int j = 0;
#endif
/*#if __amd64__  // !!!
	#if _WIN64
//	rest -= 4;
	#else
	rest -= 6;
	#endif
#endif*/
//	int f = 0; // linux
	while (types != INULL) {
		switch (car(types)) {
		case I(TVPTR): {
			void*
			#if __amd64__
				#if _WIN64
				value = i <= 4
				        ? *(void**) &argi[i]
				        : *(void**) &rest[i-4];
				#else
				value = i <= 6
						? *(void**) &argi[i]
						: *(void**) &rest[i-6]; // ???
				#endif
				i++;
			#else
				value =   *(void**) &argi[i++];
			#endif
			R[a] = (word) new_vptr(value);
			break;
		}
		case I(TINTN): // deprecated
		case I(TINTP): {
			int_t
			#if __amd64__
				#if _WIN64
				value = i <= 4
				        ? *(int_t*) &argi[i]
				        : *(int_t*) &rest[i-4];
				#else
				value = i <= 6
						? *(int_t*) &argi[i]
						: *(int_t*) &rest[i-6]; // ???
				#endif
				i++;
			#else
				value =   *(int_t*) &argi[i++];
			#endif
			R[a] = I(value);
			break;
		}
		case I(TFLOAT): {
			float
			#if __amd64__
				#if _WIN64
				value = i <= 4
				        ? *(float*) &argf[i]
				        : *(float*) &rest[i-6];
				i++;
				#else
				value = j <= 8
						? *(float*) &argf[j]
						: *(float*) &rest[j-8]; // ???
				j++;
				#endif
			#else
				value =   *(float*) &argi[i++];
			#endif
			ol->heap.fp = fp;
			R[a] = d2ol(ol, value);
			fp = ol->heap.fp;
			break;
		}
		case I(TDOUBLE): {
			double
			#if __amd64__
				#if _WIN64
				value = i <= 4
				        ? *(double*) &argf[i]
				        : *(double*) &rest[i-4];
				i++;
				#else
				value = i <= 8
						? *(double*) &argf[j]
						: *(double*) &rest[j-8]; // ???
				j++;
				#endif
			#else
				value =   *(double*) &argi[i++]; i++;
			#endif
			ol->heap.fp = fp;
			R[a] = d2ol(ol, value);
			fp = ol->heap.fp;
			break;
		}
		case I(TSTRING): {
			void*
			#if __amd64__
				#if _WIN64
				value = i <= 4
				        ? *(void**) &argi[i]
				        : *(void**) &rest[i-4];
				#else
				value = i <= 6
						? *(void**) &argi[i]
						: *(void**) &rest[i-6]; // ???
				#endif
				i++;
			#else
				value =   *(void**) &argi[i++];
			#endif
			R[a] = value ? (word)new_string(value) : IFALSE;
			break;
		}
//		case I(TVOID):
//			R[a] = IFALSE;
//			i++;
//			break;
		default: {
			void*
			#if __amd64__
				#if _WIN64
				value = i <= 4
				        ? *(void**) &argi[i]
				        : *(void**) &rest[i-4];
				#else
				value = i <= 6
						? *(void**) &argi[i]
						: *(void**) &rest[i-6]; // ???
				#endif
				i++;
			#else
				value =   *(void**) &argi[i++];
			#endif

			if (is_pair(car(types))) {
				//int l = llen(car(types));
				word tail = INULL;
				void** values = (void**)value;

				word argv = car(types);
				while (argv != INULL) {
					switch (car (argv)) {
					case I(TSTRING):
						tail = (word) new_pair(new_string(*values++), tail);
						break;
					case I(TVPTR):
						tail = (word) new_pair(new_vptr(*values++), tail);
						break;
					}
					argv = cdr (argv);
				}
				R[a] = tail;
				break;
			}

			// else error
			E("unknown argument type");
			break; }
		}
		a++;
		ol->arity++;
		types = cdr(types);
	}

	ol->heap.fp = fp;

	//word* r =
	runtime(ol);
	// возврат из колбека,
	// R, NR могли измениться
	R = ol->R;
	R[3] = R[NR + 3];
//	R[2] = R[NR + 2]; // не надо
//	R[1] = R[NR + 1]; // не надо
	R[0] = R[NR + 0]; // ??? может лучше IFALSE, ведь прежний R0 уже мог стать недействительным?

	// if result must be float or double,
	// do the __ASM__ with loading the result into fpu/xmm register

	return 0;
}
#endif//OLVM_CALLABLES
