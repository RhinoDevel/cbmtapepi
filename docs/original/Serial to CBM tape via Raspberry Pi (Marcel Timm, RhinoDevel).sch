<Qucs Schematic 0.0.19>
<Properties>
  <View=-20,-80,760,560,1,0,0>
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
  <.PortSym 0 600 16 0>
  <.PortSym 0 630 17 0>
  <.PortSym 0 670 15 0>
</Symbol>
<Components>
  <Port 6_gnd 1 130 40 4 12 1 2 "7" 0 "analog" 0>
  <Port 8_bcm14_txd 1 130 100 4 12 1 2 "8" 0 "analog" 0>
  <Port 10_bcm15_rxd 1 130 160 4 12 1 2 "9" 0 "analog" 0>
  <Port serial_gnd 1 40 40 -23 12 0 0 "12" 0 "analog" 0>
  <Port serial_rxd 1 40 100 -23 12 0 0 "14" 0 "analog" 0>
  <Port serial_txd 1 40 160 -23 12 0 0 "13" 0 "analog" 0>
  <Port 14_gnd 1 270 40 -23 12 0 0 "11" 0 "analog" 0>
  <Port 7_bcm4 1 270 100 -23 12 0 0 "10" 0 "analog" 0>
  <Port 3_bcm2 1 270 160 -23 12 0 0 "5" 0 "analog" 0>
  <Port 5_bcm3 1 270 220 -23 12 0 0 "6" 0 "analog" 0>
  <Port 1_3v3 1 270 440 -23 12 0 0 "17" 0 "analog" 0>
  <R R2 1 460 100 -26 -53 1 0 "100k" 1 "26.85" 0 "0.0" 0 "0.0" 0 "26.85" 0 "european" 0>
  <R R1 1 400 70 -62 -26 1 1 "80k" 1 "26.85" 0 "0.0" 0 "0.0" 0 "26.85" 0 "european" 0>
  <_BJT T 1 450 280 -26 -27 0 1 "npn" 0 "1e-16" 0 "1" 0 "1" 0 "0" 0 "0" 0 "0" 0 "0" 0 "0" 0 "1.5" 0 "0" 0 "2" 0 "100" 0 "1" 0 "0" 0 "0" 0 "0" 0 "0" 0 "0" 0 "0" 0 "0.75" 0 "0.33" 0 "0" 0 "0.75" 0 "0.33" 0 "1.0" 0 "0" 0 "0.75" 0 "0" 0 "0.5" 0 "0.0" 0 "0.0" 0 "0.0" 0 "0.0" 0 "0.0" 0 "26.85" 0 "0.0" 0 "1.0" 0 "1.0" 0 "0.0" 0 "1.0" 0 "1.0" 0 "0.0" 0 "0.0" 0 "3.0" 0 "1.11" 0 "26.85" 0 "1.0" 0>
  <R R4 1 450 350 15 -26 0 1 "1k" 1 "26.85" 0 "0.0" 0 "0.0" 0 "26.85" 0 "european" 0>
  <R R3 1 370 350 15 -26 0 1 "10k" 1 "26.85" 0 "0.0" 0 "0.0" 0 "26.85" 0 "european" 0>
  <Port cbm_1a_gnd 1 510 40 4 12 1 2 "1" 0 "analog" 0>
  <Port cbm_3c_motor 1 510 100 4 12 1 2 "2" 0 "analog" 0>
  <Port cbm_4d_read 1 510 160 4 12 1 2 "3" 0 "analog" 0>
  <Port cbm_6f_sense 1 510 220 4 12 1 2 "4" 0 "analog" 0>
  <Port cbm_5e_write 1 510 280 4 12 1 2 "16" 0 "analog" 0>
  <Port 16_bcm23 1 270 280 -23 12 0 0 "15" 0 "analog" 0>
</Components>
<Wires>
  <40 160 130 160 "" 0 0 0 "">
  <40 100 130 100 "" 0 0 0 "">
  <40 40 130 40 "" 0 0 0 "">
  <270 440 410 440 "" 0 0 0 "">
  <270 40 400 40 "" 0 0 0 "">
  <270 100 400 100 "" 0 0 0 "">
  <400 100 430 100 "" 0 0 0 "">
  <450 310 450 320 "" 0 0 0 "">
  <370 280 420 280 "" 0 0 0 "">
  <370 280 370 320 "" 0 0 0 "">
  <450 380 450 400 "" 0 0 0 "">
  <370 380 370 400 "" 0 0 0 "">
  <370 400 410 400 "" 0 0 0 "">
  <410 400 450 400 "" 0 0 0 "">
  <410 400 410 440 "" 0 0 0 "">
  <480 280 510 280 "" 0 0 0 "">
  <270 220 510 220 "" 0 0 0 "">
  <270 160 510 160 "" 0 0 0 "">
  <490 100 510 100 "" 0 0 0 "">
  <400 40 510 40 "" 0 0 0 "">
  <270 280 370 280 "" 0 0 0 "">
</Wires>
<Diagrams>
</Diagrams>
<Paintings>
  <Text 120 0 12 #000000 0 "Raspberry Pi GPIO">
  <Text -10 0 12 #000000 0 "Serial interface">
  <Text 500 0 12 #000000 0 "Commodore tape port">
  <Rectangle 120 20 210 460 #000000 2 2 #55ff7f 1 0>
  <Rectangle 500 20 160 300 #000000 2 2 #55ff7f 1 0>
  <Rectangle -10 20 110 200 #000000 2 2 #55ff7f 1 0>
  <Text -10 -30 12 #000000 0 "Use serial interface to send to and receive from Commodore tape port via Raspberry Pi.">
  <Text -10 -70 14 #000000 0 "CBM Tape Pi (Marcel Timm, RhinoDevel, 2019, rhinodevel.com)">
</Paintings>
