# connectors / pinouts

## OBD2
### pinout table
| OBD2 Pin # | OBD2 Standard            | Alfa Romeo Giulia specific |
|------------|--------------------------|----------------------------|
| 1          |                          |                            |
| 2          | SAE J1850 Bus +          |                            |
| 3          |                          | CAN IHS High               |
| 4          | Chassis                  |                            |
| 5          | Signal Ground            |                            |
| 6          | CAN High                 |                            |
| 7          | ISO9141 K Line           |                            |
| 8          |                          |                            |
| 9          |                          |                            |
| 10         | SAE J1850 Bus -          |                            |
| 11         |                          | CAN IHS Low                |
| 12         |                          | ***CAN-CH High***          |
| 13         |                          | ***CAN-CH Low***           |
| 14         | CAN Low                  |                            |
| 15         | ISO9141 L-Line           |                            |
| 16         | Vehicle Battery Positive |                            |
### socket/plug pin numbers
![OBD2 socket](https://www.obd-2.de/carcode/pics/conn_obdf.gif)
![OBD2 plug](https://www.obd-2.de/carcode/pics/conn_obdm.gif)

## DE-9 / DB-9 (for CAN-shield)
### pinout table
| DE-9 / DB-9 Pin # | DFR0370                                                |
|-------------------|--------------------------------------------------------|
| 1                 | GND                                                    |
| 2                 | GND                                                    |
| 3                 | CAN High                                               |
| 4                 |                                                        |
| 5                 | CAN Low                                                |
| 6                 |                                                        |
| 7                 |                                                        |
| 8                 |                                                        |
| 9                 | Vin (mind the power switch on the CAN-Bus shield)      |
### socket/plug pin numbers
![DE-9 / DB-9 socket](https://upload.wikimedia.org/wikipedia/commons/thumb/2/29/RS-232_DE-9_Connector_Pinouts.png/800px-RS-232_DE-9_Connector_Pinouts.png)