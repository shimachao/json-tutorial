#include "leptjson.h"
#include <assert.h>  /* assert() */
#include <stdlib.h>  /* NULL, strtod() */
#include <string.h>  /* strlen() */

#define EXPECT(c, ch)       do { assert(*c->json == (ch)); c->json++; } while(0)

typedef struct {
    const char* json;
}lept_context;

static void lept_parse_whitespace(lept_context* c) {
    const char *p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c->json = p;
}

static int lept_parse_literal(lept_context* c, lept_value* v, const char *literal)
{
    assert(literal != NULL);
    EXPECT(c, literal[0]);
    const char *p = literal;
    size_t len = strlen(literal) - 1;
    for (size_t i = 0; i < len; i++)
    {
        if (literal[i + 1] != c->json[i])
        {
            return LEPT_PARSE_INVALID_VALUE;
        }
    }
    c->json += len;
    switch (literal[0])
    {
    case 't':
        v->type = LEPT_TRUE;
        break;
    case 'f':
        v->type = LEPT_FALSE;
        break;
    case 'n':
        v->type = LEPT_NULL;
        break;
    }
    return LEPT_PARSE_OK;
}

#define ISDIGIT(ch)         ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch)     ((ch) >= '1' && (ch) <= '9')

static int lept_parse_number(lept_context* c, lept_value* v) {
    char* end;
    const char *p = c->json;
    // minus sign
    if (*p == '-')
    {
        p++;
    }
    // integer
    if (*p == '0')
    {
        p++;
    }
    else if (ISDIGIT1TO9(*p))
    {
        p++;
        while (ISDIGIT(*p))
        {
            p++;
        }
    }
    else
    {
        return LEPT_PARSE_INVALID_VALUE;
    }
    // decimal digits
    if (*p == '.')
    {
        p++;
        if (!ISDIGIT(*p))
        {
            return LEPT_PARSE_INVALID_VALUE;
        }
        p++;
        while (ISDIGIT(*p))
        {
            p++;
        }
    }
    // exponent
    if (*p == 'e' || *p == 'E')
    {
        p++;
        if (*p == '+' || *p == '-')
        {
            p++;
        }
        if (!ISDIGIT(*p))
        {
            return LEPT_PARSE_INVALID_VALUE;
        }
        p++;
        while (ISDIGIT(*p))
        {
            p++;
        }
    }

    if (p == c->json)
    {
        return LEPT_PARSE_INVALID_VALUE;
    }

    v->n = strtod(c->json, NULL);
    c->json = p;
    v->type = LEPT_NUMBER;
    return LEPT_PARSE_OK;
}

static int lept_parse_value(lept_context* c, lept_value* v) {
    switch (*c->json) {
        case 't':  return lept_parse_literal(c, v, "true");
        case 'f':  return lept_parse_literal(c, v, "false");
        case 'n':  return lept_parse_literal(c, v, "null");
        default:   return lept_parse_number(c, v);
        case '\0': return LEPT_PARSE_EXPECT_VALUE;
    }
}

int lept_parse(lept_value* v, const char* json) {
    lept_context c;
    int ret;
    assert(v != NULL);
    c.json = json;
    v->type = LEPT_NULL;
    lept_parse_whitespace(&c);
    if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {
        lept_parse_whitespace(&c);
        if (*c.json != '\0') {
            v->type = LEPT_NULL;
            ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    return ret;
}

lept_type lept_get_type(const lept_value* v) {
    assert(v != NULL);
    return v->type;
}

double lept_get_number(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->n;
}
