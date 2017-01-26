/*
 * Copyright (c) 2015-2016 Ken Bannister. All rights reserved.
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     examples
 * @{
 *
 * @file
 * @brief       gcoap CLI support
 *
 * @author      Ken Bannister <kb2ma@runbox.com>
 *
 * @}
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "net/gnrc/coap.h"
#include "od.h"
#include "fmt.h"
#include "saul_reg.h"
#include "saul.h"
#include "bme280_params.h"
#include "bme280.h"

static void _resp_handler(unsigned req_state, coap_pkt_t* pdu);
static ssize_t _stats_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len);
// static ssize_t _saul_handler_0(coap_pkt_t* pdu, uint8_t *buf, size_t len);
// static ssize_t _saul_handler_1(coap_pkt_t* pdu, uint8_t *buf, size_t len);
static ssize_t _bme280_temperature(coap_pkt_t* pdu, uint8_t *buf, size_t len);
static ssize_t _bme280_pressure(coap_pkt_t* pdu, uint8_t *buf, size_t len);
static ssize_t _bme280_humidity(coap_pkt_t* pdu, uint8_t *buf, size_t len);

static bme280_t bme280_dev;

/* CoAP resources */
static const coap_resource_t _resources[] = {
    { "/cli/stats", COAP_GET, _stats_handler },
    { "/humidity", COAP_GET, _bme280_humidity },
    { "/pressure", COAP_GET, _bme280_pressure },
    { "/temperature", COAP_GET, _bme280_temperature },
    // { "/saul/0", COAP_PUT, _saul_handler_0 },
    // { "/saul/1", COAP_GET, _saul_handler_1 },
};

static gcoap_listener_t _listener = {
    (coap_resource_t *)&_resources[0],
    sizeof(_resources) / sizeof(_resources[0]),
    NULL
};

/* Counts requests sent by CLI. */
static uint16_t req_count = 0;

/*
 * Response callback.
 */
static void _resp_handler(unsigned req_state, coap_pkt_t* pdu)
{
    if (req_state == GCOAP_MEMO_TIMEOUT) {
        printf("gcoap: timeout for msg ID %02u\n", coap_get_id(pdu));
        return;
    }
    else if (req_state == GCOAP_MEMO_ERR) {
        printf("gcoap: error in response\n");
        return;
    }

    char *class_str = (coap_get_code_class(pdu) == COAP_CLASS_SUCCESS)
                            ? "Success" : "Error";
    printf("gcoap: response %s, code %1u.%02u", class_str,
                                                coap_get_code_class(pdu),
                                                coap_get_code_detail(pdu));
    if (pdu->payload_len) {
        if (pdu->content_type == COAP_FORMAT_TEXT
                || pdu->content_type == COAP_FORMAT_LINK
                || coap_get_code_class(pdu) == COAP_CLASS_CLIENT_FAILURE
                || coap_get_code_class(pdu) == COAP_CLASS_SERVER_FAILURE) {
            /* Expecting diagnostic payload in failure cases */
            printf(", %u bytes\n%.*s\n", pdu->payload_len, pdu->payload_len,
                                                          (char *)pdu->payload);
        }
        else {
            printf(", %u bytes\n", pdu->payload_len);
            od_hex_dump(pdu->payload, pdu->payload_len, OD_WIDTH_DEFAULT);
        }
    }
    else {
        printf(", empty payload\n");
    }
}

int bme280_start(void) {
    int result = 1;
    result = bme280_init(&bme280_dev, &bme280_params[0]);
    if (result == -1) {
        puts("[Error] The given i2c is not enabled");
        return 1;
    }

    if (result == -2) {
        printf("[Error] The sensor did not answer correctly at address 0x%02X\n", TEST_I2C_ADDR);
        return 1;
    }
    return 0;
}

static ssize_t _bme280_temperature(coap_pkt_t* pdu, uint8_t *buf, size_t len)
{
    int16_t temperature;
    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
    temperature = bme280_read_temperature(&bme280_dev);
    size_t payload_len = fmt_s16_dfp((char *)pdu->payload, temperature,2);
    return gcoap_finish(pdu, payload_len, COAP_FORMAT_TEXT);
}

static ssize_t _bme280_pressure(coap_pkt_t* pdu, uint8_t *buf, size_t len)
{
    uint32_t pressure;
    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
    pressure = bme280_read_pressure(&bme280_dev);
    size_t payload_len = fmt_u32_dec((char *)pdu->payload, pressure);
    return gcoap_finish(pdu, payload_len, COAP_FORMAT_TEXT);
}

static ssize_t _bme280_humidity(coap_pkt_t* pdu, uint8_t *buf, size_t len)
{
    uint16_t humidity;
    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
    // Temp en press have to be read for calib
    bme280_read_temperature(&bme280_dev);
    bme280_read_pressure(&bme280_dev);
    humidity = bme280_read_humidity(&bme280_dev);
    size_t payload_len = fmt_s16_dfp((char *)pdu->payload, humidity,2);
    return gcoap_finish(pdu, payload_len, COAP_FORMAT_TEXT);
}

/*
 * Server callback for /cli/stats. Returns the count of packets sent by the
 * CLI.
 */
static ssize_t _stats_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len)
{
    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);

    size_t payload_len = fmt_u16_dec((char *)pdu->payload, req_count);

    return gcoap_finish(pdu, payload_len, COAP_FORMAT_TEXT);
}

static size_t _send(uint8_t *buf, size_t len, char *addr_str, char *port_str)
{
    ipv6_addr_t addr;
    uint16_t port;
    size_t bytes_sent;

    /* parse destination address */
    if (ipv6_addr_from_str(&addr, addr_str) == NULL) {
        puts("gcoap_cli: unable to parse destination address");
        return 0;
    }
    /* parse port */
    port = (uint16_t)atoi(port_str);
    if (port == 0) {
        puts("gcoap_cli: unable to parse destination port");
        return 0;
    }

    bytes_sent = gcoap_req_send(buf, len, &addr, port, _resp_handler);
    if (bytes_sent > 0) {
        req_count++;
    }
    return bytes_sent;
}

int gcoap_cli_cmd(int argc, char **argv)
{
    /* Ordered like the RFC method code numbers, but off by 1. GET is code 0. */
    char *method_codes[] = {"get", "post", "put"};
    uint8_t buf[GCOAP_PDU_BUF_SIZE];
    coap_pkt_t pdu;
    size_t len;

    if (argc == 1) {
        /* show help for main commands */
        goto end;
    }

    for (size_t i = 0; i < sizeof(method_codes) / sizeof(char*); i++) {
        if (strcmp(argv[1], method_codes[i]) == 0) {
            if (argc == 5 || argc == 6) {
                if (argc == 6) {
                    gcoap_req_init(&pdu, &buf[0], GCOAP_PDU_BUF_SIZE, i+1, argv[4]);
                    memcpy(pdu.payload, argv[5], strlen(argv[5]));
                    len = gcoap_finish(&pdu, strlen(argv[5]), COAP_FORMAT_TEXT);
                }
                else {
                    len = gcoap_request(&pdu, &buf[0], GCOAP_PDU_BUF_SIZE, i+1,
                                                                           argv[4]);
                }
                printf("gcoap_cli: sending msg ID %u, %u bytes\n", coap_get_id(&pdu),
                                                                   (unsigned) len);
                if (!_send(&buf[0], len, argv[2], argv[3])) {
                    puts("gcoap_cli: msg send failed");
                }
                return 0;
            }
            else {
                printf("usage: %s <get|post|put> <addr> <port> <path> [data]\n",
                                                                       argv[0]);
                return 1;
            }
        }
    }

    if (strcmp(argv[1], "info") == 0) {
        if (argc == 2) {
            uint8_t open_reqs;
            gcoap_op_state(&open_reqs);

            printf("CoAP server is listening on port %u\n", GCOAP_PORT);
            printf(" CLI requests sent: %u\n", req_count);
            printf("CoAP open requests: %u\n", open_reqs);
            return 0;
        }
    }

    end:
    printf("usage: %s <get|post|put|info>\n", argv[0]);
    return 1;
}

void gcoap_cli_init(void)
{
    gcoap_register_listener(&_listener);
}
