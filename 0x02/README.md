# (1)FRR SETUP
spec.yamlは与えられたものをコピーする
>tinet up -c spec.yaml | sudo sh -x
>tinet conf -c spec.yaml | sudo sh -x

を実行
>docker container exec -it R1 vtysh

でvtyに入る

?で使えるコマンド一覧を表示
<tab>でコマンド補完
showコマンドで様々な設定などを見れる
# (2)BGP Peer Setup
R1のvtyに入ったのち、
>configure t

でconfigモードに入る。
>router bgp 1

を実行してconfig-routerモードに入って
>bgp router-id 10.255.1.1
>neighbor 10.255.1.2 remote-as 2

を実行する

R2に対しても同様の操作を実行する。
# (3)BGP Route Advertise
R1についてvtyに入ったのちconfigモードに入り、以下のコマンドを実行する
```
router bgp 1
 bgp router-id 10.255.1.1
 neighbor 10.255.1.2 remote-as 2
 !
 address-family ipv4 unicast
  network 10.1.0.0/24
 exit-address-family
!
```

R2についても同様の操作を実行する。

# (4)Capture BGP Packets

ターミナルをもう一つ立ち上げてssh接続し、R1のvtyに入り、
>docker container exec -it R1 tcpdump -nni net0 -w /tmp/in.pcap

を実行する
その状態で(2),(3)を実行する。
コンテナR1内の/tmpにin.pcapが入っているのでそれを docker container cp コマンドでVM上にコピーし、
それをgitにあげる

# (5)Analyze BGP Packets
analyze.mdを参照

