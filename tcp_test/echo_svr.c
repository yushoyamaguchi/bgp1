/*
 *  TCP server
 */

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

/* 接続要求の受付数 */
#define MAXPENDING 5

/*
 *  サーバ情報を格納する
 */
struct server_info {
    unsigned short sv_port;     /* サーバのポート番号 */

    int sd;                     /* ソケットディスクリプタ */
    struct sockaddr_in sv_addr; /* サーバのアドレス構造体 */
};
typedef struct server_info sv_info_t;

/*!
 * @brief      受信したメッセージをそのままクライアントに返す
 * @param[in]  clnt_sd クライアントのソケットディスクリプタ
 * @param[out] buff    受信したメッセージ
 * @param[out] errmsg  エラーメッセージ格納先
 * @return     成功ならば0、失敗ならば-1を返す。
 */
static int
tcp_echo_process(int clnt_sd, char *buff, char *errmsg)
{
    int rc = 0;
    int recv_msglen = 0;

    /* クライアントからメッセージを受信する */
    recv_msglen = recv(clnt_sd, buff, BUFSIZ, 0);
    if(recv_msglen < 0){
        sprintf(errmsg, "(line:%d) %s", __LINE__, strerror(errno));
        return(-1);
    }

    while (recv_msglen > 0){
        rc = send(clnt_sd, buff, recv_msglen, 0);
        if(rc != recv_msglen){
            sprintf(errmsg, "(line:%d) %s", __LINE__, strerror(errno));
            return(-1);
        }

        /* 追加の受信データがないか確認する */
        recv_msglen = recv(clnt_sd, buff, BUFSIZ, 0);
        if(recv_msglen < 0){
            sprintf(errmsg, "(line:%d) %s", __LINE__, strerror(errno));
            return(-1);
        }
    }

    return(0);
}

/*!
 * @brief      TCPメッセージを受信する
 * @param[in]  info   クライアント接続情報
 * @param[out] errmsg エラーメッセージ格納先
 * @return     成功ならば0、失敗ならば-1を返す。
 */
static int
tcp_echo_server(sv_info_t *info, char *errmsg)
{
    int rc = 0;
    int exit_flag = 1;
    char buff[BUFSIZ];
    struct sockaddr_in cl_addr;
    int clnt_sd = 0;
    int clnt_len = sizeof(cl_addr);

    while(exit_flag){
        /* クライアントからの接続を待ち受ける */
        clnt_sd = accept(info->sd, (struct sockaddr *)&cl_addr, &clnt_len);
        if(clnt_sd < 0){
            sprintf(errmsg, "(line:%d) %s", __LINE__, strerror(errno));
            return(-1);
        }

        /* エコーサーバ処理を行う */
        rc = tcp_echo_process(clnt_sd, buff, errmsg);
        if(rc != 0) return(-1);

        fprintf(stdout, "[client: %s]%s\n", inet_ntoa(cl_addr.sin_addr),
                buff);
        /* クライアントからのサーバ終了命令を確認する */
        if(strcmp(buff, "terminate") == 0){
            exit_flag = 0;
        }
    }

    return( 0 );
}

/*!
 * @brief      ソケットの初期化
 * @param[in]  info   クライアント接続情報
 * @param[out] errmsg エラーメッセージ格納先
 * @return     成功ならば0、失敗ならば-1を返す。
 */
static int
socket_initialize(sv_info_t *info, char *errmsg)
{
    int rc = 0;

    /* ソケットの生成 : TCPを指定する */
    info->sd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(info->sd < 0){
        sprintf(errmsg, "(line:%d) %s", __LINE__, strerror(errno));
        return(-1);
    }

    /* サーバのアドレス構造体を作成する */
    info->sv_addr.sin_family = AF_INET;
    info->sv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    info->sv_addr.sin_port = htons(info->sv_port);

    /* ローカルアドレスへバインドする */
    rc = bind(info->sd, (struct sockaddr *)&(info->sv_addr),
              sizeof(info->sv_addr));
    if(rc != 0){
        sprintf(errmsg, "(line:%d) %s", __LINE__, strerror(errno));
        return(-1);
    }

    /* ソケットを接続待ち状態にする */
    rc = listen(info->sd, MAXPENDING);
    if(rc != 0){
        sprintf(errmsg, "(line:%d) %s", __LINE__, strerror(errno));
        return(-1);
    }

    return(0);
}

/*!
 * @brief      ソケットの終期化
 * @param[in]  info   クライアント接続情報
 * @return     成功ならば0、失敗ならば-1を返す。
 */
static void
socket_finalize(sv_info_t *info)
{
    /* 接続を閉じる */
    shutdown(info->sd, SHUT_RDWR);  

    /* ソケット破棄 */
    if(info->sd != 0) close(info->sd);

    return;
}

/*!
 * @brief      TCPサーバ実行
 * @param[in]  info   クライアント接続情報
 * @param[out] errmsg エラーメッセージ格納先
 * @return     成功ならば0、失敗ならば-1を返す。
 */
static int
tcp_server(sv_info_t *info, char *errmsg)
{
    int rc = 0;

    /* ソケットの初期化 */
    rc = socket_initialize(info, errmsg);
    if(rc != 0) return(-1);

    /* 文字列を受信する */
    rc = tcp_echo_server(info, errmsg);

    /* ソケットの終期化 */
    socket_finalize(info);

    return(rc);
}

/*!
 * @brief      初期化処理。待受ポート番号を設定する。
 * @param[in]  argc   コマンドライン引数の数
 * @param[in]  argv   コマンドライン引数
 * @param[out] info   サーバ情報
 * @param[out] errmsg エラーメッセージ格納先
 * @return     成功ならば0、失敗ならば-1を返す。
 */
static int
initialize(int argc, char *argv[], sv_info_t *info, char *errmsg)
{
    if(argc != 2){
        sprintf(errmsg, "Usage: %s <port>\n", argv[0]);
        return(-1);
    }

    memset(info, 0, sizeof(sv_info_t));
    info->sv_port = atoi(argv[1]);

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
    sv_info_t info = {0};
    char errmsg[256];

    rc = initialize(argc, argv, &info, errmsg);
    if(rc != 0){
        fprintf(stderr, "Error: %s\n", errmsg);
        return(-1);
    }

    rc = tcp_server(&info, errmsg);
    if(rc != 0){
        fprintf(stderr, "Error: %s\n", errmsg);
        return(-1);
    }

    return(0);
}