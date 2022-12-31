# KAMC_Tool
Konami Antiques MSX Collectionのデコードツール  

CD内のファイルはLZ(77)系の圧縮がされているので、それを復号してエミュレータなどで実行できる形式にします。

## 対象
PS: コナミアンティークスMSXコレクション vol.1/2/3(*)  
SS: コナミアンティークスMSXコレクション ウルトラパック  
<br>
* vol.3のみ動作未確認だが、おそらくSS版と同じと予想  

## 使い方
ディスク内の<title.BIN>を任意のディレクトリにコピーして本ツールを実行します。  
ゲーム以外にもMENU.BINなども含まれていますが、対応していません。

```
kcm_tool.exe option [in.bin] [out.bin]
 option:
  e:encode
  d:decode
 example:
  kcm_tool.exe d ANTADV.BIN "Adventure (1984) (Konami) (J).rom"
```

## 注意
コナミアンティークスMSXコレクション内のデータには独自コードが入っている為に、エミュレータなどで動かないゲームも多いです。





