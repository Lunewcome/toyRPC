/** 
* @brief    Thread pool.
* @date     2014-03-28
* @Authoer  XinlaiLu(luxinlai@baidu.com), Hi(newcome_lu)
*/

#ifndef BASE_BASICS_H_
#define BASE_BASICS_H_

#define DO_NOT_COPY_AND_ASSIGN(CLASS) \
    CLASS(const CLASS &); \
    CLASS& operator=(const CLASS &);

typedef unsigned long long uint64;
typedef unsigned int uint32;
typedef unsigned char uint8;

#endif  // BASE_BASICS_H_
