#ifndef TEST_H
#define TEST_H

typedef struct
{
    int var_int;
    double var_double;
    char var_string[128];
} t_TEST_STRUCT;

extern t_TEST_STRUCT g_test_variable;

#endif // TEST_H
