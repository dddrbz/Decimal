#ifndef SRC_S21_DECIMAL_H_
#define SRC_S21_DECIMAL_H_

#include <math.h>
#include <stdio.h>

typedef struct {
    int bits[4];
} s21_decimal;

int s21_add(s21_decimal value_1, s21_decimal value_2, s21_decimal *result);
int s21_sub(s21_decimal value_1, s21_decimal value_2, s21_decimal *result);
int s21_mul(s21_decimal value_1, s21_decimal value_2, s21_decimal *result);
int s21_div(s21_decimal value_1, s21_decimal value_2, s21_decimal *result);
int s21_mod(s21_decimal value_1, s21_decimal value_2, s21_decimal *result);

int s21_is_equal(s21_decimal value_1, s21_decimal value_2);
int s21_is_not_equal(s21_decimal value_1, s21_decimal value_2);
int s21_is_less(s21_decimal value_1, s21_decimal value_2);
int s21_is_less_or_equal(s21_decimal value_1, s21_decimal value_2);
int s21_is_greater(s21_decimal value_1, s21_decimal value_2);
int s21_is_greater_or_equal(s21_decimal value_1, s21_decimal value_2);

int s21_truncate(s21_decimal value, s21_decimal *result);
int s21_floor(s21_decimal value, s21_decimal *result);
int s21_negate(s21_decimal value, s21_decimal *result);
int s21_round(s21_decimal value, s21_decimal *result);

int s21_from_int_to_decimal(int src, s21_decimal *dst);
int s21_from_float_to_decimal(float src, s21_decimal *dst);
int s21_from_decimal_to_float(s21_decimal src, float *dst);
int s21_from_decimal_to_int(s21_decimal src, int *dst);

void init_decimal(s21_decimal *decimal);
void int_render(int number);
void decimal_render(s21_decimal decimal);
char get_int_bit(int number, int position);
char get_decimal_bit(s21_decimal decimal, int position);
void equate_decimal(s21_decimal basic, s21_decimal *decimal);
void set_int_bit(int *number, int position, int bit_value);
char add_int(int value_1, int value_2, int *result, int transfer);
s21_decimal reverse_decimal(s21_decimal value);
void move_right_decimal_onese(s21_decimal *decimal);
int move_left_decimal_onese(s21_decimal *decimal);
int move_left_decimal(s21_decimal *decimal, int num);
int bank_round_of_number(s21_decimal decimal, s21_decimal *result);
int bank_round(s21_decimal *decimal, s21_decimal number);
int round_in_add(s21_decimal *value_1, s21_decimal *value_2,
                 s21_decimal *result);
int compare_scale(s21_decimal *value_1, s21_decimal *value_2);
int get_first_position(s21_decimal value);
int get_scale(s21_decimal value);
void set_scale(int scale, s21_decimal *value);
int multiply_scale(s21_decimal *decimal, int scale);
int demultiply_scale(s21_decimal *decimal, int scale);
int get_sign(s21_decimal value);
void little_add(s21_decimal value_1, s21_decimal value_2, s21_decimal *result);
int overflow_check(s21_decimal value_1, s21_decimal value_2);
int get_first_zero(s21_decimal decimal);
int is_zero_decimal(s21_decimal decimal);
void little_sub(s21_decimal value_1, s21_decimal value_2, s21_decimal *result);
void div_integer_part(s21_decimal *value_1, s21_decimal *value_2,
                      s21_decimal *result);
void div_fractional_part(s21_decimal *value_1, s21_decimal *value_2,
                         s21_decimal *result);
void decimal_to_string(s21_decimal decimal, char *str);
void string_to_decimal(const char *str, s21_decimal *decimal);
int convert_fractional_part(float src, s21_decimal *dst);
void convert_integer_part(float src, s21_decimal *dst);
int change_bit(int number, int index);

#endif  // SRC_S21_DECIMAL_H_
