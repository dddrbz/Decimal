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
