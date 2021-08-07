OPENメッセージ
Optional parameter
type
version
as番号
BGP-identifier
hold-time
などの情報が送られている。


UPDATEメッセージ
ネットワークの到達性の情報とパスアトリビュートが送られる。
パスアトリビュートには、
origin
as_path
next_hop
multi_exit_disc
の情報が含まれる。



tcp.port == 179 と bgp の違い
tcp.port == 179ではbgpのパケットだけでなく、それに関わるtcpのパケットも含まれる。


bgp.type == 2 ではなにが filterされるか
updateメッセージ

