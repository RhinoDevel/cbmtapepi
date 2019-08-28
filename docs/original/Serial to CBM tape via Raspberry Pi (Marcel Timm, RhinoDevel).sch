<Qucs Schematic 0.0.19>
<Properties>
  <View=-50,-110,740,720,1,0,0>
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
  <.PortSym 0 700 19 0>
  <.PortSym 0 740 18 0>
  <.PortSym -40 770 20 0>
  <.PortSym -40 810 21 0>
</Symbol>
<Components>
  <Port 6_gnd 1 130 40 4 12 1 2 "7" 0 "analog" 0>
  <Port 8_bcm14_txd 1 130 100 4 12 1 2 "8" 0 "analog" 0>
  <Port 10_bcm15_rxd 1 130 160 4 12 1 2 "9" 0 "analog" 0>
  <Port serial_gnd 1 40 40 -23 12 0 0 "12" 0 "analog" 0>
  <Port serial_rxd 1 40 100 -23 12 0 0 "14" 0 "analog" 0>
  <Port serial_txd 1 40 160 -23 12 0 0 "13" 0 "analog" 0>
  <Port 14_gnd 1 270 100 -23 12 0 0 "11" 0 "analog" 0>
  <Port 7_bcm4 1 270 160 -23 12 0 0 "10" 0 "analog" 0>
  <Port 3_bcm2 1 270 220 -23 12 0 0 "5" 0 "analog" 0>
  <Port 5_bcm3 1 270 280 -23 12 0 0 "6" 0 "analog" 0>
  <Port 1_3v3 1 270 500 -23 12 0 0 "17" 0 "analog" 0>
  <R R1 1 400 130 -62 -26 1 1 "80k" 1 "26.85" 0 "0.0" 0 "0.0" 0 "26.85" 0 "european" 0>
  <R R4 1 450 410 15 -26 0 1 "1k" 1 "26.85" 0 "0.0" 0 "0.0" 0 "26.85" 0 "european" 0>
  <R R3 1 370 410 15 -26 0 1 "10k" 1 "26.85" 0 "0.0" 0 "0.0" 0 "26.85" 0 "european" 0>
  <Port 16_bcm23 1 270 340 -23 12 0 0 "15" 0 "analog" 0>
  <Port 11_bcm17 1 270 560 -23 12 0 0 "19" 0 "analog" 0>
  <R R5 1 400 560 -26 15 0 0 "10k" 1 "26.85" 0 "0.0" 0 "0.0" 0 "26.85" 0 "european" 0>
  <Port P1 5 450 560 4 12 1 2 "20" 0 "analog" 0>
  <Port B 1 530 560 -23 12 0 0 "21" 0 "analog" 0>
  <R R6 1 400 40 -26 15 0 0 "10k" 1 "26.85" 0 "0.0" 0 "0.0" 0 "26.85" 0 "european" 0>
  <Lib D1 1 490 40 -26 13 1 2 "LEDs" 0 "red" 0>
  <Port cbm_1a_gnd 1 550 100 4 12 1 2 "1" 0 "analog" 0>
  <Port cbm_3c_motor 1 550 160 4 12 1 2 "2" 0 "analog" 0>
  <Port cbm_4d_read 1 550 220 4 12 1 2 "3" 0 "analog" 0>
  <Port cbm_6f_sense 1 550 280 4 12 1 2 "4" 0 "analog" 0>
  <Port cbm_5e_write 1 550 340 4 12 1 2 "16" 0 "analog" 0>
  <R R2 1 460 160 -26 -53 1 0 "100k" 1 "26.85" 0 "0.0" 0 "0.0" 0 "26.85" 0 "european" 0>
  <_BJT T 1 450 340 -26 -27 0 1 "npn" 0 "1e-16" 0 "1" 0 "1" 0 "0" 0 "0" 0 "0" 0 "0" 0 "0" 0 "1.5" 0 "0" 0 "2" 0 "100" 0 "1" 0 "0" 0 "0" 0 "0" 0 "0" 0 "0" 0 "0" 0 "0.75" 0 "0.33" 0 "0" 0 "0.75" 0 "0.33" 0 "1.0" 0 "0" 0 "0.75" 0 "0" 0 "0.5" 0 "0.0" 0 "0.0" 0 "0.0" 0 "0.0" 0 "0.0" 0 "26.85" 0 "0.0" 0 "1.0" 0 "1.0" 0 "0.0" 0 "1.0" 0 "1.0" 0 "0.0" 0 "0.0" 0 "3.0" 0 "1.11" 0 "26.85" 0 "1.0" 0>
  <Port 13_bcm27 1 270 40 -23 12 0 0 "18" 0 "analog" 0>
</Components>
<Wires>
  <40 160 130 160 "" 0 0 0 "">
  <40 100 130 100 "" 0 0 0 "">
  <40 40 130 40 "" 0 0 0 "">
  <270 500 410 500 "" 0 0 0 "">
  <270 100 400 100 "" 0 0 0 "">
  <270 160 400 160 "" 0 0 0 "">
  <370 340 370 380 "" 0 0 0 "">
  <450 440 450 460 "" 0 0 0 "">
  <370 440 370 460 "" 0 0 0 "">
  <370 460 410 460 "" 0 0 0 "">
  <410 460 450 460 "" 0 0 0 "">
  <410 460 410 500 "" 0 0 0 "">
  <270 340 370 340 "" 0 0 0 "">
  <270 560 370 560 "" 0 0 0 "">
  <430 560 450 560 "" 0 0 0 "">
  <530 500 530 560 "" 0 0 0 "">
  <410 500 530 500 "" 0 0 0 "">
  <270 40 370 40 "" 0 0 0 "">
  <430 40 460 40 "" 0 0 0 "">
  <270 280 550 280 "" 0 0 0 "">
  <270 220 550 220 "" 0 0 0 "">
  <400 100 520 100 "" 0 0 0 "">
  <520 100 550 100 "" 0 0 0 "">
  <520 40 520 100 "" 0 0 0 "">
  <400 160 430 160 "" 0 0 0 "">
  <490 160 550 160 "" 0 0 0 "">
  <370 340 420 340 "" 0 0 0 "">
  <450 370 450 380 "" 0 0 0 "">
  <480 340 550 340 "" 0 0 0 "">
</Wires>
<Diagrams>
</Diagrams>
<Paintings>
  <Text 120 0 12 #000000 0 "Raspberry Pi GPIO">
  <Text -10 0 12 #000000 0 "Serial interface">
  <Rectangle 120 20 210 660 #000000 2 2 #55ff7f 1 0>
  <Rectangle -10 20 110 200 #000000 2 2 #55ff7f 1 0>
  <Text -10 -30 12 #000000 0 "Use serial interface to send to and receive from Commodore tape port via Raspberry Pi.">
  <Text -10 -70 14 #000000 0 "CBM Tape Pi (Marcel Timm, RhinoDevel, 2019, rhinodevel.com)">
  <Line 460 540 60 0 #000000 3 1>
  <Rectangle 470 530 40 10 #000000 3 1 #c0c0c0 1 0>
  <Rectangle 540 20 160 360 #000000 2 2 #55ff7f 1 0>
  <Text 540 0 12 #000000 0 "Commodore tape port">
</Paintings>
