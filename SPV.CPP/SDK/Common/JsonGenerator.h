/*
 * Copyright (c) 2019 Elastos Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __JSON_GENERATOR_H__
#define __JSON_GENERATOR_H__

#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define JSON_GENERATOR_MAX_DEEPS        32

typedef struct JsonGenerator {
    size_t capacity;
    size_t pos;
    short deep;
    uint8_t state[JSON_GENERATOR_MAX_DEEPS];
    char *buffer;
} JsonGenerator;

JsonGenerator *JsonGenerator_Initialize(JsonGenerator *generator);

int JsonGenerator_WriteStartObject(JsonGenerator *generator);

int JsonGenerator_WriteEndObject(JsonGenerator *generator);

int JsonGenerator_WriteStartArray(JsonGenerator *generator);

int JsonGenerator_WriteEndArray(JsonGenerator *generator);

int JsonGenerator_WriteFieldName(JsonGenerator *generator, const char *name);

int JsonGenerator_WriteString(JsonGenerator *generator, const char *value);

int JsonGenerator_WriteNumber(JsonGenerator *generator, int value);

int JsonGenerator_WriteDouble(JsonGenerator *generator, double value);

int JsonGenerator_WriteBoolean(JsonGenerator *generator, bool value);

int JsonGenerator_WriteStringField(JsonGenerator *generator,
        const char *name, const char *value);

const char *JsonGenerator_Finish(JsonGenerator *generator);

void JsonGenerator_Destroy(JsonGenerator *generator);

#ifdef __cplusplus
}
#endif

#endif //__JSON_GENERATOR_H__