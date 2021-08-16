/*
 *  TCP client
 */

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

/*
 *  クライアントの接続先サーバ情報
 */
struct client_info {
    unsigned short sv_port;     /* サーバのポート番号 */
    char *sv_ipaddr;            /* サーバのIPアドレス */
    char *msg;                  /* 送信メッセージ */

    int sd;                     /* ソケットディスクリプタ */
    struct sockaddr_in sv_addr; /* サーバのアドレス構造体 */
};
typedef struct client_info cl_info_t;

/*!
 * @brief      応答メッセージを受信する
 * @param[in]  info   クライアント接続情報
 * @param[out] errmsg エラーメッセージ格納先
 * @return     成功ならば0、失敗ならば-1を返す。
 */
static int
tcp_receive_msg(cl_info_t *info, char *errmsg)
{
    int recv_msglen = 0;
    char buff[BUFSIZ];

    /* 応答メッセージを受信する */
    recv_msglen = recv(info->sd, buff, BUFSIZ, 0);
    if(recv_msglen < 0){
        sprintf(errmsg, "(line:%d) %s", __LINE__, strerror(errno));
        return(-1);
    }

    /* 応答メッセージを出力する */
    buff[recv_msglen] = '\0';   /* null-terminate */
    fprintf(stdout, "Received: %s\n", buff);

    return(0);
}

/*!
 * @brief      TCP接続してメッセージを送る
 * @param[in]  info   クライアント接続情報
 * @param[out] errmsg エラーメッセージ格納先
 * @return     成功ならば0、失敗ならば-1を返す。
 */
static int
tcp_send_msg(cl_info_t *info, char *errmsg)
{
    int rc = 0;
    int msg_len = strlen(info->msg) + 1;

    /* メッセージの送信 */
    rc = send(info->sd, info->msg, msg_len, 0);
    if(rc != msg_len){
        sprintf(errmsg, "(line:%d) %s", __LINE__, strerror(errno));
        return(-1);
    }

    return(0);
}

/*!
 * @brief      エコークライアントを実行する
 * @param[in]  info   クライアント接続情報
 * @param[out] errmsg エラーメッセージ格納先
 * @return     成功ならば0、失敗ならば-1を返す。
 */
static int
tcp_echo_client(cl_info_t *info, char *errmsg)
{
    int rc = 0;

    rc = connect(info->sd, (struct sockaddr *)&(info->sv_addr),
                 sizeof(info->sv_addr));
    if(rc != 0){
        sprintf(errmsg, "(line:%d) %s", __LINE__, strerror(errno));
        return(-1);
    }

    /* メッセージを送信する */
    rc = tcp_send_msg(info, errmsg);
    if(rc != 0) return(-1);

    /* 応答メッセージを受信する */
    rc = tcp_receive_msg(info, errmsg);
    if(rc != 0) return(-1);

    return(0);
}

/*!
 * @brief      ソケットの初期化
 * @param[in]  info   クライアント接続情報
 * @param[out] errmsg エラーメッセージ格納先
 * @return     成功ならば0、失敗ならば-1を返す。
 */
static int
socket_initialize(cl_info_t *info, char *errmsg)
{
    /* ソケットの生成 : TCPを指定する */
    info->sd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(info->sd < 0){
        sprintf(errmsg, "(line:%d) %s", __LINE__, strerror(errno));
        return(-1);
    }

    /* サーバのアドレス構造体を作成する */
    info->sv_addr.sin_family = AF_INET;
    info->sv_addr.sin_addr.s_addr = inet_addr(info->sv_ipaddr);
    info->sv_addr.sin_port = htons(info->sv_port);

    return(0);
}

/*!
 * @brief      ソケットの終期化
 * @param[in]  info   クライアント接続情報
 * @return     成功ならば0、失敗ならば-1を返す。
 */
static void
socket_finalize(cl_info_t *info)
{
    /* ソケット破棄 */
    if(info->sd != 0) close(info->sd);

    return;
}

/*!
 * @brief      TCPクライアント実行
 * @param[in]  info   クライアント接続情報
 * @param[out] errmsg エラーメッセージ格納先
 * @return     成功ならば0、失敗ならば-1を返す。
 */
static int
tcp_client(cl_info_t *info, char *errmsg)
{
    int rc = 0;

    /* ソケットの初期化 */
    rc = socket_initialize(info, errmsg);
    if(rc != 0) return(-1);

    /* メッセージの送受信を行う */
    rc = tcp_echo_client(info, errmsg);

    /* ソケットの終期化 */
    socket_finalize(info);

    return(rc);
}

/*!
 * @brief      初期化処理。IPアドレスとポート番号を設定する。
 * @param[in]  argc   コマンドライン引数の数
 * @param[in]  argv   コマンドライン引数
 * @param[out] info   クライアント接続情報
 * @param[out] errmsg エラーメッセージ格納先
 * @return     成功ならば0、失敗ならば-1を返す。
 */
static int
initialize(int argc, char *argv[], cl_info_t *info, char *errmsg)
{
    if(argc != 4){
        sprintf(errmsg, "Usage: %s <ip-addr> <port> <msg>", argv[0]);
        return(-1);
    }

    memset(info, 0, sizeof(cl_info_t));
    info->sv_ipaddr = argv[1];
    info->sv_port   = atoi(argv[2]);
    info->msg       = argv[3];

    return(0);
}

/*!
 * @brief   main routine
 * @return  成功ならば0、失敗ならば-1を返す。
 */
int
main(int argc, char *argv[])
{
    int rc = 0;
    cl_info_t info = {0};
    char errmsg[BUFSIZ];

    rc = initialize(argc, argv, &info, errmsg);
    if(rc != 0){
        fprintf(stderr, "Error: %s\n", errmsg);
        return(-1);
    }

    rc = tcp_client(&info, errmsg);
    if(rc != 0){
        fprintf(stderr, "Error: %s\n", errmsg);
        return(-1);
    }

    return(0);
}
