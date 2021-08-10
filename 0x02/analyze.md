# BGPパケットのフォーマット
Marker

Length

Type(種類)

の値が先頭に入っていて、その後に種類ごとのメッセージの内容が入る。


# OPENメッセージ
上記の情報に続いて入る内容としては、

type

version

as番号

hold-time

BGP-identifier

Optional parameter

がこの順で入る。


# KEEPALIVEメッセージ

上記に続く内容は何もない。


# UPDATEメッセージ

上記の情報に加えて、ネットワークの到達性の情報とパスアトリビュートが送られる。

パスアトリビュートには、

origin

as_path

next_hop

multi_exit_disc

の情報が含まれる。

到達性の情報はIPアドレスとサブネットマスクの値で構成されており、この経路でどのアドレスに辿り着けるか(または辿り着けなくなってしまったか)を表すアドレスである。

この情報を元にBGPテーブルの中身が変更されてゆく。





# tcp.port == 179 と bgp の違い
tcp.port == 179ではbgpのパケットだけでなく、それに関わるtcpのパケットも含まれる。


# bgp.type == 2 ではなにが filterされるか
updateメッセージ

