/* mqtt-sub.c
 *
 * Copyright (C) 2006-2025 wolfSSL Inc.
 *
 * This file is part of wolfMQTT.
 *
 * wolfMQTT is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * wolfMQTT is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1335, USA
 */

/* Include the autoconf generated config.h */
#ifdef HAVE_CONFIG_H
    #include <config.h>
#endif

#include "examples/mqttnet.h"
#include "examples/pub-sub/mqtt-pub-sub.h"

/* Locals */
static int mStopRead = 0;

/* Configuration */

/* Maximum size for network read/write callbacks. There is also a v5 define that
 * describes the max MQTT control packet size, DEFAULT_MAX_PKT_SZ. */
#ifndef MAX_BUFFER_SIZE
#define MAX_BUFFER_SIZE 1024
#endif

#ifdef WOLFMQTT_PROPERTY_CB
#define MAX_CLIENT_ID_LEN 64
char gClientId[MAX_CLIENT_ID_LEN] = {0};
#endif

#ifdef WOLFMQTT_DISCONNECT_CB
/* callback indicates a network error occurred */
static int mqtt_disconnect_cb(MqttClient* client, int error_code, void* ctx)
{
    MQTTCtx* mqttCtx = (MQTTCtx*)client->ctx;
    (void)ctx;

    if (mqttCtx->debug_on) {
        PRINTF("Network Error Callback: %s (error %d)",
            MqttClient_ReturnCodeToString(error_code), error_code);
    }
    return 0;
}
#endif

static int mqtt_message_cb(MqttClient *client, MqttMessage *msg,
    byte msg_new, byte msg_done)
{
    byte buf[PRINT_BUFFER_SIZE+1];
    word32 len;
    MQTTCtx* mqttCtx = (MQTTCtx*)client->ctx;

    (void)mqttCtx;

    if (msg_new) {
        /* Determine min size to dump */
        len = msg->topic_name_len;
        if (len > PRINT_BUFFER_SIZE) {
            len = PRINT_BUFFER_SIZE;
        }
        XMEMCPY(buf, msg->topic_name, len);
        buf[len] = '\0'; /* Make sure its null terminated */

        if (mqttCtx->debug_on) {
            /* Print incoming message */
            PRINTF("MQTT Message: Topic %s, Qos %d, Len %u",
                buf, msg->qos, msg->total_len);
        }
    }

    /* Print message payload */
    len = msg->buffer_len;
    if (len > PRINT_BUFFER_SIZE) {
        len = PRINT_BUFFER_SIZE;
    }
    XMEMCPY(buf, msg->buffer, len);
    buf[len] = '\0'; /* Make sure its null terminated */
    if (mqttCtx->debug_on) {
        PRINTF("Payload (%d - %d) printing %d bytes:" LINE_END "%s",
            msg->buffer_pos, msg->buffer_pos + msg->buffer_len, len, buf);
    }
    else {
        PRINTF("%s", buf);
    }

    #ifdef WOLFMQTT_V5
    {
        /* Properties can be checked in the message callback */
        MqttProp *prop = msg->props;
        while (prop != NULL)
        {
            if (prop->type == MQTT_PROP_CONTENT_TYPE) {
                PRINTF("Content type: %.*s", prop->data_str.len,
                                             prop->data_str.str);
            }
            prop = prop->next;
        }
    }
    #endif

    if (msg_done && mqttCtx->debug_on) {
        PRINTF("MQTT Message: Done");
    }

    /* Return negative to terminate publish processing */
    return MQTT_CODE_SUCCESS;
}

#ifdef WOLFMQTT_PROPERTY_CB
/* The property callback is called after decoding a packet that contains at
   least one property. The property list is deallocated after returning from
   the callback. */
static int mqtt_property_cb(MqttClient *client, MqttProp *head, void *ctx)
{
    MqttProp *prop = head;
    int rc = 0;
    MQTTCtx* mqttCtx;

    if ((client == NULL) || (client->ctx == NULL)) {
        return MQTT_CODE_ERROR_BAD_ARG;
    }
    mqttCtx = (MQTTCtx*)client->ctx;

    while (prop != NULL) {
        PRINTF("Property CB: Type %d", prop->type);
        switch (prop->type) {
            case MQTT_PROP_ASSIGNED_CLIENT_ID:
                /* Store client ID in global */
                mqttCtx->client_id = &gClientId[0];

                /* Store assigned client ID from CONNACK*/
                XSTRNCPY((char*)mqttCtx->client_id, prop->data_str.str,
                         MAX_CLIENT_ID_LEN - 1);
                /* should use strlcpy() semantics, but non-portable */
                ((char*)mqttCtx->client_id)[MAX_CLIENT_ID_LEN - 1] = '\0';
                break;

            case MQTT_PROP_SUBSCRIPTION_ID_AVAIL:
                mqttCtx->subId_not_avail =
                        prop->data_byte == 0;
                break;

            case MQTT_PROP_TOPIC_ALIAS_MAX:
                mqttCtx->topic_alias_max =
                 (mqttCtx->topic_alias_max < prop->data_short) ?
                 mqttCtx->topic_alias_max : prop->data_short;
                break;

            case MQTT_PROP_MAX_PACKET_SZ:
                if ((prop->data_int > 0) &&
                    (prop->data_int <= MQTT_PACKET_SZ_MAX))
                {
                    client->packet_sz_max =
                        (client->packet_sz_max < prop->data_int) ?
                         client->packet_sz_max : prop->data_int;
                }
                else {
                    /* Protocol error */
                    rc = MQTT_CODE_ERROR_PROPERTY;
                }
                break;

            case MQTT_PROP_SERVER_KEEP_ALIVE:
                mqttCtx->keep_alive_sec = prop->data_short;
                break;

            case MQTT_PROP_MAX_QOS:
                client->max_qos = prop->data_byte;
                break;

            case MQTT_PROP_RETAIN_AVAIL:
                client->retain_avail = prop->data_byte;
                break;

            case MQTT_PROP_REASON_STR:
                PRINTF("Reason String: %.*s",
                        prop->data_str.len, prop->data_str.str);
                break;

            case MQTT_PROP_USER_PROP:
                PRINTF("User property: key=\"%.*s\", value=\"%.*s\"",
                        prop->data_str.len, prop->data_str.str,
                        prop->data_str2.len, prop->data_str2.str);
                break;

            case MQTT_PROP_PAYLOAD_FORMAT_IND:
            case MQTT_PROP_MSG_EXPIRY_INTERVAL:
            case MQTT_PROP_CONTENT_TYPE:
            case MQTT_PROP_RESP_TOPIC:
            case MQTT_PROP_CORRELATION_DATA:
            case MQTT_PROP_SUBSCRIPTION_ID:
            case MQTT_PROP_SESSION_EXPIRY_INTERVAL:
            case MQTT_PROP_TOPIC_ALIAS:
            case MQTT_PROP_TYPE_MAX:
            case MQTT_PROP_RECEIVE_MAX:
            case MQTT_PROP_WILDCARD_SUB_AVAIL:
            case MQTT_PROP_SHARED_SUBSCRIPTION_AVAIL:
            case MQTT_PROP_RESP_INFO:
            case MQTT_PROP_SERVER_REF:
            case MQTT_PROP_AUTH_METHOD:
            case MQTT_PROP_AUTH_DATA:
            case MQTT_PROP_NONE:
                break;
            case MQTT_PROP_REQ_PROB_INFO:
            case MQTT_PROP_WILL_DELAY_INTERVAL:
            case MQTT_PROP_REQ_RESP_INFO:
            default:
                /* Invalid */
                rc = MQTT_CODE_ERROR_PROPERTY;
                break;
        }
        prop = prop->next;
    }

    (void)ctx;

    return rc;
}
#endif /* WOLFMQTT_PROPERTY_CB */

int sub_client(MQTTCtx *mqttCtx)
{
    int rc = MQTT_CODE_SUCCESS, i;

    if (mqttCtx->debug_on) {
        PRINTF("Subscribe Client: QoS %d, Use TLS %d", mqttCtx->qos,
                mqttCtx->use_tls);
    }
    /* Initialize Network */
    rc = MqttClientNet_Init(&mqttCtx->net, mqttCtx);
    if (mqttCtx->debug_on) {
        PRINTF("MQTT Net Init: %s (%d)",
                MqttClient_ReturnCodeToString(rc), rc);
    }
    if (rc != MQTT_CODE_SUCCESS) {
        goto exit;
    }

    /* setup tx/rx buffers */
    mqttCtx->tx_buf = (byte*)WOLFMQTT_MALLOC(MAX_BUFFER_SIZE);
    mqttCtx->rx_buf = (byte*)WOLFMQTT_MALLOC(MAX_BUFFER_SIZE);

    /* Initialize MqttClient structure */
    rc = MqttClient_Init(&mqttCtx->client, &mqttCtx->net,
        mqtt_message_cb,
        mqttCtx->tx_buf, MAX_BUFFER_SIZE,
        mqttCtx->rx_buf, MAX_BUFFER_SIZE,
        mqttCtx->cmd_timeout_ms);

    if (mqttCtx->debug_on) {
        PRINTF("MQTT Init: %s (%d)",
                MqttClient_ReturnCodeToString(rc), rc);
    }
    if (rc != MQTT_CODE_SUCCESS) {
        goto exit;
    }
    /* The client.ctx will be stored in the cert callback ctx during
       MqttSocket_Connect for use by mqtt_tls_verify_cb */
    mqttCtx->client.ctx = mqttCtx;

#ifdef WOLFMQTT_DISCONNECT_CB
    /* setup disconnect callback */
    rc = MqttClient_SetDisconnectCallback(&mqttCtx->client,
        mqtt_disconnect_cb, NULL);
    if (rc != MQTT_CODE_SUCCESS) {
        goto exit;
    }
#endif
#ifdef WOLFMQTT_PROPERTY_CB
    rc = MqttClient_SetPropertyCallback(&mqttCtx->client,
            mqtt_property_cb, NULL);
    if (rc != MQTT_CODE_SUCCESS) {
        goto exit;
    }
#endif

    /* Connect to broker */
    rc = MqttClient_NetConnect(&mqttCtx->client, mqttCtx->host,
           mqttCtx->port,
        DEFAULT_CON_TIMEOUT_MS, mqttCtx->use_tls, mqtt_tls_cb);

    if (mqttCtx->debug_on) {
        PRINTF("MQTT Socket Connect: %s (%d)",
            MqttClient_ReturnCodeToString(rc), rc);
    }
    if (rc != MQTT_CODE_SUCCESS) {
        goto exit;
    }

    /* Build connect packet */
    XMEMSET(&mqttCtx->connect, 0, sizeof(MqttConnect));
    mqttCtx->connect.keep_alive_sec = mqttCtx->keep_alive_sec;
    mqttCtx->connect.clean_session = mqttCtx->clean_session;
    mqttCtx->connect.client_id = mqttCtx->client_id;

    /* Last will and testament sent by broker to subscribers
        of topic when broker connection is lost */
    XMEMSET(&mqttCtx->lwt_msg, 0, sizeof(mqttCtx->lwt_msg));
    mqttCtx->connect.lwt_msg = &mqttCtx->lwt_msg;
    mqttCtx->connect.enable_lwt = mqttCtx->enable_lwt;
    if (mqttCtx->enable_lwt) {
        /* Send client id in LWT payload */
        mqttCtx->lwt_msg.qos = mqttCtx->qos;
        mqttCtx->lwt_msg.retain = 0;
        mqttCtx->lwt_msg.topic_name = WOLFMQTT_TOPIC_NAME"lwttopic";
        mqttCtx->lwt_msg.buffer = (byte*)mqttCtx->client_id;
        mqttCtx->lwt_msg.total_len = (word16)XSTRLEN(mqttCtx->client_id);

#ifdef WOLFMQTT_V5
        {
            /* Add a 5 second delay to sending the LWT */
            MqttProp* prop = MqttClient_PropsAdd(&mqttCtx->lwt_msg.props);
            prop->type = MQTT_PROP_WILL_DELAY_INTERVAL;
            prop->data_int = 5;
        }
#endif
    }
    /* Optional authentication */
    mqttCtx->connect.username = mqttCtx->username;
    mqttCtx->connect.password = mqttCtx->password;
#ifdef WOLFMQTT_V5
    mqttCtx->client.packet_sz_max = mqttCtx->max_packet_size;

    {
        /* Request Response Information */
        MqttProp* prop = MqttClient_PropsAdd(&mqttCtx->connect.props);
        prop->type = MQTT_PROP_REQ_RESP_INFO;
        prop->data_byte = 1;
    }
    {
        /* Request Problem Information */
        MqttProp* prop = MqttClient_PropsAdd(&mqttCtx->connect.props);
        prop->type = MQTT_PROP_REQ_PROB_INFO;
        prop->data_byte = 1;
    }
    {
        /* Maximum Packet Size */
        MqttProp* prop = MqttClient_PropsAdd(&mqttCtx->connect.props);
        prop->type = MQTT_PROP_MAX_PACKET_SZ;
        prop->data_int = (word32)mqttCtx->max_packet_size;
    }
    {
        /* Topic Alias Maximum */
        MqttProp* prop = MqttClient_PropsAdd(&mqttCtx->connect.props);
        prop->type = MQTT_PROP_TOPIC_ALIAS_MAX;
        prop->data_short = mqttCtx->topic_alias_max;
    }
    if (mqttCtx->clean_session == 0) {
        /* Session expiry interval */
        MqttProp* prop = MqttClient_PropsAdd(&mqttCtx->connect.props);
        prop->type = MQTT_PROP_SESSION_EXPIRY_INTERVAL;
        prop->data_int = DEFAULT_SESS_EXP_INT; /* Session does not expire */
    }
#endif

    /* Send Connect and wait for Connect Ack */
    rc = MqttClient_Connect(&mqttCtx->client, &mqttCtx->connect);

    if (mqttCtx->debug_on) {
        PRINTF("MQTT Connect: Proto (%s), %s (%d)",
            MqttClient_GetProtocolVersionString(&mqttCtx->client),
            MqttClient_ReturnCodeToString(rc), rc);
    }
    if (rc != MQTT_CODE_SUCCESS) {
        goto disconn;
    }

#ifdef WOLFMQTT_V5
    if (mqttCtx->connect.props != NULL) {
        /* Release the allocated properties */
        MqttClient_PropsFree(mqttCtx->connect.props);
    }
    if (mqttCtx->lwt_msg.props != NULL) {
        /* Release the allocated properties */
        MqttClient_PropsFree(mqttCtx->lwt_msg.props);
    }
#endif

    if (mqttCtx->debug_on) {
        /* Validate Connect Ack info */
        PRINTF("MQTT Connect Ack: Return Code %u, Session Present %d",
            mqttCtx->connect.ack.return_code,
            (mqttCtx->connect.ack.flags &
                MQTT_CONNECT_ACK_FLAG_SESSION_PRESENT) ?
                1 : 0
        );
#ifdef WOLFMQTT_PROPERTY_CB
        /* Print the acquired client ID */
        PRINTF("MQTT Connect Ack: Assigned Client ID: %s",
                mqttCtx->client_id);
#endif
    }

    /* Build list of topics */
    XMEMSET(&mqttCtx->subscribe, 0, sizeof(MqttSubscribe));

    i = 0;
    mqttCtx->topics[i].topic_filter = mqttCtx->topic_name;
    mqttCtx->topics[i].qos = mqttCtx->qos;

#ifdef WOLFMQTT_V5
    if (mqttCtx->subId_not_avail != 1) {
        /* Subscription Identifier */
        MqttProp* prop;
        prop = MqttClient_PropsAdd(&mqttCtx->subscribe.props);
        prop->type = MQTT_PROP_SUBSCRIPTION_ID;
        prop->data_int = DEFAULT_SUB_ID;
    }
#endif

    /* Subscribe Topic */
    mqttCtx->subscribe.packet_id = mqtt_get_packetid();
    mqttCtx->subscribe.topic_count =
            sizeof(mqttCtx->topics) / sizeof(MqttTopic);
    mqttCtx->subscribe.topics = mqttCtx->topics;

    rc = MqttClient_Subscribe(&mqttCtx->client, &mqttCtx->subscribe);

#ifdef WOLFMQTT_V5
    if (mqttCtx->subscribe.props != NULL) {
        /* Release the allocated properties */
        MqttClient_PropsFree(mqttCtx->subscribe.props);
    }
#endif

    if (mqttCtx->debug_on) {
        PRINTF("MQTT Subscribe: %s (%d)",
            MqttClient_ReturnCodeToString(rc), rc);
    }
    if (rc != MQTT_CODE_SUCCESS) {
        goto disconn;
    }

    if (mqttCtx->debug_on) {
        /* show subscribe results */
        for (i = 0; i < mqttCtx->subscribe.topic_count; i++) {
            MqttTopic *topic = &mqttCtx->subscribe.topics[i];
            PRINTF("  Topic %s, Qos %u, Return Code %u",
                topic->topic_filter,
                topic->qos, topic->return_code);
        }
    }
    /* Read Loop */
    if (mqttCtx->debug_on) {
        PRINTF("MQTT Waiting for message...");
    }

    do {
        /* Try and read packet */
        rc = MqttClient_WaitMessage(&mqttCtx->client,
                                            mqttCtx->cmd_timeout_ms);

    #ifdef WOLFMQTT_NONBLOCK
        /* Track elapsed time with no activity and trigger timeout */
        rc = mqtt_check_timeout(rc, &mqttCtx->start_sec,
            mqttCtx->cmd_timeout_ms/1000);
    #endif

        if (mStopRead) {
            rc = MQTT_CODE_SUCCESS;
            if (mqttCtx->debug_on) {
                PRINTF("MQTT Exiting...");
            }
            break;
        }

        /* check return code */
    #ifdef WOLFMQTT_ENABLE_STDIN_CAP
        else if (rc == MQTT_CODE_STDIN_WAKE) {
            XMEMSET(mqttCtx->rx_buf, 0, MAX_BUFFER_SIZE);
            if (XFGETS((char*)mqttCtx->rx_buf, MAX_BUFFER_SIZE - 1,
                    stdin) != NULL) {
            }
        }
    #endif
        else if (rc == MQTT_CODE_ERROR_TIMEOUT) {
            /* Keep Alive */
            if (mqttCtx->debug_on) {
                PRINTF("Keep-alive timeout, sending ping");
            }
            rc = MqttClient_Ping_ex(&mqttCtx->client, &mqttCtx->ping);
            if (rc != MQTT_CODE_SUCCESS) {
                PRINTF("MQTT Ping Keep Alive Error: %s (%d)",
                    MqttClient_ReturnCodeToString(rc), rc);
                break;
            }
        }
        else if (rc != MQTT_CODE_SUCCESS) {
            /* There was an error */
            PRINTF("MQTT Message Wait: %s (%d)",
                MqttClient_ReturnCodeToString(rc), rc);
            break;
        }
    } while (!mStopRead);

    /* Check for error */
    if (rc != MQTT_CODE_SUCCESS) {
        goto disconn;
    }

    /* Unsubscribe Topics */
    XMEMSET(&mqttCtx->unsubscribe, 0, sizeof(MqttUnsubscribe));
    mqttCtx->unsubscribe.packet_id = mqtt_get_packetid();
    mqttCtx->unsubscribe.topic_count =
        sizeof(mqttCtx->topics) / sizeof(MqttTopic);
    mqttCtx->unsubscribe.topics = mqttCtx->topics;

    /* Unsubscribe Topics */
    rc = MqttClient_Unsubscribe(&mqttCtx->client,
           &mqttCtx->unsubscribe);

    if (mqttCtx->debug_on) {
        PRINTF("MQTT Unsubscribe: %s (%d)",
            MqttClient_ReturnCodeToString(rc), rc);
    }
    if (rc != MQTT_CODE_SUCCESS) {
        goto disconn;
    }
    mqttCtx->return_code = rc;

disconn:
    /* Disconnect */
    XMEMSET(&mqttCtx->disconnect, 0, sizeof(mqttCtx->disconnect));
#ifdef WOLFMQTT_V5
    {
        /* Session expiry interval */
        MqttProp* prop = MqttClient_PropsAdd(&mqttCtx->disconnect.props);
        prop->type = MQTT_PROP_SESSION_EXPIRY_INTERVAL;
        prop->data_int = 0;
    }
    #if 0 /* enable to test sending a disconnect reason code */
    if (mqttCtx->enable_lwt) {
        /* Disconnect with Will Message */
        mqttCtx->disconnect.reason_code = MQTT_REASON_DISCONNECT_W_WILL_MSG;
    }
    #endif
#endif
    rc = MqttClient_Disconnect_ex(&mqttCtx->client, &mqttCtx->disconnect);
#ifdef WOLFMQTT_V5
    if (mqttCtx->disconnect.props != NULL) {
        /* Release the allocated properties */
        MqttClient_PropsFree(mqttCtx->disconnect.props);
    }
#endif

    if (mqttCtx->debug_on) {
        PRINTF("MQTT Disconnect: %s (%d)",
            MqttClient_ReturnCodeToString(rc), rc);
    }
    rc = MqttClient_NetDisconnect(&mqttCtx->client);

    if (mqttCtx->debug_on) {
        PRINTF("MQTT Socket Disconnect: %s (%d)",
            MqttClient_ReturnCodeToString(rc), rc);
    }
exit:

    /* Free resources */
    if (mqttCtx->tx_buf) WOLFMQTT_FREE(mqttCtx->tx_buf);
    if (mqttCtx->rx_buf) WOLFMQTT_FREE(mqttCtx->rx_buf);

    /* Cleanup network */
    MqttClientNet_DeInit(&mqttCtx->net);

    MqttClient_DeInit(&mqttCtx->client);

    return rc;
}


/* so overall tests can pull in test function */
    #ifdef USE_WINDOWS_API
        #include <windows.h> /* for ctrl handler */

        static BOOL CtrlHandler(DWORD fdwCtrlType)
        {
            if (fdwCtrlType == CTRL_C_EVENT) {
                mStopRead = 1;
                return TRUE;
            }
            return FALSE;
        }
    #elif HAVE_SIGNAL
        #include <signal.h>
        static void sig_handler(int signo)
        {
            if (signo == SIGINT) {
                mStopRead = 1;
            }
        }
    #endif

#if defined(NO_MAIN_DRIVER)
int mqttSub_main(int argc, char** argv)
#else
int main(int argc, char** argv)
#endif
{
    int rc;
    MQTTCtx mqttCtx;

    /* init defaults */
    mqtt_init_ctx(&mqttCtx);

    /* Set default client ID */
    mqttCtx.client_id = "wolfMQTT_sub";

    /* Set default host to localhost */
    mqttCtx.host = "localhost";

    /* Set example debug messages off (turn on with '-d') */
    mqttCtx.debug_on = 0;

    /* parse arguments */
    rc = mqtt_parse_args(&mqttCtx, argc, argv);
    if (rc != 0) {
        if (rc == MY_EX_USAGE) {
            /* return success, so make check passes with TLS disabled */
            return 0;
        }
        return rc;
    }

#ifdef USE_WINDOWS_API
    if (SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler,
          TRUE) == FALSE)
    {
        PRINTF("Error setting Ctrl Handler! Error %d", (int)GetLastError());
    }
#elif HAVE_SIGNAL
    if (signal(SIGINT, sig_handler) == SIG_ERR) {
        PRINTF("Can't catch SIGINT");
    }
#endif

    rc = sub_client(&mqttCtx);

    mqtt_free_ctx(&mqttCtx);

    return (rc == 0) ? 0 : EXIT_FAILURE;
}

