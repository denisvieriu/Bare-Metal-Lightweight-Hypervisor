#ifndef _STDARG_H
#define ARGS_H
//
//typedef unsigned char *va_list;
//#define va_start(list, param) (list = (((va_list)&param) + sizeof(param)))
//#define va_arg(ap, t)                                               \
//        ((sizeof(t) > sizeof(__int64) || (sizeof(t) & (sizeof(t) - 1)) != 0) \
//            ? **(t**)((ap += sizeof(__int64)) - sizeof(__int64))             \
//            :  *(t* )((ap += sizeof(__int64)) - sizeof(__int64)))
//#define va_end(ap)        ((void)(ap = (va_list)0))

#endif // !ARGS_H
