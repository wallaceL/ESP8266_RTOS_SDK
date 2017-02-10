/*
 * tcp_server.c
 *
 *  Created on: Feb 10, 2017
 *      Author: liubo
 */

#include "c_types.h"

#include "esp_common.h"
#include "espconn.h"
#include "user_config.h"

//#include "tcp_test.h"



#define DEMO_AP_SSID      "WT_TEST"
#define DEMO_AP_PASSWORD "1234567abc"

#define SOFT_AP_SSID "DEMO_AP"
#define SOFT_AP_PASSWORD "12345678"

#define TCP_SERVER_KEEP_ALIVE_ENABLE
#define TCP_SERVER_KEEP_ALIVE_IDLE_S (100)
#define TCP_SERVER_RETRY_INTVL_S (5)
#define TCP_SERVER_RETRY_CNT     (3)
#define TCP_SERVER_GREETING "Hello!This is a tcp server test\n"


 #define DBG_PRINT(fmt,...)  do{\
         os_printf("[Dbg]");\
         os_printf(fmt,##__VA_ARGS__);\
     }while(0)


 #define ERR_PRINT(fmt,...) do{\
         os_printf("[Err] Fun:%s Line:%d ",__FUNCTION__,__LINE__);\
         os_printf(fmt,##__VA_ARGS__);\
     }while(0)
 #define DBG_LINES(v) os_printf("------------------%s---------------\n",v)


void TcpServerClientConnect(void*arg)
{
    struct espconn* tcp_server_local=arg;
#if defined(TCP_SERVER_KEEP_ALIVE_ENABLE)
	espconn_set_opt(tcp_server_local,BIT(3));//enable keep alive ,this api must call in connect callback

	uint32 keep_alvie=0;
	keep_alvie=TCP_SERVER_KEEP_ALIVE_IDLE_S;
	espconn_set_keepalive(tcp_server_local,ESPCONN_KEEPIDLE,&keep_alvie);
	keep_alvie=TCP_SERVER_RETRY_INTVL_S;
	espconn_set_keepalive(tcp_server_local,ESPCONN_KEEPINTVL,&keep_alvie);
	keep_alvie=keep_alvie=TCP_SERVER_RETRY_INTVL_S;
	espconn_set_keepalive(tcp_server_local,ESPCONN_KEEPCNT,&keep_alvie);
	DBG_PRINT("keep alive enable\n");
#endif
	DBG_LINES("TCP server CONNECT");
	DBG_PRINT("tcp client ip:%d.%d.%d.%d port:%d",tcp_server_local->proto.tcp->remote_ip[0],
		                                          tcp_server_local->proto.tcp->remote_ip[1],
		                                          tcp_server_local->proto.tcp->remote_ip[2],
		                                          tcp_server_local->proto.tcp->remote_ip[3],
		                                          tcp_server_local->proto.tcp->remote_port
		                                          );
	espconn_send(tcp_server_local,TCP_SERVER_GREETING,strlen(TCP_SERVER_GREETING));
}
void TcpServerClientDisConnect(void* arg)
{
    struct espconn* tcp_server_local=arg;
	DBG_LINES("TCP server DISCONNECT");
	DBG_PRINT("tcp client ip:%d.%d.%d.%d port:%d\n",tcp_server_local->proto.tcp->remote_ip[0],
		                                          tcp_server_local->proto.tcp->remote_ip[1],
		                                          tcp_server_local->proto.tcp->remote_ip[2],
		                                          tcp_server_local->proto.tcp->remote_ip[3],
		                                          tcp_server_local->proto.tcp->remote_port
		                                          );
}

void TcpServerClienSendCb(void* arg)
{
    struct espconn* tcp_server_local=arg;
	DBG_LINES("TCP server SendCb");
	DBG_PRINT("tcp client ip:%d.%d.%d.%d port:%d\n",tcp_server_local->proto.tcp->remote_ip[0],
		                                          tcp_server_local->proto.tcp->remote_ip[1],
		                                          tcp_server_local->proto.tcp->remote_ip[2],
		                                          tcp_server_local->proto.tcp->remote_ip[3],
		                                          tcp_server_local->proto.tcp->remote_port
		                                          );
}



void TcpServerRecvCb(void *arg, char *pdata, unsigned short len)
{
   struct espconn* tcp_server_local=arg;
   DBG_PRINT("Recv tcp client ip:%d.%d.%d.%d port:%d len:%d\n",tcp_server_local->proto.tcp->remote_ip[0],
		                                          tcp_server_local->proto.tcp->remote_ip[1],
		                                          tcp_server_local->proto.tcp->remote_ip[2],
		                                          tcp_server_local->proto.tcp->remote_ip[3],
		                                          tcp_server_local->proto.tcp->remote_port,
		                                          len);
   espconn_send(tcp_server_local,pdata,len);
}
void TcpServerReconnectCb(void *arg, sint8 err)
{
    struct espconn* tcp_server_local=arg;
	DBG_LINES("TCP server RECONNECT");
	DBG_PRINT("status:%d\n",err);
	DBG_PRINT("tcp client ip:%d.%d.%d.%d port:%d\n",tcp_server_local->proto.tcp->remote_ip[0],
		                                          tcp_server_local->proto.tcp->remote_ip[1],
		                                          tcp_server_local->proto.tcp->remote_ip[2],
		                                          tcp_server_local->proto.tcp->remote_ip[3],
		                                          tcp_server_local->proto.tcp->remote_port\
		                                          );
}


void TcpLocalServer(void* arg)
{

	static struct espconn tcp_server_local;
	static esp_tcp tcp;
	tcp_server_local.type=ESPCONN_TCP;
	tcp_server_local.proto.tcp=&tcp;
	tcp.local_port=80;

	espconn_regist_connectcb(&tcp_server_local,TcpServerClientConnect);
	espconn_regist_recvcb(&tcp_server_local,TcpServerRecvCb);
	espconn_regist_reconcb(&tcp_server_local,TcpServerReconnectCb);
	espconn_regist_disconcb(&tcp_server_local,TcpServerClientDisConnect);
	espconn_regist_sentcb(&tcp_server_local,TcpServerClienSendCb);

	espconn_accept(&tcp_server_local);
	espconn_regist_time(&tcp_server_local,20,0);
	vTaskDelete(NULL);
}
