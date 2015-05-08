#pragma once
#ifndef __OLVM_H__0F78631C_47C6_11E4_BBBE_64241D5D46B0__
#define	__OLVM_H__0F78631C_47C6_11E4_BBBE_64241D5D46B0__

// тут игра слов OL <> 0L
//	нулевой порог вхождения (Lisp - очень простой язык)
//	сокращение от Owl-Lisp
//	тег нумерованного списка в html - (еще одна отсылка к lisp)
struct OL;

// defaults. please don't change. use -DOPTIONSYMBOL commandline option instead
#ifndef HAS_SOCKETS
#define HAS_SOCKETS 1 // system sockets support
#endif

#ifndef HAS_DLOPEN
#define HAS_DLOPEN 1  // dlopen/dlsym support
#endif

#ifndef HAS_PINVOKE
#define HAS_PINVOKE 1 // pinvoke (for dlopen/dlsym) support
#endif

// internal option
#define NO_SECCOMP
//efine STANDALONE // самостоятельный бинарник без потоков

//-- end of options

//-- common header
#ifdef __cplusplus
	extern "C" {
#endif

// todo: add vm_free or vm_delete or vm_destroy or something

#ifndef STANDALONE
struct OL*
vm_new(unsigned char* language, void (*release)(void*));

//int vm_alive(struct OL* vm); // (возможно не нужна) проверяет, что vm еще работает

int vm_puts(struct OL* vm, char *message, int n);
int vm_gets(struct OL* vm, char *message, int n);
int vm_feof(struct OL* vm);  // все ли забрали из входящего буфера

#ifdef __cplusplus
struct OL
{
private:
	OL* vm;
public:
	OLvm(unsigned char* language) { vm = vm_new(language); }
	virtual ~OLvm() { free(vm); }

	int stop() { puts(vm, ",quit", 5); }

	int puts(char *message, int n) { vm_puts(vm, message, n);
	int gets(char *message, int n) { vm_gets(vm, message, n);
};
#else
typedef struct OL OL;
#endif

#endif

//-- end of header
#ifdef __cplusplus
	}
#endif

#endif//__OLVM_H__0F78631C_47C6_11E4_BBBE_64241D5D46B0__
