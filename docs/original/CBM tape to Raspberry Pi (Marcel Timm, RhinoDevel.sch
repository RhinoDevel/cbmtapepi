<Qucs Schematic 0.0.19>
<Properties>
  <View=-50,-110,740,720,1,0,0>
  <Grid=10,10,1>
  <DataSet=CBM tape to Raspberry Pi (Marcel Timm, RhinoDevel.dat>
  <DataDisplay=CBM tape to Raspberry Pi (Marcel Timm, RhinoDevel.dpl>
  <OpenDisplay=1>
  <Script=CBM tape to Raspberry Pi (Marcel Timm, RhinoDevel.m>
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
  <.PortSym 40 300 1 0>
  <.PortSym 40 340 2 0>
  <.PortSym 40 380 10 0>
  <.PortSym 40 420 11 0>
  <.PortSym 0 600 16 0>
  <.PortSym 0 630 17 0>
  <.PortSym 0 670 15 0>
  <.PortSym 0 740 18 0>
</Symbol>
<Components>
  <Port 14_gnd 1 70 100 -23 12 0 0 "11" 0 "analog" 0>
  <Port 7_bcm4 1 70 160 -23 12 0 0 "10" 0 "analog" 0>
  <Port 3_bcm2 1 70 220 -23 12 0 0 "5" 0 "analog" 0>
  <Port 5_bcm3 1 70 280 -23 12 0 0 "6" 0 "analog" 0>
  <R R1 1 200 130 -62 -26 1 1 "80k" 1 "26.85" 0 "0.0" 0 "0.0" 0 "26.85" 0 "european" 0>
  <R R4 1 250 410 15 -26 0 1 "1k" 1 "26.85" 0 "0.0" 0 "0.0" 0 "26.85" 0 "european" 0>
  <R R3 1 170 410 15 -26 0 1 "10k" 1 "26.85" 0 "0.0" 0 "0.0" 0 "26.85" 0 "european" 0>
  <Port 16_bcm23 1 70 340 -23 12 0 0 "15" 0 "analog" 0>
  <Lib D1 1 290 40 -26 13 1 2 "LEDs" 0 "red" 0>
  <Port cbm_1a_gnd 1 350 100 4 12 1 2 "1" 0 "analog" 0>
  <Port cbm_3c_motor 1 350 160 4 12 1 2 "2" 0 "analog" 0>
  <Port cbm_4d_read 1 350 220 4 12 1 2 "3" 0 "analog" 0>
  <Port cbm_6f_sense 1 350 280 4 12 1 2 "4" 0 "analog" 0>
  <Port cbm_5e_write 1 350 340 4 12 1 2 "16" 0 "analog" 0>
  <R R2 1 260 160 -26 -53 1 0 "100k" 1 "26.85" 0 "0.0" 0 "0.0" 0 "26.85" 0 "european" 0>
  <_BJT T 1 250 340 -26 -27 0 1 "npn" 0 "1e-16" 0 "1" 0 "1" 0 "0" 0 "0" 0 "0" 0 "0" 0 "0" 0 "1.5" 0 "0" 0 "2" 0 "100" 0 "1" 0 "0" 0 "0" 0 "0" 0 "0" 0 "0" 0 "0" 0 "0.75" 0 "0.33" 0 "0" 0 "0.75" 0 "0.33" 0 "1.0" 0 "0" 0 "0.75" 0 "0" 0 "0.5" 0 "0.0" 0 "0.0" 0 "0.0" 0 "0.0" 0 "0.0" 0 "26.85" 0 "0.0" 0 "1.0" 0 "1.0" 0 "0.0" 0 "1.0" 0 "1.0" 0 "0.0" 0 "0.0" 0 "3.0" 0 "1.11" 0 "26.85" 0 "1.0" 0>
  <Port 13_bcm27 1 70 40 -23 12 0 0 "18" 0 "analog" 0>
  <Port 1_3v3 1 70 460 -23 12 0 0 "17" 0 "analog" 0>
  <R R5 1 200 40 -26 15 0 0 "10k" 1 "26.85" 0 "0.0" 0 "0.0" 0 "26.85" 0 "european" 0>
</Components>
<Wires>
  <70 100 200 100 "" 0 0 0 "">
  <70 160 200 160 "" 0 0 0 "">
  <170 340 170 380 "" 0 0 0 "">
  <250 440 250 460 "" 0 0 0 "">
  <170 440 170 460 "" 0 0 0 "">
  <170 460 250 460 "" 0 0 0 "">
  <70 340 170 340 "" 0 0 0 "">
  <70 40 170 40 "" 0 0 0 "">
  <230 40 260 40 "" 0 0 0 "">
  <70 280 350 280 "" 0 0 0 "">
  <70 220 350 220 "" 0 0 0 "">
  <200 100 320 100 "" 0 0 0 "">
  <320 100 350 100 "" 0 0 0 "">
  <320 40 320 100 "" 0 0 0 "">
  <200 160 230 160 "" 0 0 0 "">
  <290 160 350 160 "" 0 0 0 "">
  <170 340 220 340 "" 0 0 0 "">
  <250 370 250 380 "" 0 0 0 "">
  <280 340 350 340 "" 0 0 0 "">
  <70 460 170 460 "" 0 0 0 "">
</Wires>
<Diagrams>
</Diagrams>
<Paintings>
  <Text -10 -30 12 #000000 0 "Raspberry Pi Commodore datassette emulator simple-as-possible interface.">
  <Text -10 -70 14 #000000 0 "CBM Tape Pi v1.6.0 (Marcel Timm, RhinoDevel, 2019, rhinodevel.com)">
  <Rectangle -10 20 140 480 #000000 2 2 #55ff7f 1 0>
  <Rectangle 340 20 160 360 #000000 2 2 #55ff7f 1 0>
  <Text 340 0 12 #000000 0 "Commodore tape port">
  <Text -10 0 12 #000000 0 "Raspberry Pi GPIO">
</Paintings>
