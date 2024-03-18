#include <Arduino.h>
//#include <stdint.h>
//#include <WiFi.h>

#include "ping/ping_sock.h"

// https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/protocols/icmp_echo.html

ip_addr_t target_addr;
esp_ping_handle_t ping_h;
bool ping_running = false;
uint32_t packets_received;

static void on_ping_success(esp_ping_handle_t hdl, void *args)
{
    // optionally, get callback arguments
    // const char* str = (const char*) args;
    // printf("%s\r\n", str); // "foo"
    uint8_t ttl;
    uint16_t seqno;
    uint32_t elapsed_time, recv_len;
    ip_addr_t target_addr;
    esp_ping_get_profile(hdl, ESP_PING_PROF_SEQNO, &seqno, sizeof(seqno));
    esp_ping_get_profile(hdl, ESP_PING_PROF_TTL, &ttl, sizeof(ttl));
    esp_ping_get_profile(hdl, ESP_PING_PROF_IPADDR, &target_addr, sizeof(target_addr));
    esp_ping_get_profile(hdl, ESP_PING_PROF_SIZE, &recv_len, sizeof(recv_len));
    esp_ping_get_profile(hdl, ESP_PING_PROF_TIMEGAP, &elapsed_time, sizeof(elapsed_time));
//    printf("%d bytes from %s icmp_seq=%d ttl=%d time=%d ms\n",
//           recv_len, inet_ntoa(target_addr.u_addr.ip4), seqno, ttl, elapsed_time);
    printf("Received %d bytes icmp_seq=%d ttl=%d time=%d ms\n", recv_len, seqno, ttl, elapsed_time);
}

static void on_ping_timeout(esp_ping_handle_t hdl, void *args)
{
    uint16_t seqno;
    ip_addr_t target_addr;
    esp_ping_get_profile(hdl, ESP_PING_PROF_SEQNO, &seqno, sizeof(seqno));
    esp_ping_get_profile(hdl, ESP_PING_PROF_IPADDR, &target_addr, sizeof(target_addr));
//    printf("From %s icmp_seq=%d timeout\n", inet_ntoa(target_addr.u_addr.ip4), seqno);
    printf("icmp_seq=%d timeout\n", seqno);
}

static void on_ping_end(esp_ping_handle_t hdl, void *args)
{
    uint32_t transmitted;
    uint32_t total_time_ms;

    esp_ping_get_profile(hdl, ESP_PING_PROF_REQUEST, &transmitted, sizeof(transmitted));
    esp_ping_get_profile(hdl, ESP_PING_PROF_REPLY, &packets_received, sizeof(packets_received));
    esp_ping_get_profile(hdl, ESP_PING_PROF_DURATION, &total_time_ms, sizeof(total_time_ms));
    printf("%d packets transmitted, %d received, time %dms\n", transmitted, packets_received, total_time_ms);
    ping_running = false;
}

void initialize_ping()
{
    esp_ping_config_t ping_config = ESP_PING_DEFAULT_CONFIG();
    ping_config.target_addr = target_addr;          // target IP address
    ping_config.count = 3; // ESP_PING_COUNT_INFINITE;    // ping in infinite mode, esp_ping_stop can stop it

    /* set callback functions */
    esp_ping_callbacks_t cbs;
    cbs.on_ping_success = on_ping_success;
    cbs.on_ping_timeout = on_ping_timeout;
    cbs.on_ping_end = on_ping_end;
//    cbs.cb_args = "foo";  // arguments that feeds to all callback functions, can be NULL
//  cbs.cb_args = eth_event_group; // ??

    esp_ping_handle_t ping;
    esp_ping_new_session(&ping_config, &cbs, &ping_h);
}

void destroy_ping(){
    ping_running = false;
    esp_ping_stop(ping_h);
    esp_ping_delete_session(ping_h);
}


bool ppiinngg (uint32_t IP_to_ping){
    printf("Ping...\n");

    target_addr.type = IPADDR_TYPE_V4;
    target_addr.u_addr.ip4.addr = IP_to_ping;

    initialize_ping();
    packets_received = 0;
    ping_running = true;
    esp_ping_start(ping_h);

    for (int i = 0; i < 20; i++)  // max 20 sec
    {
//        printf("..");
        if (!ping_running) break;
        //sys_delay_ms(1000);
        delay(1000);
        yield();        
    }    

    destroy_ping();
    printf("Ping finished\n");
    return (packets_received > 0);
}


