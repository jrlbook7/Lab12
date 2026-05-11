#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define SENTINEL -999999.99

double extractNumeric(const char* str) {
    int n = (int)strlen(str);
    int i = 0;

    while (i < n) {

        /* --- Skip garbage characters --- */
        /* A sign is only valid if immediately followed by a digit or '.' */
        if (str[i] == '+' || str[i] == '-') {
            int j = i;
            while (j < n && (str[j] == '+' || str[j] == '-')) j++;
            /* If nothing numeric follows the sign run, skip one char and continue */
            if (j >= n || (!isdigit((unsigned char)str[j]) && str[j] != '.')) {
                i++;
                continue;
            }
            /* Otherwise fall through to attempt parsing from i */
        } else if (!isdigit((unsigned char)str[i]) && str[i] != '.') {
            i++;
            continue;
        }

        /* --- Attempt to parse a number starting at i --- */
        int start = i;

        /* Step 1: consume consecutive signs; the last one is the effective sign */
        int sign = 1;
        if (str[i] == '+' || str[i] == '-') {
            while (i < n && (str[i] == '+' || str[i] == '-')) {
                sign = (str[i] == '-') ? -1 : 1;
                i++;
            }
        }

        /* Step 2: consume integer part */
        double intPart = 0.0;
        int intDigitCount = 0;
        while (i < n && isdigit((unsigned char)str[i])) {
            intPart = intPart * 10.0 + (str[i] - '0');
            intDigitCount++;
            i++;
        }
        /* More than 19 integer digits risks double overflow */
        if (intDigitCount > 19) return SENTINEL;

        /* Step 3: optional decimal point and fractional part */
        double fracPart = 0.0;
        int hasDot = 0;
        int hasFracDigits = 0;
        if (i < n && str[i] == '.') {
            hasDot = 1;
            i++;
            double place = 0.1;
            while (i < n && isdigit((unsigned char)str[i])) {
                fracPart += (str[i] - '0') * place;
                place *= 0.1;
                hasFracDigits = 1;
                i++;
            }
            /* A second decimal point makes the whole number invalid */
            if (i < n && str[i] == '.') return SENTINEL;
        }

        /* Must have at least one digit on either side of dot (or no dot at all) */
        int validBase = (intDigitCount > 0 || hasFracDigits);
        if (hasDot && intDigitCount == 0 && !hasFracDigits) validBase = 0; /* lone '.' */

        if (!validBase) {
            i = start + 1;
            continue;
        }

        /* Step 4: check for the "12a3.45" ambiguity rule —
           if a non-e/E letter immediately follows, and numeric chars appear beyond it,
           the whole string's numeric intent is ambiguous => invalid */
        if (i < n && isalpha((unsigned char)str[i]) &&
            str[i] != 'e' && str[i] != 'E') {
            int j = i + 1;
            while (j < n) {
                if (isdigit((unsigned char)str[j]) || str[j] == '.') return SENTINEL;
                j++;
            }
        }

        /* Step 5: optional scientific notation */
        double expValue = 0.0;
        int hasExp = 0;
        if (i < n && (str[i] == 'e' || str[i] == 'E')) {
            i++; /* consume 'e'/'E' */

            /* optional sign for exponent */
            double expSign = 1.0;
            if (i < n && (str[i] == '+' || str[i] == '-')) {
                if (str[i] == '-') expSign = -1.0;
                i++;
                /* Two consecutive signs in exponent => invalid */
                if (i < n && (str[i] == '+' || str[i] == '-')) return SENTINEL;
            }

            /* exponent must have at least one digit */
            double expAbs = 0.0;
            int expDigits = 0;
            while (i < n && isdigit((unsigned char)str[i])) {
                expAbs = expAbs * 10.0 + (str[i] - '0');
                expDigits++;
                i++;
            }

            /* exponent cannot contain a decimal point */
            if (i < n && str[i] == '.') return SENTINEL;

            if (expDigits == 0) return SENTINEL; /* missing exponent digits */

            expValue = expSign * expAbs;
            hasExp = 1;
        }

        /* Step 6: assemble and apply sign */
        double base = (intPart + fracPart) * sign;

        /* Step 7: apply exponent */
        double result = base;
        if (hasExp) {
            if (expValue > 308) {
                if (base != 0.0) return SENTINEL; /* out of range */
            } else if (expValue < -308) {
                result = 0.0; /* underflow to zero */
            } else {
                result = base * pow(10.0, expValue);
            }
        }

        /* Step 8: final range/validity check */
        if (isinf(result) || isnan(result)) return SENTINEL;

        return result;
    }

    return SENTINEL;
}

int main(void) {
    char input[1024];

    while (1) {
        printf("Enter a string (or 'END' to quit): ");
        if (!fgets(input, sizeof(input), stdin)) break;

        // Strip the trailing newline 
        int len = (int)strlen(input);
        if (len > 0 && input[len - 1] == '\n') input[--len] = '\0';

        if (strcmp(input, "END") == 0) {
            printf("Program terminated.\n");
            break;
        }

        double result = extractNumeric(input);
        if (result == SENTINEL) {
            printf("Invalid input: no valid floating-point number found\n");
        } else {
            printf("Extracted number: %.4f\n", result);
        }
        printf("\n");
    }

    return 0;
}
