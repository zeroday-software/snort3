//--------------------------------------------------------------------------
// Copyright (C) 2014-2016 Cisco and/or its affiliates. All rights reserved.
// Copyright (C) 2008-2013 Sourcefire, Inc.
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License Version 2 as published
// by the Free Software Foundation.  You may not use, modify or distribute
// this program under any other version of the GNU General Public License.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//--------------------------------------------------------------------------

#include "dce_utils.h"
#include "main/snort_debug.h"

/********************************************************************
 * Function: DCE2_GetValue()
 *
 * Parses integer values up to 64 bit unsigned.  Stores the value
 * parsed in memory passed in as an argument.
 *
 * Arguments:
 *  char *
 *      Pointer to the first character in the string to parse.
 *  char *
 *      Pointer to the byte after the last character of
 *      the string to parse.
 *  void *
 *      Pointer to the memory where the parsed integer should
 *      be stored on successful parsing.
 *  int
 *      Non-zero if the parsed value should be negated.
 *      Zero if the parsed value should not be negated.
 *  DCE2_IntType
 *      The type of integer we want to parse and the integer type
 *      that the pointer that the parsed value will be put in is.
 *  uint8_t
 *      The base that the parsed value should be converted to.
 *      Only 8, 10 and 16 are supported.
 *
 * Returns:
 *  DCE2_Ret
 *      DCE2_RET__SUCCESS if we were able to successfully parse the
 *          integer to the type specified.
 *      DCE2_RET__ERROR if an error occured in parsing.
 *
 ********************************************************************/
DCE2_Ret DCE2_GetValue(char* start, char* end, void* int_value, int negate,
    DCE2_IntType int_type, uint8_t base)
{
    uint64_t value = 0;
    uint64_t place = 1;
    uint64_t max_value = 0;

    if ((end == nullptr) || (start == nullptr) || (int_value == nullptr))
        return DCE2_RET__ERROR;

    if (start >= end)
        return DCE2_RET__ERROR;

    for (end = end - 1; end >= start; end--)
    {
        uint64_t add_value;
        char c = *end;

        if ((base == 16) && !isxdigit((int)c))
            return DCE2_RET__ERROR;
        else if ((base != 16) && !isdigit((int)c))
            return DCE2_RET__ERROR;

        if (isdigit((int)c))
            add_value = (uint64_t)(c - '0') * place;
        else
            add_value = (uint64_t)((toupper((int)c) - 'A') + 10) * place;

        if ((UINT64_MAX - value) < add_value)
            return DCE2_RET__ERROR;

        value += add_value;
        place *= base;
    }

    switch (int_type)
    {
    case DCE2_INT_TYPE__INT8:
        max_value = ((UINT8_MAX - 1) / 2);
        if (negate)
            max_value++;
        break;
    case DCE2_INT_TYPE__UINT8:
        max_value = UINT8_MAX;
        break;
    case DCE2_INT_TYPE__INT16:
        max_value = ((UINT16_MAX - 1) / 2);
        if (negate)
            max_value++;
        break;
    case DCE2_INT_TYPE__UINT16:
        max_value = UINT16_MAX;
        break;
    case DCE2_INT_TYPE__INT32:
        max_value = ((UINT32_MAX - 1) / 2);
        if (negate)
            max_value++;
        break;
    case DCE2_INT_TYPE__UINT32:
        max_value = UINT32_MAX;
        break;
    case DCE2_INT_TYPE__INT64:
        max_value = ((UINT64_MAX - 1) / 2);
        if (negate)
            max_value++;
        break;
    case DCE2_INT_TYPE__UINT64:
        max_value = UINT64_MAX;
        break;
    }

    if (value > max_value)
        return DCE2_RET__ERROR;

    if (negate)
        value *= -1;

    switch (int_type)
    {
    case DCE2_INT_TYPE__INT8:
        *(int8_t*)int_value = (int8_t)value;
        break;
    case DCE2_INT_TYPE__UINT8:
        *(uint8_t*)int_value = (uint8_t)value;
        break;
    case DCE2_INT_TYPE__INT16:
        *(int16_t*)int_value = (int16_t)value;
        break;
    case DCE2_INT_TYPE__UINT16:
        *(uint16_t*)int_value = (uint16_t)value;
        break;
    case DCE2_INT_TYPE__INT32:
        *(int32_t*)int_value = (int32_t)value;
        break;
    case DCE2_INT_TYPE__UINT32:
        *(uint32_t*)int_value = (uint32_t)value;
        break;
    case DCE2_INT_TYPE__INT64:
        *(int64_t*)int_value = (int64_t)value;
        break;
    case DCE2_INT_TYPE__UINT64:
        *(uint64_t*)int_value = (uint64_t)value;
        break;
    }

    return DCE2_RET__SUCCESS;
}

const char* DCE2_UuidToStr(const Uuid* uuid, DceRpcBoFlag byte_order)
{
#define UUID_BUF_SIZE  50
    static char uuid_buf1[UUID_BUF_SIZE];
    static char uuid_buf2[UUID_BUF_SIZE];
    static int buf_num = 0;
    char* uuid_buf;

    if (buf_num == 0)
    {
        uuid_buf = uuid_buf1;
        buf_num = 1;
    }
    else
    {
        uuid_buf = uuid_buf2;
        buf_num = 0;
    }

    snprintf(uuid_buf, UUID_BUF_SIZE,
        "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
        DceRpcHtonl(&uuid->time_low, byte_order),
        DceRpcHtons(&uuid->time_mid, byte_order),
        DceRpcHtons(&uuid->time_high_and_version, byte_order),
        uuid->clock_seq_and_reserved, uuid->clock_seq_low,
        uuid->node[0], uuid->node[1], uuid->node[2],
        uuid->node[3], uuid->node[4], uuid->node[5]);

    uuid_buf[UUID_BUF_SIZE - 1] = '\0';

    return uuid_buf;
}

void DCE2_PrintPktData(const uint8_t* data, const uint16_t len)
{
    unsigned int i, j = 0, line_len = 0;
    uint8_t hex_buf[16];
    uint8_t char_buf[16];

    for (i = 0; i < len; i++)
    {
        hex_buf[j] = data[i];

        if (isascii((int)data[i]) && isprint((int)data[i]))
            char_buf[j] = data[i];
        else
            char_buf[j] = '.';

        if (line_len == 15)
        {
            unsigned int k, sub_line_len = 0;
            for (k = 0; k <= j; k++)
            {
                Debug::print(nullptr, 0, DEBUG_DCE_COMMON,"%02x ", hex_buf[k]);
                if (sub_line_len >= 7)
                {
                    Debug::print(nullptr, 0, DEBUG_DCE_COMMON,"%s"," ");
                    sub_line_len = 0;
                }
                else
                {
                    sub_line_len++;
                }
            }

            Debug::print(nullptr, 0, DEBUG_DCE_COMMON,"%s"," ");

            sub_line_len = 0;
            for (k = 0; k <= j; k++)
            {
                Debug::print(nullptr, 0, DEBUG_DCE_COMMON,"%c", char_buf[k]);
                if (sub_line_len >= 7)
                {
                    Debug::print(nullptr, 0, DEBUG_DCE_COMMON,"%s"," ");
                    sub_line_len = 0;
                }
                else
                {
                    sub_line_len++;
                }
            }

            Debug::print(nullptr, 0, DEBUG_DCE_COMMON,"%s","\n");

            j = line_len = 0;
        }
        else
        {
            j++;
            line_len++;
        }
    }

    if (line_len > 0)
    {
        unsigned int k, sub_line_len = 0;
        for (k = 0; k < j; k++)
        {
            Debug::print(nullptr, 0, DEBUG_DCE_COMMON,"%02x ", hex_buf[k]);
            if (sub_line_len >= 7)
            {
                Debug::print(nullptr, 0, DEBUG_DCE_COMMON,"%s"," ");
                sub_line_len = 0;
            }
            else
            {
                sub_line_len++;
            }
        }

        if (k < 8)
            Debug::print(nullptr, 0, DEBUG_DCE_COMMON,"%s","   ");
        else
            Debug::print(nullptr, 0, DEBUG_DCE_COMMON,"%s","  ");

        while (k < 16)
        {
            Debug::print(nullptr, 0, DEBUG_DCE_COMMON,"%s","   ");
            k++;
        }

        sub_line_len = 0;
        for (k = 0; k < j; k++)
        {
           Debug::print(nullptr, 0, DEBUG_DCE_COMMON,"%c", char_buf[k]);
            if (sub_line_len >= 7)
            {
                Debug::print(nullptr, 0, DEBUG_DCE_COMMON,"%s"," ");
                sub_line_len = 0;
            }
            else
            {
                sub_line_len++;
            }
        }
    }

    Debug::print(nullptr, 0, DEBUG_DCE_COMMON,"%s","\n");
}

