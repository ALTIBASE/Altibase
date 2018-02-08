/*
 * Copyright 2011, Altibase Corporation or its subsidiaries.
 * All rights reserved.
 */

#ifndef __EXCEPTION_H__
#define __EXCEPTION_H__

#define EXCEPTION(label_name)                   \
    goto EXCEPTION_END_LABEL;                   \
label_name :

#define EXCEPTION_END                           \
    EXCEPTION_END_LABEL:                        \
    do                                          \
    {                                           \
    } while (0)

#define EXCEPTION_TEST(expression)              \
    do                                          \
    {                                           \
        if (expression)                         \
        {                                       \
            goto EXCEPTION_END_LABEL;           \
        }                                       \
        else                                    \
        {                                       \
        }                                       \
    } while (0)

#define EXCEPTION_TEST_RAISE(expression, label_name)    \
    do                                                  \
    {                                                   \
        if (expression)                                 \
        {                                               \
            goto label_name;                            \
        }                                               \
        else                                            \
        {                                               \
        }                                               \
    } while(0)

#define EXCEPTION_RAISE(label_name)             \
    do                                          \
    {                                           \
        goto label_name;                        \
    } while(0)

#endif /* __EXCEPTION_H__ */
