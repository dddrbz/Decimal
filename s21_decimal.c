#include "s21_decimal.h"

int s21_add(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
    int res = 0;

    init_decimal(result);
    res = round_in_add(&value_1, &value_2, result);
    if (!res && is_zero_decimal(*result)) {
        if (overflow_check(value_1, value_2) == 2) {
            bank_round_of_number(value_1, &value_1);
            bank_round_of_number(value_2, &value_2);
        }
        if ((res = overflow_check(value_1, value_2)) == 0) {
            if (get_sign(value_1) && get_sign(value_2)) {
                little_add(value_1, value_2, result);
                s21_negate(*result, result);
            } else if (get_sign(value_1)) {
                s21_negate(value_1, &value_1);
                little_sub(value_2, value_1, result);
                s21_negate(value_1, &value_1);
            } else if (get_sign(value_2)) {
                s21_negate(value_2, &value_2);
                little_sub(value_1, value_2, result);
                s21_negate(value_2, &value_2);
            } else {
                little_add(value_1, value_2, result);
            }
            set_scale(get_scale(value_1), result);
        } else {
            if (get_sign(value_1) && get_sign(value_2)) res = 2;
        }
    }
    return res;
}

int s21_sub(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
    int res = 0;

    init_decimal(result);
    res = compare_scale(&value_1, &value_2);
    if (!res) {
        if (get_sign(value_1) && get_sign(value_2)) {
            little_sub(value_1, value_2, result);
            s21_negate(*result, result);
        } else if (get_sign(value_1)) {
            s21_negate(value_2, &value_2);
            res = s21_add(value_1, value_2, result);
        } else if (get_sign(value_2)) {
            s21_negate(value_2, &value_2);
            res = s21_add(value_1, value_2, result);
        } else {
            little_sub(value_1, value_2, result);
        }
        set_scale(get_scale(value_1), result);
    }
    return res;
}

int s21_mul(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
    int res = 0, scale_1 = get_scale(value_1), scale_2 = get_scale(value_2);
    int result_sign = (get_sign(value_1) ^ get_sign(value_2));
    int scale_sum = scale_1 + scale_2;

    init_decimal(result);
    set_scale(0, &value_1);
    set_scale(0, &value_2);
    for (int i = 0; i <= get_first_position(value_2); ++i) {
        if (get_decimal_bit(value_2, i)) {
            if (scale_sum) {
                for (int i = 1;
                     scale_sum > 0 && overflow_check(value_1, *result) == 1;
                     scale_sum--, i++) {
                    set_scale(i, result);
                }
            }
            res = s21_add(*result, value_1, result);
        }
        if ((move_left_decimal_onese(&value_1) &&
             i != get_first_position(value_2)) ||
            res) {
            if (scale_sum && !res) {
                move_right_decimal_onese(&value_1);
                set_int_bit(&value_1.bits[2], 31, 1);
                move_right_decimal_onese(result);
                scale_sum--;
            } else {
                res = 1;
                break;
            }
        }
    }
    for (; scale_sum > 28; scale_sum--) {
        set_scale(1, result);
        bank_round_of_number(*result, result);
    }
    set_scale(scale_sum, result);
    set_int_bit(&result->bits[3], 31, result_sign);
    if (res && result_sign) res = 2;
    return res;
}

int s21_div(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
    int flag = 0, res = 0;

    if (is_zero_decimal(value_2)) {
        res = 3;
    } else {
        init_decimal(result);
        if (get_sign(value_1)) {
            s21_negate(value_1, &value_1);
            flag = 1;
        }
        if (get_sign(value_2)) {
            s21_negate(value_2, &value_2);
            flag = 1;
        }
        compare_scale(&value_1, &value_2);
        if (is_zero_decimal(value_2)) {
            res = 1;
        } else if (is_zero_decimal(value_1)) {
            res = 2;
        } else {
            div_integer_part(&value_1, &value_2, result);
            div_fractional_part(&value_1, &value_2, result);
            if (flag) {
                s21_negate(*result, result);
            }
        }
    }
    return res;
}

int s21_mod(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
    s21_decimal factor = {0};
    int flag = 0, res = 0;

    if (get_sign(value_1)) {
        flag = 1;
        s21_negate(value_1, &value_1);
    }
    if (get_sign(value_2)) s21_negate(value_2, &value_2);
    init_decimal(result);
    if (is_zero_decimal(value_2)) res = 1;
    if (s21_is_less(value_1, value_2)) {
        equate_decimal(value_1, result);
    } else {
        res = compare_scale(&value_1, &value_2);
        if (is_zero_decimal(value_1) || is_zero_decimal(value_2)) {
            res = 1;
        } else {
            equate_decimal(value_1, &factor);
            div_integer_part(&factor, &value_2, result);
            s21_mul(*result, value_2, result);
            s21_sub(value_1, *result, result);
        }
    }
    if (flag && (!is_zero_decimal(*result))) {
        s21_negate(*result, result);
    }
    return res;
}

int s21_is_less(s21_decimal value_1, s21_decimal value_2) {
    int res = 0;
    int sign_value_1 = get_sign(value_1), sign_value_2 = get_sign(value_2);

    if (sign_value_1 < sign_value_2) {
        res = 0;
    } else if (sign_value_1 > sign_value_2) {
        res = 1;
    } else if (sign_value_1 == sign_value_2) {
        compare_scale(&value_1, &value_2);
        for (int i = 95; i >= 0; --i) {
            int bit_value_1 = get_decimal_bit(value_1, i);
            int bit_value_2 = get_decimal_bit(value_2, i);
            if (bit_value_1 < bit_value_2) {
                res = 1;
                break;
            } else if (bit_value_1 > bit_value_2) {
                res = 0;
                break;
            }
        }
        if (sign_value_1 == 1) res = (res) ? 0 : 1;
    }
    return res;
}

int s21_is_less_or_equal(s21_decimal value_1, s21_decimal value_2) {
    int res = 0;

    if (s21_is_less(value_1, value_2) || s21_is_equal(value_1, value_2)) {
        res = 1;
    }
    return res;
}

int s21_is_greater(s21_decimal value_1, s21_decimal value_2) {
    int res = 0;

    if (!s21_is_less(value_1, value_2) && !s21_is_equal(value_1, value_2)) {
        res = 1;
    }
    return res;
}

int s21_is_greater_or_equal(s21_decimal value_1, s21_decimal value_2) {
    int res = 0;

    if (s21_is_greater(value_1, value_2) || s21_is_equal(value_1, value_2)) {
        res = 1;
    }
    return res;
}

int s21_is_equal(s21_decimal value_1, s21_decimal value_2) {
    int res = 0;

    compare_scale(&value_1, &value_2);
    for (int i = 3; i >= 0; --i) {
        if (value_1.bits[i] == value_2.bits[i]) {
            if (i == 0) res = 1;
        } else {
            break;
        }
    }
    return res;
}

int s21_is_not_equal(s21_decimal value_1, s21_decimal value_2) {
    return (!s21_is_equal(value_1, value_2)) ? 1 : 0;
}

int s21_from_int_to_decimal(int src, s21_decimal *dst) {
    int error = 0;

    if (dst) {
        init_decimal(dst);
        if (src < 0) {
            set_int_bit(&dst->bits[3], 31, 1);
            src *= -1;
        }
        dst->bits[0] = src;
    } else {
        error = 1;
    }
    return error;
}

int s21_from_float_to_decimal(float src, s21_decimal *dst) {
    int error = 0;

    if (dst && src <= 79228162514264337593543950335.0 &&
        src >= -79228162514264337593543950335.0 &&
        (isnormal(src) || src == 0)) {
        init_decimal(dst);
        int minus_flag = 0;
        if (src < 0) {
            src *= -1;
            minus_flag = 1;
        }
        convert_integer_part(src, dst);
        error = convert_fractional_part(src, dst);
        if (minus_flag) set_int_bit(&dst->bits[3], 31, 1);
    } else {
        error = 1;
    }
    return error;
}

int s21_from_decimal_to_int(s21_decimal src, int *dst) {
    int error = 0;
    s21_decimal clean_src = {0};

    if (dst) {
        s21_truncate(src, &clean_src);
        *dst = clean_src.bits[0];
        if (get_int_bit(src.bits[3], 31)) *dst *= -1;
    } else {
        error = 1;
    }
    return error;
}

int s21_from_decimal_to_float(s21_decimal src, float *dst) {
    int error = 0;

    if (dst) {
        *dst = 0;
        int scale = get_scale(src);
        double tmp_test = 0;
        for (int i = 0; i <= 2; i++) {
            double tmp = (unsigned int)src.bits[i];
            tmp_test += tmp * (pow(2, i * 32) / pow(10, scale));
        }
        *dst = (float)tmp_test;
        if (get_int_bit(src.bits[3], 31)) {
            *dst *= -1;
        }
    } else {
        error = 1;
    }
    return error;
}

int s21_floor(s21_decimal value, s21_decimal *result) {
    s21_decimal one = {{1, 0, 0, 0}};
    int sign = get_sign(value);

    init_decimal(result);
    if (sign) s21_negate(value, &value);
    s21_truncate(value, result);
    if (sign && s21_is_not_equal(value, *result)) {
        little_add(*result, one, result);
    }
    if (sign) s21_negate(*result, result);
    return 0;
}

int s21_round(s21_decimal value, s21_decimal *result) {
    s21_decimal one = {{1, 0, 0, 0}};
    s21_decimal five = {{5, 0, 0, 0}};
    s21_decimal mod = {0};
    int sign = get_sign(value);

    init_decimal(result);
    if (sign) s21_negate(value, &value);
    s21_truncate(value, result);
    s21_mod(value, one, &mod);
    set_scale(1, &five);
    if (s21_is_less_or_equal(five, mod)) {
        little_add(*result, one, result);
    }
    if (sign) s21_negate(*result, result);
    return 0;
}

int s21_truncate(s21_decimal value, s21_decimal *result) {
    int scale = get_scale(value);
    int sign = get_sign(value);
    s21_decimal one = {{1, 0, 0, 0}};

    init_decimal(result);
    if (sign) s21_negate(value, &value);
    s21_mod(value, one, result);
    s21_sub(value, *result, result);
    demultiply_scale(result, scale);
    if (sign) s21_negate(*result, result);
    return 0;
}

int s21_negate(s21_decimal value, s21_decimal *result) {
    int mask = -2147483648;

    init_decimal(result);
    equate_decimal(value, result);
    result->bits[3] ^= mask;
    return 0;
}

char get_int_bit(int number, int position) {
    char res = 0;

    number = number >> position;
    if (number & 1) res = 1;
    return res;
}

char get_decimal_bit(s21_decimal decimal, int position) {
    char res = 0;

    res = get_int_bit(decimal.bits[position / 32], position % 32);
    return res;
}

void set_int_bit(int *number, int position, int bit_value) {
    int number_copy = 1;

    if (get_int_bit(*number, position) != bit_value) {
        number_copy = number_copy << position;
        *number ^= number_copy;
    }
}

char add_int(int value_1, int value_2, int *result, int transfer) {
    char sum = 0;
    int bit_sum = 0;
    char p1 = 0;
    char p2 = 0;

    for (char pos = 0; pos <= 31; pos++) {
        p1 = get_int_bit(value_1, pos);
        p2 = get_int_bit(value_2, pos);
        if ((p1 ^ p2))
            sum = 1;
        else
            sum = 0;
        if (sum ^ transfer)
            sum = 1;
        else
            sum = 0;
        set_int_bit(&bit_sum, pos, sum);
        if ((p1 && p2) || (p1 && transfer) || (transfer && p2))
            transfer = 1;
        else
            transfer = 0;
    }
    *result = bit_sum;
    return transfer;
}

int round_in_add(s21_decimal *value_1, s21_decimal *value_2,
                 s21_decimal *result) {
    s21_decimal cp_value_1 = {0};
    s21_decimal cp_value_2 = {0};
    int res = 0;

    equate_decimal(*value_1, &cp_value_1);
    equate_decimal(*value_2, &cp_value_2);
    compare_scale(&cp_value_1, &cp_value_2);
    if (is_zero_decimal(cp_value_2) && !is_zero_decimal(*value_2)) {
        res = bank_round(value_1, *value_2);
        if (!res) equate_decimal(*value_1, result);
    } else if (is_zero_decimal(cp_value_1) && !is_zero_decimal(*value_1)) {
        res = bank_round(value_2, *value_1);
        if (!res) equate_decimal(*value_2, result);
    } else {
        equate_decimal(cp_value_1, value_1);
        equate_decimal(cp_value_2, value_2);
    }
    return res;
}

void little_add(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
    int decimal_transfer = 0;

    decimal_transfer = add_int(value_1.bits[0], value_2.bits[0],
                               &result->bits[0], decimal_transfer);
    decimal_transfer = add_int(value_1.bits[1], value_2.bits[1],
                               &result->bits[1], decimal_transfer);
    add_int(value_1.bits[2], value_2.bits[2], &result->bits[2],
            decimal_transfer);
}

int overflow_check(s21_decimal value_1, s21_decimal value_2) {
    int fone_value_1 = get_first_position(value_1);
    int fone_value_2 = get_first_position(value_2);
    int fzero_value_1 = get_first_zero(value_1);
    int fzero_value_2 = get_first_zero(value_2);
    int scale = get_scale(value_1), res = 0;

    if (get_sign(value_1) == get_sign(value_2)) {
        if (fone_value_1 > 94) {
            if (fone_value_2 > fzero_value_1) {
                res = 2;
                if (!scale) res = 1;
            }
        }
        if (fone_value_2 > 94) {
            if (fone_value_1 > fzero_value_2) {
                res = 2;
                if (!scale) res = 1;
            }
        }
    }
    return res;
}

int get_first_zero(s21_decimal decimal) {
    int position = -1;

    for (int i = get_first_position(decimal); i >= 0; --i) {
        if (!get_decimal_bit(decimal, i)) {
            position = i;
            break;
        }
    }
    return position;
}

int bank_round(s21_decimal *decimal, s21_decimal number) {
    s21_decimal five = {{5, 0, 0, 0}};
    s21_decimal one = {{1, 0, 0, 0}};
    int res = 0;

    if (get_scale(number) != 0) {
        set_scale(get_scale(number) - 1, &number);
    }
    if (get_sign(number)) {
        s21_negate(number, &number);
        s21_sub(*decimal, one, decimal);
    }
    if (s21_is_less(five, number)) {
        res = s21_add(*decimal, one, decimal);
    } else if (s21_is_equal(five, number)) {
        if (get_decimal_bit(*decimal, 0)) {
            res = s21_add(*decimal, one, decimal);
        }
    }
    return res;
}

int compare_scale(s21_decimal *value_1, s21_decimal *value_2) {
    int scale_value_1 = get_scale(*value_1),
        scale_value_2 = get_scale(*value_2);
    int res_1_m = 0, res_2_m = 0;
    int res_1_d = 0, res_2_d = 0;
    s21_decimal cp_value_1 = {0}, cp_value_2 = {0};

    equate_decimal(*value_1, &cp_value_1);
    equate_decimal(*value_2, &cp_value_2);
    while (get_scale(*value_1) != get_scale(*value_2)) {
        if (!res_1_m && !res_2_m) {
            if (scale_value_1 < scale_value_2) {
                res_1_m = multiply_scale(&cp_value_1, 1);
                if (!res_1_m) equate_decimal(cp_value_1, value_1);
            } else if (scale_value_1 > scale_value_2) {
                res_2_m = multiply_scale(&cp_value_2, 1);
                if (!res_2_m) equate_decimal(cp_value_2, value_2);
            }
        } else if (res_1_m) {
            res_2_d = bank_round_of_number(cp_value_2, &cp_value_2);
            if (!res_2_d)
                equate_decimal(cp_value_2, value_2);
            else
                break;
        } else if (res_2_m) {
            res_1_d = bank_round_of_number(cp_value_1, &cp_value_1);
            if (!res_1_d)
                equate_decimal(cp_value_1, value_1);
            else
                break;
        }
    }
    return (res_1_d || res_2_d) ? 1 : 0;
}

void equate_decimal(s21_decimal basic, s21_decimal *decimal) {
    init_decimal(decimal);
    for (int i = 0; i < 4; ++i) {
        decimal->bits[i] = basic.bits[i];
    }
}

void little_sub(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
    s21_decimal reverse_value = {0};

    init_decimal(result);
    if (s21_is_less(value_1, value_2)) {
        reverse_value = reverse_decimal(value_1);
        little_add(value_2, reverse_value, result);
        s21_negate(*result, result);
    } else if (s21_is_greater(value_1, value_2)) {
        reverse_value = reverse_decimal(value_2);
        little_add(value_1, reverse_value, result);
    }
}

void div_integer_part(s21_decimal *value_1, s21_decimal *value_2,
                      s21_decimal *result) {
    int first_value_1 = 0, first_value_2 = 0;

    if (s21_is_less_or_equal(*value_2, *value_1)) {
        first_value_1 = get_first_position(*value_1);
        first_value_2 = get_first_position(*value_2);
        move_left_decimal(value_2, first_value_1 - first_value_2);
        while (get_first_position(*value_2) >= first_value_2) {
            s21_sub(*value_1, *value_2, value_1);
            if (get_sign(*value_1)) {
                s21_add(*value_1, *value_2, value_1);
                set_int_bit(&result->bits[0], 0, 0);
            } else {
                set_int_bit(&result->bits[0], 0, 1);
            }
            if (get_first_position(*value_2) == first_value_2) break;
            move_right_decimal_onese(value_2);
            move_left_decimal_onese(result);
        }
    }
}

void div_fractional_part(s21_decimal *value_1, s21_decimal *value_2,
                         s21_decimal *result) {
    int scale = 0;
    s21_decimal ten = {{10, 0, 0, 0}};
    s21_decimal one = {{1, 0, 0, 0}};
    s21_decimal digit = {0};
    s21_decimal five = {{5, 0, 0, 0}};

    while (!is_zero_decimal(*value_1)) {
        init_decimal(&digit);
        while (s21_is_less(*value_1, *value_2)) {
            ++scale;
            s21_mul(*value_1, ten, value_1);
            if (scale == 29) break;
        }
        while (s21_is_less_or_equal(*value_2, *value_1)) {
            s21_sub(*value_1, *value_2, value_1);
            s21_add(digit, one, &digit);
        }
        if (scale == 29) {
            if (s21_is_less(five, digit)) {
                set_scale(28, &one);
                s21_add(*result, one, result);
            } else if (s21_is_equal(digit, five) &&
                       get_int_bit(result->bits[0], 0)) {
                set_scale(28, &one);
                s21_add(*result, one, result);
            }
            break;
        }
        set_scale(scale, &digit);
        s21_add(*result, digit, result);
    }
}

int multiply_scale(s21_decimal *decimal, int scale) {
    int res = 0;
    s21_decimal factor = {0};

    equate_decimal(*decimal, &factor);
    if (get_scale(*decimal) + scale <= 28) {
        for (int i = 0; i < scale; i++) {
            for (int j = 0; j < 9; ++j) {
                res = s21_add(*decimal, factor, decimal);
                if (res) break;
            }
            if (res) break;
            equate_decimal(*decimal, &factor);
        }
        if (!res) set_scale(get_scale(*decimal) + scale, decimal);
    }
    return res;
}

int demultiply_scale(s21_decimal *decimal, int scale) {
    int decimal_scale = get_scale(*decimal);
    s21_decimal ten = {{10, 0, 0, 0}};
    int res = 0;

    if (decimal_scale >= scale) {
        for (int i = 0; i < scale; ++i) {
            res = s21_div(*decimal, ten, decimal);
            if (res) break;
        }
        if (!res) set_scale(decimal_scale - scale, decimal);
    } else {
        res = 1;
    }
    return res;
}

int bank_round_of_number(s21_decimal decimal, s21_decimal *result) {
    s21_decimal ten = {{10, 0, 0, 0}};
    s21_decimal one = {{1, 0, 0, 0}};
    s21_decimal difference = {0};
    s21_decimal round = {0};
    int flag = 0, res = 0, scale = get_scale(decimal);

    if (scale >= 1) {
        init_decimal(result);
        if (get_sign(decimal)) {
            s21_negate(decimal, &decimal);
            flag = 1;
        }
        set_scale(0, &decimal);
        s21_mod(decimal, ten, &round);
        s21_sub(ten, round, &difference);
        if (s21_is_less(difference, round)) {
            s21_add(decimal, difference, result);
            s21_div(*result, ten, result);
        } else if (s21_is_greater(difference, round)) {
            s21_sub(decimal, round, result);
            s21_div(*result, ten, result);
        } else if (s21_is_equal(difference, round)) {
            s21_sub(decimal, round, &decimal);
            s21_div(decimal, ten, result);
            if (get_int_bit(result->bits[0], 0)) {
                s21_add(*result, one, result);
            }
        }
        set_scale(scale - 1, result);
        if (flag) {
            s21_negate(*result, result);
        }
    } else {
        res = 1;
    }
    return res;
}

// void int_render(int number) {
//     for (int i = 31; i >= 0; i--) {
//         long mask = pow(2, i);
//         if (number & mask)
//             printf("1");
//         else
//             printf("0");
//         if (!(i % 8)) printf(" ");
//     }
// }

int is_zero_decimal(s21_decimal decimal) {
    int res = 0;

    if (decimal.bits[2] == 0 && decimal.bits[1] == 0 && decimal.bits[0] == 0) {
        res = 1;
    }
    return res;
}

void move_right_decimal_onese(s21_decimal *decimal) {
    for (int i = 0; i <= 2; ++i) {
        decimal->bits[i] = decimal->bits[i] >> 1;
        if (i < 2 && get_int_bit(decimal->bits[i + 1], 0)) {
            set_int_bit(&decimal->bits[i], 31, 1);
        } else {
            set_int_bit(&decimal->bits[i], 31, 0);
        }
    }
}

// Сегодня вечером, в 21:45 то бишь без пятнадцати десять

int move_left_decimal_onese(s21_decimal *decimal) {
    int res = 0;

    if (get_decimal_bit(*decimal, 95)) {
        res = 1;
    } else {
        for (int i = 2; i >= 0; --i) {
            decimal->bits[i] = decimal->bits[i] << 1;
            if (i > 0 && get_int_bit(decimal->bits[i - 1], 31)) {
                set_int_bit(&decimal->bits[i], 0, 1);
            }
        }
    }
    return res;
}

int move_left_decimal(s21_decimal *decimal, int num) {
    int res = 0;

    for (int i = 0; i < num; ++i) {
        if ((res = move_left_decimal_onese(decimal))) break;
    }
    return res;
}

void init_decimal(s21_decimal *decimal) {
    decimal->bits[0] = 0;
    decimal->bits[1] = 0;
    decimal->bits[2] = 0;
    decimal->bits[3] = 0;
}

// void decimal_render(s21_decimal decimal) {
//     for (int j = 3; j >= 0; j--) {
//         int_render(decimal.bits[j]);
//         printf("bits[%d]\n", j);
//     }
// }

s21_decimal reverse_decimal(s21_decimal value) {
    s21_decimal result = {0}, one = {0};

    set_int_bit(&one.bits[0], 0, 1);
    for (int i = 2; i >= 0; --i) {
        value.bits[i] = ~value.bits[i];
    }
    little_add(value, one, &result);
    return result;
}

int get_first_position(s21_decimal value) {
    int position = -1;

    for (int i = 2; i >= 0; --i) {
        if (value.bits[i] == 0) continue;
        for (int j = 0; j <= 31; ++j) {
            if (get_int_bit(value.bits[i], j)) {
                position = j + i * 32;
            }
        }
        break;
    }
    return position;
}

int get_scale(s21_decimal value) {
    int scale = 0;

    for (int i = 7; i >= 0; --i) {
        if (get_int_bit(value.bits[3], 16 + i)) {
            scale += pow(2, i);
        }
    }
    return scale;
}

void set_scale(int scale, s21_decimal *value) {
    if (0 <= scale && scale <= 28) {
        for (int i = 16; i <= 23; i++) {
            int mask = pow(2, i - 16);
            if (scale & mask)
                set_int_bit(&value->bits[3], i, 1);
            else
                set_int_bit(&value->bits[3], i, 0);
        }
    }
}

int get_sign(s21_decimal value) { return get_int_bit(value.bits[3], 31); }

void convert_integer_part(float src, s21_decimal *dst) {
    float integer_part = 0;
    double depth = 0;
    int digit = 0;
    s21_decimal dec_tmp = {0}, dec_10 = {0};

    s21_from_int_to_decimal(10, &dec_10);
    modff(src, &integer_part);
    if (integer_part) {
        depth = 1;
        while ((integer_part / depth) >= 10) depth *= 10;
    }
    for (int i = 0; depth >= 1; i++) {
        digit = integer_part / depth;
        integer_part -= digit * depth;
        if (i != 0) s21_mul(*dst, dec_10, dst);
        s21_from_int_to_decimal(digit, &dec_tmp);
        s21_add(*dst, dec_tmp, dst);
        depth /= 10;
    }
}

int convert_fractional_part(float src, s21_decimal *dst) {
    float integer_part = 0;
    int degree = 0, digit = 0, error = 0;
    s21_decimal dec_tmp = {0}, dec_10 = {0}, zero = {0}, one = {{1, 0, 0, 0}};

    s21_from_int_to_decimal(10, &dec_10);
    modff(src, &integer_part);
    double fractional_part = src - integer_part;
    while (fractional_part) {
        digit = (int)(fractional_part * 10);
        s21_from_int_to_decimal(digit, &dec_tmp);
        degree++;
        set_scale(degree, &dec_tmp);
        error = s21_add(*dst, dec_tmp, dst);
        fractional_part = fmod(fractional_part * 10, 1);
        if (degree == 28) {
            digit = (int)(fractional_part * 10);
            if ((digit > 5) || (digit == 5 && get_int_bit(dst->bits[0], 0))) {
                set_scale(degree, &one);
                s21_add(*dst, one, dst);
            }
            if (s21_is_equal(*dst, zero)) error = 1;
            break;
        }
    }
    return error;
}

void decimal_to_string(s21_decimal decimal, char *str) {
    for (int j = 3; j >= 0; j--) {
        for (int i = 31; i >= 0; i--) {
            long mask = pow(2, i);
            if (decimal.bits[j] & mask)
                *str = '1';
            else
                *str = '0';
            str++;
        }
        *str = ' ';
        str++;
    }
    str--;
    *str = '\0';
}

void string_to_decimal(const char *str, s21_decimal *decimal) {
    int i = 0;

    init_decimal(decimal);
    for (int j = 0; j < 4; j++) {
        int k = 32 + j * 33;
        unsigned int tmp = 0;
        for (; i < k; i++) {
            tmp += ((unsigned int)str[i] - 48) * pow(2, 31 - i + 33 * j);
        }
        decimal->bits[3 - j] = tmp;
        i++;
    }
}

int change_bit(int number, int index) {
    int mask = 1 << (index % 32);

    return number ^ mask;
}
