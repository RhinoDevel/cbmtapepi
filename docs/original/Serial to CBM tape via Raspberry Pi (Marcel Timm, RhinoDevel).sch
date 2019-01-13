<Qucs Schematic 0.0.19>
<Properties>
  <View=0,-60,1392,883,0.826446,0,0>
  <Grid=10,10,1>
  <DataSet=Serial to CBM tape via Raspberry Pi (Marcel Timm, RhinoDevel).dat>
  <DataDisplay=Serial to CBM tape via Raspberry Pi (Marcel Timm, RhinoDevel).dpl>
  <OpenDisplay=1>
  <Script=Serial to CBM tape via Raspberry Pi (Marcel Timm, RhinoDevel).m>
  <RunScript=0>
  <showFrame=0>
  <FrameText0=Serial to CBM tape via Raspberry Pi>
  <FrameText1=Drawn By:Marcel Timm, RhinoDevel>
  <FrameText2=Date:13. January 2019>
  <FrameText3=Revision:1>
</Properties>
<Symbol>
  <.PortSym 40 20 3 0>
  <.PortSym 40 60 4 0>
  <.PortSym 40 100 6 0>
  <.PortSym 40 140 5 0>
  <.PortSym 40 180 7 0>
  <.PortSym 40 220 8 0>
  <.PortSym 40 260 9 0>
  <.PortSym 40 300 1 0>
  <.PortSym 40 340 2 0>
  <.PortSym 40 380 10 0>
  <.PortSym 40 420 11 0>
  <.PortSym -30 450 12 0>
  <.PortSym -30 490 14 0>
  <.PortSym -30 530 13 0>
</Symbol>
<Components>
  <Port 14_gnd 1 290 320 -23 12 0 0 "11" 0 "analog" 0>
  <Port 7_bcm4 1 290 380 -23 12 0 0 "10" 0 "analog" 0>
  <Port cbm_1a_gnd 1 530 320 4 12 1 2 "1" 0 "analog" 0>
  <Port cbm_3c_motor 1 530 380 4 12 1 2 "2" 0 "analog" 0>
  <Port 3_bcm2 1 290 440 -23 12 0 0 "5" 0 "analog" 0>
  <Port cbm_4d_read 1 530 440 4 12 1 2 "3" 0 "analog" 0>
  <Port 5_bcm3 1 290 500 -23 12 0 0 "6" 0 "analog" 0>
  <Port cbm_6f_sense 1 530 500 4 12 1 2 "4" 0 "analog" 0>
  <Port 6_gnd 1 210 100 4 12 1 2 "7" 0 "analog" 0>
  <Port 8_bcm14_txd 1 210 160 4 12 1 2 "8" 0 "analog" 0>
  <Port 10_bcm15_rxd 1 210 220 4 12 1 2 "9" 0 "analog" 0>
  <Port sender_gnd 1 70 100 -23 12 0 0 "12" 0 "analog" 0>
  <Port sender_rxd 1 70 160 -23 12 0 0 "14" 0 "analog" 0>
  <Port sender_txd 1 70 220 -23 12 0 0 "13" 0 "analog" 0>
  <R R2 1 470 380 -26 -53 1 0 "100k" 1 "26.85" 0 "0.0" 0 "0.0" 0 "26.85" 0 "european" 0>
  <R R1 1 410 350 -62 -26 1 1 "80k" 1 "26.85" 0 "0.0" 0 "0.0" 0 "26.85" 0 "european" 0>
</Components>
<Wires>
  <290 380 410 380 "" 0 0 0 "">
  <290 320 410 320 "" 0 0 0 "">
  <290 440 530 440 "" 0 0 0 "">
  <290 500 530 500 "" 0 0 0 "">
  <500 380 530 380 "" 0 0 0 "">
  <70 220 210 220 "" 0 0 0 "">
  <70 160 210 160 "" 0 0 0 "">
  <70 100 210 100 "" 0 0 0 "">
  <410 380 440 380 "" 0 0 0 "">
  <410 320 530 320 "" 0 0 0 "">
</Wires>
<Diagrams>
</Diagrams>
<Paintings>
  <Text 20 -40 14 #000000 0 "CBM Tape Pi (Marcel Timm, RhinoDevel, 2019, rhinodevel.com)">
  <Rectangle 180 60 160 500 #000000 2 2 #55ff7f 1 0>
  <Rectangle 520 280 160 280 #000000 2 2 #55ff7f 1 0>
  <Rectangle 20 60 140 220 #000000 2 2 #55ff7f 1 0>
  <Text 20 40 12 #000000 0 "Sender">
  <Text 180 40 12 #000000 0 "Raspberry Pi GPIO">
  <Text 520 260 12 #000000 0 "Commodore tape port">
  <Text 20 0 12 #000000 0 "Send from serial interface to Commodore tape port via Raspberry Pi.">
</Paintings>
